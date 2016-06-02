#Final Project- Impelmentation Spec
Team: Lapis
Shirley Zhang, Deven Orie, Drew Waterman

***** Game Server *****

1) Program parses command line arguments using parse_arguments function.
   Checks for wrong number of arguments, unreadable codedrop filepath. Checks for raw log switch

2) Uses load_codedrops function to read from the codedrop file and create a
   codedrop struct for each. In each code drop there is also a pointer to
   location struct which stores the float values of longitude and latitude of
   the codedrop. Each struct is stored in a list of codedrops. List is a
   module written in lab 3 with key as a string and data as void* , which in
   this case is the codedrop struct. Also create a count of unneutralized
   codedrops.

3) Initialize playerlist(list that contains pebbleid as key, and FA struct as
   data), guidelist(list that contains guideid as key and GA struct as data),
   teamlist(list that contains teamName as key and team struct as data),
   hashtable for keeping track of maybe-capture agents(key is hexcode, data is
   pointer to maybe-capture FA struct).

4) Sets up sockets using createSocket function. A while loop will continue to
   run until the game ends and it will listen for datagrams.

5) For each message, process it by breaking it down into the parts separated
   by pipes using function process_message.

6) Validate opcode, see whether message is from GA or FA.

7) If message if from FA, validate gameId, pebbleId, teamName, and playerName,
   and then check what the opcode is , process appropriately using switches.

  - FA_ LOCATION: If FA is new to the game, use create_player function to
    initialize a new FA with information from message. Then, either add to the
    given team’s existing FA list or create a new team, add to new team’s FA
    list and and insert new team into team list. Check the status token and
    give updates if requested

  - FA_ NEUTRALIZE: list_ find the codeid that came in the message. If NULL is
    not returned then the code is valid. Then check this code is not already
    neutralized by looking into the corresponding codedrop structs teamname
    field. If teamname is NULL and the absolute difference between the codedrop
    location and position of the FA is less than 10m, then this is a valid
    neutralization. Change the code’s status to neutralized and decrement the
    counter for remaining codedrops. Send success message back.

  - FA_ CAPTURE:
    - * If captureID is 0, loop through player list to find all players within
      10m radius and not on this FA’s team. Send each a random hexcode using
      helper function generate_ random_ hex. Use time() to get start time for
      capturing. Give this start time to each maybe-capture FA’s capture_ start
      field. Insert in a hashtable with key as the random hex and data as
      pointer to each maybe-capture player struct.
    - * If request to capture a player, lookup hashtable to find player
      corresponding to the hexcode. check capture start time and find
      difference with current time, if less than 60, change player status to
      inactive. Look into the captured player’s teamname. Find corresponding
      team struct in team list, check status of all FAs in the team, if all
      are captured, team is inactive. Send message to the capturing FA and
      captured FA using sendto.

8) If message is from GA, validate the gameId, guideId, teamName, and
   playerName, and then check what the opcode, process appropriately using
   switches.  
   
 - GA_ STATUS: If GA doesn’t exist, create a new GA and add to existing team
   or create new team. If GA does exist, check the team name sent in matches
   the team name of the GA already stored in the guidelist. If GA requested
   update loop through player list to obtain update appropriate information
   and send to GA.

 - GA_HINT: Lookup the pebbleid in playerlist and send the message to that FA
   using sendto and the stored address of the FA in the FA struct.

9) Send out updates when requested. Send update to each FA by going to team
   list and looping through the FA list in each team struct and update info.
   Send this updated info, along with codedrop info to each FA in that team.
   Repeat for all teams. After, send information about the whole game to each
   GA by going through guidelist.

10) End game and clean up memory appropriately and close socket when all
    codedrops have been neutralized or the timer has run out, send the ending message to all the players
    by looping through playerlist and GA list.


***** Field Agent *****

1) If the game server and proxy is not active, updates will not run corretly

2) As soon as the application is opened the user will be prompted to enter his/her user-name & team-name. These names are customizeable to the user and are not a pre-set list.

3) After there input options, the timer_callback is called in the code and the location is constantly updated and this message is sent to the server every 15 seconds. 
  
4) The user will then be able to Neutralize a Code, Capture a Player, See the last hint sent to them, or request an update. 

* When the user neutralizes a code it is sent to the server in a construcutred OP code that follows a strict standard

