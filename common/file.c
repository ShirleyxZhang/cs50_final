/* 
 * file utilities - reading a word, line, or entire file
 * 
 * See file.h for documentation.
 * 
 * David Kotz, May 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "file.h"


/**************** lines_in_file ****************/
int
lines_in_file(FILE *fp)
{
  if (fp == NULL)
    return 0;

  rewind(fp);

  int nlines = 0;
  char c = '\0';
  while ( (c = fgetc(fp)) != EOF)
    if (c == '\n')
      nlines++;

  rewind(fp);
  
  return nlines;
}

/**************** utility stopfuncs ****************/
// for use with readuntil()
static int never(int c) { return (0); }
static int isnewline(int c) { return (c == '\n'); }

/**************** readfile ****************/
char *readfile(FILE *fp) { return readuntil(fp, never); }

/**************** readline ****************/
char *readline(FILE *fp) { return readuntil(fp, isnewline); }

/**************** readword ****************/
char *readword(FILE *fp) { return readuntil(fp, isspace); }

/**************** readuntil ****************/
char *
readuntil(FILE *fp, int (*stopfunc)(int c))
{
  if (stopfunc == NULL)
    stopfunc = never;

  // allocate buffer big enough for "typical" words/lines
  int len = 81;
  char *buf = calloc(len, sizeof(char));

  // Read characters from file until stop-character or EOF, 
  // expanding the buffer when needed to hold more.
  int pos = 0;
  char c;
  for (pos = 0; (c = fgetc(fp)) != EOF && !(*stopfunc)(c); pos++) {
    // We need to save buf[pos+1] for the terminating null
    // and buf[len-1] is the last usable slot, 
    // so if pos+1 is past that slot, we need to grow the buffer.
    if (pos+1 > len-1)
      buf = realloc(buf, ++len);
    buf[pos] = c;
  }

  if (pos == 0 && c == EOF) {
    // no characters were read and we reached EOF
    free(buf);
    return NULL;
  } else {
    // pos characters were read into buf[0]..buf[pos-1].
    buf[pos] = '\0'; // terminate string
    return buf;
  }
}

/**************** file2string ****************/
/* 
 * See file.h for documentation.
 * Adapted from code by Ray Jenkins.
 */
char *
file2string(FILE *fp)
{
  int n, pos;
  char *buffer;

  // How big is the file?
  fseek (fp, 0, SEEK_END);
  int len = ftell (fp);
  rewind(fp);

  // allocate buffer big enough for file, plus terminating null char
  buffer = calloc(len + 1, sizeof(char));

  // Try to read the whole file, but fread might return less;
  // thus, we must loop, calling fread, until it returns 0 (EOF).
  pos = 0;
  while ((n = fread((buffer + pos), sizeof(char), len, fp)) > 0)
    pos += n;

  // did we get the whole file?
  if (pos != len) {
    // read error
    free(buffer);
    return NULL;
  }

  return buffer;
}


#ifdef QUICKTEST
int main(int argc, char *argv[])
{
  if (argc != 2) 
    exit(1);

  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    printf("can't open %s\n", argv[1]);
    exit(2);
  }

  char *word;
  while ( (word = readword(fp)) != NULL) {
    printf("[%s] ", word);
    free (word);
  }
}
#endif
