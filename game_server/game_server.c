/*
 * game_server.c - written for final project, mission incomputable, cs50 Spring 2016
 *
 * This is the game server for mission incomputable. It sends and received datagrams from field agents(pebble watches)
 * and guide agents. The game server allows new players to join the game, processes code neutralizations and captures,and
 * sends status updates to field agents and guide agents. During the game, the gam server simply waits for messages from 
 * the field agents and gudie agents
 *
 * Shirley Zhang, May 2016 (Team Lapis)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>	     
#include <time.h>
#include <unistd.h>	      
#include <strings.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <arpa/inet.h>	      
#include "../lib/counters/counters.h" 
#include "../lib/list/list.h"
#include "../lib/hashtable/hashtable.h"
#include "../common/word.h" 
#include "../common/file.h"
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
	list_t* FA_list;
	struct GA *GA;
	bool status;
	int active_players;
	int total_players;
	int claimed_codes;
	int captured_players;
	int captures;
} team_t;

typedef struct GA {
	char* teamname; 
	struct sockaddr_in address;
	time_t contact;
	char* guideid;
	char* playername;
} GA_t;

typedef struct FA {
	char* teamname; 
	struct sockaddr_in address;
	int status;                 // 0 is active, 1 is maybe-capture, 2 is captured
	char* pebbleid;
	char* name;
	struct location *location;
	time_t capture_start;       // place to hold the start time of capturing for an FA
	time_t contact;
} FA_t;


//*************function prototypes*************
static void parse_arguments(const int argc, char *argv[], bool* raw_switch);
static void check_NULL(void* item);
static list_t* load_codedrops(char* , int* codedrop_num);
static location_t* new_location(float latitude, float longitude);
static void process_message(char* buffer, char* message[]);
static int validateOP (char* message[]);
static bool validate_hint(char* hint);
static void generate_hex(char ID[], int length);
static FA_t* create_player(char* teamname, struct sockaddr_in address, char* pebbleid, char* playername, float latitude, float longitude, list_t* playerlist);
static team_t* create_team(void);
static GA_t* create_guide(char* teamname, struct sockaddr_in address,char* guideid ,char* playername) ;
static codedrop_t* new_codedrop(void);
static int socket_setup(int port);
static bool capture_player(FA_t* found_player);
static bool within_radius(location_t* loc1, location_t* loc2);
static void within_capture_area(void *arg, char *key, void *data, void *optional);
static void get_codedrop_info(void *arg, char* key, void *data, void *optional);
static void get_fa_info(void *arg, char* key, void *data, void *optional);
static void print_list_item(void *arg, char *key, void *data, void *optional);
static void send_hints(void *arg, char* key, void *data, void* optional);
static void send_gameover_GA(void *arg, char* key, void *data, void* optional);
static void send_gameover_FA(void *arg, char* key, void *data, void* optional);
static void finish_game(void *arg, char* key, void *data, void* optional);

static int nbytes;
static int comm_sock;

int main(const int argc, char *argv[])
{
	bool raw_switch = false;
	//validate arguments
	parse_arguments(argc, argv,&raw_switch);

	//make list of codedrops files
	char* codedrop_path = argv[1];
	int totalFA = 0;
	int codedrop_num = 0;
	list_t* codedrop_list = load_codedrops(codedrop_path, &codedrop_num);
	list_iterate(codedrop_list,print_list_item,NULL,NULL);
	printf("%d\n",codedrop_num );
	
	//initialize gameID
	int ID_LENGTH = rand()%9;
	char gameID[ID_LENGTH];
	generate_hex(gameID, ID_LENGTH);

	//initialize playerlist, guidelist and teamlist. initialize table for capturing
	list_t* playerlist = list_new(NULL);
	check_NULL(playerlist);
	list_t* guidelist = list_new(NULL);
	check_NULL(guidelist);
	list_t* teamlist = list_new(NULL);
	check_NULL(teamlist);
	const int TABLE_SIZE = 100;
	hashtable_t *maybe_capture_table = hashtable_new(TABLE_SIZE,NULL);
	check_NULL(maybe_capture_table);

	// set up a socket on which to receive messages
  	comm_sock = socket_setup(atoi(argv[2]));

  	// Receive datagrams and handles appropriately
  	int BUFSIZE = 8000;
  	while (true) {
  		char buf[BUFSIZE];	      // buffer for reading data from socket
    	struct sockaddr_in them;  // sender's address
   		struct sockaddr *themp = (struct sockaddr *) &them;
    	socklen_t themlen = sizeof(them);
    	nbytes = recvfrom(comm_sock, buf, BUFSIZE-1, 0, themp, &themlen);

    	if (nbytes < 0) {
      		fprintf(stderr, "error receiving from socket");
    	}
    	else
    	{ 
       		buf[nbytes] = '\0';     // null terminate string

	      	// where was it from?
	     	if (them.sin_family != AF_INET)
	 			printf("From non-Internet address: Family %d\n", them.sin_family);
	     	else
	 			printf("From port %d: '%s'\n", ntohs(them.sin_port), buf);  

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

				if (strcmp(request,"FA_LOCATION") == 0) {
					float latitude = atof(message[5]);
					float longitude = atof(message[6]);
					int status = atoi(message[7]);

					if (gameid == 0) {  //new player in game
						if (list_find(playerlist,pebbleid) == NULL){
							FA_t* new_player = create_player(teamname,them,pebbleid,playername,latitude,longitude, playerlist);
							totalFA++;
							team_t* found_team = (team_t*)list_find(teamlist,teamname);
							if (found_team == NULL) {
								//create new team
								team_t* new_team = create_team();
								list_insert(teamlist,new_team->teamname,new_team);
								new_team->active_players = new_team->active_players + 1;
								new_team->active_players = new_team->total_players + 1;
							}
							//put new player into team struct's list of FAs
							list_insert(found_team->FA_list, new_player->pebbleid, new_player);
							found_team->active_players = found_team->active_players + 1;
						}
					}
					FA_t* curr_player = list_find(playerlist,pebbleid);

					if (curr_player->status == 0) {
						curr_player->contact = time(NULL);
					}
					if (status == 1) {   //FA wants update
						team_t* myteam =(team_t*)list_find(teamlist,teamname);
						char* guideid = myteam->GA->guideid;
						int myfas = myteam->active_players;
						int otherfas = totalFA - myfas;
						char* update = malloc(BUFSIZE);
						sprintf(update,"GAME_STATUS|%s|%s|%d|%d|%d",gameid,guideid,codedrop_num,myfas,otherfas);
						if (sendto(comm_sock,update, nbytes, 0, (struct sockaddr *) &them, sizeof(them)) < 0)
								fprintf(stderr,"error sending in datagram socket");
						free(update);
					}
				}

				else if (strcmp(request,"FA_NEUTRALIZE") == 0) {
					float latitude = atof(message[5]);
					float longitude = atof(message[6]);
					char* codeid = message[7];
					location_t* player_loc = new_location(latitude,longitude);
					codedrop_t* cd;
					if ((cd = (codedrop_t*)list_find(codedrop_list, codeid)) != NULL) {  //matched cd
						if ( cd->teamname == NULL &&
							within_radius(cd->location,player_loc)) {  //if not neutralized and within distance
							cd->teamname = teamname; // current player's team claims it
							team_t* neutralized_team = (team_t*)list_find(teamlist,teamname);
							neutralized_team->claimed_codes = neutralized_team->claimed_codes + 1;
							//send success notification
							char* response = malloc(BUFSIZE);
							char* rescode = "MI_NEUTRALIZED";
							char* humanresponse = "successfully neutralized a codedrop!";
							sprintf(response, "GS_RESPONSE|%s|%s|%s", gameID, rescode, humanresponse);
							if (sendto(comm_sock,"MI_NEUTRALIZED", nbytes, 0, (struct sockaddr *) &them, sizeof(them)) < 0)
								fprintf(stderr,"error sending in datagram socket");
							free(response);
							codedrop_num--;
						}
					} 
				}

				else if (strcmp(request,"FA_CAPTURE") == 0){
					//get request to capture. i.e. captureID is 0
					char* captureid = message[5];
					if (atoi(captureid) == 0) {
						//loop through player list to find all players within 10m radius, not on this players team
						list_iterate(playerlist,within_capture_area,list_find(playerlist, pebbleid), maybe_capture_table);
					}
					else
					{	//if the player being captured is indeed a maybe-capture player
						FA_t* found_player = hashtable_find(maybe_capture_table,captureid);
						if (found_player != NULL) {
							//attempt to capture it
							if (capture_player(found_player)){
								team_t* captured_pteam = list_find(teamlist,teamname);
								captured_pteam->active_players = captured_pteam->active_players - 1;
								captured_pteam->captured_players = captured_pteam->captured_players + 1;
								totalFA--;
								if (captured_pteam->active_players == 0)
									captured_pteam->status = false;
								team_t* curr_team = (team_t*)list_find(teamlist,teamname);
								curr_team->captures = curr_team->captures +1;
								//if successful notify capturing player and capture player
								char* response = malloc(BUFSIZE);
								char* rescode = "MI_CAPTURE_SUCCESS";
								char* humanresponse = "successfully captured!";
								sprintf(response, "GS_RESPONSE|%s|%s|%s", gameID, rescode, humanresponse);
								if (sendto(comm_sock,response, nbytes, 0, (struct sockaddr *) &them, sizeof(them)) < 0)
								fprintf(stderr,"error sending in datagram socket");
								rescode = "MI_CAPTURED";
								humanresponse = "You have been captured!";
								sprintf(response, "GS_RESPONSE|%s|%s|%s", gameID, rescode, humanresponse);
								if (sendto(comm_sock,"MI_CAPTURED", nbytes, 0, (struct sockaddr *) &(found_player->address), sizeof(found_player->address)) < 0)
								fprintf(stderr,"error sending in datagram socket");
								free(response);
							}
						}
					}
				}
			}

			// if message is from GA
			else if (agent_type == 1) {
				char* guideid = message[2];
				char* teamname = message[3];
				char* playername = message[4];
				if (strcmp(request,"GA_STATUS") == 0) {
					int status = atoi(message[5]);
					if(gameid == 0) {
						if (list_find(guidelist,guideid) == NULL){
							GA_t* new_guide = create_guide(teamname,them,guideid,playername);
							list_insert(guidelist,new_guide->guideid,new_guide);
							team_t* found_team = (team_t*)list_find(teamlist,teamname);
							if (found_team == NULL) {
								//create new team
								team_t* new_team = create_team();
								new_team->GA = new_guide;
								list_insert(teamlist,new_team->teamname,new_team);
							}else{
							//put new guide player as team's guide
							found_team->GA = new_guide;
							}
						}
						else {
							if(strcmp(((GA_t*)list_find(guidelist,guideid))->teamname, teamname) != 0){
								char* rescode = "MI_ERROR_INVALID_TEAMNAME";
								char* humanresponse = "invalid teamname";
								char* response = malloc(BUFSIZE);
								sprintf(response, "GS_RESPONSE|%s|%s|%s", gameID, rescode, humanresponse);
								if (sendto(comm_sock,response, nbytes, 0, (struct sockaddr *) &them, sizeof(them)) < 0)
									fprintf(stderr,"error sending in datagram socket");
							}

						}
					}
					GA_t* curr_guide = (GA_t*)list_find(guidelist,guideid);
					if (curr_guide != NULL)
						curr_guide->contact = time(NULL);
					if (status == 1) {  //GA wants update
						char* update = malloc(BUFSIZE);
						sprintf(update,"GA_STATUS|%s|",gameID);

						list_iterate(playerlist,get_fa_info,update, NULL);   //write info for each fa to game status file
						sprintf(update, "%s|",update);
						list_iterate(codedrop_list,get_codedrop_info,update, NULL); //write info for each codedrop to file
						if (sendto(comm_sock,update, nbytes, 0, (struct sockaddr *) &them, sizeof(them)) < 0)
								fprintf(stderr,"error sending in datagram socket");
						fprintf(stderr, "%s\n", update);
						free(update);
					}
				}

				else if (strcmp(request,"GA_HINT") == 0) {
					char* pebbleid = message[5];
					char* hint = message[6];
					if (strcmp(pebbleid,"*") == 0) {
						//send to all fas on the guide's
						team_t* found_team = (team_t*)list_find(teamlist,teamname);
						if (validate_hint(hint)) {
							list_iterate(found_team->FA_list,send_hints, buf, NULL);
						}
					}
					else {
						//verify this pebble is on ga's team and a valid message
						team_t* found_team = (team_t*)list_find(teamlist,teamname);
						if (list_find(found_team->FA_list,pebbleid) != NULL && validate_hint(hint)) {
							struct sockaddr_in address = ((FA_t*)list_find(playerlist,pebbleid))->address;
							if (sendto(comm_sock,buf, nbytes, 0, (struct sockaddr *) &address, sizeof(address)) < 0)
									fprintf(stderr,"error sending in datagram socket");
						}
					}
					GA_t* curr_guide = list_find(guidelist,guideid);
					if (curr_guide != NULL)
						curr_guide->contact = time(NULL);
				}
			}

			// invalid op
			else {
				char* rescode = "MI_ERROR_INVALID_OPCODE";
				char* humanresponse = "invalid opcode";
				char* response = malloc(17+strlen(gameID)+strlen(rescode)+strlen(humanresponse));
				sprintf(response, "GS_RESPONSE|%s|%s|%s", gameID, rescode, humanresponse);
				if (sendto(comm_sock,response, nbytes, 0, (struct sockaddr *) &them, sizeof(them)) < 0)
					fprintf(stderr,"error sending in datagram socket");
				
			}
	  	}
	  	if (codedrop_num == 0){
			char* notice = malloc(BUFSIZE);
	  		char* opcode = "GAME_OVER";
			sprintf(notice, "%s|%s|",opcode,gameID);
			list_iterate(teamlist,finish_game,notice,NULL);
			list_iterate(playerlist,send_gameover_FA,notice,NULL);
			list_iterate(guidelist,send_gameover_GA,notice,NULL);
			free(notice);
		}
	}
  close(comm_sock);
	
}

/*******create_team******
* creates new team struct and returns pointer to the new team
*/
static team_t* create_team()
{
	team_t *new_team = malloc(sizeof(team_t));
	check_NULL(new_team);
	list_t* FA_list = list_new(NULL);
	check_NULL(FA_list);

	//initialize new team with an empty FA list, no guide and status as active
	new_team->FA_list = FA_list;
	new_team->GA = NULL;
	new_team->status = true;
	new_team->active_players = 0;
	new_team->total_players = 0;
	new_team->claimed_codes = 0;
	new_team->captured_players = 0;
	new_team->captures = 0;
	return new_team;

}

