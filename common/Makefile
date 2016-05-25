# Makefile  for 'common' module
#
# Divya Kalidindi and Drew Waterman, April 2016

LIB = common.a
OBJS = web.o word.o file.o

CFLAGS = -Wall -pedantic -std=c11 -ggdb
CC = gcc
MAKE = make

# build the common.a
$(LIB): $(OBJS)
	ar cr $@ $^

# common.a source dependencies
web.o: web.c web.h
word.o: word.c word.h
file.o: file.c file.h

# clean up
clean:
	rm -f *~
	rm -f *.o
	rm -f $(LIB)
