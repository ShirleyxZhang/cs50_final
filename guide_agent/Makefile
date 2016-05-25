# Makefile for Guide Agent
#
# Drew Waterman (Team Lapis), May 2016


CFLAGS = -Wall -pedantic -std=c11 -ggdb
PROG = guide
OBJS = guide.o
GTKFLAGS = `pkg-config --cflags --libs gtk+-3.0` 
MAKE = make

$(PROG): $(OBJS)
	gcc $(CFLAGS) $^ $(GTKFLAGS) -o $@

# all: guide


# guide: guide.o
# 	mygcc $(GTKFLAGS) -o guide guide.c ../common/file.c ../common/file.h

guide.o: ../common/file.c ../common/file.h 



clean:
	rm -f *~
	rm -f *.o
