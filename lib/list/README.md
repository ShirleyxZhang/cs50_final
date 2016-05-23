README for list.c
Drew Waterman, April 2016

Compiling:
	make

Usage:
 /*
 * listtest.c - a program to test list.c
 *
 * Drew Waterman, April 2016
 */

and

/*
 * list.c - program for creating linked list data structures
 *
 * program based on tree.c from Professor Kotz's tree6 module
 *
 * Drew Waterman, April 2016
 */
      
Example command line:
  ./listtest

Exit status:
  0 - success

Assumptions:
- I only tested the list module using strings for data, but it should work
  with other data types
- this program does not worry about memory leaks, as it does not have any
  sort of delete functions

Limitations:
- Items are only inserted at the end of the linked list
- There is not delete function to free allocated memory and prevent
  memory leaks