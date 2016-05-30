/*
 * guide.c - One of three parts for the game "Mission Imcomputable"
 * Works with a Game Server and Field Application to play the game.
 * Receives status updates from the Game Server and allows the person 
 * playing as the Guide Agent to send hints to Field Agents through the
 * Game Server. 
 *
 * Usage: 
 * ./guide [-v] teamName playerName GShost GSport
 *
 * Drew Waterman (Team Lapis), May 2016
 *
 * code for socket setup (createSocket), handling stdin (handleStdin), and 
 * handling information sent by the socket (handleSocket) based on Professor 
 * Kotz's chatclient.c (5/18/16) 
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
#include <time.h>
#include <getopt.h>
#include "../common/file.h"
#include "../lib/list/list.h"
#include "../lib/bag/bag.h"
#include "../lib/counters/counters.h"
/* #include <cairo.h> */
/* #include <gtk/gtk.h> */

typedef struct codedrop {
  char *status;
  char *hexcode;
  char *latitude;
  char *longitude;
  char *teamname;
} codedrop_t;


typedef struct FA {
  char *teamname;
  char *address;
  bool capture;
  char *pebbleid;
  char *name;
  char *latitude;
  char *longitude;
  char *contact;
} FA_t;

typedef struct recordStruct {
  char *teamName;
  char *numPlayers;
  char *numCaptures;
  char *numCaptured;
  char *numNeutralized;
} recordStruct_t;



static int createSocket(int argc, char *argv[], struct sockaddr_in *gsp,
			int logSwitch, int n);
bool isNumber(const char *arg);
char *getGuideID(void);
void sendFirstMessage(char *guideID, char *teamName, char *playerName,
		      int comm_sock, struct sockaddr_in gs);
static int handleSocket(int comm_sock, struct sockaddr_in *gsp, list_t *list,
			char **gameID, FILE *fp, int logSwitch);
char *handleStdin(int comm_sock, struct sockaddr_in *gsp, list_t *list,
		  char *teamName);
void printPebbleIDs(void *arg, char *key, void *data, void *option);
void getNumAgents(void *arg, char *key, void *data, void *option);
void deletefunc(void *data);
int printGameOver(char *gameID, FILE *fp, int logSwitch);
int sendStatusRequest(char *gameID, char *guideID, char *teamName,
            char *playerName, int comm_sock, const struct sockaddr *gsp);
void printTime(FILE *fp);
void deleteNodes(void *arg, char *key, void *data, void *option);

static int BUFSIZE = 8000;



