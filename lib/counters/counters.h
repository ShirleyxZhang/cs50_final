/*
 * counters.h - header file for counters.c
 *
 * this header file was based on tree.h from Professor Kotz's tree6 module
 *
 * Drew Waterman, April 2016
 */

#ifndef __COUNTERS_H
#define __COUNTERS_H


/**************** global types ****************/
typedef struct counters counters_t;


/**************** functions ****************/

/* Create a new empty counters module
 * Return NULL if there are any errors
 */
counters_t *counters_new(void);


/* If a given key exists, increment its counter
 * Otherwise, insert key into the list and set its coutner to 0
 */
void counters_add(counters_t *ctrs, int key);

/* Return the current value of the counter for the given key
 * Return 0 if the key is not found
 */
int counters_get(counters_t *ctrs, int key);

/* Delete the whole counters data sturcture and its contents
 */
void counters_delete(counters_t *ctrs);

/* Sets the counter fields to the values passed in as parameters */
void counters_set(counters_t *ctrs, int key, int count);

/* Iterates through the counters */
void
counters_iterate(counters_t *ctrs,
		 void (*itemfunc)(void *arg, int key, int count),
		 void *arg);

#endif // __COUNTERS_H
