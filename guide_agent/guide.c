/*
 * guide.c - One of three parts for the game "Mission Imcomputable"
 * Works with a Game Server and Field Application to play the game.
 * Receives status updates from the Game Server and allows the person 
 * playing as the Guide Agent to send hints to Field Agents through the
 * Game Server. 
 *
 * Usage: 
 * ./guide [-log=raw] teamName playerName GShost GSport
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


/* typedef struct standardIn { */
/*   int comm_sock; */
/*   struct sockaddr_in *gsp; */
/*   list_t *list; */
/*   char *teamName; */
/* } standardIn_t; */


/* typedef struct handleSock { */
/*   int comm_sock; */
/*   struct sockaddr_in *gsp; */
/*   list_t *list; */
/*   char **gameID; */
/*   FILE *fp; */
/* } handleSock_t; */

static int createSocket(int argc, char *argv[], struct sockaddr_in *gsp,
			int logSwitch);
bool isNumber(const char *arg);
char *getGuideID(void);
static int handleSocket(int comm_sock, struct sockaddr_in *gsp, list_t *list,
			char **gameID, FILE *fp, int logSwitch);
char *handleStdin(int comm_sock, struct sockaddr_in *gsp, list_t *list,
		  char *teamName);
/* int handleSocket(GIOChannel *gio, GIOCondition condition, gpointer data); */
/* int handleStdin(GIOChannel *gio, GIOCondition condition, gpointer data); */

void printPebbleIDs(void *arg, char *key, void *data);
void getNumAgents(void *arg, char *key, void *data);
void deletefunc(void *data);
void printGameOver(char *gameID, FILE *fp);
int sendStatusRequest(char *gameID, const char *guideID, char *teamName,
            const char *playerName, int comm_sock, const struct sockaddr *gsp);
void printTime(FILE *fp);

//void initialize_window(GtkWidget* window);
/* static void custom_background(cairo_t* cr); */
/* static void background(cairo_t* cr); */



/* int HEIGHT=811; */
/* int WIDTH=1012; */
/* cairo_surface_t* pic = NULL; */
static int BUFSIZE = 8000;


