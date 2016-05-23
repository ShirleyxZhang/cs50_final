/*
 * game_server.c - written for final project, mission incomputable, cs50 Spring 2016
 *
 * This is the game server for mission incomputable. It sends and received datagrams from field agents(pebble watches)
 * and guide agents. The game server allows new players to join the game, processes code neutralizations and captures,and
 * sends status updates to field agents and guide agents. During the game, the gam server simply waits for messages from 
 * the field agents and gudie agents
 *
 * Shirley Zhang, May 2016 (Teap Lapis)
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>	      // bool type
#include <unistd.h>	      // read, write, close
#include <strings.h>	      // bcopy, bzero
#include <arpa/inet.h>	      // socket-related calls


typedef struct codedrop {
  bool status;
  char* hexcode;
  struct location *location;
  char* teamname;
} codedrop_t;

typedef struct location {
	float latitude;
	float longitude;
} location_t;

typedef struct team {
	char* teamname;
	list_t *FA_list;
	struct GA *GA;
	bool status;
} team_t;

typedef struct GA {
	char* teamname; 
	char* address;
	float contact;
} GA_t;

typedef struct FA {
	char* teamname; 
	char* address;
	bool capture;
	char* pebbleid;
	char* name;
	struct location *location;
	float capture_start;       // place to hold the start time of capturing for an FA
	float contact;
} GA_t;

 static void parse_arguments(const int argc, char *argv[]);
 static list_t* load_codedrops(char* codedrop_path);



int main(const int argc, char *argv[])
{
	parse_arguments(argc, argv);
	char* codedrop_path = argv[1];
	list_t *codedrop_list = load_codedrops(codedrop_path);
	//count number of codedrops

	//initialize stuff
	const int ID_LENGTH = 8
	char gameID[ID_LENGTH];
	generate_ID(gameID);

	list_t *playerlist = list_new();
	if (playerlist == NULL) {
		fprintf(stderr, "Could not make playerlist")
		exit(4);
	}

	list_t *guidelist = list_new();
	if (guidelist == NULL) {
		fprintf(stderr, "Could not make guidelist")
		exit(4);
	}

	list_t *teamlist = list_new();
	if (teamlist == NULL) {
		fprintf(stderr, "Could not make teamlist")
		exit(4);
	}


     // Create socket on which to listen (file descriptor)
    int comm_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (comm_sock < 0) {
   	 perror("opening datagram socket");
   	 exit(5);
  	}

  	// Name socket using wildcards
  	struct sockaddr_in server;  // server address
  	server.sin_family = AF_INET;
  	server.sin_addr.s_addr = INADDR_ANY;
  	server.sin_port = htons(argv[2]);
  	if (bind(comm_sock, (struct sockaddr *) &server, sizeof(server))) {
    	perror("binding socket name");
    	exit(5);
  	}

  	// Print our assigned port number
 	 socklen_t serverlen = sizeof(server);	// length of the server address
 	if (getsockname(comm_sock, (struct sockaddr *) &server, &serverlen)) {
 		perror("getting socket name");
    	exit(5);
  	}
  	printf("Ready at port %d\n", ntohs(server.sin_port));
  
  	// Receive datagrams and handles appropriately
  	while (true) {
  		char buf[BUFSIZE];	      // buffer for reading data from socket
    	struct sockaddr_in them;  // sender's address
   		struct sockaddr *themp = (struct sockaddr *) &them;
    	socklen_t themlen = sizeof(them);
    	int nbytes = recvfrom(comm_sock, buf, BUFSIZE-1, 0, themp, &themlen);

    	if (nbytes < 0) {
      		fprintf(stderr, " error receiving from socket");
    	}
    	else {      
      		buf[nbytes] = '\0';     // null terminate string

	     	// where was it from?
	    	if (them.sin_family != AF_INET)
				printf("From non-Internet address: Family %d\n", them.sin_family);
	    	else
				printf("From %s, port %d: '%s'\n", net_ntoa(them.sin_addr), ntohs(them.sin_port), buf);

			//process the buffer, make an array of tokens
			char* message[strlen(buf)];
			process_message(buf,message);
			//check the request is valid
			int agent_type = validateOP(message);
			char* request = message[0];
			char* gameid = message[1];

			//if the message is from FA
			if (agent_type == 0) {
				char* pebbleid = message[2];
				char* teamname = message[3];
				char* playername = message[4];

				//validate gameId, pebbleId, teamName, and playerName. Then

				switch(request) {
					case "FA_LOCATION":
						float latitude = message[5];
						float longitude = message[6];
						int status = message[7];

						if (game == 0) {  //new player in game
							if (list_find(playerlist,pebbleid) == NULL){
								FA_t* new_player = create_player(teamname,(struct sockaddr *) &them,pebbleid,playername,latitude,longitude);
								team_t* found_team = (team_t*)list_find(teamlist,teamname);
								if (found_team == NULL) {
									//create new team
									team_t* new_team = create_team();
								}
								//put new player into team struct's list of FAs
								list_insert(found_team->FA, new_player->pebbleid, new_player);
							}
						}
						FA_t* curr_player = list_find(playerlist,pebbleid);

						if (!(curr_player->capture) {
							curr_player->contact = time();
						}
						if (status == 1) {   //FA wants update
							//send update
						}


					case "FA_NEUTRALIZE":
						float latitude = message[5];
						float longitude = message[6];
						char* codeid = message[7];
						if ((cd = (codedrop_t*)list_find(codedrop_list)) != NULL) {  //matched cd
							if ( cd->teamname == NULL &&
								(abs(cd->latitude - latitude) < 10)&&
								(abs(cd->latitude - latitude) < 10)) {  //if not neutralized and within distance
								cd->teamname == teamname; // current player's team claims it
								//send success notification
								if (sendto(comm_sock,"MI_NEUTRALIZED", nbytes, 0, (struct sockaddr *) &them, sizeof(them)) < 0)
									fprintf(stderr, "%s\n", );("error sending in datagram socket");
								//decrement number of code drops, check is game is over if number = 0
							}
						} 



					case "FA_CAPTURE":
						//get request to capture. i.e. captureID is 0
						//loop through player list to find all players within 10m radius, not on this players team
						//send each a random hexcode
						//start time for each player. createstart = time()
						// player-> capturestarttime
						//hashtable - key-uniquehex - insert player stuct as data 
						
						//else get request to capture a player
						// lookuphahstable to find palyer corresponding to the hexcode. check capturestarttime and find
						//difference to current time, if less than 60, change player status to inactive(capture= true)
						// tap into captured player's teamname. find team struct in team list, check status of all FAs in the team
						// if all are inactive- set team to inactive.
						//MI_CAPTURE_SUCCESS
						//MI_CAPTURED


					
				}
			}

			// if message is from GA
			else if (agent_type == 1) {
				char* guideid = message[2];
				char* teamname = message[3];
				char* playername = message[4];
				switch(request) {
					case "GA_STATUS":
						int status = message[5];
						if(gameid == 0) {
							if (list_find(guidelist,guideid) == NULL){
								FA_t* new_guide = create_guide(teamname,(struct sockaddr *) &them,guide,playername,);
								team_t* found_team = (team_t*)list_find(teamlist,teamname);
								if (found_team == NULL) {
									//create new team
									team_t* new_team = create_team();
								}
								//put new guide player as team's guide
								list_insert(found_team->GA, new_guide->guideid, new_guide);
							} else {
								if(strcmp(list_find(guidelist,guideid)->teamname, teamname) != 0)
									//error message
							}

						}
						
						GA_t* curr_guide = list_find(guidelist,guideid);

						if (curr_guide->GA != NULL) {
							curr_guide->contact = time();
						}
						if (status == 1) {   //GA wants update
							//send update
						}


					case "GA_HINT":
				}
			}

			// invalid op
			else {
				if (sendto(comm_sock,"MI_ERROR_INVALID_OPCODE", nbytes, 0, (struct sockaddr *) &them, sizeof(them)) < 0)
									fprintf(stderr, "%s\n", );("error sending in datagram socket");
				
			}
		}
	}
  close(comm_sock);
	
}

/*******create_team******
* creates new team
*/
static team_t* create_team()
{
	team_t *new_team = malloc(sizeof(team_t));
	if (new_team == NULL){
		fprintf(stderr, "couldn't make new team as malloc failed\n");
		exit(4);
	}
	list_t* FA_list = list_new(NULL);
	if (FA_list == NULL){
		fprintf(stderr, "couldn't make FA list for team as malloc failed\n");
		exit(4);
	}

	//initialize new team with an empty FA list, no guide and status as active
	new_team->FA = FA_list;
	new_team->GA = NULL;
	new_team->status = true;
	return new_team;

}