int main(int argc, char *argv[]) {

  // Represents whether or not the -v switch was used
  // Default is 0 if the switch was not called by the user, and 1 if it was.
  int logSwitch = 0;

  // Represents how many switches were used. All switches that come before non-
  // optional arguments will be ignored. Any switches called after non-optional
  // arguments will cause the program to exit.
  int n = 0;

  char *teamName;
  char *playerName;
  char *guideID;
  char *gameID = NULL;
  
  int ch;
  while((ch = getopt(argc, argv, "v")) != -1) {
    switch (ch) {

    case 'v':
      // User called the [-v] option, so change the logSwitch to indicate that
      logSwitch = 1;
      break;

    default:
      break;
    }
  }

  // Increment n for every iteration of the while loop.
  // After the while loop, n will represent the number of switches on the
  // command line that come before the non-optional arguments
  int i = 1;
  while (argv[i][0] == '-') {
    n++;
    i++;
  }

  // Check to make sure the user entered the correct number of arguments
  if (argc != n + 5) {
    printf("Error: incorrect number of argments.\n");
    printf("Usage: ./guide [-v] teamName playerName GShost GSport\n");
    exit(1);
  }
  

  // assign the team name input by the user to a variable
  // position of this team name on the command line varies depending
  // on how many switches were called
  teamName = argv[n + 1];
  
  // assign player's name based on command line input
  playerName = argv[n + 2];
  
  // prompt user for an ID used to identify the guide agent
  guideID = getGuideID();

  // If the user entered "^D" before entering an ID, exit the program
  if (strcmp(guideID, "EOF") == 0) {
    printf("\n");
    exit(0);
  }
  
  // the server's address
  struct sockaddr_in gs;
  const struct sockaddr *gsp = (struct sockaddr *) &gs;

  // set up a socket on which to communicate  
  int comm_sock = createSocket(argc, argv, &gs, logSwitch, n);

  
  /***** send message to GS to announce presence *****/
  sendFirstMessage(guideID, teamName, playerName, comm_sock, gs);

  
  // open up a file to write the guide agent log to
  FILE *logp;
  if ((logp = fopen("../log/guideagent.log", "w")) == NULL) {
    printf("could not open log file");
  }
  else if (logSwitch == 0) {
    fprintf(logp, "Guide Agent %s joined the game with ID %s\n", playerName,
	    guideID);
  }
  else {
    // user input -v, so we also give them the time of each action
    printTime(logp);
    fprintf(logp, "Guide Agent %s joined the game with ID %s\n\n", playerName,
	    guideID);
  }

  // list to hold information about field agents
  list_t *FAList = list_new(deletefunc);
  
  /*** enter while loop that will run until the game ends ***/
  while (true) {
    // for use with select()
    fd_set rfds;              // set of file descriptors we want to read
    struct timeval timeout;   // how long we're willing to wait
    const struct timeval fifteensec = {15,0}; // fifteen seconds

    // Watch stdin (fd 0) and the UDP socket to see when either has input
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);         // stdin
    FD_SET(comm_sock, &rfds); // the UDP socket
    int nfds = comm_sock + 1; // highest-numbered fd in rfds

    // Wait for input on either source, up to fifteen seconds
    timeout = fifteensec;
    int select_response = select(nfds, &rfds, NULL, NULL, &timeout);
    // note: 'rfds' updated, and value of 'timeout' is now undefined

    if (select_response < 0) {
      // Some error occured
      printf("Error\n");
      exit(9);
    }
    else if (select_response == 0) {
      // timeout occured; request status from Game Server
      if (gameID != NULL) {
	int stat = sendStatusRequest(gameID, guideID, teamName, playerName,
				     comm_sock, gsp);

	if (stat > 0)
	  // an error occured while trying to send a status request
	  exit(stat);
      }
      else
	// have not yet joined a game
	printf("Waiting for a game to join. Please be patient.\n");
    }
    else if (select_response > 0) {
      // some data is ready on either source, or both
      if (FD_ISSET(0, &rfds)) {

	// get the pebble ID and hint input into stdin by the user
	char *idAndHint = handleStdin(comm_sock, &gs, FAList, teamName);

	// don't send a hint if handleStdin returned NULL; that means that
	// there are no active agents on the guide's team to send hints to
	if (strcmp(idAndHint, "NULL") != 0) {
	  // If there were any problems getting the ID and hint, or if the user
	  // chose to exit the program using ^D, break fromt the while loop 
	  if (strcmp(idAndHint, "EOF") == 0)
	    break;

	  // Check to make sure we've joined a game that we can send hints to
	  if ((gameID != NULL)) {

	    // Malloc space for a hint to send
	    char *sendHint = malloc(13 + strlen(gameID) + strlen(guideID) +
				    strlen(teamName) + strlen(playerName) +
				    strlen(idAndHint));

	    // Make sure that malloc did not return NULL
	    if (sendHint == NULL) {
	      printf("Error allocating memory for message.\n");
	      exit(5);
	    }

	    // Combine the necessary information into one sting to send to
	    // teh game server
	    sprintf(sendHint, "GA_HINT|%s|%s|%s|%s|%s", gameID, guideID,
		    teamName, playerName, idAndHint);

	    // Send the message to the game server
	    if (sendto(comm_sock, sendHint, strlen(sendHint), 0,
		       (struct sockaddr *) gsp, sizeof(*gsp)) < 0){
	      perror("error sending datagram\n");
	      exit(5);  // exit if there were any errors
	    }
	    else {
	      printf("Your hint was sent to the game server.\n");
	      free(sendHint);
	    }		
	  }
	  else {
	    // gameID is NULL, which means we haven't joined a game yet
	    printf("You can't send hints until you've joined the game!\n");
	    fflush(stdout);
	  } 
	}
	free(idAndHint);
      }
	  
      if (FD_ISSET(comm_sock, &rfds)) {

	// handle socket
	if (handleSocket(comm_sock, &gs, FAList, &gameID, logp,
			 logSwitch) == 0) {
	  // 0 means we received a GAME_OVER message
	  break;
	}
	printf("Enter a hint to send to a field agent.\n");
	fflush(stdout);
      }
    }
  }
  list_delete(FAList);
  //  list_delete(numAgents);
  free(guideID);
  if (gameID != NULL)
    free(gameID);
  close(comm_sock);
  putchar('\n');
  fclose(logp);
  exit(0);

}