/**********create_guide*********
* Creates new guide agent struct and returns pointer to new guide agent
*/
static GA_t* create_guide(char* teamname,struct sockaddr_in address,char* guideid,char* playername) 
{
	GA_t *new_guide = malloc(sizeof(GA_t));
	check_NULL(new_guide);

	new_guide->teamname = teamname; 
	new_guide->playername = playername;
	new_guide->guideid = guideid;
	new_guide->address = address;
	time_t curr_time = time(NULL);
	new_guide->contact = curr_time;
	return new_guide;

}

/******create_player****
* Creates new FA and returns pointer to new field agent
*/
static FA_t* create_player(char* teamname, struct sockaddr_in address, char* pebbleid, char* playername,
 float latitude, float longitude, list_t* playerlist)
{
	FA_t *new_FA = malloc(sizeof(FA_t));
	check_NULL(new_FA);
	location_t* current_location= new_location(latitude,longitude);
	check_NULL(current_location);

	new_FA->location = current_location;
	new_FA->teamname = teamname;
	new_FA->address = address;
	new_FA->pebbleid = pebbleid;
	new_FA->name = playername;
	new_FA->status = 0;
	new_FA->capture_start = 0;
	if (!list_insert(playerlist,playername,new_FA)){
		fprintf(stderr,"Couldn't insert player %s into playerlist\n", pebbleid);
	}
	return new_FA;

}