* The OP code is stringed together and if the location and code match the data on the server, then a successful screen should pop up for a short duration.

5) Once the user clicks the capture button, a unique OP Capture Code is sent to the game server. This initializes the game server to prepare for a capture code.
  *When a capture code is then inputed and sent, if successful, a message will be displayed.

6) When a user clicks on the hints menu option, they would see the last hint that was sent to the.

7) Request an update, sends a FA_LOCATION message to the server and allows for updates to take place in terms of the variables and their values.



  NEUTRALIZE- Here the user is a able to enter a 4 digit hex code and check to see if the code trying to be neutralized is in the correct proximity and is being sent through OP codes

  CAPTURE- Here the user is able to enter a players id code if they are able to effectively send the right op code to the server, and once handled, this would allow a field agent to capture another person.

  REQUEST UPDATE- Updates common variables and allows users to get updates on a shorter time stamp rather than every 15 seconds.

  HINTS -  Just an animate window with the most up to date data from the server

8) If being captured, the field agent will know that it is being captured due to the parsed code sent from the game server to the pebble.
  * A screen will display for 60 seconds alerting the player that he/she is being captured
  * A code will also be on the screen where the player who was in the proximity & click captured is then able to go in, type in the code, and successfully neutralize this player.
              
-If not found return error message else return success
message, this occurs in my message handlers

-If already in inactive state return message 

-Sends a signal to the Game Server to relay the ID of the player being captured;

* Information parsed from the OP Code sent from the game server 

* Incorporation of a 60 second time limitation

* Check on if a player has already been captured

* Check on if all players of one team have been captured


8) Buttons & Layered Fields

    -menu layers, windows, text layers
    -config_provider
    -handlers
    -callbacks
    -mesaging system to use API & Allow Communications

9) Check the connection between the Game Server and Game Agent 

  *Process/Display the  selected Data
    -hints, capture message, neutraliation message 

10) Checks on the init and deint

11) Check on the information being parsed and the information that is sent 

***** Guide Agent *****

1) Program checks to make sure that the number of arguments is correct and
   parses the argument, and checks to see if the user used the -v option

2) Program calls getGuideID() -- prompts the user to enter a 8-digit
   hexadecimal ID for the guide agent. If the user does not enter a valid ID,
   the function keeps prompting the user until they enter a good one, at which
   point it assigns the guide agent that ID and stops prompting.

3) startASCII -- calls the necessary functions to start the ASCII interface and
   prints a welcoming message that the Guide Agents sees until the program
   receives input from the socket.

4) Create a socket using the GShost and GSport command line arguments.

5) Send a message to the Game Server to announce the Guide Agent's presence and
   join the game.

6) Open up a file for logging and initialize pebbleList, a list that will hold
   pebble ID's as keys and the corresponding Field Agent's team name as data.

6) Wait in an infinite while loop for datagrams from the game server and for
   input from the guide agent simultaneously.
     * If a datagram comes in from the game server:
       	  * Parse the message
	  * Ignore any messages with OPCODES that the program does not
	    recognize (anything other than GAME_STATUS, GAME_OVER and
	    GS_RESPONSE)
	  * If the message is valid (i.e. it contains all of the expected
	    fields) and is a GAME_STATUS message:
	    * If it’s the first message received, store the game ID relayed in
	      the message into a gameID variable
            * Update the appropriate interface from the information from the
	      status update from the game server
	  * If the message is valide and is a GAME_OVER message:
	    * Print the relevant stastistics that were sent in the message, and
	      tell the user to exit the game.
          * If the message is not valid, ignore it and don't print anything to
	    the user interface
     
     * If a datagram comes in from the guide agent/stdin, the user either wants
       to send a hint or exit the program
       	  * If the user entered 'quit', exit the interface and quit the program
       	  * Otherwise, they entered a hint, so check to make sure the hint is
	    valid (is not too long, is not a blank line)
	  * If it is, print out a list of pebble ID's belonging to active Field
	    Agents on the Guide's team. If there are none, tell the user and
	    don't send the hint. Otherwise, print out the list and have the
	    user enter in the pebbleID of the agent they want to send the hint
	    to.
	  * Then send the hint in an appropriate GA_HINT message to the Game
	    Server

7) For every message that is sent/received in the game, log the activity to a
   log file, guideagent.log.
   
8) When the user exits the game, clean up any malloced memory that has not yet
   been freed and close the socket and the log file. 

