CC=g++
CFLAGS=-O3 -ggdb3 -std=gnu++11 -Werror -Wall -pedantic
EXTRAFLAGS=-lpqxx -lpq
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
all: test

test: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(EXTRAFLAGS)

%.o:%.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~ *.o test

clobber:
	rm -f *~ *.o
