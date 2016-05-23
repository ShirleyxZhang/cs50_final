README for hashtable.c
Drew Waterman, April 2016

Compiling:
	make

Usage:
/*
 * hashtable.c - a hashtable module
 * implemented using an array and linked lists
 *
 * The outline/structure of this program was modeled off of
 * tree.c for the tree6 module, written by Professor David Kotz
 *
 * Drew Waterman, April 2016
 */

and

/*
 * hashtest.c - a program to test the hashtable module
 *
 * Drew Waterman, April 2016
 */

[My testing program is called hashtest.c, not hashtabletest.c, but my Makefile
creates a file called hashtabletest]

Example command line:
	./hashtabletest

Exit status:
  0 - success

Assumptions:
- I could only test hashtable.c using strings for data, but the module should
  work with other data types as well
- The user can make the hashtable as big or as small as they want
- The hashtable is implemented using a linked list


Limitations:
- hashtable.c does not have any delete functions to free memory, so memory
  leaks are to be expected