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
#include <gtk/gtk.h>

static int createSocket(int argc, char *argv[],
			struct sockaddr_in *gsp, int logSwitch);
bool isNumber(const char *arg);
char *getPlayerName(void);
char *getGuideID(void);
static void handleSocket(int comm_sock, struct sockaddr_in *gsp, list_t *list);
char *handleStdin(int comm_sock, struct sockaddr_in *gsp);
//char * getPebbleID(char *hint, list_t *FAList);
  

static int BUFSIZE = 8000;


static void
print_hello (GtkWidget *widget,
	     gpointer   data)
{
  g_print ("Hello World\n");
}


static void activate (GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *button;
  GtkWidget *button_box;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);


  button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add (GTK_CONTAINER (window), button_box);

  button = gtk_button_new_with_label ("Hello World");
  g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_container_add (GTK_CONTAINER (button_box), button);

  gtk_widget_show_all (window);
}




int main(int argc, char *argv[]) {

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


  const char *playerName = getPlayerName();
  //  printf("player name is %s\n", playerName);
  if (strcmp(playerName, "EOF") == 0) {
    printf("\n");
    exit(0);
  }
      
  const char *guideID = getGuideID();
  //  printf("guideId is %s\n", guideID);
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

  printf("initial message is: %s\n", message);
  
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


  printf("before all this shit\n");
  
  GtkApplication *app;
  int status;

  printf("application_new\n");
  app = gtk_application_new ("guide.agent", G_APPLICATION_FLAGS_NONE);

  printf("g_signal_connect\n");
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);

  printf("g_application_run\n");
  status = g_application_run (G_APPLICATION (app), argc, argv);

  printf("g_object_unref\n");
  g_object_unref (app);

  
  printf("status is %d\n", status);
  // return status;
  


  /* initialize struct for field agents on team so we have access to pebbleID's
   * potentially a list of pebbleID's
   */
  list_t FAList = list_new(NULL);
  

  /*
   * put pebbleID's somewhere in interface so GA has access to them and knows where
   * to send them
   */

  
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
	char *hint = handleStdin(comm_sock, &gs);
	// if hint is NULL, do nothing
	// else
	// scan hint to get pebbleID
	// char *pebbleID = getPebbleID(hint, FAList);
	//    scans hint for pebbleID, tells user it's invalid if there is no FA with that pebble ID
	// if the returned ID is null, don't send the message
	// otherwise, send the message
	
	if (strcmp(hint, "EOF") == 0)
	  break;
	/* char * message = malloc(strlen(hint) + 14 + strlen(gameID) + strlen(guideID) + strlen(teamName) + strlen(playerName) + strlen(pebbleID); */
      }

      if (FD_ISSET(comm_sock, &rfds)) {
	// handle socket
	handleSocket(comm_sock, &gs, FAList);
      }
    }
  }

  close(comm_sock);
  putchar('\n');
  fclose(logp);
  exit(0);

}




/* Parse arguments and set up socket
 */
static int createSocket(int argc, char *argv[],
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

    if (playerName == NULL)
      return "EOF";

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



static void
handleSocket(int comm_sock, struct sockaddr_in *gsp, list_t *list)
{

  // GAME_STATUS|gameId|fa1:fa2:...:faN|cd1:cd2:...:cdM
  // do something with gameID??

  // use strtok
  //    fa field (update interface to represent location/status of FA's):
  //    update list of fa's so we know who on the game is on our team and available
  //    to receive hints
  //       use strtok again, separated by colons
  //          use strtok again, separated by commas
  //    cd field (update interface to represent location/status of cd's):
  //       use strtok again, separated by colons
  //          use strtok again, separated by commas  
  
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

	// do stuff
	printf("%s\n", buf);
	
      }
      else {
	// don't do stuff?
      }
    }
  }
}




char *
handleStdin(int comm_sock, struct sockaddr_in *gsp)
{
  char *hint = readline(stdin);
  if (hint == NULL)
    return "EOF";

  //  char *message = malloc(strlen(hint) + 

  if (gsp->sin_family != AF_INET) {
    printf("I am confused: server is not AF_INET.\n");
    fflush(stdout);
    return "0";
  }

  if (strlen(hint) > 140) {
    printf("Hint must be 140 characters or less!");
    return NULL;
  }

  else if (sendto(comm_sock, hint, strlen(hint), 0, (struct sockaddr *) gsp, sizeof(*gsp)) < 0){
    perror("error sending datagram\n");
    exit(5);
  }
  //  free(hint);
  
  return hint;
  
}




/* char * getPebbleID(char *hint, list_t *FAList) */
/* { */
/*   char *ID; */
/*   sscanf(hint, "%s ", ID); */

/*   // check to see if pebbleID is in our known list of FA's */
/*   // if not, prompt user for different hint and return NULL */

/* } */
