#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <unordered_set>

#define NUM_ARGS 2
#define WORDLEN_MAX 128
#define perror_and_exit() do { perror(""); exit(EXIT_FAILURE); } while(0)

void split_filebuf(char *buf, size_t buf_size, char **chunks, size_t num_chunks);
void parse_words(char *begin, char *end, std::unordered_set<std::string> *words);

int main(int argc, char **argv)
{
    if(argc != NUM_ARGS)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int fd = open(argv[1], O_RDONLY);
    if(fd == -1)
        perror_and_exit();
    struct stat statbuf;
    if(fstat(fd, &statbuf) == -1)
        perror_and_exit();
    char *filebuf_begin = (char *) mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(filebuf_begin == MAP_FAILED)
        perror_and_exit();
    close(fd);

    size_t num_threads = std::thread::hardware_concurrency();
    char *chunks[num_threads];
    split_filebuf(filebuf_begin, statbuf.st_size, chunks, num_threads);
    std::thread threads[num_threads];
    std::unordered_set<std::string> storage[num_threads];
    for(size_t i = 0; i < num_threads; ++i)
        threads[i] = std::thread(parse_words, chunks[i],
                                 i < num_threads-1 ? chunks[i+1] : filebuf_begin+statbuf.st_size,
                                 &storage[i]);
    for(auto &thread : threads)
        thread.join();
    for(size_t i = 1; i < num_threads; ++i)
        storage[0].merge(storage[i]);
    printf("%lu\n", storage[0].size());
    munmap(filebuf_begin, statbuf.st_size);

    return 0;
}

void
split_filebuf(char *buf, size_t buf_size, char **chunks, size_t num_chunks)
{
    for(size_t i = 0; i < num_chunks; ++i)
    {
        chunks[i] = buf + (buf_size / num_chunks) * i;
        if(i > 0)
            while(!isspace(*chunks[i]++))
                ;
    }
}

void
parse_words(char *begin, char *end, std::unordered_set<std::string> *words)
{
    enum { OUT = 0, IN } state = OUT;
    char wordbuf[WORDLEN_MAX];
    char *wordbuf_ptr = wordbuf;
    while(begin != end)
    {
        int space = isspace(*begin);
        if(state == OUT)
        {
            if(!space)
            {
                state = IN;
                *wordbuf_ptr++ = *begin;
            }
        }
        else
        {
            if(space)
            {
                *wordbuf_ptr++ = '\0';
                words->insert(wordbuf);
                wordbuf_ptr = wordbuf;
                state = OUT;
            }
            else
            {
                *wordbuf_ptr++ = *begin;
            }
        }
        ++begin;
    }
}
