/*
 * hashtest.c - a program to test the hashtable module
 *
 * Drew Waterman, April 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "jhash.h"
#include "../list/list.h"

void itemdelete(void *data);

typedef struct test {
  char *data;
} test_t;


int main()
{
  // number of slots in the hashtable
  int numslots = 11;

  // create new hashtable with corresponding number of slots
  hashtable_t *table = hashtable_new(numslots, itemdelete);


  test_t *test1 = malloc(sizeof(test_t));
  test1->data = "hello";
  //  strcpy(test1->data, "hello");

  test_t *test2 = malloc(sizeof(test_t));
  test2->data = "goodbye";

  hashtable_insert(table, "hello", test1);

  hashtable_insert(table, "goodbye", test2);

  /* // search through empty hashtable for arbitrary string */
  /* // making sure there are no errors with searching empty table */
  /* hashtable_find(table, "hey"); */
  
  /* // insert various keys and data into the hash table */
  /* hashtable_insert(table, "this is a ", "test"); */
  /* hashtable_insert(table, "hash", "table"); */
  /* hashtable_insert(table, "hello world", "goodbye world"); */
  /* hashtable_insert(table, "hi", "bye"); */
  /* hashtable_insert(table, "drew", "waterman"); */
  /* hashtable_insert(table, "Drew", "Waterman"); */

  /* /\* find the strings we inserted and attempt to insert them */
  /*  * print their data and hash values if found *\/ */

  /* // string1 should be "test" */
  /* char *string1 = hashtable_find(table, "this is a "); */
  /* if (string1 != NULL) { */
  /*   printf("%ld ", JenkinsHash("this is a ", numslots)); */
  /*   printf("%s\n", string1); */
  /* } */

  /* // string2 should be NULL since we didn't insert "meow" as a key */
  /* char *string2 = hashtable_find(table, "meow"); */
  /* if (string2 != NULL) { */
  /*   printf("%ld ", JenkinsHash("meow", numslots)); */
  /*   printf("%s\n", string2); */
  /* } */

  /* // string3 should be "bye" */
  /* char *string3 = hashtable_find(table, "hi"); */
  /* if (string3 != NULL) { */
  /*   printf("%ld ", JenkinsHash("hi", numslots)); */
  /*   printf("%s\n", string3); */
  /* } */

  /* // string4 should be "goodbye world" */
  /* char *string4 = hashtable_find(table, "hello world"); */
  /* if (string4 != NULL) { */
  /*   printf("%ld ", JenkinsHash("hello world", numslots)); */
  /*   printf("%s\n", string4); */
  /* } */

  /* // string5 should be "waterman" */
  /* char *string5 = hashtable_find(table, "drew"); */
  /* if (string5 != NULL) { */
  /*   printf("%ld ", JenkinsHash("drew", numslots)); */
  /*   printf("%s\n", string5); */
  /* } */

  /* // string6 should be "Waterman" */
  /* char *string6 = hashtable_find(table, "Drew"); */
  /* if (string6 != NULL) { */
  /*   printf("%ld ", JenkinsHash("Drew", numslots)); */
  /*   printf("%s\n", string6); */
  /* } */

  hashtable_delete(table);

  return 0;

}

void itemdelete(void *data) {
  if (data)
    free(data);
}
