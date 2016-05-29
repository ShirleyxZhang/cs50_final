/*
 * list.h - header file for list.c
 *
 * based on tree.h from Professor Kotz's tree6 module
 *
 * Drew Waterman, April 2016
 */

#include <stdbool.h>

#ifndef __LIST_H
#define __LIST_H

/**************** global types ****************/
typedef struct list {
  struct listnode *head; // head of linked list
  void (*itemdelete)(void *data);
}list_t;


void list_delete(list_t *list);

/**************** functions ****************/

/* Create a new empty list
 * return NULL if there are any errors
 */
list_t *list_new(void (*itemdelete)(void *data) );

/* Look for the key given as a parameter in the list
 * If found, return the data associated with that key
 * If not found, or if the list is NULL, return NULL
 */
void *list_find(list_t *list, char *key);

/* Insert an item with the given key and data into the given list
 * If an item with the same key already exists in the list, or if
 * the item could not be inserted for some other reason, return false
 * If the item was inserted successfully, return true
 */
bool list_insert(list_t *list, char *key, void *data);


/* Iterate over all items in list (in undefined order):
 * call itemfunc for each item, passing (arg, key, data).
 */
void list_iterate(list_t *list,
		  void (*itemfunc)(void *arg, char *key, void *data, void* optional),
		  void *arg, void* optional);


#endif // __LIST_H