/**************************** createSocket ***************************/

/* Parse arguments and set up socket
 */
static int createSocket(int argc, char *argv[],
			struct sockaddr_in *gsp, int logSwitch, int n)
{
  // store IP address input by user in a variable
  // position for this argument varies as well depending on switch
  const char* GShost;

  //  if (logSwitch == 0)
  /*   GShost = argv[3]; */
  /* else */
  /*   GShost = argv[4]; */
  GShost = argv[n + 3];

  // store port number input by user in a variable
  // position for this argument varies as well depending on switch
  int GSport;

  /* if (logSwitch == 0) { */
  /*   if (isNumber(argv[4])) */
  /*     GSport = atoi(argv[4]); */
  /*   else { */
  /*     printf("Port number can only consist of integers.\n"); */
  /*     exit(1); */
  /*   } */
  /* } */
  /* else { */
  /*   if (isNumber(argv[5])) */
  /*     GSport = atoi(argv[5]); */
  /*   else { */
  /*     printf("Port number can only consist of integers.\n"); */
  /*     exit(1); */
  /*   } */
  /* } */

  if (isNumber(argv[n + 4]))
    GSport = atoi(argv[n + 4]);
  else {
    printf("Port number can only consist of integers.\n");
    exit(1);
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




/********************* getGuideID ************************/

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
    
    if (guideID == NULL)
      return "EOF";
    
    if (strlen(guideID) != 8)
      printf("guide ID must be 8 hexadecimal characters\n");
    else {
      for (int i = 0; i < strlen(guideID); i++) {
	if (isxdigit(guideID[i]) == 0) {
	  goodID = 0;
	  break;
	}
      }
    
      goodID = 1;
    }
  }
  return guideID;
}



/******************** sendFirstMessage *********************/

void sendFirstMessage(char *guideID, char *teamName, char *playerName,
		      int comm_sock, struct sockaddr_in gs)
{
  char *message = malloc(strlen("GA_STATUS") + strlen(guideID)
			 + strlen(teamName) + strlen(playerName)+ 8);

  // check to make sure there were no problems allocating memory
  if (message == NULL) {
    printf("Error allocating memory for first message\n");
    exit(3);
  }

  // prepare the first message to send to the Game Server
  sprintf(message, "GA_STATUS|0|%s|%s|%s|1", guideID, teamName, playerName);
  
  // send the message
  if (sendto(comm_sock, message, strlen(message), 0, (struct sockaddr *) &gs, sizeof(gs)) < 0) {
    printf("Error sending startup message\n");
    exit(4);
  }

  printf("Sent first message to the game server to join the game.\n");
  free(message);

}




/********************** handleSocket *********************/


