README for common
Drew Waterman, Divya Kalidindi, May 2016

Compiling:
        make

Usage:
/*
* Common directory,  which can be used at this time, to run
* crawler and indexer only.
*
* Drew Waterman, Divya Kalidindi, May 2016
*/

### Important Information ###

- Ensure that the following files are present in the common directory:
  
Makefile, file.c, file.h, index.c, index.h, web.c, web.h, word.c, word.h

- All web, file, and web .c and .h files, are provided by Professor Kotz

Index.c contains functions used by both indexer.c and indextest.c:
These are:
- index_save, which creates a file and writes the index to that file
- indexerator, an iterator function that iterates through the hashtable
- counterator, an iterator function that iterates through the counter

Index.h is the header file for index.c and contains function prototypes.

Running the Makefile will build the library and create a common.a file. 