/*********new_location()*****
* Creates a new location struct, containinf given latitude and longitude. returns pointer to the new location.
*/
static location_t* new_location(float latitude, float longitude)
{
	location_t *location = malloc(sizeof(location_t));
	check_NULL(location); 
	location->latitude = latitude;
	location->longitude = longitude;
  	return location;
}

/*********new_codedrop()*******
* Creates a codedrop struct and returns pointer to the ncodedrop
*/
static codedrop_t* new_codedrop()
{
	codedrop_t* codedrop = malloc(sizeof(codedrop_t));
	check_NULL(codedrop);
	codedrop->status = false;
	codedrop->hexcode = NULL;
	codedrop->location = NULL;
	codedrop->teamname = NULL;
	return codedrop;
}

/******generate_hex*****
* Generates hexcode
*
*/ 
static void generate_hex(char ID[], int length)
{
	const char *hex_digits = "0123456789ABCDEF";
    for( int i = 0 ; i < length; i++ ) {
      ID[i] = hex_digits[ ( rand() % 16 ) ];
    }
}

/*********parse_arguments***********
* Validates commandline arguments. Checks count is correct, codedrop file is readable
* and game server port number is the one assigned to team lapis
*/

static void parse_arguments(const int argc, char *argv[], bool* raw_switch)
 {
 	// If argument count is wrong exit
 	if (argc !=3 && argc!=4) {
 		fprintf(stderr, "Wrong number of arguemnts. usage: /gameserver codeDropPath GSport \n");
 		exit(1);
 	}

 	// int c;
 	// if((c = getopt(argc,argv,"v")) != -1){
 	// 	if (c == "v")
 	// 		*raw_switch = true;
 	// 	else{
 	// 		fprintf(stderr,"invalid switch %c used\n",c);
 	// 		exit(1);
 	// 	}
 	// }

 	// If codedrop file cannot be opened, exit
 	char* codedrop_file = argv[1];
 	if (codedrop_file == NULL) {
 		fprintf(stderr, "codedrop path cannot be NULL \n");
 		exit(2);
 	}

 	FILE* fp = fopen(codedrop_file, "r");
 	if (fp == NULL) {
 		fprintf(stderr, "cannot open codedrop file \n");
 		exit(3);
 	}
 	fclose(fp);
 }

