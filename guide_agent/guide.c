/*
 * guide.c - One of three parts for the game "Mission Imcomputable"
 * Works with a Game Server and Field Application to play the game.
 * Receives status updates from the Game Server and allows the person 
 * playing as the Guide Agent to send hints to Field Agents through the
 * Game Server. 
 *
 * Usage: 
 *    ./guide [-v] teamName playerName GShost GSport
 * 
 * Upon receipt of valid command line parameters, Guide Agent sets up a socket.
 * It joins a game by sending a message to the Game Server, and then it listens
 * for input from stdin and the socket. Input from the socket includes game
 * updates from the Game Server and "Game Over" notifications from the Game 
 * Sever. Input from stdin includes hints that the Guide Agent enters to send
 * to one of its Field Agents. It sends these hints via formatted messages to
 * the Game Server.
 *
 * Team Lapis (Drew Waterman, Deven Orie, Shirley Zhang), May 2016
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

/* Struct used to hold information about code drops
 * in messages sent by the Game Server */
typedef struct codedrop {
  char *hexcode;   // the ID of each code
  char *latitude;
  char *longitude;
  char *teamname;  // name of the team that neutralized the code drop; "NONE",
                   // if it has not yet been neutralized
} codedrop_t;


typedef struct FA {
  char *teamname;  // name of the field agent's team
  bool capture;    // true if the agent has been captured, false if not
  char *pebbleid;  
  char *name;      // name of the field agent
  char *latitude;  // field agent's current latitude
  char *longitude; // field agent's current longitude
  char *contact;   // seconds since last contact with the game server
} FA_t;


/* Struct used to hold "records" for each team when printing the end game
 * statistics upon the receipt of a GAME_OVER message from the game server */
typedef struct recordStruct {
  char *teamName;       // name of the team whose record we are printing
  char *numPlayers;     // total number of player ever on team
  char *numCaptures;    // total number of other players captured by this team
  char *numCaptured;    // number of players on this team who were captured
  char *numNeutralized; // number of code drops neutralized by this team
} recordStruct_t;



static int createSocket(int argc, char *argv[], struct sockaddr_in *gsp,
			int logSwitch, int n);
char *getGuideID(void);
void sendFirstMessage(char *guideID, char *teamName, char *playerName,
		      int comm_sock, struct sockaddr_in gs);
static int handleSocket(int comm_sock, struct sockaddr_in *gsp, list_t *list,
			char **gameID, FILE *fp, int logSwitch);
char *handleStdin(int comm_sock, struct sockaddr_in *gsp, list_t *list,
		  char *teamName);
bag_t *getAgents(bag_t *agentBag, list_t *list, FILE *fp, int logSwitch);
bag_t *getCodedrops(bag_t *codedropBag, FILE *fp, int logSwitch);
int printGameOver(char *gameID, FILE *fp, int logSwitch);
int sendStatusRequest(char *gameID, char *guideID, char *teamName,
		 char *playerName, int comm_sock, const struct sockaddr *gsp);
void printPebbleIDs(void *arg, char *key, void *data, void *option);
void getNumAgents(void *arg, char *key, void *data, void *option);
bool isNumber(const char *arg);
void deletefunc(void *data);
void printTime(FILE *fp);
void deleteNodes(void *arg, char *key, void *data, void *option);
void freeBag(bag_t *bag);


