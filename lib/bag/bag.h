/*
 * bag.h - header file for bag.c
 *
 * bag.h based on tree.h, written by Professor Kotz for the tree6 module
 *
 * Drew Waterman, April 2016
 */

#ifndef __BAG_H
#define __BAG_H


/**************** global types ****************/
typedef struct bag bag_t;


/**************** functions ****************/

void
bag_delete(bag_t *bag);

/* create a new empty bag
 * return NULL if there are any errors
 */
bag_t *bag_new();


/* insert item into the bag
 * return NULL if there are any errors
 * the item consists of the data
 */
void bag_insert(bag_t *bag, void *data);


/* remove an item from the bag and return it to the caller
 * return NULL if the bag is empty
 */  
void *bag_extract(bag_t *bag);


#endif // __BAG_H