/******check_NULL()****
* Checks for malloc failures. Exits game if malloc indeed failed
*/
static void check_NULL(void* item)
{
	if (item == NULL){
		fprintf(stderr, "malloc failure");
		exit(4);
	}

}

/**********validateOP*******
* Checks that incoming message has a valid OP
*
*/
static int validateOP (char* message[])
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
* Fills an array with words from incoming message
*/
static void process_message(char* buffer, char* message[])
{
	char* token = strtok(buffer, "|");
	if (token == NULL) {
		fprintf(stderr,"not a valid message");
	}

	//Keep filling the message array until the message is in the array
	int pos = 0;
	while (token != NULL) {
   		message[pos] = token;
    	token = strtok(NULL,"|"); // get next word
    	pos++;
    }

}

/*************load_codedrops*************
* Reads each line in the codedrop file and initializes a new struct for each codedrop and its location.
* Adds the new codedrop structs to a list of codedrops. The list of codedrop has key as the hexcode
* and data as the corresponding codedrop struct.
*/
static list_t* load_codedrops(char* codedrop_path, int* codedrop_num)
{
	list_t *codedrop_list = list_new(NULL); ///FREEEEEEEE, and add deletefunc
	FILE *fp = fopen(codedrop_path, "r");
	//for each line in this file create a new codedrop struct
	char* codedrop_info;
	while((codedrop_info = readline(fp)) != NULL) {
		printf("here about to make codedrop %d\n",*codedrop_num);
		float latitude = atof(strtok(codedrop_info, ","));
		float longtitude = atof(strtok(NULL, ","));
		char* hexcode = strtok(NULL, ",");
		location_t *location = new_location(latitude,longtitude);//new location struct for this codedrop
		codedrop_t* codedrop = new_codedrop();					//initialize new codedrop with correct information
		codedrop->hexcode = hexcode;
		codedrop->location = location;
		list_insert(codedrop_list, hexcode, codedrop);			// add new code drop to list of codedrops
		*codedrop_num = (*codedrop_num + 1);
  	}
  	free(codedrop_info);
  	return codedrop_list;
}

