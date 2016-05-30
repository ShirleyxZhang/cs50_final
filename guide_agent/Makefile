# Makefile for Guide Agent
#
# Drew Waterman (Team Lapis), May 2016


CFLAGS = -Wall -pedantic -std=c99 -ggdb
PROG = guide
OBJS = ../common/file.o ../lib/list/list.o ../lib/bag/bag.o ../lib/counters/counters.o guide.o
GTKFLAGS =`pkg-config --cflags --libs gtk+-3.0`
MAKE = make

# $(PROG): $(OBJS)
# 	gcc $(CFLAGS) $^ $(GTKFLAGS) -o $@


guide: guide.o
# 	gcc $(CFLAGS)  $(GTKFLAGS) guide.o ../common/file.o ../lib/list/list.o ../lib/bag/bag.o ../lib/counters/counters.o -o guide
	 gcc $(CFLAGS) -o guide guide.c ../common/file.c ../common/file.h ../lib/list/list.h ../lib/list/list.c ../lib/bag/bag.h ../lib/bag/bag.c ../lib/counters/counters.h


../common/file.o: ../common/file.c ../common/file.h

../lib/list/list.o: ../lib/list/list.h

../lib/bag/bag.o: ../lib/bag/bag.h ../lib/bag/bag.c

../lib/counters/counters.o: ../lib/counters/counters.c ../lib/counters/counters.h


clean:
	rm -f *~
	rm -f *.o
