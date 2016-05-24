# Makefile for Guide Agent
#
# Drew Waterman (Team Lapis), May 2016


CFLAGS = -Wall -pedantic -std=c11 -ggdb
GTKFLAGS = `pkg-config --cflags --libs gtk+-3.0` 
MAKE = make

all: guide


guide: guide.o
	gcc  $(CFLAGS) -o guide guide.c ../common/file.c ../common/file.h 


clean:
	rm -f *~
	rm -f *.o