// size of buffer when reading from stdin or socket
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
    }
    else if (select_response > 0) {
      // some data is ready on either source, or both
      if (FD_ISSET(0, &rfds)) {

	// get the pebble ID and hint input into stdin by the user
	char *idAndHint = handleStdin(comm_sock, &gs, FAList, teamName);

	// don't send a hint if handleStdin returned NULL; that means that
	// there are no active agents on the guide's team to send hints to
	if (strcmp(idAndHint, "NULL") != 0 && strcmp(idAndHint, "140") != 0) {
	  
	  // If there were any problems getting the ID and hint, or if the user
	  // chose to exit the program using ^D, break fromt the while loop 
	  if (strcmp(idAndHint, "EOF") == 0)
	    break;

	  if (strcmp(idAndHint, "blank") == 0)
	    printf("\n");

	  // If handleStdin returned "0", that means that the AF_INET did
	  // not match
	  else if (strcmp(idAndHint, "0") == 0)
	    printf("Could not send the message to the Game Server.\n");

	  // Check to make sure we've joined a game that we can send hints to
	  else if ((gameID != NULL)) {

	    // Malloc space for a hint to send
	    char *sendHint = malloc(13 + strlen(gameID) + strlen(guideID) +
				    strlen(teamName) + strlen(playerName) +
				    strlen(idAndHint));

	    // Make sure that malloc did not return NULL
	    if (sendHint == NULL) {
	      printf("Error allocating memory for message.\n");
	      exit(3);
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
	    free(idAndHint);
	  }
	  else {
	    // gameID is NULL, which means we haven't joined a game yet
	    printf("You can't send hints until you've joined the game!\n");
	    fflush(stdout);
	    free(idAndHint);
	  }
	}
      }
	  
      if (FD_ISSET(comm_sock, &rfds)) {

	// handle the datagram coming in from the socket
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
  // free all memory we allocated before exiting
  list_delete(FAList);
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
 * Returns integer returned by socket() function
 */
static int createSocket(int argc, char *argv[],
			struct sockaddr_in *gsp, int logSwitch, int n)
{
  const char* GShost;     // server host name
  int GSport;             // port number

  GShost = argv[n + 3];

  if (isNumber(argv[n + 4]))
    GSport = atoi(argv[n + 4]);
  else {
    printf("Port number can only consist of integers.\n");
    exit(1);
  }		  

  // Look up the hostname specified on command line
  struct hostent *hostp = gethostbyname(GShost);
  if (hostp == NULL) {
    printf("Error: unknown host %s\n", GShost);
    exit(2);
  }

  // Initialize fields of the server address
  gsp->sin_family = AF_INET;
  bcopy(hostp->h_addr_list[0], &gsp->sin_addr, hostp->h_length);
  gsp->sin_port = htons(GSport);

  // Create socket
  int comm_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (comm_sock < 0) {
    printf("Error opening datagram socket\n");
    exit(4);
  }

  return comm_sock;
}




/********************* getGuideID ************************/

/* Repeatedly prompts user for 8-character hexadecimal ID until they 
 * enter a valid one. Returns their first valid entry.
 */
char *getGuideID(void)
{
  int goodID = 0;  // 0 if ID is valid, 1 if it is not valid
  char *guideID;

  // repeatedly prompt the user until they enter a valid ID
  while (goodID == 0) {
    goodID = 1;
    
    printf("What is the guide's ID?: ");

    // read the line that the user enters their preferred ID on
    guideID = readline(stdin);

    if (guideID == NULL)
      return "EOF";  // user decided to quit before entering their ID

    if(strcmp(guideID, "") == 0)
      goodID = 0;   // user pressed enter without entering any characters
    
    if (strlen(guideID) != 8) 
      goodID = 0;
  
    else {
      // check each character to make sure it's a hexadecimal digit
      for (int i = 0; i < strlen(guideID); i++) {
	if (isxdigit(guideID[i]) == 0) {
	  goodID = 0;
	}
      }
    }
    if (goodID == 0) {
      printf("guide ID must be 8 hexadecimal characters\n");
      free(guideID);
    }
  }
  return guideID;
}



/******************** sendFirstMessage *********************/

/* Send first message to the game server to announce to the 
 * game server that the guide agent wants to join the game.
 * The message will also request a status update from the server.
 */
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
    exit(5);
  }

  printf("Sent first message to the game server to join the game.\n");
  free(message);
}




/********************** handleSocket *********************/
/* Parese incoming messages from the game server and prints out appropraite
 * information about the game. If the messages received from the game server
 * were in an invalid format, the function returns before printing anything.
 * Returns 0 upon receipt of a valid GAME_OVER message from the server
 */
static int
handleSocket(int comm_sock, struct sockaddr_in *gsp, list_t *list,
	     char **gameID, FILE *fp, int logSwitch)
{
  // socket has input ready
  struct sockaddr_in sender;            // sender of this message
  struct sockaddr *senderp = (struct sockaddr *) &sender;
  socklen_t senderlen = sizeof(sender); // must pass address to length
  char buf[BUFSIZE];          // buffer for reading data from socket
  int nbytes = recvfrom(comm_sock, buf, BUFSIZE-1, 0, senderp, &senderlen);

  if (nbytes < 0) {
    perror("receiving from socket");
    exit(1);
  }
  else {
    buf[nbytes] = '\0';     // null terminate string

    // where was it from?
    if (sender.sin_family != AF_INET)
      printf("From non-Internet address: Family %d\n", sender.sin_family);
    else {
      // was it from the expected server?
      if (sender.sin_addr.s_addr == gsp->sin_addr.s_addr &&
	  sender.sin_port == gsp->sin_port) {

	// Log the message 
	if (logSwitch == 1) {
	  printTime(fp);
	  fprintf(fp, "Received the following message from the ");
	  fprintf(fp, "game server: %s\n", buf);
	}
	else
	  fprintf(fp, "Received a message from the game server.\n");

	// Get the OPCODE and game ID from the message from the server
	char *OPCODE = strtok(buf, "|");
	char *ID = strtok(NULL, "|");

	// Check to make sure the format of the message is valid
	if (OPCODE == NULL) {
	  // If it's not valid, log it and return, ignoring the message
	  if (logSwitch == 1)
	    printTime(fp);
	  fprintf(fp, "Received an invalid message and ignored it.\n");
	  return 1;
	}
	// Check to make sure ID is there
	if (ID == NULL) {
	  // Log it and return if it's not
	  if (logSwitch == 1)
	    printTime(fp);
	  fprintf(fp, "Received an invalid message and ignored it.\n");
	  return 1;
	}
	
	// Check to make sure ID is 8 characters or fewer 
	if (strlen(ID) > 8) {
	  if (logSwitch == 1)
	    printTime(fp);
	  fprintf(fp, "Received an invalid message and ignored it.\n");
	  return 1;
	}

	// Check to make sure the ID consists only of hexadecimal digits
	for (int i = 0; i < strlen(ID); i++) {
	  if (isxdigit(ID[i]) == 0) {
	    if (logSwitch == 1)
	      printTime(fp);
	    fprintf(fp, "Received an invalid message and ignored it.\n");
	    return 1;
	  }
	}
	
	/***** receipt of GAME_STATUS message *****/
	
	if (strcmp(OPCODE, "GAME_STATUS") == 0) {

	  // field containing info about all the field agents in the game
	  char *AllAgents = strtok(NULL, "|");
	  if (AllAgents == NULL) {
	    if (logSwitch == 1)
	      printTime(fp);
	    fprintf(fp, "Received an invalid message and ignored it.\n");
	    return 1;
	  }
	  
	  // Field containing info about all the code drops in the game
	  char *AllCodedrops = strtok(NULL, "|");	  
	  if (AllCodedrops == NULL) {
	    if (logSwitch == 1)
	      printTime(fp);
	    fprintf(fp, "Received an invalid message and ignored it.\n");
	    return 1;
	  }
	  
	  // Bag holds strings of each agent field for convenience so
	  // we can strtok them later
	  bag_t *agentBag = bag_new();

	  // Agent represents the comma-separated subfield holding info
	  // about individual agents
	  char *agent = strtok(AllAgents, ":");
	  if (agent == NULL) {
	    if (logSwitch == 1) {
	      printTime(fp);
	      fprintf(fp, "%s\n", buf);
	    }
	    fprintf(fp, "Received an invalid message and ignored it.\n");
	    return 1;
	  }
	  // Insert agent into the bag so we can extract it and strtok its
	  // contents later
	  bag_insert(agentBag, agent);

	  // Strtok all the agent fields and put each in the bag
	  while ((agent = strtok(NULL, ":")) != NULL) {
	    bag_insert(agentBag, agent);
	  }

	  // If the all of the agent fields are valid, get a bag of
	  // field agent structs (see getAgents)
	  bag_t *FABag = getAgents(agentBag, list, fp, logSwitch);
	  if (FABag == NULL)
	    return 1;

	  // code drop struct to store the strtok'd code drop info in
	  //codedrop_t *cd;

	  // Bag to hold strtok'd code drop strings
	  bag_t *codedropBag = bag_new();

	  // strtok the first code drop field and check if it's NULL
	  char *codedrop = strtok(AllCodedrops, ":");
	  if (codedrop == NULL) {
	    if (logSwitch == 1)
	      printTime(fp);
	    fprintf(fp, "Received an invalid message and ignored it.\n");
	    return 1;	    
	  }
	  bag_insert(codedropBag, codedrop);

	  // Strtok all of the colon-separated codedrop fields and
	  // put the string in the codedropBag  
	  while ((codedrop = strtok(NULL, ":")) != NULL) {
	    bag_insert(codedropBag, codedrop);
	  }

	  // Call getCodedrops to create a bag of codedrop structs
	  bag_t *CDBag = getCodedrops(codedropBag, fp, logSwitch);

	  // If the pointer is NULL, that means there was a problem parsing
	  // somewhere along the line during the getCodedrops function
	  if (CDBag == NULL) {	    
	    freeBag(FABag);
	    bag_delete(FABag);
	    return 1;
	  }
	  
	  // gameID is NULL if we have not yet recieved a message from the
	  // game server
	  if (*gameID == NULL) {
	    // gameID takes on the value of the gameID field of the first
	    // valid message it gets from the game server
	    *gameID = malloc(strlen(ID) + 1);
	    strcpy(*gameID, ID);
	    if (logSwitch == 1)
	      printTime(fp);
	    fprintf(fp, "Guide agent received first message from game ");
	    fprintf(fp, "server. The ID of this game is %s\n", *gameID);
	    printf("Joined a game!\n");
	  }
	  else if (strcmp(ID, *gameID) != 0) {
	    // the message we just received contains a different gameID
	    if (logSwitch == 1)
	      printTime(fp);
	    fprintf(fp, "Guide agent received a message from a different ");
	    fprintf(fp, "game and ignored it. ");
	    fprintf(fp, "The ID of this game was %s\n", ID);
	    return 1;
	  }

	  /***** Message has been validated; time to start printing *****/
	  printf("%s:\n", OPCODE);
	  printf("Game ID is %s\n", ID);
	  printf("Agents:\n");

	  FA_t *myFA;

	  // Extract all the FA structs from the FABag, and print the
	  // information we stored in them
	  while ((myFA = bag_extract(FABag)) != NULL) {
		      
	    printf("   Agent %s from team %s: pebble ID is %s.\n",
		   myFA->name, myFA->teamname, myFA->pebbleid);
	    
	    if (myFA->capture == true)
	      printf("     %s has been captured and is no longer active.\n",
		     myFA->name);
	    else
	      printf("     %s has not been captured and is still active.\n",
		     myFA->name);

	    printf("     Last known latitude: %s. ", myFA->latitude);
	    printf("Last known longitude is :%s\n", myFA->longitude);
	    printf("     Seconds since last contact is %s.\n", myFA->contact);

	    // Insert the FA structs into the list for later access when
	    // printing the pebbleID's for the Guide Agent to choose from
	    list_insert(list, myFA->pebbleid, myFA);
	  }
	  bag_delete(FABag);
		      
	  printf("Code drops:\n");

	  codedrop_t *cd;

	  // Extract all the codedrop structs from the CDBag, and print the
	  // information we stored in them
	  while((cd = bag_extract(CDBag)) != NULL) {
	    
	    printf("   Code drop %s: ", cd->hexcode);
	    printf("latitude is %s, ", cd->latitude);
	    printf("longitude is %s. ", cd->longitude);
	    if (strcmp(cd->teamname, "NONE") == 0)
	      printf("Has not yet been neutralized.\n");
	    else
	      printf("Neutralized by %s.\n", cd->teamname);
	    fflush(stdout);
	    free(cd);
	  }
	  bag_delete(CDBag);
   	}    
	
	/***** OPCODE is GAME_OVER *****/
	
	else if (strcmp(OPCODE, "GAME_OVER") == 0) {
	  
	  if (*gameID == NULL) {
	    // We should ignore this message since the Guide Agent has not
	    // joined a game yet.
	    if (logSwitch == 1) 
	      printTime(fp);
	    fprintf(fp,"Guide agent received game over message before ");
	    fprintf(fp, "joining a game. Message was therefore ignored.\n");
	  }
	  else if (strcmp(ID, *gameID) != 0) {
	    // We recieved a message from a different game. Ignore it.
	    if (logSwitch == 1) 
	      printTime(fp);
	    fprintf(fp, "Guide agent received a message from a different ");
	    fprintf(fp, "game and ignored it. ");
	    fprintf(fp, "The ID of this game was %s\n", ID);
	  }
	  else {
	    // Print the statistics for the game. If it returns 0, the message
	    // was correctly formatted. Return 0 so we can exit the program.
	    if(printGameOver(ID, fp, logSwitch) == 0)
	      return 0;
	  }
	}
	else if (strcmp(OPCODE, "GS_RESPONSE") == 0) {
	  if (gameID == NULL) {
	    // Ignore this message because the guide agent has not yet joined
	    // the game, but log what happened.
	    if (logSwitch == 1) 
	      printTime(fp);
	    fprintf(fp, "Guide agent received a GS_RESPONSE message before ");
	    fprintf(fp, "joining the game and ignored it.\n");
	  }
	  else if (strcmp(ID, *gameID) != 0) {
	    // Message is from a different game; ignore it
	    if (logSwitch == 1)
	      printTime(fp);
	    fprintf(fp, "Guide agent received a message from a game with a ");
	    fprintf(fp, "different game ID and ignored it.\n");
	  }

	  // respCode is the field in the message after the game ID that tells
	  // the Guide Agent which error they made.
	  char *respCode = strtok(NULL, "|");
	  if (respCode == NULL) {
	    if (logSwitch == 1) {
	      printTime(fp);
	      fprintf(fp, "%s\n", buf);
	    }
	    fprintf(fp, "Guide agent received an ivalid message from the ");
	    fprintf(fp, "game server and ignored it.\n");
	  }

	  /*** Log the different MI_ERROR codes ***/
	  
	  if (strcmp(respCode, "MI_ERROR_INVALID_OPCODE") == 0) {
	    if (logSwitch == 1) {
	      printTime(fp);
	      fprintf(fp, "%s\n", buf);
	    }
	    fprintf(fp, "MI_ERROR_INVALID_OPCODE: ");	  
	    fprintf(fp, "Guide agent sent an invalid OPCODE ");
	    fprintf(fp, "to the game server.\n");
	  }
	  else if (strcmp(respCode, "MI_ERROR_INVALID_GAME_ID") == 0) {
	    if (logSwitch == 1) {
	      printTime(fp);
	      fprintf(fp, "MI_ERROR_INVALID_GAME_ID: ");
	    }
	    fprintf(fp, "Guide agent sent an invalid game ID to the game ");
	    fprintf(fp, "server.\n");
	  }
	  else if (strcmp(respCode, "MI_ERROR_INVALID_TEAMNAME") == 0) {
	    if (logSwitch == 1) {
	      printTime(fp);
	      fprintf(fp, "MI_ERROR_INVALID_TEAMNAME: ");
	    }
	    fprintf(fp, "Guide agent sent an invalid team name to the game ");
	    fprintf(fp, "server.\n");
	  }
	  else if (strcmp(respCode, "MI_ERROR_INVALID_ID") == 0) {
	    if (logSwitch == 1) {
	      printTime(fp);
	      fprintf(fp, "MI_ERROR_INVALID_ID: ");
	    }
	    fprintf(fp, "Guide agent sent an invalid ID to the game server.");
	    fprintf(fp, "\n");
	  }
	  else {
	    if (logSwitch == 1) {
	      printTime(fp);
	      fprintf(fp, "%s\n", buf);
	    }
	    fprintf(fp, "Guide agent received a message with an ");
	    fprintf(fp, "unrecognized OPCODE from the game server.\n");
	  }
	}
      }
    }
  }
  return 1;  // return 1 because we did not receive a valid GAME_OVER message
}




/************************** handleStdin ***************************/
/* Reads input from standard in. If the user entered command + D, it returns
 * "EOF" to tell the program to exit the game. 
 * Returns "0" if there was a problem connecting to the server
 * If the user typed a message that was 140 characters or less, handleStdin
 * then prints a list of pebble ID's correspodint to the Field Agents on the
 * Guide Agent's team and pprompts the user for a pebble ID to send the 
 * message to. It reads that line, and if there were no erros, it returns a 
 * string, "idAndHint" that has the format "pebbleID|hint"
 * Returns the string "NULL" if the Guide Agent has no active agents to send
 * hints to. 
 * Returns the string "blank" if the user entered a blank line
 */
char *handleStdin(int comm_sock, struct sockaddr_in *gsp, list_t *list,
		  char *teamName)
{
  // Get the message that the user typed in
  char *hint = readline(stdin);

  // readline returns NULL if user entered command + D
  // return "EOF" to signal that the user wants to quit
  if (hint == NULL) {
    return "EOF";
  }

  // User entered a blank line
  if (strcmp(hint, "") == 0) {
    free(hint);
    return "blank";
  }

  int lineLength = (int) strlen(hint);

  // Let the user know if they entered a hint that was too long
  if (lineLength > 140) {
    printf("Hint must be 140 characters or less!\n");
    printf("Enter a hint to send to the game server.\n");
    fflush(stdout);
    free(hint);
    return "140";
  }

  if (gsp->sin_family != AF_INET) {
    printf("I am confused: server is not AF_INET.\n");
    fflush(stdout);
    free(hint);
    return "0";
  }

  // num keeps track of the number of active agents on the guide's team
  int num = 0;
  // iterate through our list of total field agents to find num
  list_iterate(list, getNumAgents, &num, teamName);
  
  fflush(stdout);

  // If num is 0, there are no active Field Agents on the guide's team
  // Let them know that they don't have anyone to send hints to, and
  // dont' send a GA_HINT message to the Game Server
  if (num == 0) {
    printf("You have no active agents to send hints to!\n");
    free(hint);
    fflush(stdout);
    return "NULL";
  }

  // Print prompts for the user to enter the pebble ID of one of their agents
  printf("Send hint to which field agent? ");
  printf("Your active agents have the following ID's:\n");
  printf("You may also enter '*' to send to all agents on your team.\n");

  // Print a list of the pebble ID's of active agents on the guide's team
  list_iterate(list, printPebbleIDs, teamName, NULL);
  printf("\n");

  // Read the line that the user enters
  char *pebble = readline(stdin);
  // If user entered command + D, return
  if (pebble == NULL) {
    free(hint);
    return "EOF";  
  }

  // If the user entered a blank line, ignore it.
  while (strcmp(pebble, "") == 0) {
    free(pebble);
    pebble = readline(stdin);
    if (pebble == NULL) {
      free(hint);
      return "EOF";
    }
  }

  // If we have not yet returned, we have a valid message and pebble ID to send
  // to the game server. Concatenate a string with the format "pebbleID|hint"
  char* idAndHint = malloc(strlen(pebble) + strlen(hint) + 2);
  sprintf(idAndHint, "%s|%s", pebble, hint);
  
  fflush(stdout);
  free(pebble);
  free(hint);

  return idAndHint;
}


/*********************** getAgents() **********************/
/* Takes a bag of strings pertaining to the field containing agent info in the 
 * GAME_STATUS message. Parses the string, and if valid, assigns the relevant
 * information to members of an FA struct and puts the FA in another bag. If
 * the message was parsed and found to be valid, the function returns the bag
 * full of FA structs. Otherwise, it frees everything in the bag and returns
 * NULL.
 */
bag_t *
getAgents(bag_t *agentBag, list_t *list, FILE *fp, int logSwitch)
{
  list_iterate(list, deleteNodes, NULL, NULL);

  // FABag holds field agent structs, which we make using the info
  // in each "agent" string from the agentBag
  bag_t *FABag = bag_new();
  
  char *agent; // keeps track of each colon-separated agent subfield
               // held inside the agentBag

  /* Extract each colon-separated agent subfield from the agentBag 
   * and parse it, assigning its comma-separated subfields to
   * FA members */
  while ((agent = bag_extract(agentBag)) != NULL) {

    FA_t *FA = malloc(sizeof(struct FA));

    // Check to make sure there were no memory allocation problems
    if (FA == NULL) {
      printf("Error allocating memory.\n");
      bag_delete(agentBag);
      bag_delete(FABag);
      return NULL;
    }

    // srtok the next field to get the pebbleID. If there were any problems,
    // free memory associated with the bags and FA structs
    char *pebbleID = strtok(agent, ", ");
    if (pebbleID == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Received an invalid message and ignored it.\n");
      free(FA);
      bag_delete(agentBag);
      freeBag(FABag);
      bag_delete(FABag);
      return NULL;
    }
    // If there were no problems, assign the correlated FA field
    // to the value we just strtok'd
    FA->pebbleid = pebbleID;
    

    // Repeat the above process for the rest of the comma-separated subfields
    
    char *teamName = strtok(NULL, ", ");
    if (teamName == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Received an invalid message and ignored it.\n");
      free(FA);
      bag_delete(agentBag);
      freeBag(FABag);
      bag_delete(FABag);
      return NULL;
    }
    FA->teamname = teamName;

    char *playerName = strtok(NULL, ", ");
    if (playerName == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Received an invalid message and ignored it.\n");
      free(FA);
      bag_delete(agentBag);
      freeBag(FABag);
      bag_delete(FABag);
      return NULL;
    }
    FA->name = playerName;

    char *playerStatus = strtok(NULL, ", ");
    if (playerStatus == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Received an invalid message and ignored it.\n");
      free(FA);
      bag_delete(agentBag);
      freeBag(FABag);
      bag_delete(FABag);
      return NULL;
    }
    if (strcmp(playerStatus, "captured") == 0)
      FA->capture = true;
    else if (strcmp(playerStatus, "active") == 0)
      FA->capture = false;
    else {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Received an invalid message and ignored it.\n");
      free(FA);
      bag_delete(agentBag);
      freeBag(FABag);
      bag_delete(FABag);
      return NULL;
    }

    char *lastKnownLat = strtok(NULL, ", ");
    if (lastKnownLat == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Received an invalid message and ignored it.\n");
      free(FA);
      bag_delete(agentBag);
      freeBag(FABag);
      bag_delete(FABag);
      return NULL;
    }

    FA->latitude = lastKnownLat;

    char *lastKnownLong = strtok(NULL, ", ");
    if (lastKnownLong == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Received an invalid message and ignored it.\n");
      free(FA);
      bag_delete(agentBag);
      freeBag(FABag);
      bag_delete(FABag);
      return NULL;
    }
    FA->longitude = lastKnownLong;

    char *secondsSinceLastContact = strtok(NULL, ", ");
    if (secondsSinceLastContact == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Received an invalid message and ignored it.\n");
      free(FA);
      bag_delete(agentBag);
      freeBag(FABag);
      bag_delete(FABag);
      return NULL;
    }
    FA->contact = secondsSinceLastContact;

    // If the fields for all of the agent information were valid, we finally
    // insert the FA struct that we made into the FABag
    bag_insert(FABag, FA);
  }
  // Delete the old bag since it's empty
  bag_delete(agentBag);
  // Return the bag full of FA structs
  return(FABag);
}


/*********************** getCodedrops() **********************/
/* Takes a bag of strings of colon-separated code drop fields from the
 * GAME_STATUS message from the Game Server. It parses and validate the 
 * message, returning a NULL pointer if the message was invalid in any
 * way. If the message was valid, it returns a bag of codedrop structs,
 * with members whose values correspond to the information sent in the 
 * GAME_STATUS message.
 */
bag_t *
getCodedrops(bag_t *codedropBag, FILE *fp, int logSwitch)
{
  bag_t *CDBag = bag_new();  // bag to hold codedrop structs
  char *codedrop;            // the codedrop field we are parsing
  codedrop_t *cd;            // codedrop struct whose members we are defining

  /* Extract the codedrop strings out of the codedropBag until there are none
   * left, or until we find and error and return NULL. Parse each string
   * and assign its value the corresponding member of the codedrop struct.
   */
  while ((codedrop = bag_extract(codedropBag)) != NULL) {

    // Malloc space for cd, and check to make sure there were no errors
    cd = malloc(sizeof(codedrop_t));
    if (cd == NULL) {
      printf("Error locating memory.\n");
      bag_delete(codedropBag);
      bag_delete(CDBag);
      return NULL;
    }

    // strtok the first comma-separated subfield, and check to make
    // sure that strtok does not return NULL. If it does, retrun NULL so
    // that we exit the function and free any memory that we need to
    char *codeID = strtok(codedrop, ", ");
    if (codeID == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Received an invalid message and ignored it.\n");
      free(cd);
      bag_delete(codedropBag);
      bag_delete(CDBag);
      return NULL;
    }
    // If there were no errors, assign the value to the respective member of
    // the codedrop struct
    cd->hexcode = codeID;

    /*Repeat the above process for the rest of the comma-separated subfields*/

    char* lat = strtok(NULL, ", ");
    if (lat == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Received an invalid message and ignored it.\n");
      free(cd);
      bag_delete(codedropBag);
      bag_delete(CDBag);
      return NULL;
    }
    cd->latitude = lat;

    char* Long = strtok(NULL, ", ");
    if (Long == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Received an invalid message and ignored it.\n");
      free(cd);
      bag_delete(codedropBag);
      bag_delete(CDBag);
      return NULL;
    }
    cd->longitude = Long;

    char *neutralizingTeam = strtok(NULL, ", ");
    if (neutralizingTeam == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Received an invalid message and ignored it.\n");
      free(cd);
      bag_delete(codedropBag);
      bag_delete(CDBag);
      return NULL;
    }
    cd->teamname = neutralizingTeam;

    // Insert cd, the codedrop struct, into the CDBag if we didn't return
    // because of an error in the message.
    bag_insert(CDBag, cd);
  }

  // Delete the bag here because the program is done with it
  bag_delete(codedropBag);

  // Return the new bag of codedrop structs
  return CDBag;
}




/*********************** printGameOver() **********************/
/* Prints a game over message. Takes the current game ID, a file to log to,
 * and an integer logSwitch that indicates the whether or not to log everything
 * that happens in verbose or game mode. Parses the rest of the GAME_OVER
 * message after the OPCODE and gameID fields using strtok. 
 * Checks to make sure that the message received from the Game Server is valid.
 * If the message is valid and the game over message is printed for the user
 * to see, return 0 so the program can exit the game. Otherwise, return 1
 * to indicate that there was some sort of problem parsing the message.
 */
int printGameOver(char *gameID, FILE *fp, int logSwitch) {

  /* strtok the remaining two pipeline-separated fields and check to make
   * sure they are not NULL. If they are, write to the log that we received
   * a bad message and return 1, ignoring the rest of the GAME_OVER message.*/
  
  char *numRemainingCodeDrops = strtok(NULL, "|");
  if (numRemainingCodeDrops == NULL) {
    if (logSwitch == 1) 
      printTime(fp);
    fprintf(fp, "Guide Agent received a poorly-formatted message and ");
    fprintf(fp, "ignored it.\n");
    return 1;
  }
  char *teamRecords = strtok(NULL, "|");
  if (teamRecords == NULL) {
    if (logSwitch == 1)
      printTime(fp);
    fprintf(fp, "Guide Agent received a poorly-formatted message and ");
    fprintf(fp, "ignored it.\n");
    return 1;
  }

  // Create a bag to hold the colon-separated subfields pertaining to the
  // information about each team in the game.
  bag_t *recordBag = bag_new();

  // strtok the first colon-separated subfield and make sure it is not NULL
  char *record = strtok(teamRecords, ":");
  if (record == NULL) {
    if (logSwitch == 1)
      printTime(fp);
    fprintf(fp, "Guide Agent received a poorly-formatted message and ");
    fprintf(fp, "ignored it.\n");
    bag_delete(recordBag);
    return 1;
  }

  // For each team record in the message, strtok it and put the string in the
  // recordBag so that we can further parse it later
  while (record != NULL) {
    bag_insert(recordBag, record);
    record = strtok(NULL, ":");
  }
  
  printf("GAME OVER! Here are the records for this game:\n");
  printf("Number of remaining unneutralized code drops is %s\n",
	 numRemainingCodeDrops);

  
  // string consisting of a team's records that we need to parse further
  char *recordPrint = bag_extract(recordBag);

  // bag that holds record structs, which contain information about each
  // team's statistics at the end of the game
  bag_t *recBag = bag_new();

  /* Go through all the strings that we temporarily stored in the recordBag,
   * checking each field to make sure that the message is valid. If the
   * field is valid, assign its value to the associated member in the record
   * struct */
  while (recordPrint != NULL) {

    // Malloc space for a new struct and check to make sure it is not NULL
    recordStruct_t *myRec = malloc(sizeof(recordStruct_t));
    if (myRec == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Error allocating memory.\n");
      return 1;
    }

    // strtok the first field and make sure it is not NULL
    char *teamName = strtok(recordPrint, ", ");
    if (teamName == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Guide Agent received a poorly-formatted message and ");
      fprintf(fp, "ignored it.\n");
      bag_delete(recordBag);
      bag_delete(recBag);
      return 1;
    }
    // If the field wasn't NULL, assign its value to the appropriate member
    // of the recordStruct
    myRec->teamName = teamName;

    // Repeat the above process for the rest of the comma-separated subfields
    
    char *numPlayers = strtok(NULL, ", ");
    if (numPlayers == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Guide Agent received a poorly-formatted message and ");
      fprintf(fp, "ignored it.\n");
      bag_delete(recordBag);
      bag_delete(recBag);
      return 1;
    }
    myRec->numPlayers = numPlayers;
    
    char *numCaptures = strtok(NULL, ", ");
    if (numCaptures == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Guide Agent received a poorly-formatted message and ");
      fprintf(fp, "ignored it.\n");
      bag_delete(recordBag);
      bag_delete(recBag);
      return 1;
    }
    myRec->numCaptures = numCaptures;

    
    char *numCaptured = strtok(NULL, ", ");
    if (numCaptured == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Guide Agent received a poorly-formatted message and ");
      fprintf(fp, "ignored it.\n");
      bag_delete(recordBag);
      bag_delete(recBag);
      return 1;
    }
    myRec->numCaptured = numCaptured;
    
    char *numNeutralized = strtok(NULL, ", ");
    if (numNeutralized == NULL) {
      if (logSwitch == 1)
	printTime(fp);
      fprintf(fp, "Guide Agent received a poorly-formatted message and ");
      fprintf(fp, "ignored it.\n");
      bag_delete(recordBag);
      bag_delete(recBag);
      return 1;
    }
    myRec->numNeutralized = numNeutralized;

    // If we didn't return because of any errors, add myRec to the recBag
    bag_insert(recBag, myRec);

    // Extract the next team record for parsing
    recordPrint = bag_extract(recordBag);
  }

  /*** print the information that we got from the Game Server in an 
       appropriate format ***/
  
  recordStruct_t *myRec;
  while ((myRec = bag_extract(recBag)) != NULL) {
    printf("Team %s:\n", myRec->teamName);
    printf("   Total number of active players: %s\n", myRec->numPlayers);
    printf("   Captured %s players from other teams.\n", myRec->numCaptures);
    printf("   %s players on this team were captured.\n", myRec->numCaptured);
    printf("   Neutralized %s code drops.\n", myRec->numNeutralized);
    free(myRec);
  }

  // Free the memory associated with the created bags
  bag_delete(recordBag);
  bag_delete(recBag);
  printf("Good game, everybody!");
  fflush(stdout);
  return 0; // return 0 to indicate that we received a valid GAME_OVER message
}





/*********************** sendStatusRequest() **********************/
/* Takes 4 strings: gameID, guideID, teamName, and playerName, and it takes
 * an int comm_sock and a const struct sockaddr *gsp. With this information,
 * sendStatusRequest() sends a status request to the game server in a properly-
 * formatted message with the OPCODE GA_STATUS.
 */
int sendStatusRequest(char *gameID, char *guideID, char *teamName,
		  char *playerName, int comm_sock, const struct sockaddr *gsp)
{
  // allocate memory for the GA_STATUS message
  char *request = malloc(16 + strlen(gameID) + strlen(guideID) +
			 strlen(teamName) + strlen(playerName));

  // Check to make sure there were no errors allocating memory
  if (request == NULL) {
    printf("Error\n");
    return 3;
  }

  // Concatenate the string to send to the Game Server
  sprintf(request, "GA_STATUS|%s|%s|%s|%s|1", gameID, guideID, teamName,
	  playerName);

  // Send the message
  if (sendto(comm_sock, request, strlen(request), 0, (struct sockaddr *) gsp,
	     sizeof(*gsp)) < 0){
    perror("error sending datagram\n");
    free(request);
    return 5;
  }

  free(request);
  return 0;      // return 0 if there were no problems
}



/*********************** printPebbleIDs() **********************/
/* Used when iterating over a list of FA structs. 
 * This function prints a list of pebble ID's belonging to active players on 
 * the Guide Agent's team
 */
void printPebbleIDs(void *arg, char *key, void *data, void *option)
{
  FA_t *myAgent = data;
  char *teamName = arg;

  // Check to make sure the data is not NULL by some chance
  if (myAgent != NULL) {
    
    // Print the pebble ID only if the Field Agent belongs to the Guide's team
    // and has not been captured yet
    if (myAgent->capture == false && strcmp(myAgent->teamname, teamName) == 0){
      printf("%s ", key);
      fflush(stdout);
    }
  }
}



/********************* getNumAgents() **********************/
/* Used when iterating over a list of FA structs.
 * Given a pointer to an integer as the void *arg argument, the function
 * increments its value for every active agent on the Guide's team
 */
void getNumAgents(void *arg, char *key, void *data, void *option)
{
  int *numAgents = arg;
  FA_t *current = data;
  char *teamName = option;

  // Only increment numAgent's value if the current agent has not been captured
  // and is on the same team as the Guide Agent
  if (current->capture == false && strcmp(teamName, current->teamname) == 0) {
    (*numAgents)++;
  }
}


/*********************** isNumber() **********************/
/* Given a string, isNumber() return true if the string consists only of 
 * digits, and it returns false if it contains any non-digit characters 
 */
bool isNumber(const char *arg)
{
  for (int i = 0; i < strlen(arg); i++) {
    if (!isdigit(arg[i]))
      return false;
  }
  return true;
}





/********************* deletefunc() **********************/
/* Used to help free memory for items in a list
 * Pass deletefunc in as a parameter to list_new() for any list
 * that will hold items that have malloc'd or calloc'd data
 */
void deletefunc(void *data)
{
  if (data)
    free(data);
}

/********************* printTime() **********************/
/* Given a file pointer, the function will print the current time onto a line
 * of that file
 */
void printTime(FILE *fp)
{
  time_t myTime = time(NULL);
  char* timeString = ctime(&myTime);
  fprintf(fp, "%s ", timeString);
}


/********************* deleteNodes() **********************/
/* This function "deletes" the nodes in a current list by setting the key
 * for each of the existing nodes to "NONE". It does not actually delete the
 * nodes or their data, but makes them invisible to our other functions in this
 * program. The data in each of the nodes will still have to be freed later
 * by the list_delete function.
 */
void deleteNodes(void *arg, char *key, void *data, void *option)
{
  char **keyp = &key;
  *keyp = "NONE";
}



/********************* freeBag() **********************/
/* This function is called whenever there are items still in a bag that needs
 * to be freed. freeBag() extracts every item and frees its data so that we
 * can later free the bag itself with no memory leaks.
 */ 
void freeBag(bag_t *bag)
{
  void *bagItem;

  while ((bagItem = bag_extract(bag)) != NULL)
    free(bagItem);
}
