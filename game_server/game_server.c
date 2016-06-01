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
#include <getopt.h>	      
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
	double latitude;
	double longitude;
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
	char* gameid;
} FA_t;


//*************function prototypes*************
static void parse_arguments(const int argc, char *argv[]);
static void check_NULL(void* item);
static list_t* load_codedrops(char* , int* codedrop_num);
static location_t* new_location(double latitude, double longitude);
static void process_message(char* buffer, char* message[]);
static int validateOP (char* message[]);
static bool validate_hint(char* hint);
static void generate_hex(char ID[], int length);
static FA_t* create_player(char* teamname, struct sockaddr_in address, char* pebbleid, char* playername, double latitude, double longitude, list_t* playerlist, char gameid[]);
static team_t* create_team(char* teamname);
static GA_t* create_guide(char* teamname, struct sockaddr_in address,char* guideid ,char* playername) ;
static codedrop_t* new_codedrop(void);
static int socket_setup(int port);
static bool capture_player(FA_t* found_player);
static bool within_radius(location_t* loc1, location_t* loc2);
static void within_capture_area(void *arg, char *key, void *data, void *optional);
static void get_codedrop_info(void *arg, char* key, void *data, void *optional);
static void get_fa_info(void *arg, char* key, void *data, void *optional);
static void send_to_fa(void *arg, char* key, void *data, void* optional);
static void send_to_GA(void *arg, char* key, void *data, void* optional);
static void finish_game(void *arg, char* key, void *data, void* optional);
static void failure_log (char* message, char* id);
static void success_log (char* message, char* id);
static void logfile(char* log_this);
static bool validate_fa_message(struct sockaddr_in them, char* gameid,char* pebbleid, char* teamname, char* playername, char true_gameid[], list_t* playerlist,list_t* teamlist);
static bool validate_ga_message(struct sockaddr_in them, char* gameid,char* guideid, char* teamname,char* playername, char true_gameid[], list_t* guidelist,list_t* teamlist);
static void send_message (struct sockaddr_in address, char* message, char* id);
static bool haversine(double lat1, double lat2, double lon1, double lon2);
static void itemdelete_fa(void* data);
static void itemdelete_ga(void* data);
static void itemdelete_team(void* data);
static void itemdelete_cd(void* data);
//static void print_list_item(void *arg, char *key, void *data, void *optional);


static int comm_sock;
static bool raw_switch = false;
static char* log_message ; //buffer for log messages
static const int BUFSIZE = 8191; //max buffer size
static FILE* logp;

