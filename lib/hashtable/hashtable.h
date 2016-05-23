/*
 * hashtable.h - header file for hashtable.c
 *
 * The outline for this header file is based on tree.h for
 * the tree6 module written by Professor David Kotz
 *
 * Drew Waterman, April 2016
 */

#include <stdbool.h>
#include "../list/list.h"


#ifndef __HASHTABLE_H
#define __HASHTABLE_H

/**************** global types ****************/
typedef struct hashtable hashtable_t;
typedef struct list list_t;


/**************** functions ****************/

/* Deletes the hashtable data structure */
void hashtable_delete(hashtable_t *ht);

/* Creates a new empty hashtable with the number of slots specified
 */
hashtable_t *
hashtable_new(const int num_slots, void (*itemdelete)(void *data) );


/* Searches for a given key in a given hashtable
 * Returns corresponding data for the given key
 * Returns NULL if the key is not find
 */
void *hashtable_find(hashtable_t *ht, char *key);


/* Inserts a given key and data into the given hashtable
 * Returns false if the key is already in the table or an error occurs
 * Returns true if the key was inserted successfully
 */
bool hashtable_insert(hashtable_t *ht, char *key, void *data);

/* Iterates through the hashtable data structure */
void
hashtable_iterate(hashtable_t *table,
		  void (*itemfunc)(void *arg, char *key, void *data),
		  void *arg);


#endif // __HASHTABLE_H  
