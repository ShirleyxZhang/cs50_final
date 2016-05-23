/*
 * listtest.c - a program to test list.c
 *
 * Drew Waterman, April 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"

void itemdelete(void *data);

void dont_delete();

typedef struct test {
  char *data;
} test_t;

int main() {

  // create a new list
  list_t *list = list_new(itemdelete);

  test_t *test1 = malloc(sizeof(test_t));
  test1->data = "hello";
    //  strcpy(test1->data, "hello");
  
  test_t *test2 = malloc(sizeof(test_t));
  test2->data = "goodbye";


  list_insert(list, "hello", test1);

  list_insert(list, "goodbye", test2);
  
  /* list_find(list, "sup"); */
  
  /* // (attempt to) insert two items into the list */
  /* bool insertion1 = list_insert(list, "this is a", "test"); */
  /* bool insertion2 = list_insert(list, "hello", "goodbye"); */

  /* // this insertion should be a failure because there is */
  /* // already an item with the key "hello" in the list */
  /* bool insertion3 = list_insert(list, "hello", "world"); */

  /* // look for these items and store whatever list_find */
  /* // returns into a variable */
  /* char *find1 = list_find(list, "this is a"); */
  /* char *find2 = list_find(list, "hello"); */
  /* char *find3 = list_find(list, "meow"); */

  /* // (attempt to) print the values that list_find returns */
  /* if (find1 != NULL) */
  /*   printf("%s\n", find1); */
  /* if (find2 != NULL) */
  /*   printf("%s\n", find2); */

  /* // this should not print a value since "meow" was not inserted as a key */
  /* if (find3 != NULL) */
  /*    printf("%s\n", find3); */

  /* // print the boolean return values of the list_insert calls */
  /* printf("%d\n", insertion1); */
  /* printf("%d\n", insertion2); */
  /* printf("%d\n", insertion3);; */


  
  list_delete(list);
  return 0;

}


void itemdelete(void *data) {
  if (data)
    free(data);

}


void dont_delete() {

}