int main(const int argc, char *argv[])
{
	//initialize
	log_message = malloc(BUFSIZE);
	if ((logp = fopen("../log/gameserver.log", "w")) == NULL) {
    	fprintf(stderr,"could not open log file");
    	exit(4);
  	} else {
  		printf(logp,"Logfile for game of Mission Incomputable");
  	}
  	fclose(logp);
	//validate arguments
	parse_arguments(argc, argv);

	char* codedrop_path;
	//make list of codedrops files
	if (!raw_switch)
		codedrop_path = argv[1];
	else
		codedrop_path = argv[2];

	int totalFA = 0;
	int codedrop_num = 0;
	list_t* codedrop_list = load_codedrops(codedrop_path, &codedrop_num);
	
	//initialize gameID
	srand(time(NULL));
	int ID_LENGTH = (rand() % 8)+1;
	char gameID[ID_LENGTH];
	generate_hex(gameID, ID_LENGTH);
	gameID[ID_LENGTH] = '\0';
	printf("The gameID is: %s\n",gameID );

	//gameover switch
	bool over = false;

	//initialize playerlist, guidelist and teamlist. initialize table for capturing
	list_t* playerlist = list_new(itemdelete_fa);
	check_NULL(playerlist);
	list_t* guidelist = list_new(itemdelete_ga);
	check_NULL(guidelist);
	list_t* teamlist = list_new(itemdelete_team);
	check_NULL(teamlist);
	const int TABLE_SIZE = 100;
	hashtable_t *maybe_capture_table = hashtable_new(TABLE_SIZE,NULL);
	check_NULL(maybe_capture_table);

	// set up a socket to receive messages
	char* port;
	if(!raw_switch)
		port = argv[2];
	else
		port = argv[3];

  	comm_sock = socket_setup(atoi(port));

  	time_t endwait;
    time_t start = time(NULL);
    time_t seconds = 300; // end after this many second

    endwait = start + seconds;

  	while (!over && (start < endwait)) {
  		char buf[BUFSIZE];	      // buffer for reading data from socket
    	struct sockaddr_in them;  // sender's address
   		struct sockaddr *themp = (struct sockaddr *) &them;
    	socklen_t themlen = sizeof(them);
    	int nbytes = recvfrom(comm_sock, buf, BUFSIZE-1, 0, themp, &themlen);

    	//print out the list of codedrops for debugging
     	//list_iterate(codedrop_list,print_list_item,NULL,NULL);
     	//codedrop_t* coded;
		// coded = (codedrop_t*)list_find(codedrop_list,"9231");
		// if (coded != NULL) {
		// 	printf("found it\n");
		// } else {
		// 	printf("retrned null\n");
		// }

    	if (nbytes < 0) {
      		fprintf(stderr, "error receiving from socket");
    	}
    	else
    	{ 
       		buf[nbytes] = '\0';     // null terminate string

	      	//where was it from?
	     	if (them.sin_family != AF_INET)
	 			printf("From non-Internet address: Family %d\n", them.sin_family);
	     	else
	 			printf("From port %d: '%s'\n", ntohs(them.sin_port), buf);
       		if (raw_switch){
				sprintf(log_message,"received message: %s from",buf);
				logfile(log_message);
			}

			//process the buffer, make an array of tokens
			char* message[strlen(buf)];
			char* for_hint[strlen(buf)];
			memcpy(for_hint,buf,strlen(buf)+1);
			process_message(buf,message);
			int agent_type = validateOP(message);
			char* request = message[0];
			char* gameid = message[1];

			//if the message is from FA
			if (agent_type == 0) {
				char* pebbleid = message[2];
				char* teamname = message[3];
				char* playername = message[4];

				//validate gameId, pebbleId, teamName, and playerName.

				bool proceed = validate_fa_message(them, gameid, pebbleid, teamname, playername, gameID,playerlist,teamlist);

				if (proceed && strcmp(request,"FA_LOCATION") == 0) {
					double latitude = atof(message[5]);
					double longitude = atof(message[6]);
					int status = atoi(message[7]);

					if (atoi(gameid) == 0) {  //new player in game
						if (list_find(playerlist,pebbleid) == NULL){
							FA_t* new_player = create_player(teamname,them,pebbleid,playername,latitude,longitude, playerlist,gameID);
							sprintf(log_message,"A new player %s with pebbleid %s has joined",playername,pebbleid);
							logfile(log_message);
							totalFA++;
							team_t* found_team = (team_t*)list_find(teamlist,teamname);
							if (found_team == NULL) {
								//create new team
								team_t* new_team = create_team(teamname);
								list_insert(teamlist,new_team->teamname,new_team);
								list_insert(new_team->FA_list, new_player->pebbleid, new_player);
								new_team->active_players = new_team->active_players + 1;
								new_team->total_players = new_team->total_players + 1;

							} else{
								//put new player into team struct's list of FAs
								list_insert(found_team->FA_list, new_player->pebbleid, new_player);
								found_team->active_players = found_team->active_players + 1;
								found_team->total_players = found_team->total_players + 1;
							}
						}
					}
					FA_t* curr_player = list_find(playerlist,pebbleid);

					if (curr_player->status == 0) {
						curr_player->contact = time(NULL);
						curr_player->location->longitude = longitude;
						curr_player->location->latitude = latitude;
					}
					if (status == 1) {   //FA wants update
						team_t* myteam =(team_t*)list_find(teamlist,teamname);
						char* guideid;
						if(myteam->GA != NULL){
							guideid = myteam->GA->guideid;
						}else{
							guideid = "no active guide";
						}	 
						int myfas = myteam->active_players;
						int otherfas = totalFA - myfas;
						char* update = malloc(BUFSIZE);
						sprintf(update,"GAME_STATUS|%s|%s|%d|%d|%d",gameID,guideid,codedrop_num,myfas,otherfas);

						//send and log appropriatedly
						send_message(them, update, pebbleid);
						free(update);
					}
				}

				else if (proceed && strcmp(request,"FA_NEUTRALIZE") == 0) {
					double latitude = atof(message[5]);
					double longitude = atof(message[6]);
					char* codeid = message[7];

					location_t* player_loc = new_location(latitude,longitude);
					codedrop_t* cd = (codedrop_t*)list_find(codedrop_list,codeid);

					if (cd != NULL) {  //matched cd
						if ( cd->teamname == NULL &&
							within_radius(cd->location,player_loc)) {  //if not neutralized and within distance
							cd->teamname = malloc(17);
							strcpy(cd->teamname,teamname); // current player's team claims it
							team_t* neutralized_team = (team_t*)list_find(teamlist,teamname);
							neutralized_team->claimed_codes = neutralized_team->claimed_codes + 1;
							sprintf(log_message,"%s successfully neutralized codedrop %s",playername,cd->hexcode);
							logfile(log_message);

							//send success notification
							char* response = malloc(BUFSIZE);
							char* rescode = "MI_NEUTRALIZED";
							char* humanresponse = "successfully neutralized a codedrop!";
							sprintf(response, "GS_RESPONSE|%s|%s|%s", gameID, rescode, humanresponse);
							send_message(them, response, pebbleid);
							free(response);
							codedrop_num--;
						}
					} 
					free(player_loc);

				}

				else if (proceed && strcmp(request,"FA_CAPTURE") == 0){
					//get request to capture. i.e. captureID is 0
					char* captureid = message[5];
					if (strcmp(captureid, "0") == 0) {
						//loop through player list to find all players within 10m radius, not on this players team
						list_iterate(playerlist,within_capture_area,list_find(playerlist, pebbleid), maybe_capture_table);
					}
					else
					{	//if the player being captured is indeed a maybe-capture player
						FA_t* found_player = (FA_t*)hashtable_find(maybe_capture_table,captureid);
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

								sprintf(log_message,"player %s was captured by player %s",found_player->pebbleid,pebbleid);
								logfile(log_message);

								//if successful notify capturing player and capture player
								char* response = malloc(BUFSIZE);
								char* rescode = "MI_CAPTURE_SUCCESS";
								char* humanresponse = "successfully captured!";
								sprintf(response, "GS_RESPONSE|%s|%s|%s", gameID, rescode, humanresponse);
								send_message(them, response, pebbleid );
								rescode = "MI_CAPTURED";
								humanresponse = "You have been captured!";
								sprintf(response, "GS_RESPONSE|%s|%s|%s", gameID, rescode, humanresponse);
								send_message(found_player->address, response, found_player->pebbleid);
								free(response);
							}
						}
					}
				}
				FA_t* curr_player = (FA_t*)list_find(playerlist,pebbleid);
				if (curr_player != NULL)
						curr_player->address = them;
			}

			// if message is from GA
			else if (agent_type == 1) {
				char* guideid = message[2];
				char* teamname = message[3];
				char* playername = message[4];
				bool proceed = validate_ga_message(them, gameid, guideid, teamname, playername, gameID,guidelist,teamlist);

				if (strcmp(request,"GA_STATUS") == 0 && proceed) {
					int status = atoi(message[5]);
					if(atoi(gameid) == 0) {
						if (list_find(guidelist,guideid) == NULL){
							GA_t* new_guide = create_guide(teamname,them,guideid,playername);
							list_insert(guidelist,new_guide->guideid,new_guide);
							team_t* found_team = (team_t*)list_find(teamlist,teamname);
							if (found_team == NULL) {
								//create new team
								team_t* new_team = create_team(teamname);
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
								char* humanresponse = "Given teamname of GA and actual teamname of GA don't match";
								char* response = malloc(BUFSIZE);
								sprintf(response, "GS_RESPONSE|%s|%s|%s", gameID, rescode, humanresponse);
								send_message(them, response, guideid);
								free(response);
							}
						}
					}
					GA_t* curr_guide = (GA_t*)list_find(guidelist,guideid);
					if (curr_guide != NULL)
						curr_guide->contact = time(NULL);
					if (status == 1) {  //GA wants update
						char* update = malloc(BUFSIZE);
						sprintf(update,"GAME_STATUS|%s|",gameID);
						team_t* curr_team = (team_t*)list_find(teamlist,teamname);
						if (curr_team != NULL && curr_team->total_players == 0){
							sprintf(update, "%sNO_AGENTS",update);
						} else {
							list_iterate(playerlist,get_fa_info,update, NULL);   //write info for each fa to game status file
						}
						sprintf(update, "%s|",update);
						list_iterate(codedrop_list,get_codedrop_info,update, NULL); //write info for each codedrop to file

						//send update to guide agent
						send_message(them, update, guideid);
						free(update);
					}
				}

				else if (proceed && strcmp(request,"GA_HINT") == 0) {
					char* pebbleid = message[5];
					char* hint = message[6];
					if (strcmp(pebbleid,"*") == 0) {
						//send to all fas on the guide's
						team_t* found_team = (team_t*)list_find(teamlist,teamname);
						if (validate_hint(hint)) {
							list_iterate(found_team->FA_list,send_to_fa,for_hint,NULL);
						}
					}
					else {
						//verify this pebble is on ga's team and a valid message
						team_t* found_team = (team_t*)list_find(teamlist,teamname);
						if (list_find(found_team->FA_list,pebbleid) != NULL && validate_hint(hint)) {
							struct sockaddr_in address = ((FA_t*)list_find(playerlist,pebbleid))->address;
							send_message(address, (char*)for_hint, guideid);
						}
					}
					GA_t* curr_guide = (GA_t*)list_find(guidelist,guideid);
					if (curr_guide != NULL)
						curr_guide->contact = time(NULL);
				}

				GA_t* curr_player = (GA_t*)list_find(guidelist,guideid);
				if(curr_player != NULL)
						curr_player->address = them;
			}
			// invalid op
			else {
				char* rescode = "MI_ERROR_INVALID_OPCODE";
				char* humanresponse = "invalid opcode";
				char* response = malloc(BUFSIZE);
				sprintf(response, "GS_RESPONSE|%s|%s|%s", gameID, rescode, humanresponse);
				send_message(them, response, "an agent");
				free(response);
			}

			start = time(NULL);

			if (codedrop_num == 0 || (start > endwait)){
				char* notice = malloc(BUFSIZE);
		  		char* opcode = "GAME_OVER";
		  		over = true;
				sprintf(notice, "%s|%s|%d|",opcode,gameID,codedrop_num);
				list_iterate(teamlist,finish_game,notice,NULL);
				list_iterate(playerlist,send_to_fa,notice,NULL);
				list_iterate(guidelist,send_to_GA,notice,NULL);
				logfile("The game has ended.");
				free(notice);
			}
			
	  	}
	  	
	}
  close(comm_sock);
  free(log_message);
  list_delete(playerlist);
  list_delete(guidelist);
  list_delete(teamlist);
  list_delete(codedrop_list);
  hashtable_delete(maybe_capture_table);
	
}

