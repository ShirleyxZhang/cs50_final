# Makefile for Guide Agent
#
# Shirley Zhang (Team Lapis), May 2016

# L = ../lib
# C = ../common
# PROG = game_server
# OBJS = game_server.o
# CFLAGS = -Wall -pedantic -std=c11 -ggdb
# CC = gcc
# MAKE = make

# all: 
# 	$(MAKE) $(PROG)
# 	@echo "TODO: make game_servergame_server..."

# # build the querier
# $(PROG): c $(LLIBS)
# 	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

# #source dependencies; add others as needed
# game_server.o: $(C)/file.h $(C)/word.h $(L)/hashtable/hashtable.h $(L)/counters/counters.h $(L)/list/list.h


# clean:
# 	@echo "TODO: make clean game_server..."
# 	rm -f *~
# 	rm -f *.o
# 	rm -f $(PROG)


CFLAGS = -Wall -pedantic -std=c11 -ggdb
PROG = game_server
OBJS = game_server.o
###GTKFLAGS = `pkg-config --cflags --libs gtk+-3.0` 
MAKE = make

# $(PROG): $(OBJS)
# 	gcc $(CFLAGS) $^ $(GTKFLAGS) -o $@

all: game_server


game_server: game_server.o
	gcc $(CFLAGS) -o game_server game_server.c ../common/file.c ../common/file.h ../lib/list/list.c ../lib/list/list.h ../lib/hashtable/hashtable.c ../lib/hashtable/hashtable.h ../lib/hashtable/jhash.c ../lib/hashtable/jhash.h
#guide.o: ../common/file.c ../common/file.h 



clean:
	rm -f *~
	rm -f *.o
