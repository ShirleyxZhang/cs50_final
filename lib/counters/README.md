README for counterstest.c
Drew Waterman, April 2016

Compiling:
	make

Usage:
/*
 * counterstest.c - a program to test counters.c
 *
 * Drew Waterman
 */

and

/*
 * counters.c - counters module that keeps track of how many times
 * a key has been inserted into the structure, a simple linked list
 *
 * program based on tree.c from Professor Kotz's tree6 module
 *
 * Drew Waterman, April 2016
 */

Example command line:
  ./counterstest

Exit status:
  0 - success

Assumptions:
- the user will only pass integers as the second parameter to the
  counters_add and counters_insert functions
- after deleting a counters structure, the user will have to set its
  pointer to NULL in order to prevent any segmentation faults since
  the counters_delete function cannot do that itself


Limitations:
- the counters_delete function cannot set the pointer of the counters
  structure to NULL

