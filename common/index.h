/* index.h - header file for index.c
 *
 * Divya Kalidindi and Drew Waterman, May 2016
 *
 */


#ifndef __INDEX_H
#define __INDEX_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include "../lib/hashtable/hashtable.h"
#include "../lib/counters/counters.h"
#include "word.h"
#include "file.h"

/**************** functions ****************/

/* Load the word count information from the old file given into an index */
bool index_load(char *oldIndexFilename, hashtable_t *index);

/* itemfunc used while iterating through hashtable */
void indexerator(void *arg, char *key, void *data);

/* itemfunc used while iterating through counters */
void counterator(void *arg, int key, int count);

/* Creates a file and write the index to that file, in the format specified 
 in .c file */
bool index_save(char *filename, hashtable_t *index);

/* Checks to see if the directory has been used to run crawler */
bool IsCrawlerDirectory(char *dir);




#endif // __INDEX_H
