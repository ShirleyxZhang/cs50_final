/*
 * counters.c - counters module that keeps track of how many times
 * a key has been inserted into the structure, a simple linked list
 *
 * program based on tree.c from Professor Kotz's tree6 module
 * 
 * Drew Waterman, April 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include "counters.h"


/**************** local types ****************/ 
typedef struct countersnode {
  int key;
  // counter keeps track of how many times a given keyt has been inserted
  int counter;
  struct countersnode *next;  // pointer to next item in linked list
} countersnode_t;


/**************** global types ****************/
typedef struct counters {
  struct countersnode *head;
} counters_t;


/**************** global functions ****************/
/* that is, visible outside this file */
/* see counters.h for comments about exported functions */


/**************** local functions ****************/
/* not visible outside this file */
static countersnode_t *countersnode_new(int key);


/**************** counters_new() ****************/
counters_t *
counters_new(void)
{
  // allocate memory for the new counter structure
  counters_t *ctrs = malloc(sizeof(counters_t));

  // return NULL if there was a problem allocating memory
  if (ctrs == NULL) {
    return NULL;
  }
  // if there were no problems:
  else {
    ctrs->head = NULL; // set the head of the list to NULL 
    return ctrs;       // return the newly created structure
  }
}

/**************** countersnode_new() ****************/
static countersnode_t *
countersnode_new(int key)
{
  // allocate memeory for new node
  countersnode_t *node = malloc(sizeof(countersnode_t));

  // if there was a problem, return NULL
  if (node == NULL)
    return NULL;
  else {
    node->key = key;    // assign key
    node->counter = 1;  // default counter is 1
    node->next = NULL;  // assign node's next pointer to NULL
  }
  return node;  // return the node we just created
}


/**************** counters_add() ****************/
void
counters_add(counters_t *ctrs, int key)
{
  // check to make sure the structure passed to us is not NULL
  if (ctrs != NULL) {
    
    // if the head is NULL, the list is empty
    // create a new node and make it the head of the list
    if (ctrs->head == NULL) {
      countersnode_t *head = countersnode_new(key);
      ctrs->head = head;
    }

    else {
      // loop through list until we either find an item that
      // already has the key we are adding, or until we reach
      // the end of the linked list
      countersnode_t *current = ctrs->head;
      while(current->next != NULL && current->key != key) {
	current = current->next;
      }

      // if we found the key, increment its counter
      if (current->key == key) 
	current->counter++;

      // if we didn't find the key, we reached the end of the list
      else {
	// create a new node using the given key
	countersnode_t *newnode = countersnode_new(key);
	// add the new node to the end of the list
	current->next = newnode; 
      }
    }
  }
}

/**************** counters_get() ****************/
int counters_get(counters_t *ctrs, int key) {

  // if pointer passed through this function is NULL, return 0
  if (ctrs == NULL) {
    return 0;
  }

  // If the head is NULL, the list is empty. Return 0.
  if (ctrs->head == NULL)
    return 0;

  countersnode_t *current = ctrs->head;

  // loop through list until we either find the key pased in
  // as a parameter or reach the end of the list
  while (current->next != NULL && current->key != key)
    current = current->next;

  // If we found the key, return the value of its counter
  if (current->key == key)
    return current->counter;
  else
    return 0;  // key was not found
}


/**************** counters_delete() ****************/
void counters_delete(counters_t *ctrs)
{
  // only delete the structure if ctrs is not NULL
  if (ctrs != NULL) {

    // If the list is not empty, go through and delete the nodes
    if (ctrs->head != NULL) {
     
      countersnode_t *current;
      countersnode_t *current_prev;  // node before current
    
      // loop through list, deleting element at the end of
      // the list after each loop
      while (ctrs->head->next != NULL) {

	current = ctrs->head;

	// loop through the list until we get to the end
	while (current->next != NULL) {
	  current_prev = current;
	  current = current->next;
	}

	// set the next pointer of the node before current to NULL
	current_prev->next = NULL;
	// free the current node at the end of the list
	free(current);
	current = NULL;
      }

      // free the memory allocated to the head node
      free(ctrs->head);
      ctrs->head = NULL;
    }
    // free the memory allocated to the counter structure itself   
    free(ctrs);
    ctrs = NULL;

  }
}

/**************** counters_set() ****************/
void counters_set(counters_t *ctrs, int key, int count)
{ 
  // check to make sure the structure passed to us is not NULL
  if (ctrs != NULL) {

    // if the head is NULL, the list is empty
    // create a new node and make it the head of the list
    if (ctrs->head == NULL) {
      countersnode_t *head = countersnode_new(key);
      head->counter = count;
      ctrs->head = head;
    }

    else {
      // loop through list until we either find an item that
      // already has the key we are adding, or until we reach
      // the end of the linked list
      countersnode_t *current = ctrs->head;
      while(current->next != NULL && current->key != key) {
	current = current->next;
      }
      // if we found the key, set its counter
      if (current->key == key)
	current->counter = count;

      // if we didn't find the key, we reached the end of the list
      else {
	// create a new node using the given key
	countersnode_t *newnode = countersnode_new(key);
	newnode->counter = count;
	// add the new node to the end of the list
	current->next = newnode;
      }
    }
  }
}

/**************** counters_iterate() ****************/
void counters_iterate(counters_t *ctrs,
		 void (*itemfunc)(void *arg, int key, int count),
		      void *arg)
  
{
  if (ctrs == NULL || itemfunc == NULL)
    return;
  else {
    // loop through all the counters in the list and pass in the key and counter
    for (countersnode_t *node = ctrs->head; node != NULL; node = node->next) {
      (*itemfunc)(arg, node->key, node->counter);
    }
  }
  return;
}
