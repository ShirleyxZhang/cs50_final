# Makefile for Guide Server

# Shirley Zhang (Team Lapis), May 2016

# PROG = game_server
# OBJS = game_server.o
# CFLAGS = -Wall -pedantic -std=c11 -ggdb
# CC = gcc
# MAKE = make


	

# # build the game_server
# $(PROG): $(OBJS)
# 	@echo "TODO: make game_servergame_server..."
# 	$(CC) $(CFLAGS) $^ -o $@

# #source dependencies; add others as needed
# game_server.o: $../common/file.h $../common/word.h $../lib/hashtable/hashtable.h $../lib/counters/counters.h $../lib/list/list.h


# clean:
# 	@echo "TODO: make clean game_server..."
# 	rm -f *~
# 	rm -f *.o
# 	rm -f $(PROG)


# CFLAGS = -Wall -pedantic -std=c11 -ggdb -lm
# PROG = game_server
# OBJS = game_server.o
# MAKE = make

# # $(PROG): $(OBJS)
# # 	gcc $(CFLAGS) $^ $(GTKFLAGS) -o $@

# all: game_server


# game_server: game_server.o
# 	 gcc $(CFLAGS) -o game_server game_server.c ../common/file.c ../common/file.h ../lib/list/list.c ../lib/list/list.h ../lib/hashtable/hashtable.c ../lib/hashtable/hashtable.h ../lib/hashtable/jhash.c ../lib/hashtable/jhash.h
# #guide.o: ../common/file.c ../common/file.h 



# clean:
# 	rm -f *~
# 	rm -f *.o
# 	rm -f $(PROG)


L = ../lib
C = ../common
PROG = game_server
OBJS = game_server.o
#LIBS = -lcurl
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