//  ********print_list_item*******
static void print_list_item(void *arg, char *key, void *data, void* optional) {
	printf("%s",key);
	codedrop_t* current_codedrop= (codedrop_t*)data;
	printf("long:%.13f lat:%.13f\n",current_codedrop->location->longitude,current_codedrop->location->latitude);
}

/**************** socket_setup ****************/
/* All the ugly work of preparing the datagram socket;
 * exit program on any error.
 */
static int socket_setup(int port)
{
	// Create socket on which to listen (file descriptor)
	int comm_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (comm_sock < 0) {
		fprintf(stderr,"opening datagram socket");
	 	exit(5);
	}

	// Name socket using wildcards
	struct sockaddr_in server;  // server address
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);
	if (bind(comm_sock, (struct sockaddr *) &server, sizeof(server))) {
		fprintf(stderr,"binding socket name");
		exit(5);
	}

	// Print our assigned port number
	socklen_t serverlen = sizeof(server);	// length of the server address
	if (getsockname(comm_sock, (struct sockaddr *) &server, &serverlen)) {
		fprintf(stderr,"getting socket name");
		exit(5);
	}
	printf("Ready at port %d\n", ntohs(server.sin_port));

	return (comm_sock);
}

/********within_capture_area****
* Checks whether a player is within the capturing area
* sends the player a hexcode and keep track of this player
* by putting player into maybe_capture table with hexcode as key and the player
* pointer as data. Gives the player struct a startcapture time   
*/
static void within_capture_area(void *arg, char *key, void *data, void *optional)
{
	FA_t* capturing_player = (FA_t*)arg;
	FA_t* maybe_capture_player = (FA_t*)data;
	hashtable_t* maybe_capture_table = (hashtable_t*)optional;
	//if maybe capture player is within 10min radius, not on same team, and active player, send hexcode
	if (within_radius(capturing_player->location,maybe_capture_player->location) &&
		strcmp(maybe_capture_player->teamname,capturing_player->teamname) != 0 &&
		maybe_capture_player->status == 0) {
		const int LENGTH = 8;
		char rand_hex[LENGTH];
		generate_hex(rand_hex,LENGTH);
		if (sendto(comm_sock,rand_hex, nbytes, 0, (struct sockaddr *) &(maybe_capture_player->address), sizeof(maybe_capture_player->address)) < 0)
			fprintf(stderr,"error sending in datagram socket");
		else {
			hashtable_insert(maybe_capture_table,rand_hex,maybe_capture_player);
			maybe_capture_player->capture_start = time(NULL);
		}
	}

}

