CC=g++
LDFLAGS=-pthread
CXXFLAGS=-Wall -Wextra -std=c++2a
CXXFLAGS_DEBUG=$(CXXFLAGS) -O0 -ggdb
CXXFLAGS_RELEASE=$(CXXFLAGS) -O3

all: efp efp.debug

efp.debug: main.cpp
	$(CC) $(CXXFLAGS_DEBUG) $< -o $@ $(LDFLAGS)

efp: main.cpp
	$(CC) $(CXXFLAGS_RELEASE) $< -o $@ $(LDFLAGS)

clean:
	rm -rf efp*
