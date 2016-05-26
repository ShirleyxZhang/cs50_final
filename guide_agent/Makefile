# Makefile for Guide Agent
#
# Drew Waterman (Team Lapis), May 2016


CFLAGS = -Wall -pedantic -std=c11 -ggdb
PROG = guide
OBJS = guide.o
###GTKFLAGS = `pkg-config --cflags --libs gtk+-3.0` 
MAKE = make

# $(PROG): $(OBJS)
# 	gcc $(CFLAGS) $^ $(GTKFLAGS) -o $@

all: guide


guide: guide.o
	gcc $(CFLAGS) -o guide guide.c ../common/file.c ../common/file.h ../lib/list/list.h ../lib/list/list.c ../lib/bag/bag.h ../lib/bag/bag.c

#guide.o: ../common/file.c ../common/file.h 



clean:
	rm -f *~
	rm -f *.o