static int
handleSocket(int comm_sock, struct sockaddr_in *gsp, list_t *list, char **gameID, FILE *fp, int logSwitch)
{
  
  struct sockaddr_in sender;
  struct sockaddr *senderp = (struct sockaddr *) &sender;
  socklen_t senderlen = sizeof(sender);
  char buf[BUFSIZE];
  int nbytes = recvfrom(comm_sock, buf, BUFSIZE-1, 0, senderp, &senderlen);

  if (nbytes < 0) {
    perror("receiving from socket");
    // exit?
  }
  else {
    buf[nbytes] = '\0';     // null terminate string

    if (sender.sin_family != AF_INET)
      printf("From non-Internet address: Family %d\n", sender.sin_family);
    else {
      // was it from the expected server?
      if (sender.sin_addr.s_addr == gsp->sin_addr.s_addr &&
	  sender.sin_port == gsp->sin_port) {

	fprintf(fp, "Received the following message from the game server: %s\n", buf);
	
	char *OPCODE = strtok(buf, "|");
	char *ID = strtok(NULL, "|");
	
	if (OPCODE == NULL) {
	  if (logSwitch == 1)
	    printTime(fp);
	  fprintf(fp, "Received an invalid message and ignored it.\n");
	  return 1;
	}
	if (ID == NULL) {
	  if (logSwitch == 1)
	    printTime(fp);
	  fprintf(fp, "Received an invalid message and ignored it.\n");
	  return 1;
	}
	// if OPCODE is GAME_STATUS
	if (strcmp(OPCODE, "GAME_STATUS") == 0) {

	  // field containing info about all the field agents in the game
	  char *AllAgents = strtok(NULL, "|");
	  
	  // field containing info about all the code drops in the game
	  char *AllCodedrops = strtok(NULL, "|");
	  if (AllCodedrops == NULL) {
	    if (logSwitch == 1)
	      printTime(fp);
	    fprintf(fp, "Received an invalid message and ignored it.\n");
	    return 1;
	  }
	  
	  else {
	    // go through the remaining fields
	    
	    if (AllAgents == NULL) {
	      printf("No angents have joined the game yet.\n");
	    }
	    else {
	      // Bag holds strings of each agent field for convenience so
	      // we can strtok them later
      	      bag_t *agentBag = bag_new();

	      // agent represents the comma-separated subfield holding info
	      // about individual agents
	      char *agent = strtok(AllAgents, ":");
	      
      	      if (agent == NULL) {
		// list is empty?
		fprintf(fp, "There are currently no active agents.\n");
	      }
	      else {
		// go through each agent		
		bag_insert(agentBag, agent);

		while ((agent = strtok(NULL, ":")) != NULL) {
		  bag_insert(agentBag, agent);
		}

		list_iterate(list, deleteNodes, NULL, NULL);
				
		bag_t *FABag = bag_new();
		
		while ((agent = bag_extract(agentBag)) != NULL) {
		  FA_t *FA = malloc(sizeof(struct FA));
		  if (FA == NULL) {
		    printf("Error allocating memory.\n");
		    bag_delete(agentBag);
		    bag_delete(FABag);
		    return 1;
		  }		    

		  char *pebbleID = strtok(agent, ", ");
		  if (pebbleID == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n");
		    free(FA);
                    bag_delete(agentBag);
		    bag_delete(FABag);
		    return 1;
		  }		    
		  FA->pebbleid = pebbleID;
		  
		  char *teamName = strtok(NULL, ", ");
		  if (teamName == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n");
		    free(FA);
		    bag_delete(agentBag);
		    bag_delete(FABag);
		    return 1;
		  }		    
		  FA->teamname = teamName;
		  
		  char *playerName = strtok(NULL, ", ");
		  if (playerName == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n");
		    free(FA);
		    bag_delete(agentBag);
		    bag_delete(FABag);
		    return 1;
		  }
		  FA->name = playerName;
		  
		  char *playerStatus = strtok(NULL, ", ");
 		  if (playerStatus == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n"			    );
		    free(FA);
		    bag_delete(agentBag);
		    bag_delete(FABag);
		    return 1;
		  } 
		  if (strcmp(playerStatus, "captured") == 0)
		    FA->capture = true;
		  else if (strcmp(playerStatus, "active") == 0)		    
		    FA->capture = false;
		  else {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n"			    );
		    free(FA);
		    bag_delete(agentBag);
		    bag_delete(FABag);
		    return 1;
		  }		  
		      
		  char *lastKnownLat = strtok(NULL, ", ");
		  if (lastKnownLat == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n"			    );
		    free(FA);
                    bag_delete(agentBag);
		    bag_delete(FABag);
		    return 1;
		  }
		  
		  FA->latitude = lastKnownLat;
		  
		  char *lastKnownLong = strtok(NULL, ", ");
		  if (lastKnownLong == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n"	 			    );
		    free(FA);
		    bag_delete(agentBag);
		    bag_delete(FABag);
		    return 1;
		  }		  
		  FA->longitude = lastKnownLong;
		  
		  char *secondsSinceLastContact = strtok(NULL, ", ");
		  if (secondsSinceLastContact == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n"			    );
		    free(FA);
                    bag_delete(agentBag);
		    bag_delete(FABag);
		    return 1;
		  }		  
		  FA->contact = secondsSinceLastContact;

		  bag_insert(FABag, FA);
		  list_insert(list, FA->pebbleid, FA);
		}
		bag_delete(agentBag);

		codedrop_t *cd;

		if (AllCodedrops == NULL)
		  fprintf(fp, "There was something wrong with the FA field in the message from the\
 game server.\n");
		else {

		  bag_t *codedropBag = bag_new();

		  char *codedrop = strtok(AllCodedrops, ":");

		  if (codedrop == NULL) {
		    // list is empty?
		    fprintf(fp, "There are currently no codedrops\n");
		  }
		  else {
		    bag_insert(codedropBag, codedrop);

		    while ((codedrop = strtok(NULL, ":")) != NULL) {
		      bag_insert(codedropBag, codedrop);
		    }

		    bag_t *CDBag = bag_new();
		      
		    while ((codedrop = bag_extract(codedropBag)) != NULL) {

		      cd = malloc(sizeof(codedrop_t));
		      if (cd == NULL) {
			printf("Error locating memory.\n");
			bag_delete(FABag);
			bag_delete(codedropBag);
			bag_delete(CDBag);
			return 1;
		      }

		      char *codeID = strtok(codedrop, ", ");
		      if (codeID == NULL) {
			if (logSwitch == 1)
			  printTime(fp);
			fprintf(fp, "Received an invalid message and ignored it.\n");
			bag_delete(FABag);
			bag_delete(codedropBag);
			bag_delete(CDBag);
			return 1;
		      }
		      cd->hexcode = codeID;

		      char* lat = strtok(NULL, ", ");
		      if (lat == NULL) {
			if (logSwitch == 1)
			  printTime(fp);
			fprintf(fp, "Received an invalid message and ignored it.\n");
			bag_delete(FABag);
			bag_delete(codedropBag);
			bag_delete(CDBag);
			return 1;
		      }
		      cd->latitude = lat;

		      char* Long = strtok(NULL, ", ");
		      if (Long == NULL) {
			if (logSwitch == 1)
			  printTime(fp);
			fprintf(fp, "Received an invalid message and ignored it.\n");
			bag_delete(FABag);
			bag_delete(codedropBag);
			bag_delete(CDBag);
			return 1;
		      }
		      cd->longitude = Long;

		      char *neutralizingTeam = strtok(NULL, ", ");
		      if (neutralizingTeam == NULL) {
			if (logSwitch == 1)
			  printTime(fp);
			fprintf(fp, "Received an invalid message and ignored it.\n");
			bag_delete(FABag);
			bag_delete(codedropBag);
			bag_delete(CDBag);
			return 1;
		      }
		      cd->teamname = neutralizingTeam;

		      bag_insert(CDBag, cd);
		    }

		    bag_delete(codedropBag);
		    // gameID is NULL if we have not yet recieved a message from the
		    // game server
		    if (*gameID == NULL) {
		      // gameID takes on the value of the gameID field of the first
		      // message it gets from the game server
		      *gameID = malloc(strlen(ID) + 1);
		      strcpy(*gameID, ID);
		      fprintf(fp, "Guide agent received first message from game server. The ID of this game is %s\n", *gameID);
		      printf("Joined a game!\n");
		    }
		    else if (strcmp(ID, *gameID) != 0) {
		      // the message we just received contains a different gameID
		      fprintf(fp, "Guide agent received a message from a different game and ignored it. The \
ID of this game was %s\n", ID);
		      return 1;
		    }

		    printf("%s:\n", OPCODE);
		    printf("Game ID is %s\n", ID);
		    printf("Agents:\n");

		    FA_t *myFA;

		    while ((myFA = bag_extract(FABag)) != NULL) {
		      
		      printf("   Agent %s from team %s: pebble ID is %s.\n", myFA->name, myFA->teamname, myFA->pebbleid);
		      if (myFA->capture == true)
			printf("     %s has been captured and is no longer active.\n", myFA->name);
		      else
			printf("     %s has not been captured and is still active.\n", myFA->name);

		      printf("     Last known latitude: %s. Last known longitude is :%s\n", myFA->latitude, myFA->longitude);
		      printf("     Seconds since last contact is %s.\n", myFA->contact);

		      //		      free(myFA);
		    }
		    bag_delete(FABag);
		      
		    printf("Code drops:\n");

		    codedrop_t *cd;

		    while((cd = bag_extract(CDBag)) != NULL) {
		      printf("   Code drop %s: latitude is %s, longitude is %s. ", cd->hexcode, cd->latitude, cd->longitude);
		      if (strcmp(cd->teamname, "NONE") == 0)
			printf("Has not yet been neutralized.\n");
		      else
			printf("Neutralized by %s.\n", cd->teamname);
		      fflush(stdout);
		      free(cd);
		    }
		    bag_delete(CDBag);
		  }
		}  		    
	      }		
	    } 
	  }
	}    
	// else if OPCODE is GAME_OVER
	else if (strcmp(OPCODE, "GAME_OVER") == 0) {
	  
	  if (*gameID == NULL) {
	    fprintf(fp,"Guide agent received game over message before joining a game. Message was therefore ignored.\n");
	    //ignore; we haven't even goined a game yet
	  }
	  else if (strcmp(ID, *gameID) != 0) {
	    // if gameID doesn't match,
	    fprintf(fp, "Guide agent received a message from a different game and ignored it. The ID of this game was %s\n", ID);
	  }
	  else {
	    if(printGameOver(ID, fp, logSwitch) == 0)
	      return 0;
	  }
	}
	else if (strcmp(OPCODE, "MI_ERROR_INVALID_OPCODE") == 0) {
	  if (logSwitch == 1) {
	    printTime(fp);
	    fprintf(fp, "MI_ERROR_INVALID_OPCODE: ");
	  }
	  fprintf(fp, "Guide agent sent an invalid OPCODE to the game server.\n");
	}
      else if (strcmp(OPCODE, "MI_ERROR_INVALID_GAME_ID") == 0) {
	if (logSwitch == 1) {
	  printTime(fp);
	  fprintf(fp, "MI_ERROR_INVALID_GAME_ID: ");
	}
	fprintf(fp, "Guide agent sent an invalid game ID to the game server.\n");
      }
      else if (strcmp(OPCODE, "MI_ERROR_INVALID_TEAMNAME") == 0) {
	if (logSwitch == 1) {
	  printTime(fp);
	  fprintf(fp, "MI_ERROR_INVALID_TEAMNAME: ");
	}
	fprintf(fp, "Guide agent sent an invalid team name to the game server.\n");
      }
      else if (strcmp(OPCODE, "MI_ERROR_INVALID_ID") == 0) {
	if (logSwitch == 1) {
	  printTime(fp);
	  fprintf(fp, "MI_ERROR_INVALID_ID: ");
	}
	fprintf(fp, "Guide agent sent an invalid ID to the game server.\n");
      }
      else {
	if (logSwitch == 1) {
	  printTime(fp);
	  fprintf(fp, "%s\n", buf);
	}
	fprintf(fp, "Guide agent received a message with an unrecognized OPCODE from the game server\n");
      }	
      }
    }
  }
  return 1;
}