/*******create_team******
* creates new team struct and returns pointer to the new team
*/
static team_t* create_team(char* teamname)
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
	new_team->teamname = malloc(17);
	strcpy(new_team->teamname,teamname);
	return new_team;

}

/**********create_guide*********
* Creates new guide agent struct and returns pointer to new guide agent
*/
static GA_t* create_guide(char* teamname,struct sockaddr_in address,char* guideid,char* playername) 
{
	GA_t *new_guide = malloc(sizeof(GA_t));
	check_NULL(new_guide);

	new_guide->teamname = malloc(17);
	check_NULL(new_guide->teamname);
	strcpy(new_guide->teamname,teamname); 

	new_guide->playername = malloc(17);
	check_NULL(new_guide->playername);
	strcpy(new_guide->playername,playername);

	new_guide->guideid = malloc(9);
	check_NULL(new_guide->guideid);
	strcpy(new_guide->guideid,guideid);

	new_guide->address = address;
	time_t curr_time = time(NULL);
	new_guide->contact = curr_time;
	return new_guide;

}

/******create_player****
* Creates new FA and returns pointer to new field agent
*/
static FA_t* create_player(char* teamname, struct sockaddr_in address, char* pebbleid, char* playername,
 double latitude, double longitude, list_t* playerlist, char* gameID)
{
	FA_t *new_FA = malloc(sizeof(FA_t));
	check_NULL(new_FA);
	location_t* current_location= new_location(latitude,longitude);
	check_NULL(current_location);

	new_FA->location = current_location;
	new_FA->teamname = malloc(17);
	check_NULL(new_FA->teamname);
	strcpy(new_FA->teamname,teamname);
	new_FA->address = address;

	new_FA->pebbleid = malloc(200);
	check_NULL(new_FA->pebbleid);
	strcpy(new_FA->pebbleid,pebbleid);

	new_FA->name = malloc(17);
	check_NULL(new_FA->name);
	strcpy(new_FA->name,playername);

	new_FA->status = 0;
	new_FA->capture_start = 0;
	new_FA->gameid = malloc(9);
	check_NULL(new_FA->gameid);
	strcpy(new_FA->gameid,gameID);

	if (!list_insert(playerlist,pebbleid,new_FA)){
		fprintf(stderr,"Couldn't insert player %s into playerlist\n", pebbleid);
	}
	return new_FA;

}