/******create_player****
* Creates new FA
*/
static FA_t* create_player(char* teamname, char* address, char* pebbleid, char* playername, float latitude, float longitude)
{
	FA_t *new_FA = malloc(sizeof(FA_t));
	if (new_FA == NULL){
		fprintf(stderr, "coudln't make new FA as malloc failed\n");
		exit(4);
	}
	location_t* current_location= new_location(latitude,longitude);
	if (current_location == NULL) {
		fprintf(stderr, "coudln't make new FA as malloc for location failed\n");
		exit(4);
	}
	new_FA->location = current_location;
	new_FA->teamname = teamname;
	new_FA->address = address;
	new_FA->pebbleid = pebbleid;
	new_FA->name = playername;
	new_FA->capture = false;
	new_FA->capture_start = NULL;
	if (!list_insert(playerlist,playername,new_FA)){
		fprintf("Couldn't insert player %s into playerlist\n", pebbleid);
	}
	else {
		return new_FA;
	}

}

/******generate_ID*****
* Generates random 8 hexdecimal ID
*
*/ 
static void generate_ID(char ID[], int length) {
	const char *hex_digits = "0123456789ABCDEF";
    int i;
    for( i = 0 ; i < 8; i++ ) {
      ID[i] = hex_digits[ ( rand() % 16 ) ];
    }
}