/****within_radius***
* Checks whether two locations are within 10m radius of each other.
*/
static bool within_radius(location_t* loc1, location_t* loc2)
{
	return (pow((loc1->latitude - loc2->latitude),2.0)+ pow((loc1->longitude - loc2->longitude),2.0) < 100);
}

/******capture_player*****
* Check whether a play can be captured successfully
* by looking at the current time and time the player entered maybe-capture mode
*/
static bool capture_player(FA_t* found_player)
{
	time_t curr = time(NULL);
	if (curr - found_player->capture_start < 60) {
		found_player->status = 2;
		return true;
	}
	return false;
}

/*******get_fa_info********
* Finds the information needed for a game status
*
*/
static void get_fa_info(void *arg, char* key, void *data, void* optional)
{
	FA_t* curr_fa = (FA_t*)data;
	char* update=(char*)arg;
	char* curr_status;
	if (curr_fa->status != 2) {
		curr_status = "active";
	}
	else{
		curr_status = "captured";
	}
	int timesincecontact = time(NULL) - curr_fa->contact;
	sprintf(update,"%s%s,%s,%s,%s,%f,%f,%d:",update,key,curr_fa->teamname,curr_fa->name,curr_status,curr_fa->location->latitude,curr_fa->location->longitude,timesincecontact);
	
}