/************************** handleStdin ***************************/

char *handleStdin(int comm_sock, struct sockaddr_in *gsp, list_t *list,
		  char *teamName)
{  
  char *hint = readline(stdin);
  
  if (hint == NULL) {
    return "EOF";
  }

  int lineLength = (int) strlen(hint);
  
  if (lineLength > 140) {
    printf("Hint must be 140 characters or less!");
    fflush(stdout);
    return hint;
  }

  if (gsp->sin_family != AF_INET) {
    printf("I am confused: server is not AF_INET.\n");
    fflush(stdout);
    return "0";
  }


  int num = 0;
  list_iterate(list, getNumAgents, &num, NULL);
  //  int *num = list_find(numAgents, teamName);
  
  fflush(stdout);

  if (num == 0) {
    printf("You have no active agents to send hints to!\n");
    free(hint);
    fflush(stdout);
    return "NULL";
  }
  
  printf("Send hint to which field agent? Your active agents have the following ID's:\n");
  printf("You may also enter '*' to send to all agents on your team.\n");

  list_iterate(list, printPebbleIDs, teamName, NULL);
  printf("\n");
  
  char *pebble = readline(stdin);
  if (pebble == NULL)
    return "EOF";

  char* idAndHint = malloc(strlen(pebble) + strlen(hint) + 2);
  sprintf(idAndHint, "%s|%s", pebble, hint);
  
  fflush(stdout);
  free(pebble);
  free(hint);
  
  return idAndHint;
}



