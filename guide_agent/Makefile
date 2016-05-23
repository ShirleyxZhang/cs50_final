# Makefile for Guide Agent
#
# Drew Waterman (Team Lapis), May 2016


CFLAGS = -Wall -pedantic -std=c11 -ggdb
MAKE = make

all: guide


guide: guide.o
	gcc $(CFLAGS) -o guide guide.c


clean:
	rm -f *~
	rm -f *.o