/*********new_location()*****
* Creates a new location struct, containinf given latitude and longitude. returns pointer to the new location.
*/
static location_t* new_location(double latitude, double longitude)
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
	codedrop->hexcode = malloc(5);
	check_NULL(codedrop->hexcode);
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

static void parse_arguments(const int argc, char *argv[])
 {
 	// If argument count is wrong exit
 	if (argc !=3 && argc!=4) {
 		fprintf(stderr, "Wrong number of arguemnts. usage: /gameserver codeDropPath GSport \n");
 		exit(1);
 	}

 	// checking switched
 	int c;
 	while((c = getopt(argc,argv,"v")) != -1){
 		switch(c) {
 			case 'v':
 				raw_switch = true;
 				break;
 			default:
 				fprintf(stderr,"invalid switch %c used\n",c);
 				exit(1);
 		}
 	}

 	//positions of others are different depending on whether we have a switch
 	char* codedrop_file;
 	char* port;
 	if (!raw_switch){
 		codedrop_file = argv[1];
 		port = argv[2];
 	}
 	else {
 		codedrop_file = argv[2];
 		port = argv[3];
 	}

 	//Log starting info
	char* message = malloc(BUFSIZE);
	sprintf(message,"the codedropfile is %s, port is %s\n",codedrop_file, port);
	logfile(message);
	free(message);

 	// If codedrop file cannot be opened, exit
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
	int toke_count = 0;
	char* token = strtok(buffer, "|");
	if (token == NULL) {
		fprintf(stderr,"not a valid message");
	}

	//Keep filling the message array until the message is in the array
	int pos = 0;
	while (token != NULL) {
		toke_count++;
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
	list_t* codedrop_list = list_new(itemdelete_cd);
	FILE *fp = fopen(codedrop_path, "r");
	//for each line in this file create a new codedrop struct
	char* codedrop_info;
	while((codedrop_info = readline(fp)) != NULL) {
		double latitude = atof(strtok(codedrop_info, ","));
		double longtitude = atof(strtok(NULL, ","));
		char* hexcode = strtok(NULL, " ");
		location_t *location = new_location(latitude,longtitude);//new location struct for this codedrop
		codedrop_t* codedrop = new_codedrop();					//initialize new codedrop with correct information
		strcpy(codedrop->hexcode,hexcode);
		codedrop->location = location;
		list_insert(codedrop_list, hexcode, codedrop);			// add new code drop to list of codedrops
		*codedrop_num = (*codedrop_num + 1);
		free(codedrop_info);
  	}
  	fclose(fp);
  	
  	return codedrop_list;
}

/********print_list_item*******
* Used for debugging purposes to print out the codedrop list
*/

// static void print_list_item(void *arg, char *key, void *data, void* optional) {
	
// 	codedrop_t* current_codedrop= (codedrop_t*)data;
// 	printf("%s and %s ",key,current_codedrop->hexcode);
// 	printf("long:%.13f lat:%.13f\n",current_codedrop->location->longitude,current_codedrop->location->latitude);
// }

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
		(strcmp(maybe_capture_player->teamname,capturing_player->teamname) != 0) &&
		(maybe_capture_player->status == 0)) {
		const int LENGTH = 4;
		char rand_hex[LENGTH];
		generate_hex(rand_hex,LENGTH);
		rand_hex[LENGTH] = '\0';
		char* response = malloc(BUFSIZE);
		sprintf(response,"GS_CAPTURE_ID|%s|%s",maybe_capture_player->gameid,rand_hex);
		send_message(maybe_capture_player->address, response, maybe_capture_player->pebbleid);
		free(response);   
		hashtable_insert(maybe_capture_table,rand_hex,maybe_capture_player);
		maybe_capture_player->capture_start = time(NULL);
		maybe_capture_player->status = 1;
	}

}