int printGameOver(char *gameID, FILE *fp, int logSwitch) {
  char *numRemainingCodeDrops = strtok(NULL, "|");
  if (numRemainingCodeDrops == NULL) {
    if (logSwitch == 1) 
      printTime(fp);
    fprintf(fp, "Guide Agent received a poorly-formatted message and ignored it.\n");
    return 1;
  }
  char *teamRecords = strtok(NULL, "|");
  if (teamRecords == NULL) {
    if (logSwitch == 1)
      printTime(fp);
    fprintf(fp, "Guide Agent received a poorly-formatted message and ignored it.\n");
    return 1;
  }
  
  bag_t *recordBag = bag_new();
  char *record = strtok(teamRecords, ":");
  if (record == NULL) {
    if (logSwitch == 1)
      printTime(fp);
    fprintf(fp, "Guide Agent received a poorly-formatted message and ignored it.\n");
    bag_delete(recordBag);
    return 1;
  }
  
  while (record != NULL) {
    bag_insert(recordBag, record);
    record = strtok(NULL, ":");
  }
  
  printf("GAME OVER! Here are the records for this game:\n");
  printf("Number of remaining unneutralized code drops is %s\n", numRemainingCodeDrops);

  char *recordPrint = bag_extract(recordBag);
  bag_t *recBag = bag_new();
  
  while (recordPrint != NULL) {

    recordStruct_t *myRec = malloc(sizeof(recordStruct_t));
    
    char *teamName = strtok(recordPrint, ", ");
    if (teamName == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Guide Agent received a poorly-formatted message and ignored it.\n");
      bag_delete(recordBag);
      bag_delete(recBag);
      return 1;
    }
    myRec->teamName = teamName;
    
    char *numPlayers = strtok(NULL, ", ");
    if (numPlayers == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Guide Agent received a poorly-formatted message and ignored it.\n");
      bag_delete(recordBag);
      bag_delete(recBag);
      return 1;
    }
    myRec->numPlayers = numPlayers;
    
    char *numCaptures = strtok(NULL, ", ");
    if (numCaptures == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Guide Agent received a poorly-formatted message and ignored it.\n");
      bag_delete(recordBag);
      bag_delete(recBag);
      return 1;
    }
    myRec->numCaptures = numCaptures;

    
    char *numCaptured = strtok(NULL, ", ");
    if (numCaptured == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Guide Agent received a poorly-formatted message and ignored it.\n");
      bag_delete(recordBag);
      bag_delete(recBag);
      return 1;
    }
    myRec->numCaptured = numCaptured;
    
    char *numNeutralized = strtok(NULL, ", ");
    if (numNeutralized == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Guide Agent received a poorly-formatted message and ignored it.\n");
      bag_delete(recordBag);
      bag_delete(recBag);
      return 1;
    }
    myRec->numNeutralized = numNeutralized;
    
    recordPrint = bag_extract(recordBag);
    bag_insert(recBag, myRec);
  }

  recordStruct_t *myRec;
  while ((myRec = bag_extract(recBag)) != NULL) {
    printf("Team %s:\n", myRec->teamName);
    printf("   Total number of active players: %s\n", myRec->numPlayers);
    printf("   Captured %s players from other teams.\n", myRec->numCaptures);
    printf("   %s players on this team were captured.\n", myRec->numCaptured);
    printf("   Neutralized %s code drops.\n", myRec->numNeutralized);
    free(myRec);
  }
  
  bag_delete(recordBag);
  bag_delete(recBag);
  printf("Good game, everybody!");
  fflush(stdout);
  return 0;
}





