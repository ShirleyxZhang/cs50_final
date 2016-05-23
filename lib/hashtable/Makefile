# Makefile for hashtable module
#
# Drew Waterman, April 2016

CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb
PROG = hashtabletest
OBJS = hashtest.o hashtable.o jhash.o ../list/list.o
LIBS =

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

hashtest.o: hashtable.h jhash.h ../list/list.h
hashtable.o: hashtable.h ../list/list.h jhash.h
../list/list.o: ../list/list.h
jhash.o: jhash.h

clean:
	rm -f *~
	rm -f *.o
	rm -f $(PROG)
