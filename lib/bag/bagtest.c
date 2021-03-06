/*
 * bagtest.c - a program to test bag.c
 *
 * Drew Waterman, April 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bag.h"


/* typedef struct test { */
/*   char *key; */
/*   int depth; */
/* } test_t; */


int main() {

  // create new bag
  bag_t *bag;
  bag = bag_new();

  /* test_t *test = malloc(sizeof(test_t)); */
  /* test->key = malloc(strlen("ugh") + 1); */
  /* strcpy(test->key, "ugh"); */
  /* test->depth = 8; */

  /* bag_insert(bag, test); */
  
  // check to see if bag creation worked ok
  if (bag == NULL) {
    printf("Error creating bag\n");
    return 1;
  }

  char *myString1 = malloc(sizeof("poop") + 1);
  strcpy(myString1, "poop");
  bag_insert(bag, myString1);

  

  
  // insert three items into the bag
  /* bag_insert(bag, "hello"); */
  /* bag_insert(bag, "goodbye"); */
  /* bag_insert(bag, "hello again"); */
  

  /* printf(test->key); */
  /* printf("\n%d\n", test->depth); */
  /* // (attempt to) extract 4 items from the bag */
  /* test_t *bag1 = bag_extract(bag); */
  char *bag2 = bag_extract(bag);
  void *bag3 = bag_extract(bag);

  // this extraction should return NULL since we only inserted 3 items
  // into the bag
  void *bag4 = bag_extract(bag);

  /* // before each print, check to make sure the removed item is not NULL */
  /* if (bag1 != NULL) { */
  /*   printf("here\n"); */
  /*   //    printf("%d\n", bag1->depth); */
  /* } */
  if (bag2 != NULL)
    printf("%s\n", bag2);
  if (bag3 != NULL)
    printf("%s\n", (char*) bag3);
  if (bag4 != NULL)
    printf("%s\n", (char*) bag4);

  //  bag_delete(bag);

}

/* static void delete(void *data) */
/* { */
/*   if (data) { */
/*     free(data); */
/*   } */
/* } */
