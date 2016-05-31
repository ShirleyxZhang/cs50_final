# Makefile for Guide Agent
#
# Team Lapis (Drew Waterman, Deven Orie, Shirley Zhang), May 2016  

L = ../lib
C = ../common
PROG = guide
OBJS = guide.o
LLIBS = $(L)/cs50ds.a $(C)/common.a
CFLAGS = -Wall -pedantic -std=c11 -ggdb -lncurses
CC = gcc
MAKE = make

# build the guide_agent
$(PROG): $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

# source dependencies; add others as needed
guide.o: $(C)/file.h $(C)/word.h $(L)/bag/bag.h $(L)/hashtable/hashtable.h $(L)/counters/counters.h

clean:
	rm -f *~
	rm -f *.o
	rm -f $(PROG)