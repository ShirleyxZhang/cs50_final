# Makefile for Game Server
# Shirley Zhang (Team Lapis), May 2016

L = ../lib
C = ../common
PROG = game_server
OBJS = game_server.o
LLIBS = $(L)/cs50ds.a $(C)/common.a
CFLAGS = -Wall -pedantic -std=c11 -ggdb -lm
CC = gcc
MAKE = make

# build the crawler
$(PROG): $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

# source dependencies; add others as needed
game_server.o: $(C)/file.h $(C)/word.h $(L)/bag/bag.h $(L)/hashtable/hashtable.h $(L)/counters/counters.h

clean:
	rm -f *~
	rm -f *.o
	rm -f $(PROG)




