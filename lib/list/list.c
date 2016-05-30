/*
 * list.c - program for creating linked list data structures
 *
 * program based on tree.c from Professor Kotz's tree6 module
 *
 * Drew Waterman, April 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"


/**************** local types ****************/

/* structure that holds the key and data we want to store
 * node in the linked list
 */
typedef struct listnode {
  char *key;              // key for looking up items in the list
  void *data;             // data associated with the key
  struct listnode *next;  // pointer to next item in the list
} listnode_t;


/**************** global types ****************/
//typedef struct list list_t;


/**************** global functions ****************/
/* that is, visible outside this file */
/* see list.h for comments about exported functions */

/**************** local functions ****************/
static listnode_t *listnode_new(char *key, void *data);

/**************** list_new() ****************/
list_t *
list_new(void (*itemdelete)(void *data) )
{
  // allocate memory for the list
  list_t *list = malloc(sizeof(list_t));

  if (list == NULL)
    return NULL; //error allocating list

  else {
    // initialize list's head to NULL     
    list->head = NULL;
    list->itemdelete = itemdelete;
    return list;
  }
}

/**************** listnode_new ****************/
static listnode_t *
listnode_new(char *key, void *data)
{
  // allocate memory for the listnode
  listnode_t *newnode = calloc(1, sizeof(listnode_t));

  if (newnode == NULL) {
    return NULL;  // error allocating listnode
  }
  else {
    // allocate memory corresponding to the lenght of the key
    newnode->key = calloc(1, (strlen(key) + 1));

    // if newnode's key is NULL, there was a problem making the node
    // so we should free its memory and return NULL
    if (newnode->key == NULL) {
      free(newnode);
      return NULL;
    }
    else { 
      // if there were no problems, copy the string into newnode's key value
      strcpy(newnode->key, key);
      newnode->data = data;

    // set the node's next pointer to NULL and return the node
    newnode->next = NULL;
    return newnode;
    }
  }
}

/**************** list_find() ****************/
void *list_find(list_t *list, char *key)
{ 
  // check to make sure list is not NULL
  if (list == NULL) {
    return NULL;  
  }

  // if head is NULL, the list is empty, so we return NULL
  if (list->head == NULL) {
    return NULL;
  }

  /* loop through the linked list until we either reach the end or 
   * find the key we are looking for */
  listnode_t *current = list->head;

  while (current->next != NULL && strcmp(current->key, key) != 0) {
    current = current->next;
  }

  // If we found the key, return its data
  if (strcmp(current->key, key) == 0) {
    return current->data;
  }
  // otherwise, we reached the end of the list without finding the key,
  // so return NULL
  else {
    return NULL;
  }
}


/**************** list_insert() ****************/
bool list_insert(list_t *list, char *key, void *data)
{
  // check to make sure list is valid
  if (list == NULL) {
    return false;  // return false because we don't insert anything
  }

  /* if list_find returns anything but NULL, that means that
   * an item with the same key is already in the list
   * so we do not insert the new item, and we return false */
  if (list_find(list, key) != NULL) {
    return false;
  }
  
  else {
    // create a new node using the given parameters
    listnode_t *newnode = listnode_new(key, data);

    listnode_t *current = list->head;

    // if the head is NULL, the list is empty,
    // so insert the new node as the head of the list
    if (current == NULL) {
      list->head = newnode;
      return true;  // return true because we inserted a new item
    }

    // loop through the list until it gets to the end
    while (current->next != NULL) {
      current = current->next;
    }

    // insert the new node at the end of the list
    current->next = newnode;
    return true;  // return true because we inserted a new item
  }
}

/**************** list_delete() ****************/
void
list_delete(list_t *list)
{
  if (list != NULL) {
    if (list->head != NULL) {
      listnode_t *current = list->head;
      while (current != NULL) {
	// go to the next node in the list
	listnode_t *next = current->next;
	if (list->itemdelete != NULL) // delete the node
	  (*list->itemdelete)(current->data);
	// free all memory and allocate next node
	free(current->key);
	free(current);
	current = next;
      }
    }
    free(list);
  }

}

/**************** list_iterate() ****************/
/* Iterate over all items in list (in undefined order):
 * call itemfunc for each item, passing (arg, key, data).
 */
void list_iterate(list_t *list,
		  void (*itemfunc)(void *arg, char *key, void *data, void* optional),
		  void *arg, void* optional)
{
  if (list == NULL || itemfunc == NULL)
    return;
  else {
    // loop through all the items in the list
    for (listnode_t *node = list->head; node != NULL; node = node->next) {
      (*itemfunc)(arg, node->key, node->data, optional);
    }
  }
  return;
}