/*******get_codedrop_info********
* Finds the information needed for a game status update of a codedrop
*
*/
static void get_codedrop_info(void *arg, char* key, void *data, void* optional)
{
	codedrop_t* cd = (codedrop_t*)data;
	char* update= (char*)arg;
	sprintf(update,"%s:%s,%f,%f,",update,cd->hexcode,cd->location->latitude,cd->location->longitude);
	if (cd->teamname == NULL) {
		sprintf(update,"%sNONE",update);
	}
	else {
		sprintf(update,"%s%s:",update,cd->teamname);
	}
}
/*********validate_hint()********
* Makes sure the hint from a GA is proper. i.e. 1-140 printable characters
*
*/
static bool validate_hint(char* hint)
{
	if (strlen(hint) < 0 || strlen(hint) > 140)
		return false;
	for (int i = 0 ; i < strlen(hint); i++){
		if (!isprint(hint[i]))
			return false;
	}
	return true;

}


/*******send_hints()*****
* itemfunc for list iterate to send hints to all fas on a team
*/
static void send_hints(void *arg, char* key, void *data, void* optional)
{
	char* hint = (char*)arg;
	FA_t* myfa= (FA_t*)data;
	if (sendto(comm_sock,hint,nbytes, 0,(struct sockaddr *) &(myfa->address), sizeof(myfa->address) < 0))
		fprintf(stderr,"error sending in datagram socket");
}


/*******finish_game()****
* Helps create the string that contains the wanted info of all team at the end of the game
* to send in the GAME_OVER message
*/
static void finish_game(void *arg, char* key, void *data, void* optional)
{
	char* info = (char*)arg;
	team_t* curr_team = (team_t*)data;
	sprintf(info,"%s:%s,%d,%d,%d,%d",info,key,curr_team->total_players, curr_team->captures, curr_team->captured_players, curr_team->claimed_codes);
}

/*****send_gameover_FA()******
* Send game over status to an GA agent
* used by list iterator to send notice to all agents
*/
static void send_gameover_FA(void *arg, char* key, void *data, void* optional)
{
	char* notice = (char*)arg;
	FA_t* fa = (FA_t*)data;
	if (sendto(comm_sock,notice,nbytes, 0, (struct sockaddr *) &(fa->address), sizeof(fa->address) < 0))
		fprintf(stderr,"error sending in datagram socket");
}

/*****send_gameover_GA()******
* Send game over status to an GA agent
* used by list iterator to send notice to all agents
*/
static void send_gameover_GA(void *arg, char* key, void *data, void* optional)
{
	char* notice = (char*)arg;
	GA_t* ga = (GA_t*)data;
	if (sendto(comm_sock,notice,nbytes, 0, (struct sockaddr *) &(ga->address), sizeof(ga->address) < 0))
		fprintf(stderr,"error sending in datagram socket");

}

// static void logfile(char* log_this, bool raw)
// {
// 	FILE *logp;
//   	if ((logp = fopen("../log/gameserver.log", "w")) == NULL) {
//     	printf("could not open log file");
//     	exit(4);
//   	}
//   	else if (!raw) {
//     	fprintf(logp, "%s",log_this);
//   	}
//   	else {
//     	// user input -v, so we also give them the time of each action
//     	time_t myTime = time(NULL);
//     	char* timeString = ctime(&myTime);
//     	fprintf(logp, "%s %s", timeString,log_this);
//   	}
// }


