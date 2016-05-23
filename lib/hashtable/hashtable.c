/*
 * hashtable.c - a hashtable module
 * implemented using an array and linked lists
 *
 * The outline/structure of this program was modeled off of
 * tree.c for the tree6 module, written by Professor David Kotz
 *
 * Drew Waterman, April 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "jhash.h"
#include "../list/list.h"

// list module that will be used to store items that hash
// to the same slot in a linked list
typedef struct list list_t;


/**************** global types ****************/
typedef struct hashtable {
  list_t** array;  // array represents slots in hash table
  int num_slots;
  void (*itemdelete)(void *data);
} hashtable_t;

/**************** global functions ****************/
/* that is, visible outside this file */
/* see hashtable.h for comments about exported functions */


/**************** hashtable_new() ****************/
hashtable_t *
hashtable_new(const int num_slots, void (*itemdelete)(void *data) )
{
  // allocate memory for the new hashtable
  hashtable_t *table = malloc(sizeof(hashtable_t));

  list_t **myArray = calloc(num_slots, sizeof(list_t));

  for (int i = 0; i < num_slots; i++) {
    myArray[i] = NULL;
  }
  
  // allocate memory for the array in the table
  table->array = myArray;

  // check to see there were no problems allocating memory
  if (table == NULL) {
    return NULL;  // there was a problem with malloc
  } else {
    // set the field for number of slots
    // return the new, empty table that we just created
    table->num_slots = num_slots;
    table->itemdelete = itemdelete;
    return table;
  }
}


/**************** hashtable_find() ****************/
void *
hashtable_find(hashtable_t *ht, char *key)
{
  // check to make sure the table is not NULL
  if (ht == NULL)
    return NULL;

  // obtain hash value of key using JenkinsHash
  long hash = JenkinsHash(key, ht->num_slots);

  // search the list at the slot the key hashes to
  if (ht->array[hash] != NULL) {
    list_t *list = ht->array[hash];
    return list_find(list, key);
  }
  else {
    return NULL;  // return NULL if problem accessing list
  }
}


/**************** hashtable_insert() ****************/
bool
hashtable_insert(hashtable_t *ht, char *key, void *data)
{
  // check to make sure the table is not NULL  
  if (ht == NULL) {
    return false;  // we don't insert anything if the table is NULL
  }

  // if we didn't return already, obtain hash value of key using JenkinsHash
  long hash = JenkinsHash(key, ht->num_slots);

  // check to see if list has been initialized in the slot
  if (ht->array[hash] == NULL) {
    list_t *slotList = list_new(ht->itemdelete);   // make new list
    ht->array[hash] = slotList;      // assign list to slot
    return list_insert(slotList, key, data);  // insert item
  }
  
  // if the key already exists in the table, don't insert it
  if (hashtable_find(ht, key) != NULL) {
    return false;
  }
  
  // insert item into list in slot corresponding to hash value
  list_insert((ht->array[hash]), key, data);
  
  return true; // we inserted the item, so return true
}

/**************** hashtable_delete() ****************/
/* Deletes the data structure */
void
hashtable_delete(hashtable_t *ht)
{
  if (ht != NULL) {
    int i = 0;
    // loop through the slots in the hashtable
    while (i < ht->num_slots) {
      if (ht->array[i] != NULL){
	// delete everything in the array
 	list_delete(ht->array[i]);
      }
      i++;	    
    }
    // free the memory
    free(ht->array);
    free(ht);
  }
}

/**************** hashtable_iterate() ****************/
/* Iterates through the hashtable data structure */
void
hashtable_iterate(hashtable_t *table,
		  void (*itemfunc)(void *arg, char *key, void *data),
		  void *arg)
{ 
  if (table == NULL || itemfunc == NULL)
    return;
  else {
    // loop through all the slots in the table
    for (int i = 0; i < table->num_slots; i++) {
      if (table->array[i] != NULL) {
	// iterate through the list in each slot
	list_iterate(table->array[i], itemfunc, arg);
      }
    }
  }
  return;
}