int sendStatusRequest(char *gameID, char *guideID, char *teamName,
		  char *playerName, int comm_sock, const struct sockaddr *gsp)
{
  char *request = malloc(16 + strlen(gameID) + strlen(guideID) + strlen(teamName) + strlen(playerName));

  if (request == NULL) {
    printf("Error\n");
    return 3;
  }

  sprintf(request, "GA_STATUS|%s|%s|%s|%s|1", gameID, guideID, teamName,
	  playerName);

  if (sendto(comm_sock, request, strlen(request), 0, (struct sockaddr *) gsp,
	     sizeof(*gsp)) < 0){
    perror("error sending datagram\n");
    free(request);
    return 5;
  }

  free(request);
  return 0;
}





void printPebbleIDs(void *arg, char *key, void *data, void *option)
{
  FA_t *myAgent = data;
  char* teamName = arg;

  if (myAgent != NULL) {
    if (myAgent->capture == false && strcmp(myAgent->teamname, teamName) == 0){
      printf("%s ", key);
      fflush(stdout);
    }
  }
}




void getNumAgents(void *arg, char *key, void *data, void *option)
{
  fflush(stdout);
  int *numAgents = arg;
  
  FA_t *currentAgent = data;
  
  if (currentAgent->capture == false) {
    (*numAgents)++;
    fflush(stdout);
  }
}



void deletefunc(void *data)
{
  if (data)
    free(data);
}


void printTime(FILE *fp)
{
  time_t myTime = time(NULL);
  char* timeString = ctime(&myTime);
  fprintf(fp, "%s ", timeString);
}



void deleteNodes(void *arg, char *key, void *data, void *option) {
  char **keyp = &key;
  *keyp = "NONE";
}
