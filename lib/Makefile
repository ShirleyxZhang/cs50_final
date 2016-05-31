# Builds and compiles all the data structures into a library.
#
# Shirley Zhang, May 2016
OBJS = hashtable/hashtable.o bag/bag.o hashtable/jhash.o list/list.o counters/counters.o
LIB = cs50ds.a

CFLAGS = -Wall -pedantic -std=c11 -ggdb
CC = gcc
MAKE = make

$(LIB): $(OBJS)
	ar cr $@ $^


# add other targets for the other .o files

list/list.o:list/list.h list/list.c
	cd list; $(MAKE) list.o

hashtable/jhash.o: hashtable/jhash.c hashtable/jhash.h
	cd hashtable/; $(MAKE) jhash.o

hashtable/hashtable.o: hashtable/hashtable.c hashtable/hashtable.h 
	cd hashtable/; $(MAKE) hashtable.o

bag/bag.o: bag/bag.c bag/bag.h
	cd bag; $(MAKE) bag.o

counters/counters.o: counters/counters.c counters/counters.h
	cd counters; $(MAKE) counters.o

# add cleanup commands for other subdirectories
clean:
	rm -f *~
	rm -f *.o
	rm -f $(LIB)
	cd hashtable; $(MAKE) clean
	cd bag; $(MAKE) clean
	cd list; $(MAKE) clean