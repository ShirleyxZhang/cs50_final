/*
 * guide_agent.c - 
 *
 * Drew Waterman (Team Lapis)
 * May 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include "netdb.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool isNumber(const char *arg);


int main(const int argc, const char *argv[]) {

  // represents whether or not the -log=raw switch was used
  int logSwitch = 0;

  // if the first argument is -log=raw, change logSwitch to reflect that
  if (strcmp(argv[1], "-log=raw") == 0) {
    logSwitch = 1;
  }

  // Check to make sure the user entered the right number or arguments
  // if the log switch was called, there should be 5 arguments
  if (logSwitch == 1 && argc != 5) {
    printf("Incorrect number of arguments.\n");
    exit(1);
  }
  // if the log switch was not called, there should be 4 arguments
  else if (logSwitch == 0 && argc != 4) {
    printf("Incorrect number of arguments.\n");
    exit(1);
  }

  // assign the team name input by the user to a variable
  // position of this team name on the command line varies depending
  // on whether or not the -log=raw switch was used
  char *teamName = malloc(strlen(argv[1]) + 1);
  if (teamName == NULL) {
    printf("Error allocating memory for team name\n");
    exit(2);
  }
  else if (logSwitch == 0)
    sscanf(argv[1], "%s", teamName);
  else
    sscanf(argv[2], "%s", teamName);
  

  // store IP address input by user in a variable
  // position for this argument varies as well depending on switch
  char* GShost = malloc(strlen(argv[2]) + 1);
  if (GShost == NULL) {
    printf("Error allocating memory\n");
    exit(2);
  }
  else if (logSwitch == 0)
    sscanf(argv[2], "%s", GShost);
  else
    sscanf(argv[3], "%s", GShost);


  // store port number input by user in a variable
  // position for this argument varies as well depending on switch  
  int GSport;
  
  if (logSwitch == 0) {
    if (isNumber(argv[3]))
	sscanf(argv[3], "%d", &GSport);
    else {
      printf("Port number can only consist of integers.\n");
      exit(1);
    }
	
  }
  else {
    if (isNumber(argv[4]))
      sscanf(argv[4], "%d", &GSport);
    else {
      printf("Port number can only consist of integers.\n");
      exit(1);
    }
  }

  printf("%s %s %d\n", teamName, GShost, GSport);

  return 0;

}





bool isNumber(const char *arg)
{
  for (int i = 0; i < strlen(arg); i++) {
    if (!isdigit(arg[i]))
	return false;
  }
  return true;
}
