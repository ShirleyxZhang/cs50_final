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
#include "../lib/list/list.h"
#include "../lib/bag/bag.h"
#include "../lib/counters/counters.h"
#include <cairo.h>
#include <gtk/gtk.h>

typedef struct codedrop {
  bool status;
  char* hexcode;
  struct location *location;
  char* teamname;
} codedrop_t;


// update button


typedef struct FA {
  char* teamname;
  char* address;
  bool capture;
  char* pebbleid;
  char* name;
  float latitude;
  float longitude;
  float capture_start;       // place to hold the start time of capturing for an FA
  float contact;
} FA_t;




static int createSocket(int argc, char *argv[],
			struct sockaddr_in *gsp, int logSwitch);
bool isNumber(const char *arg);
char *getGuideID(void);
static int handleSocket(int comm_sock, struct sockaddr_in *gsp, list_t *list, char **gameID, FILE *fp);
char *handleStdin(int comm_sock, struct sockaddr_in *gsp, list_t *list, char *teamName);
void printPebbleIDs(void *arg, char *key, void *data);
void getNumAgents(void *arg, char *key, void *data);
void deletefunc(void *data);
void printGameOver(char *gameID, FILE *fp);
void initialize_window(GtkWidget* window);





//cairo_image_surface_create_from_png()
//cairo_set_source_surface()


int HEIGHT=811;
int WIDTH=1012;
static int BUFSIZE = 8000;