/* mygcc `pkg-config --cflags --libs gtk+-3.0` -o guide guide.c ../common/file.c ../common/file.h ../lib/list/list.h ../lib/list/list.c ../lib/bag/bag.h ../lib/bag/bag.c ../lib/counters/counters.c ../lib/counters/counters.h */




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

  // assign player's name based on command line input
  const char *playerName;
  if (logSwitch == 0)
    playerName = argv[2];
  else
    playerName = argv[3];

  // prompt user for an ID used to identify the guide agent
  const char *guideID = getGuideID();

  // If the user entered "^D" before entering an ID, exit the program
  if (strcmp(guideID, "EOF") == 0) {
    printf("\n");
    exit(0);
  }
  
  // the server's address
  struct sockaddr_in gs;
  const struct sockaddr *gsp = (struct sockaddr *) &gs;

  // set up a socket on which to communicate  
  int comm_sock = createSocket(argc, argv, &gs, logSwitch);

  
  /***** send message to GS to announce presence *****/
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
  if (sendto(comm_sock, message, strlen(message), 0, gsp, sizeof(gs)) < 0) {
    printf("Error sending startup message\n");
    exit(4);
  }
  
  printf("Sent first message to the game server to join the game.\n");
  free(message);

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
    // user input -log=raw, so we also give them the time of each action
    time_t myTime = time(NULL);
    char* timeString = ctime(&myTime);
    fprintf(logp, "%s ", timeString);
    fprintf(logp, "Guide Agent %s joined the game with ID %s\n\n", playerName,
	    guideID);
  }






  /* gtk_init(&argc, &argv); */


  /* GtkWidget *window; */
  /* window = gtk_window_new(GTK_WINDOW_TOPLEVEL); */

  /* /\* GtkWidget *entry; *\/ */
  /* /\* entry = gtk_entry_new(); *\/ */
  
  /* GtkWidget *darea; */
  /* darea = gtk_drawing_area_new(); */
  /* //  gtk_container_add(GTK_CONTAINER(window), entry); */
  /* gtk_container_add(GTK_CONTAINER(window), darea); */
  
  /* //  g_signal_connect(G_OBJECT(entry), "draw",G_CALLBACK(handleStdin), NULL); */
  /* //  g_signal_connect(G_OBJECT(darea), "draw",G_CALLBACK(on_draw_event), NULL); */
  /* /\* g_signal_connect(window, "destroy",G_CALLBACK(gtk_main_quit), NULL); *\/ */

  /* //int comm_sock, struct sockaddr_in *gsp, list_t *list, char *teamName */
  
  /* standardIn_t *stdIn = malloc(sizeof(standardIn_t)); */
  /* stdIn->comm_sock = comm_sock; */
  /* stdIn->gsp = &gs; */
  /* stdIn->list = FAList; */
  /* stdIn->teamName = teamName; */

  /* handleSock_t *sock = malloc(sizeof(handleSock_t)); */
  /* sock->comm_sock = comm_sock; */
  /* sock->gsp = &gs; */
  /* sock->list = FAList; */
  /* sock->gameID = &gameID; */
  /* sock->fp = logp; */
  
  /* GIOChannel *server = g_io_channel_unix_new (comm_sock); */
  
  /* GIOChannel *standard = g_io_channel_unix_new (0); */
  
  /* //  g_signal_connect(server, "nfdsa", G_CALLBACK(handleSocket), *stdIn); */
  
  /* //  g_signal_connect(standard, "poop", G_CALLBACK(handleStdin), stdIn); */

  
  /* // g_io_add_watch(server, G_IO_IN | G_IO_OUT, handleSocket, sock); */
  /* g_io_add_watch(standard, G_IO_IN | G_IO_OUT, handleStdin, stdIn); */
  /* g_io_add_watch(server, G_IO_IN | G_IO_OUT, handleSocket, sock); */
  /* //  printf("%d\n", myInt); */

  /* /\* printf("before surface.\n"); *\/ */
  /* /\* cairo_surface_t *surface = cairo_image_surface_create_from_png("a.png"); *\/ */
  /* /\* printf("2"); *\/ */
  /* /\* fflush(stdout); *\/ */
  /* /\* cairo_set_source_surface(NULL, surface, 10, 10); *\/ */
  /* /\* printf("3"); *\/ */
  /* /\* fflush(stdout); *\/ */
  /* initialize_window(window); */

  /* /\* printf("Enter anything to look for a game to join.\n"); *\/ */
  /* /\* fflush(stdout); *\/ */
  
  /* //  gtk_main(); */
  /*   GMainLoop *mainLoop = g_main_loop_new(NULL, false); */

  /* printf("7"); */
  /* fflush(stdout); */

  /* g_main_loop_run(mainLoop); */
  

  char *gameID = NULL;

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

	    if (sendto(comm_sock, sendHint, strlen(sendHint), 0, (struct sockaddr *) gsp, sizeof(*gsp)) < 0){
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
	if (handleSocket(comm_sock, &gs, FAList, &gameID, logp, logSwitch) == 0) {
	  break;
	}


	printf("Enter a hint to send to a field agent.\n");
	fflush(stdout);
      }
    }

    //    list_delete(FAList);
  }
  //  free(guideID);
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
handleSocket(int comm_sock, struct sockaddr_in *gsp, list_t *list, char **gameID, FILE *fp, int logSwitch)
//int handleSocket(GIOChannel *gio, GIOCondition condition, gpointer data)
{
  /* handleSock_t *sock = data; */

  /* int comm_sock = sock->comm_sock; */
  /* struct sockaddr_in *gsp = sock->gsp; */
  /* list_t *list = sock->list; */
  /* char **gameID = sock->gameID; */
  /* FILE *fp = sock->fp; */

  
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
	/* char *AllAgents = strtok(NULL, "|"); */
	/* char *AllCodedrops = strtok(NULL, "|"); */
	
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
	  printf("opcode is %s\n", OPCODE);
	  printf("ID is %s\n", ID);
	  
	  // gameID in message from game server 
	  //	  char *ID = strtok(NULL, "|");

	  // field containing info about all the field agents in the game
	  char *AllAgents = strtok(NULL, "|");
	  //	  if (AllAgents == NULL
	  
	  // field containing info about all the code drops in the game
	  char *AllCodedrops = strtok(NULL, "|");
	  if (AllCodedrops == NULL) {
	    if (logSwitch == 1)
	      printTime(fp);
	    fprintf(fp, "Received an invalid message and ignored it.\n");
	  }
	  
	  // gameID is NULL if we have not yet recieved a message from the
	  // game server
	  if (*gameID == NULL) {
	    // gameID takes on the value of the gameID field of the first
	    // message it gets from the game server
	    *gameID = malloc(strlen(ID) + 1);
	    strcpy(*gameID, ID);
	    fprintf(fp, "Guide agent received first message from game server. The ID of this game is %s\n", *gameID);
	    printf("Joined the game!\n");
	  }
	  
	  if (strcmp(ID, *gameID) != 0) {
	  // the message we just received contains a different gameID
	    fprintf(fp, "Guide agent received a message from a different game and ignored it. The ID of this game was %s\n", ID);
	  }
	  else {
	    printf("%s:\n", OPCODE);
	    
	    // go through the remaining fields
	    printf("Game ID is %s\n", ID);
	    fflush(stdout);
	    
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

		list_delete(list);
		list = list_new(deletefunc);

		bag_t *FABag = bag_new();
		
		while ((agent = bag_extract(agentBag)) != NULL) {
		  FA_t *FA = malloc(sizeof(struct FA));
		  if (FA == NULL) {
		    printf("Error allocating memory.\n");
		    return 1;
		  }		    

		  char *pebbleID = strtok(agent, ", ");
		  if (pebbleID == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n");
		    return 1;
		  }		    
		  FA->pebbleid = pebbleID;
		  
		  char *teamName = strtok(NULL, ", ");
		  if (teamName == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n");
		    return 1;
		  }		    
		  FA->teamname = teamName;
		  
		  char *playerName = strtok(NULL, ", ");
		  if (playerName == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n");
		    return 1;
		  }
		  FA->name = playerName;
		  
		  char *playerStatus = strtok(NULL, ", ");
 		  if (playerStatus == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n"			    );
		    return 1;
		  } 
		  if (strcmp(playerStatus, "captured") == 0)
		    FA->capture = true;
		  else if (strcmp(playerStatus, "active") == 0)		    
		    FA->capture = false;
		  else {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n"\
			    );
		    return 1;
		  }		  
		      
		  char *lastKnownLat = strtok(NULL, ", ");
		  if (lastKnownLat == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n"\
			    );
		    return 1;
		  }
		  
		  FA->latitude = lastKnownLat;
		  
		  char *lastKnownLong = strtok(NULL, ", ");
		  if (lastKnownLong == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n"\
			    );
		    return 1;
		  }		  
		  FA->longitude = lastKnownLong;
		  
		  char *secondsSinceLastContact = strtok(NULL, ", ");
		  if (secondsSinceLastContact == NULL) {
		    if (logSwitch == 1)
		      printTime(fp);
		    fprintf(fp, "Received an invalid message and ignored it.\n"\
			    );
		    return 1;
		  }		  
		  FA->contact = secondsSinceLastContact;

		  bag_insert(FABag, FA);
		  list_insert(list, FA->pebbleid, FA);
		}

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
			  return 1;
			}

			char *codeID = strtok(codedrop, ", ");
			if (codeID == NULL) {
			  if (logSwitch == 1)
			    printTime(fp);
			  fprintf(fp, "Received an invalid message and ignored it.\n");
			  return 1;
			}
			cd->hexcode = codeID;

			char* lat = strtok(NULL, ", ");
			if (lat == NULL) {
			  if (logSwitch == 1)
			    printTime(fp);
			  fprintf(fp, "Received an invalid message and ignored it.\n");
			}
			cd->latitude = lat;

			char* Long = strtok(NULL, ", ");
			if (Long == NULL) {
			  if (logSwitch == 1)
			    printTime(fp);
			  fprintf(fp, "Received an invalid message and ignored it.\n");
			}
			cd->longitude = Long;

			char *neutralizingTeam = strtok(NULL, ", ");
		 	if (neutralizingTeam == NULL) {
			  if (logSwitch == 1)
			    printTime(fp);
			  fprintf(fp, "Received an invalid message and ignored it.\n");
			  return 1;
			}
			cd->teamname = neutralizingTeam;

			bag_insert(CDBag, cd);
		      }


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
		      
		      printf("Code drops:\n");

		      codedrop_t *cd;

		      while((cd = bag_extract(CDBag)) != NULL) {
		      printf("   Code drop %s: latitude is %s, longitude is %s. ", cd->hexcode, cd->latitude, cd->longitude);
		      fflush(stdout);
		      if (strcmp(cd->teamname, "NONE") == 0)
			printf("Has not yet been neutralized.\n");
		      else
			printf("Neutralized by %s.\n", cd->teamname);
		      fflush(stdout);
		      //		      free(cd);
		      }
		    }

		/* if (AllCodedrops == NULL) */
		/*   fprintf(fp, "There was something wrong with the FA field in the message from the game server.\n"); */
		/* else { */
 		/*   printf("Code drops:\n"); */
		  
		/*   bag_t *codedropBag = bag_new(); */

		/*   char *codedrop = strtok(AllCodedrops, ":"); */

		/*   if (codedrop == NULL) { */
		/*     // list is empty? */
		/*     fprintf(fp, "There are currently no codedrops\n"); */
		/*   } */
		/*   else { */
		/*     bag_insert(codedropBag, codedrop); */

		/*     while ((codedrop = strtok(NULL, ":")) != NULL) { */
		/*       bag_insert(codedropBag, codedrop); */
		/*     } */

		/*     while ((codedrop = bag_extract(codedropBag)) != NULL) { */

		/*       codedrop_t *cd = malloc(sizeof(codedrop_t)); */
		/*       if (cd == NULL) { */
		/* 	printf("Error locating memory.\n"); */
		/* 	return 1; */
		/*       } */
		      
		/*       char *codeID = strtok(codedrop, ", "); */
		/*       if (codeID == NULL) { */
		/* 	if (logSwitch == 1) */
		/* 	  printTime(fp); */
		/* 	fprintf(fp, "Received an invalid message and ignored it.\n"); */
		/* 	return 1; */
		/*       } */
		/*       cd->hexcode = codeID; */
		      
		/*       char* lat = strtok(NULL, ", "); */
		/*       if (lat == NULL) { */
		/* 	if (logSwitch == 1) */
		/* 	  printTime(fp); */
		/* 	fprintf(fp, "Received an invalid message and ignored it.\n"); */
		/*       } */
		/*       cd->latitude = lat; */
		      
		/*       char* Long = strtok(NULL, ", "); */
		/*       if (Long == NULL) { */
		/* 	if (logSwitch == 1) */
		/* 	  printTime(fp); */
		/* 	fprintf(fp, "Received an invalid message and ignored it.\n"); */
		/*       } */
		/*       cd->longitude = Long; */
		      
		/*       char *neutralizingTeam = strtok(NULL, ", "); */
		/*       if (neutralizingTeam == NULL) { */
		/* 	if (logSwitch == 1) */
		/* 	  printTime(fp); */
		/* 	fprintf(fp, "Received an invalid message and ignored it.\n"); */
		/* 	return 1; */
		/*       } */
		/*       cd->teamname = neutralizingTeam;  */


		      /* printf("   Code drop %s: latitude is %s, longitude is %s. ", cd->hexcode, cd->latitude, cd->longitude); */
		      /* fflush(stdout); */
		      /* if (strcmp(neutralizingTeam, "NONE") == 0) */
		      /* 	printf("Has not yet been neutralized.\n"); */
		      /* else */
		      /* 	printf("Neutralized by %s.\n", cd->teamname); */
		      /* fflush(stdout); */
		  }  		    
		  }		
		} 
	      }
	    }
      
     
    
	// else if OPCODE is GAME_OVER
	else if (strcmp(OPCODE, "GAME_OVER") == 0) {
	  char *ID = strtok(NULL, "|");
	  
	  // go through everything
	  if (*gameID == NULL) {
	    fprintf(fp,"Guide agent received game over message before joining a game. Message was therefore ignored.\n");
	    //ignore; we haven't even goined a game yet
	  }
	  else if (strcmp(ID, *gameID) != 0) {
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

/* int  */
/* handleStdin(GIOChannel *gio, GIOCondition condition, gpointer data) */
char *handleStdin(int comm_sock, struct sockaddr_in *gsp, list_t *list, char *teamName)
{
  //int comm_sock, struct sockaddr_in *gsp, list_t *list, char *teamName
  
  /* standardIn_t *stdIn = data; */

  /* int comm_sock = (stdIn->comm_sock); */
  /* struct sockaddr_in *gsp = stdIn->gsp; */
  /* list_t *list = stdIn->list; */
  /* char *teamName = stdIn->teamName; */
  
  char *hint = readline(stdin);
  
  if (hint == NULL) {
    //    exit(0);
    return "EOF";
  }

  int lineLength = (int) strlen(hint);
  
  if (lineLength > 140) {
    printf("Hint must be 140 characters or less!");
    fflush(stdout);
    return hint;
    //return comm_sock;
  }

  if (gsp->sin_family != AF_INET) {
    printf("I am confused: server is not AF_INET.\n");
    fflush(stdout);
    return "0";
    //    return comm_sock;
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
    //    return comm_sock;
  }
  
  printf("Send hint to which field agent? Your active agents have the following ID's:\n");
  printf("You may also enter '*' to send to all agents on your team.\n");

  list_iterate(list, printPebbleIDs, teamName);
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
  //  return comm_sock;
}



void printGameOver(char *gameID, FILE *fp) {
  char *numRemainingCodeDrops = strtok(NULL, "|");
  char *teamRecords = strtok(NULL, "|");
  
  bag_t *recordBag = bag_new();
  char *record = strtok(teamRecords, ":");
  
  while (record != NULL) {
    bag_insert(recordBag, record);
    record = strtok(NULL, ":");
  }
  
  printf("GAME OVER! Here are the records for this game:\n");
  printf("Number of remaining unneutralized code drops is %s\n", numRemainingCodeDrops);

  char *recordPrint = bag_extract(recordBag);
  
  while (recordPrint != NULL) {

    char *teamName = strtok(recordPrint, ", ");
    printf("Team %s:\n", teamName);
    
    char *numPlayers = strtok(NULL, ", ");
    printf("   Total number of active players: %s\n", numPlayers);
    
    char *numCaptures = strtok(NULL, ", ");
    printf("   Captured %s players from other teams.\n", numCaptures);
    
    char *numCaptured = strtok(NULL, ", ");
    printf("   %s players on this team were captured.\n", numCaptured);
    
    char *numNeutralized = strtok(NULL, ", ");
    printf("   Neutralized %s code drops.\n", numNeutralized);
    
    recordPrint = bag_extract(recordBag);
  }
  free(recordBag);
  printf("Good game, everybody!");
  fflush(stdout);
}





int sendStatusRequest(char *gameID, const char *guideID, char *teamName,
	    const char *playerName, int comm_sock, const struct sockaddr *gsp)
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
    return 5;
  }

  return 0;
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


void printTime(FILE *fp)
{
  time_t myTime = time(NULL);
  char* timeString = ctime(&myTime);
  fprintf(fp, "%s ", timeString);
}


/* void initialize_window(GtkWidget* window){ */
/*   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ON_PARENT); */
/*   gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT); */
/*   gtk_window_set_title(GTK_WINDOW(window), "GTK window"); */
/*   gtk_widget_show_all(window); */
/* } */



/* static void custom_background(cairo_t* cr){ */

/*   if(!pic)pic =cairo_image_surface_create_from_png("a.png"); */

/*   cairo_set_source_surface(cr, pic, 0, 0); */
/*   cairo_paint(cr); */

/* } */

/* static void background(cairo_t* cr){ */
/*   cairo_surface_t* image = cairo_image_surface_create_from_png("a.png"); */
/*   cairo_set_source_surface(cr,image, 10, 10); */
/*   cairo_paint(cr); */

/* } */
