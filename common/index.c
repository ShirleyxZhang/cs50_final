/*
 * index.c - functions needed for indexer.c and indextest.c
 *
 * Drew Waterman, Divya Kalidindi, May 2016
 */

#include "index.h"


/******************* index_load() ********************/
/* Load the word count information from the old file given into an index */
bool index_load(char *oldIndexFilename, hashtable_t *index)
{
  FILE *fp;   // the file to be read

  // the word, keyID, and count from the old file, respectively
  char *word = malloc(1000);
  if (word == NULL) {
    return false;
  }

  int keyID;
  int count;

  // go through the old file and scan the word, key, and count respectively
  if ((fp = fopen(oldIndexFilename, "r")) != NULL) {
    while (fscanf(fp, "%s ", word) == 1) {
      counters_t *newcounter = counters_new();
      while (fscanf(fp, "%d %d ", &keyID, &count) >0) {
	// set the key ID and count that have been scanned
	counters_set(newcounter, keyID, count);
      }
      // insert back into the data structure
      hashtable_insert(index, word, newcounter);
    }
  }
  // free the memory and close the file
  free(word);
  fclose(fp);
  return true;
}


/******************* index_save() ********************/
/* Creates a file and write the index to that file, in the format 
 * in which there is one line per word, one word per line, and 
 * each line provides the word and one or more (docID, count) pairs.
 */
bool index_save(char *filename, hashtable_t *index)
{
  FILE *fp;  // to read the file

  // open file corresponding to name specified
  if((fp = fopen(filename, "w")) == NULL) {
    fclose(fp);
    return false;
  }
  else {
    // go through the index
    hashtable_iterate(index, indexerator, fp);
    fclose(fp);
    return true;
  }
}

/******************* indexerator() ********************/
/* itemfunc used while iterating through hashtable              
 */
void indexerator(void *arg, char *key, void *data)
{
  FILE *fp = arg;
  // print the word
  fprintf(fp, "%s ", key);
  counters_iterate(data, counterator, fp);
  fprintf(fp, "\n");
}

/******************* counterator() ********************/
/* itemfunc used while iterating through counters 
 */
void counterator(void *arg, int key, int count)
{
  FILE *fp = arg;
  // print the key and count
  fprintf(fp, "%d %d ", key, count);
}

/******************* isCrawlerDirectory() ********************/
/* Tries to open a file named .crawler in the passed in directory;
 * if success, it means that the directory has been used to run crawler.
 * If not, then dir is not a directory, not readable, or not a crawler’s 
 * output directory.              
 */
bool
IsCrawlerDirectory(char *dir)
{
  FILE *fp;                // the file to be place in the directory
  char* slash = "/";
  char* file = ".crawler";

  // allocate enough space for URL
  char* filename = malloc(strlen(dir) + strlen(slash) + strlen(file) + 1);
  if (filename == NULL)
    return false;

  // create the URL from the concatenated string
  sprintf(filename, "%s%s%s", dir, slash, file);

  // returns false if the directory is not valid or readable, or not
  // a crawler's output directory
  if((fp = fopen(filename, "r")) == NULL) {
    free(filename);
    return false;
  }
  // otherwise, the directory is one in which the crawler was run
  else
    {
      fclose(fp);
      free(filename);
      return true;
    }
}