/****within_radius***
* Checks whether two locations are within 10m radius of each other.
*/
static bool within_radius(location_t* loc1, location_t* loc2)
 {	
 	return haversine(loc1->latitude,loc2->latitude,loc1->longitude,loc2->longitude);
}

/******* haversine****
* Calculates great circle distance returns true if such distance is smaller than  10
*/
static bool haversine(double lat1, double lat2, double lon1, double lon2) {
    // get differences first
    int EARTH_RADIUS = 6373000;
    double pi = 3.141592653589793;
    double clat1 = (pi/180)*lat1;
    double clat2 = (pi/180)*lat2;
    double clon1 = (pi/180)*lon1;
    double clon2 = (pi/180)*lon2;
    double dlon = clon1 - clon2; // difference for longitude
    double dlat = clat1  - clat2; // difference for latitude
 
    double a = pow(sin(dlat/2.0), 2.0) + cos(clat1) * cos(clat2) * pow(sin(dlon/2.0), 2.0);

    double b = 2 * atan2(sqrt(a), sqrt(1 - a));
    //the great-circle distance between two locations
    double result = EARTH_RADIUS * b;
    return (result < 10.0);
}

/******capture_player*****
* Check whether a play can be captured successfully
* by looking at the current time and time the player entered maybe-capture mode
*/
static bool capture_player(FA_t* found_player)
{
	time_t curr = time(NULL);
	if (found_player->status == 1) {
		if (curr - found_player->capture_start < 60) {
			found_player->status = 2;
			return true;
		}
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
	sprintf(update,"%s%s, %s, %s, %s, %f, %f, %d:",update,curr_fa->pebbleid,curr_fa->teamname,curr_fa->name,curr_status,curr_fa->location->latitude,curr_fa->location->longitude,timesincecontact);
	
}

/*******get_codedrop_info********
* Finds the information needed for a game status update of a codedrop
*
*/
static void get_codedrop_info(void *arg, char* key, void *data, void* optional)
{
	codedrop_t* cd = (codedrop_t*)data;
	char* update= (char*)arg;
	sprintf(update,"%s:%s,%.13f,%.13f",update,cd->hexcode,cd->location->latitude,cd->location->longitude);
	if (cd->teamname == NULL) {
		sprintf(update,"%s,NONE",update);
	}
	else {
		sprintf(update,",%s%s",update,cd->teamname);
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


/*******send_to_fa()*****
* itemfunc for list iterate to send hints to all fas on a team
*/
static void send_to_fa(void *arg, char* key, void *data, void* optional)
{
	char* hint = (char*)arg;
	FA_t* myfa= (FA_t*)data;
	send_message(myfa->address, hint, myfa->pebbleid);
}


/*******finish_game()****
* Helps create the string that contains the wanted info of all teams at the end of the game
* to send in the GAME_OVER message
*/
static void finish_game(void *arg, char* key, void *data, void* optional)
{
	char* info = (char*)arg;
	team_t* curr_team = (team_t*)data;
	sprintf(info,"%s:%s,%d,%d,%d,%d",info,key,curr_team->total_players, curr_team->captures, curr_team->captured_players, curr_team->claimed_codes);
}

/*****send_to_GA()******
* Send game over status to an GA agent
* used by list iterator to send notice to all agents
*/
static void send_to_GA(void *arg, char* key, void *data, void* optional)
{
	char* notice = (char*)arg;
	GA_t* ga = (GA_t*)data;
	send_message(ga->address, notice, ga->guideid);

}

/****logfile***
* writes a message to logfile
*/
static void logfile(char* log_this)
{
	logp = fopen("../log/gameserver.log", "a");
  	
  	if (!raw_switch) {
    	fprintf(logp, "%s\n",log_this);
  	}
  	else {
    	// user input -v, so we also give them the time of each action
    	time_t myTime = time(NULL);
    	char* timeString = ctime(&myTime);
    	fprintf(logp, "%s ",timeString);
    	fprintf(logp, "%s\n",log_this);
  	}

  	fclose(logp);
}

//logs failed to send messaged
static void failure_log (char* message, char* id)
{	
	if (raw_switch){
		sprintf(log_message,"error sending in datagram: %s to %s\n",message,id);
		logfile(log_message);
	}
}

// logs successfully send messages
static void success_log (char* message, char* id)
{
	if (raw_switch){
		sprintf(log_message,"successfully sent datagram: %s , to %s\n",message,id);
		logfile(log_message);
	}
}

/******validate_ga_message******
* validates ga the different components of a message from  GA
* returns false if message is not valid
*/
static bool validate_fa_message(struct sockaddr_in them, char* gameid,char* pebbleid, char* teamname, char* playername, char true_gameid[], list_t* playerlist, list_t* teamlist)
{
	char* newgame = "0";
	char* response = malloc(BUFSIZE);
	FA_t* fa = (FA_t*)list_find(playerlist,pebbleid);
	
	//incorrect game id
	if ((strcmp(gameid, true_gameid) != 0) && (strcmp(gameid, newgame) != 0)) {
		sprintf(response,"GS_RESPONSE|%s|MI_ERROR_INVALID_GAME_ID|invalid gameid, gameid should be %s",true_gameid,true_gameid);
		send_message(them,response,pebbleid);
		free(response);
		return false;
		
	}

	//agent doesnt exist
	else if( fa == NULL && (strcmp(gameid, newgame) != 0)){
		sprintf(response, "GS_RESPONSE|%s|MI_ERROR_INVALID_ID|this fa does not exist", true_gameid);
		send_message(them,response,pebbleid);
		free(response);
		return false;
	}

	//existing agent wanting to "rejoin"
	else if ( fa != NULL && (strcmp(gameid, newgame) == 0)){
		sprintf(response, "GS_RESPONSE|%s|MI_ERROR_INVALID_ID|This agent already exists in the game", true_gameid);
		send_message(them,response,pebbleid);
		free(response);
		return false;
	}

	//team doesnt exist
	else if((strcmp(gameid, newgame) != 0) && ((list_find(teamlist,teamname)) == NULL)) {
		sprintf(response, "GS_RESPONSE|%s|MI_ERROR_INVALID_TEAMNAME|team doesn't exist",true_gameid);
		send_message(them,response,pebbleid);
		free(response);
		return false;
	}

	//teamname doesnt match existing team of pebble
	else if((strcmp(gameid, newgame) != 0) && (strcmp(fa->teamname,teamname) != 0)) {
		sprintf(response, "GS_RESPONSE|%s|MI_ERROR_INVALID_TEAMNAME|the teamname of given fa(%s) and given teamname(%s) does not match", true_gameid,fa->teamname,teamname);
		send_message(them,response,pebbleid);
		free(response);
		return false;
	}

	else if(fa != NULL && fa->status != 0){
		free(response);
		return false;
	}

	free(response);
	return true;
}

/******validate_ga_message******
* validates ga the different components of a message from  GA
* returns false if message is not valid
*/
static bool validate_ga_message(struct sockaddr_in them,char* gameid,char* guideid, char* teamname, char* playername, char true_gameid[], list_t* guidelist, list_t* teamlist)
{
	char* newgame = "0";
	char* response = malloc(BUFSIZE);
	GA_t* ga = (GA_t*)list_find(guidelist,guideid);
	//gameid doesnt match
	if ((strcmp(gameid, true_gameid) != 0) && (strcmp(gameid, newgame) != 0)) {
		sprintf(response,"GS_RESPONSE|%s|MI_ERROR_INVALID_GAME_ID|invalid gameid, gameid should be %s",true_gameid,true_gameid);
		send_message(them,response,guideid);
		free(response);
		return false;
		
	}
	//GA doesnt exists
	else if( ga == NULL && (strcmp(gameid, newgame) != 0)){
		sprintf(response, "GS_RESPONSE|%s|MI_ERROR_INVALID_ID|this ga does not exist", true_gameid);
		send_message(them,response,guideid);
		free(response);
		return false;
	}

	//existing agent wanting to "rejoin"
	else if ( ga != NULL && (strcmp(gameid, newgame) == 0)){
		sprintf(response, "GS_RESPONSE|%s|MI_ERROR_INVALID_ID|This agent already exists in the game", true_gameid);
		send_message(them,response,guideid);
		free(response);
		return false;
	}

	//found guideagent's team does not match the given team name
	else if((strcmp(gameid, newgame) != 0) && (strcmp(ga->teamname,teamname) != 0)) {
		sprintf(response, "GS_RESPONSE|%s|MI_ERROR_INVALID_TEAMNAME|the teamname of given ga and given teamname does not match", true_gameid);
		send_message(them,response,guideid);
		free(response);
		return false;
	}

	//found guideagents ID does not match the given team's guide's ID
	team_t* found_team = (team_t*)list_find(teamlist,teamname);
	if (found_team != NULL){
		if((strcmp(found_team->GA->guideid,guideid)) != 0){
			free(response);
			return false;
		}
	}

	free(response);
	return true;

}

/****send_message***
* Helper function for sending messages
*/
static void send_message (struct sockaddr_in address, char* message, char* id) {
	 if (sendto(comm_sock,message, BUFSIZE, 0, (struct sockaddr *) &address, sizeof(address)) < 0) {
		failure_log(message,id);
		fprintf(stderr,"failed send");
	} else {
		success_log(message,id);
		fprintf(stderr,"success send: %s\n\n",message);
	}
}

/****itemdelete_fa()***
* used to delete the field agent list
*/
static void itemdelete_fa(void* data) {

	FA_t* fa = (FA_t*)data;
	free(fa->name);
	free(fa->pebbleid);
	free(fa->gameid);
	free(fa->teamname);
	free(fa->location);
	free(fa);
}

/****itemdelete_ga()***
* used to delete the guidelist
*/
static void itemdelete_ga(void* data) {

	GA_t* ga = (GA_t*)data;
	free(ga->guideid);
	free(ga->playername);
	free(ga->teamname);
	free(ga);
}

/****itemdelete_team()***
* used to delete the teamlist
*/
static void itemdelete_team(void* data) {

	team_t* team = (team_t*)data;
	free(team->teamname);
	if (team->FA_list != NULL)
		list_delete(team->FA_list);
	free(team);
}

/****itemdelete_cd()***
* used to delete the codedrop list
*/
static void itemdelete_cd(void* data) {
	codedrop_t* code = (codedrop_t*)data;
	free(code->hexcode);
	free(code->location);
	if(code->teamname != NULL)
		free(code->teamname);
	free(code);
}

