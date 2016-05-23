/*
 * counterstest.c - a program to test counters.c
 *
 * Drew Waterman
 */

#include <stdio.h>
#include <stdlib.h>
#include "counters.h"

int main()
{
  // create two new counters
  counters_t *ctrs = counters_new();
  counters_t *hello = counters_new();

  // add integers into one of the counters modules
  counters_add(ctrs, 3);
  counters_add(ctrs, 784);
  counters_add(ctrs, 3);

  counters_set(ctrs, 12, 6);
  counters_set(ctrs, 3, 245);
  
  printf("%d\n", counters_get(ctrs, 784)); // should be 1
  printf("%d\n", counters_get(ctrs, 3));   // should be 2
  printf("%d\n", counters_get(ctrs, 7));   // should be 0
  printf("%d\n", counters_get(hello, 3));  // should be 0
  printf("%d\n", counters_get(ctrs, 12));

  // delete one of the counters
  counters_delete(ctrs);
  ctrs = NULL; // set its pointer to NULL in order to prevent later errors
  
  printf("%d\n", counters_get(ctrs, 3)); // should be 0

  // delete other counter in order to prevent memory leaks
  counters_delete(hello);
  hello = NULL;

  return 0;
}
