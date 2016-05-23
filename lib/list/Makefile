# Makefile for list module
#
# Drew Waterman, April 2016

CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb
PROG = listtest
OBJS = listtest.o list.o
LIBS =

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

listtest.o: list.h
list.o: list.h

clean:
	rm -f *~
	rm -f *.o
	rm -f $(PROG)