int main(int argc, char *argv[]) {

  // represents whether or not the -log=raw switch was used
  int logSwitch = 0;

  // if the first argument is -log=raw, change logSwitch to reflect that
  if (strcmp(argv[1], "-log=raw") == 0) {
    logSwitch = 1;
  }

  // Check to make sure the user entered the right number or arguments
  // if the log switch was called, there should be 5 arguments
  if (logSwitch == 1 && argc != 6) {
    printf("Incorrect number of arguments.\n");
    exit(1);
  }
  // if the log switch was not called, there should be 4 arguments
  else if (logSwitch == 0 && argc != 5) {
    printf("Incorrect number of arguments.\n");
    exit(1);
  }
  

  // assign the team name input by the user to a variable
  // position of this team name on the command line varies depending
  // on whether or not the -log=raw switch was used    
  char *teamName;
  if (logSwitch == 0)
    teamName = argv[1];
  else
    teamName = argv[2];

  const char *playerName;
  if (logSwitch == 0)
    playerName = argv[2];
  else
    playerName = argv[3];

  
  const char *guideID = getGuideID();
  if (strcmp(guideID, "EOF") == 0) {
    printf("\n");
    exit(0);
  }
  
  // following code based on Kotz's chatclient.c
  
  struct sockaddr_in gs;
  const struct sockaddr *gsp = (struct sockaddr *) &gs;
  
  int comm_sock = createSocket(argc, argv, &gs, logSwitch);

  
  /***** send message to GS to announce presence *****/
  char *message = malloc(strlen("GA_STATUS") + strlen(guideID)
  			 + strlen(teamName) + strlen(playerName)+ 8);

  if (message == NULL) {
    printf("Error allocating memory for first message\n");
    exit(3);
  }
  sprintf(message, "GA_STATUS|0|%s|%s|%s|1", guideID, teamName, playerName);

  printf("Sent first message to the game server to join the game.\n");
  
  if (sendto(comm_sock, message, strlen(message), 0, gsp, sizeof(gs)) < 0) {
    printf("Error sending startup message\n");
    exit(4);
  }

  free(message);
  
  FILE *logp;
  if ((logp = fopen("../log/guideagent.log", "w")) == NULL) {
    printf("could not open log file");
  }
  else if (logSwitch == 0) {
    fprintf(logp, "Guide Agent %s joined the game with ID %s\n", playerName,
	    guideID);
  }
  else {
    //    write
    ;
  }
  //  fclose(logp);



  

  /*
   * put pebbleID's somewhere in interface so GA has access to them and knows where
   * to send them
   */


  char *gameID = NULL;
  
  list_t *FAList = list_new(deletefunc);





  gtk_init(&argc, &argv);


  GtkWidget *window;
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  //  GtkWidget *darea;
  /* darea = gtk_drawing_area_new(); */
  /* gtk_container_add(GTK_CONTAINER(window), darea); */

  /* g_signal_connect(G_OBJECT(darea), "draw",G_CALLBACK(on_draw_event), NULL); */
  /* g_signal_connect(window, "destroy",G_CALLBACK(gtk_main_quit), NULL); */

  /* GIOChannel *server = g_io_channel_unix_new (comm_sock); */
  /* GIOChannel *standard = g_io_channel_unix_new (0); */

  /* g_signal_connect(server, "nfdsa", G_CALLBACK(handleSocket), NULL); */
  /* g_signal_connect(standard, "poop", G_CALLBACK(handleStdin), NULL); */
  
  initialize_window(window);




  /* fd_set rfds; */
  /* /\* struct timeval timeout; *\/ */
  /* /\* const struct timeval fifteensec = {15,0}; *\/ */

  /* FD_ZERO(&rfds); */

  /* FD_SET(0, &rfds); // stdin */
  /* FD_SET(comm_sock, &rfds); */

  /* int nfds = comm_sock + 1; */

  /* timeout = fifteensec; */
  /* select(nfds, &rfds, NULL, NULL, &timeout); */
  
  
  //  gtk_main();


  
  
  
  while (true) {
 
    /* initialize struct for field agents on team so we have access to pebbleID's        
     * potentially a list of pebbleID's                                     
     */

    
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

	char *idAndHint = handleStdin(comm_sock, &gs, FAList, teamName);

	if (strcmp(idAndHint, "NULL") != 0) {
	  if (strcmp(idAndHint, "EOF") == 0)
	    break;

	  if ((gameID != NULL)) {

	    char *sendHint = malloc(13 + strlen(gameID) + strlen(guideID) + strlen(teamName) + strlen(playerName) + strlen(idAndHint));

	
	    /* make sure not null */
	    if (sendHint == NULL) {
	      printf("Error allocating memory for message.\n");
	      exit(5);
	    }

	
	    sprintf(sendHint, "GA_HINT|%s|%s|%s|%s|%s", gameID, guideID, teamName, playerName, idAndHint);

	    if (sendto(comm_sock, sendHint, strlen(sendHint), 0, (struct sockaddr *) gsp, sizeof(*gsp)) < 0\
		){
	      perror("error sending datagram\n");
	      exit(5);
	    }
	    else {
	      printf("Your hint was sent to the game server.\n");
	    }		
	  }
	  else if (strcmp(idAndHint, "NULL") == 0) {
	    printf("hint didn't work.\n");
	  }
	  else {
	    printf("You can't send hints until you've joined the game!\n");
	    fflush(stdout);
	  } 
	}
      }
	  

      if (FD_ISSET(comm_sock, &rfds)) {
	// handle socket
	if (handleSocket(comm_sock, &gs, FAList, &gameID, logp) == 0) {
	  break;
	}

	printf("Enter a hint to send to a field agent.\n");
	fflush(stdout);
      }
    }

    //    list_delete(FAList);
  }

  close(comm_sock);
  putchar('\n');
  fclose(logp);
  exit(0);

}






/**************************** createSocket ***************************/

/* Parse arguments and set up socket
 */
static int createSocket(int argc, char *argv[],
			struct sockaddr_in *gsp, int logSwitch)
{
  // store IP address input by user in a variable
  // position for this argument varies as well depending on switch
  const char* GShost;

  if (logSwitch == 0)
    GShost = argv[3];
  else
    GShost = argv[4];


  // store port number input by user in a variable
  // position for this argument varies as well depending on switch
  int GSport;

