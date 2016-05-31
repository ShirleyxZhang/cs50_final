# Makefile for 'common' library
# Builds and compiles all the data structures into a library.
#
# Shirley Zhang, Nicole Chen, April 2016
L = ../lib
C = ../common
OBJS = word.o file.o $(L)/counters/counters.o
LIB = common.a
CFLAGS = -Wall -pedantic -std=c11 -ggdb
CC = gcc
MAKE = make

$(LIB): $(OBJS)
	ar cr $@ $^

# add other targets for the other .o files

word.o: word.c word.h 

file.o: file.c file.h

# add cleanup commands for other subdirectories
clean:
	rm -f *~
	rm -f *.o
	rm -f $(LIB)
