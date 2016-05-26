README for bag.c
Drew Waterman, April 2016

Compiling:
	make

/*
 * bag.c - data structure that stores void pointers
 *
 * Code for bag.c based on tree.c from Professor Kotz's tree6 module
 *
 * Drew Waterman, April 2016
 */
      
and

/*
 * bagtest.c - a program to test bag.c
 *
 * Drew Waterman, April 2016
 */

Example command line:
	./bagtest

Exit status:
  0 - success
  1 - error creating bag structure

Assumptions:
- I only tested using strings. The module should work using other types
  of data as well. I casted the return values of the extract function
  as char* in order to print the strings that I originally inserted
- The module assumes that the user keeps track of what types of data are
  going in and out. It does not matter which types of data are stored in
  the bag module; bag.c stores everythign as void*
- The item extracted when bag_extract() is called is the item that was most
  recently inserted

Limitations:
- bag_extract will only return the last item in the list, so the user cannot
  choose which item to extract based on its data
- bag_extract returns void data, so any strings or integers must be cast into
  their appropriate types before being printed