  if (logSwitch == 0) {
    if (isNumber(argv[4]))
      GSport = atoi(argv[4]);
    else {
      printf("Port number can only consist of integers.\n");
      exit(1);
    }

  }
  else {
    if (isNumber(argv[5]))
      GSport = atoi(argv[5]);
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





/********************** handleSocket *********************/


static int
handleSocket(int comm_sock, struct sockaddr_in *gsp, list_t *list, char **gameID, FILE *fp)
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
	/* char *ID = strtok(NULL, "|"); */
	/* char *AllAgents = strtok(NULL, "|"); */
	/* char *AllCodedrops = strtok(NULL, "|"); */
	
	if (OPCODE == NULL) {
	  // do nothing?
	}
	
	// if OPCODE is GAME_STATUS
	else if (strcmp(OPCODE, "GAME_STATUS") == 0) {

	  char *ID = strtok(NULL, "|");
	  char *AllAgents = strtok(NULL, "|");
	  char *AllCodedrops = strtok(NULL, "|");
	  
	  if (*gameID == NULL) {
	    *gameID = ID;
	    fprintf(fp, "Guide agent received first message from game server. The ID of this game is %s\n", *gameID);
	    printf("Joined the game!\n");
	    fflush(stdout);
	  }
	  printf("%s:\n", OPCODE);
	  fflush(stdout);
	  
	  if (strcmp(ID, *gameID) != 0) {
	  // if gameID doesn't match,
	    fprintf(fp, "Guide agent received a message from a different game and ignored it. The ID of this game was %s\n", ID);
	  }
	  else {
	    // go through the remaining fields
	    printf("Game ID is %s\n", ID);
	    fflush(stdout);
	    
	    if (AllAgents == NULL) {
	      fprintf(fp, "There was something wrong with the FA field in the message from the game server.\n");
	    // or is the list just empty??
	    }
	    else {

      	      bag_t *agentBag = bag_new();
	      char *agent = strtok(AllAgents, ":");
	      
      	      if (agent == NULL) {
		// list is empty?
		fprintf(fp, "There are currently no active agents.\n");
	      }
	      else {
		// go through each agent
		printf("Agents:\n");

		bag_insert(agentBag, agent);

		while ((agent = strtok(NULL, ":")) != NULL) {
		  bag_insert(agentBag, agent);
		}

		list_delete(list);
		list = list_new(deletefunc);
		
		while ((agent = bag_extract(agentBag)) != NULL) {
		  FA_t *FA = malloc(sizeof(struct FA));

		  char *pebbleID = strtok(agent, ", ");
		  FA->pebbleid = pebbleID;
		  
		  char *teamName = strtok(NULL, ", ");
		  FA->teamname = teamName;
		  
		  char *playerName = strtok(NULL, ", ");
		  FA->name = playerName;
		  
		  char *playerStatus = strtok(NULL, ", ");
		  if (strcmp(playerStatus, "captured") == 0)
		    FA->capture = true;
		  else
		    FA->capture = false;
		      
		  char *lastKnownLat = strtok(NULL, ", ");
		  FA->latitude = (float) *lastKnownLat;
		  
		  char *lastKnownLong = strtok(NULL, ", ");
		  FA->longitude = (float) *lastKnownLong;
		  
		  char *secondsSinceLastContact = strtok(NULL, ", ");
		  FA->contact = (float) *secondsSinceLastContact;
		  		  
		  list_insert(list, FA->pebbleid, FA);

		  printf("   Agent %s from team %s: pebble ID is %s.\n", playerName, teamName, pebbleID);
		  if (FA->capture == true)
		    printf("     %s has been captured and is no longer active.\n", playerName);
		  else
		    printf("     %s has not been captured and is still active.\n", playerName);

		  printf("     Last known latitude: %s. Last known longitude is :%s\n", lastKnownLat, lastKnownLong);
		  printf("     Seconds since last contact is %s.\n", secondsSinceLastContact);

		  fflush(stdout);
		}

		if (AllCodedrops == NULL)
		  fprintf(fp, "There was something wrong with the FA field in the message from the game server.\n");
		else {
		  printf("Code drops:\n");

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

		    while ((codedrop = bag_extract(codedropBag)) != NULL) {
		      char *codeID = strtok(codedrop, ", ");
		      char* lat = strtok(NULL, ", ");
		      char* Long = strtok(NULL, ", ");
		      char *neutralizingTeam = strtok(NULL, ", ");

		      printf("   Code drop %s: latitude is %s, longitude is %s. ", codeID, lat, Long);
		      fflush(stdout);
		      if (strcmp(neutralizingTeam, "NONE") == 0)
			printf("Has not yet been neutralized.\n");
		      else
			printf("Neutralized by %s.\n", neutralizingTeam);
		      fflush(stdout);
		    }  		    
		  }		
		}
	      }
	    }
	  }
	}	 
	// else if OPCODE is GAME_OVER
	else if (strcmp(OPCODE, "GAME_OVER") == 0) {
	  char *ID = strtok(NULL, "|");
	  printf("game over\n");
	  printf("%s\n", *gameID);
	  fflush(stdout);
	  
	  // go through everything
	  if (gameID == NULL) {
	    fprintf(fp,"Guide agent received game over message before joining a game. Message was therefore ignored.\n");
	    //ignore; we haven't even goined a game yet
	  }
	  else if (strcmp(ID, *gameID) != 0) {
	    printf("ID is %s\n", ID);
	    printf("gameID is %s\n", *gameID);
	    // if gameID doesn't match,
	    fprintf(fp, "Guide agent received a message from a different game and ignored it. The ID of this game was %s\n", ID);
	  }
	  else {
	    printGameOver(ID, fp);
	    return 0;
	  }
	}
      }
      else {
	// don't do stuff?
	;
      }
    }
  }
  return 1;
}






