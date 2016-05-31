# Makefile for Guide Agent
#
# Team Lapis (Drew Waterman, Deven Orie, Shirley Zhang), May 2016  

CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb
PROG = guide
OBJS = guide.o ../common/file.o ../lib/list/list.o ../lib/bag/bag.o
MAKE = make



guide: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS)



guide.o: ../common/file.h ../lib/bag/bag.h ../lib/list/list.h

../common/file.o: ../common/file.h ../common/file.c
	cd ../common/; $(MAKE) ../common/file.o

../lib/list/list.o: ../lib/list/list.h ../lib/list/list.c
	cd ../lib/list/; $(MAKE) ../list/list.o

../lib/bag/bag.o: ../lib/bag/bag.h ../lib/bag/bag.c
	cd ../lib/bag/; $(MAKE) ../bag/bag.o



clean:
	rm -f *~
	rm -f *.o
