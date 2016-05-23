/*
 * guide_agent.c - 
 *
 * Drew Waterman (Team Lapis)
 *
 * code for socket setup based on Professor Kotz's chatclient.c (5/18/16) 
 * May 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <ctype.h>
#include "../common/file.h"

static int createSocket(const int argc, const char *argv[],
			struct sockaddr_in *gsp, int logSwitch);
bool isNumber(const char *arg);
char *getPlayerName(void);
char *getGuideID(void);
/* bool sendFirstMessage(const char *guideID, const char *teamName, */
/* 		      const char *playerName, const struct sockaddr *gsp); */


int main(const int argc, const char *argv[]) {

  // represents whether or not the -log=raw switch was used
  int logSwitch = 0;

  // if the first argument is -log=raw, change logSwitch to reflect that
  if (strcmp(argv[1], "-log=raw") == 0) {
    logSwitch = 1;
  }


  // assign the team name input by the user to a variable
  // position of this team name on the command line varies depending
  // on whether or not the -log=raw switch was used    
  const char *teamName;
  if (logSwitch == 0)
    teamName = argv[1];
  else
    teamName = argv[2];
  
  printf("%s ", teamName);


  const char *playerName = getPlayerName();
  printf("player name is %s\n", playerName);

  const char *guideID = getGuideID();
  printf("guideId is %s\n", guideID);
  
  
  // following code based on Kotz's chatclient.c
  
  struct sockaddr_in gs;
  const struct sockaddr *gsp = (struct sockaddr *) &gs;
  
  int comm_sock = createSocket(argc, argv, &gs, logSwitch);


  /* if (!sendFirstMessage(guideID, teamName, playerName, gsp)) { */
  /*   exit(3); */
  /* } */
  
  /***** send message to GS to announce presence *****/
  char *message = malloc(strlen("GA_STATUS") + strlen(guideID)
  			 + strlen(teamName) + strlen(playerName)+ 8);

  if (message == NULL) {
    printf("Error allocating memory for first message\n");
    exit(3);
  }
  sprintf(message, "GA_STATUS|0|%s|%s|%s|1", guideID, teamName, playerName);

  printf("initial message is: %s\n", message);
  
  if (sendto(comm_sock, message, strlen(message), 0, gsp, sizeof(gs)) < 0) {
    printf("Error sending startup message\n");
    exit(4);
  }

  free(message);
  


  while (true) {

    /* // signals whether or not the program has launched the graphical interface */
    /* int interfaceUp = 0; */

    fd_set rfds;
    struct timeval timeout;
    const struct timeval fifteensec = {15,0};
    
    FD_ZERO(&rfds);

    FD_SET(0, &rfds); // stdin
    FD_SET(comm_sock, &rfds);
    
    int nfds = comm_sock + 1;

    timeout = fifteensec;
    int select_response = select(nfds, &rfds, NULL, NULL, &timeout);


    if (select_response < 0) {
      printf("Error\n");
    }
    else if (select_response == 0) {
      ;
    }
    else if (select_response > 0) {
      if (FD_ISSET(0, &rfds)) {
	// handle stdin
	;
      }

      if (FD_ISSET(comm_sock, &rfds)) {
	// handle socket
      }
    }
  }

  close(comm_sock);
  putchar('\n');
  return 0;

}




/* Parse arguments and set up socket
 */
static int createSocket(const int argc, const char *argv[],
			struct sockaddr_in *gsp, int logSwitch)
{

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

  // store IP address input by user in a variable
  // position for this argument varies as well depending on switch
  const char* GShost;

  if (logSwitch == 0)
    GShost = argv[2];
  else
    GShost = argv[3];


  // store port number input by user in a variable
  // position for this argument varies as well depending on switch
  int GSport;

  if (logSwitch == 0) {
    if (isNumber(argv[3]))
      GSport = atoi(argv[3]);
    else {
      printf("Port number can only consist of integers.\n");
      exit(1);
    }

  }
  else {
    if (isNumber(argv[4]))
      GSport = atoi(argv[4]);
    else {
      printf("Port number can only consist of integers.\n");
      exit(1);
    }
  }

  struct hostent *hostp = gethostbyname(GShost);
  if (hostp == NULL) {
    printf("Error: unknown host %s\n", GShost);
    exit(3);
  }

  // initialize fields of the server address
  gsp->sin_family = AF_INET;
  bcopy(hostp->h_addr_list[0], &gsp->sin_addr, hostp->h_length);
  gsp->sin_port = htons(GSport);

  // Create socket
  int comm_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (comm_sock < 0) {
    printf("Error opening datagram socket\n");
    exit(2);
  }

  return comm_sock;
}




bool isNumber(const char *arg)
{
  for (int i = 0; i < strlen(arg); i++) {
    if (!isdigit(arg[i]))
	return false;
  }
  return true;
}



/* Repeatedly prompts guide agent for their playerName until they enter
 * a valid player name, and assigns them this name for the rest of the game
 */
char *getPlayerName(void)
{

  int goodName = 0;
  char *playerName;
  
  while (goodName == 0) {
    printf("What is your name?: ");

    playerName = readline(stdin);

    for (int i = 0; i < strlen(playerName); i++) {
      if (!isprint(playerName[i])) {
	goodName = 0;
	break;
      }
    }
    goodName = 1;
  }

  return playerName;
}



/* Repeatedly prompts user for 8-character hexadecimal ID until they 
 * enter a valid one. Returns their first valid entry.
 */
char *getGuideID(void)
{
  int goodID = 0;
  char *guideID;

  while (goodID == 0) {
    printf("What is the guide's ID?: ");
    guideID = readline(stdin);

    if (strlen(guideID) != 8)
      printf("guide ID must be 8 hexadecimal characters\n");
    else {
      for (int i = 0; i < strlen(guideID); i++) {
	if (ishexdigit(guideID[i]) == 0) {
	  goodID = 0;
	  break;
	}
      }
    
      goodID = 1;
    }
  }
  return guideID;
}


/* bool sendFirstMessage(const char *guideID, const char *teamName, */
/* 		      const char *playerName, const struct sockaddr *gsp) */
/* { */
/*   char *message = malloc(strlen("GA_STATUS") + strlen(guideID) */
/* 			 + strlen(teamName) + strlen(playerName)+ 8); */

/*   if (message == NULL) { */
/*     printf("Error allocating memory for first message\n"); */
/*     return false; */
/*   } */
/*   sprintf(message, "GA_STATUS|0|%s|%s|%s|1", guideID, teamName, playerName); */

/*   printf("initial message is: %s\n", message); */

/*   if (sendto(comm_sock, message, strlen(message), 0, gsp, sizeof(gs)) < 0) { */
/*     printf("Error sending startup message\n"); */
/*     return false; */
/*   } */

/*   free(message); */
/*   return true; */

/* } */