/**********validateOP*******
* Checks that incoming message has a valid OP
*
*/
static void validateOP (char* message[])
{
	char* OPCODE = message[0];
	if (strcmp(OPCODE, "FA_LOCATION") == 0 ||
		strcmp(OPCODE, "FA_NEUTRALIZE") == 0 ||
		strcmp(OPCODE, "FA_CAPTURE") == 0)
		return 0;
	else if(strcmp(OPCODE, "GA_STATUS") == 0 ||
			strcmp(OPCODE, "GA_HINT") == 0)
		return 1;
	else
		return -1;
}

/*********process_message()********
* Fills an array with words from message
*/
static void process_message(char* buffer, char* message[])
{
	char* token = strtok(buffer, "|");
	if (token == NULL) {
		fprinf(stderr,"not a valid message");
	}

	//Keep filling the message array until the message is in the array
	int pos = 0;
	while (token != NULL) {
   		message[pos] = token;
    	token = strtok(NULL," "); // get next word
    	pos++;
    }

}

static *location_t new_location(float latitude, float longitude)
{
	location_t *location = malloc(sizeof(location_t));
	if (location == NULL){
		fprintf("Unexpected error. Could not make a location struct for a codedrop\n");
		exit(4);
	} 
	else {
		location->latitude = latitude;
		location->longtitude = longtitude;
	}
  	return location;
}

static *codedrop_t new_codedrop() {
	codedrop_t* codedrop = malloc(sizeof(codedrop_t));
	if (codedrop == NULL) {
		fprintf("Unexpected error. Could not make codedrop struct");
		exit(4);
	}
	else {
		codedrop->status = false;
		codedrop->hexcode = NULL;
		codedrop->location = NULL;
		codedrop->teamname = NULL
	}
	return codedrop;

}

/*************load_codedrops*************
* Reads each line in the codedrop file and initializes a new struct for each codedrop and its location.
* Adds the new codedrop structs to a list of codedrops. The list of codedrop has key as the hexcode
* and data as the corresponding codedrop struct.
*/
static (*list_t load_codedrops(char* codedrop_path)
{
	list_t *codedrop_list = list_new(NULL); ///FREEEEEEEE, and add deletefunc
	FILE *fp = fopen(codedrop_path, "r");
	//for each line in this file create a new codedrop struct
	char* codedrop_info;
	while((codedrop_info = readline(fp)) != NULL) {
		float latitude = strtok(codedrop_info, ",");
		float longtitude = strtok(NULL, ",");
		char* hexcode = strtok(NULL ",")
		location_t *location = new_location(latitude,longtitude);   //new location struct for this codedrop
		codedrop_t *new_codedrop = new_codedrop();					//initialize new codedrop with correct information
		new_codedrop->hexcode = hexcode;
		new_codedrop->location = location;
		list_insert(codedrop_list, hexcode, new_codedrop);			// add new code drop to list of codedrops
  	}
  	free(codedrop_info);
  	return codedrop_list;
}

/*********parse_arguments***********
* Validates commandline arguments. Checks count is correct, codedrop file is readable
* and game server port number is the one assigned to team lapis
*/

static void parse_arguments(const int argc, char *argv[])
 {
 	// If argument count is wrong exit
 	if (argc != 3) {
 		fprintf(stderr, "Wrong number of arguemnts. usage: /gameserver codeDropPath GSport \n");
 		exit(1);
 	}

 	// If codedrop file cannot be opened, exit
 	char* codedrop_file = argv[1];
 	if (codedrop_file == NULL) {
 		fprintf(stderr, "codedrop path cannot be NULL \n");
 		exit(2);
 	}
 	FILE *fp = fopen(codedrop_file, "r");
 	if (fp == NULL) {
 		fprintf(stderr, "cannot open codedrop file \n");
 		exit(2);
 	} else {
 		close(fp);
 	}

 	struct hostent *hostp = gethostbyname(argv[3]);
  	if (hostp == NULL) {
    	fprintf(stderr, "unknown host '%s'\n",argv[3]);
    	exit(3);
  	}
 }