/************************** handleStdin ***************************/

char *
handleStdin(int comm_sock, struct sockaddr_in *gsp, list_t *list, char *teamName)
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
  
  list_t *numAgents = list_new(NULL);
  list_iterate(list, getNumAgents, numAgents);
  int *num = list_find(numAgents, teamName);
  fflush(stdout);

  if (num == NULL) {
    printf("You have no active agents to send hints to!\n");
    free(hint);
    fflush(stdout);
    return "NULL";
  }
  
  printf("Send hint to which field agent? Your active agents have the following ID's:\n");
  fflush(stdout);
  list_iterate(list, printPebbleIDs, teamName);
  printf("\n");
  fflush(stdout);
  
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



void printGameOver(char *gameID, FILE *fp) {
  char *numRemainingCodeDrops = strtok(NULL, "|");
  char *teamRecords = strtok(NULL, "|");

  bag_t *recordBag = bag_new();
  char *record = strtok(teamRecords, ":");

  printf("GAME OVER! Here are the records for this game:\n");
  printf("Number of remaining unneutralized code drops is %s\n", numRemainingCodeDrops);
  
  while (record != NULL) {
    char *teamName = strtok(record, ", ");
    char *numPlayers = strtok(record, ", ");
    char *numCaptures = strtok(record, ", ");
    char *numCaptured = strtok(record, ", ");
    char *numNeutralized = strtok(record, ", ");

    printf("Team %s:\n", teamName);
    printf("   Total number of active players: %s\n", numPlayers);
    printf("   Captured %s players from other teams.\n", numCaptures);
    printf("   %s players on this team were captured.\n", numCaptured);
    printf("   Neutralized %s code drops.\n", numNeutralized);

    record = strtok(teamRecords, ":");
  }
  free(recordBag);
  printf("Good game, everybody!");
  fflush(stdout);
}





void printPebbleIDs(void *arg, char *key, void *data)
{
  FA_t *myAgent = data;
  char* teamName = arg;
  
  if (myAgent->capture == false && strcmp(myAgent->teamname, teamName) == 0) {
    printf("%s ", key);
    fflush(stdout);
  }
}




void getNumAgents(void *arg, char *key, void *data)
{
  list_t *numAgents = arg;
  
  FA_t *currentAgent = data;
  
  if (currentAgent->capture == false) {
    int *nump = list_find(numAgents, currentAgent->teamname);
    if (nump == NULL) {
      int num = 1;
      list_insert(numAgents, currentAgent->teamname, &num);
    }
    else {
      int num = *nump;

      if (num == 0) {
	num++;
	list_insert(numAgents, currentAgent->teamname, &num);
      }
    }
  }
}



void deletefunc(void *data)
{
  if (data)
    free(data);
}



void initialize_window(GtkWidget* window){
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);
  gtk_window_set_title(GTK_WINDOW(window), "GTK window");
  gtk_widget_show_all(window);
}
