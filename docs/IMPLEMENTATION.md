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

1) The validity/active state of the field agent is checked 

2) Checks the connunication between the Game Server and Field Agent 
as updates are continuously sent every 15 seconds. 
  
3) Setup the SDK & Set WatchFaces

4) Set up Text Layers, Windows, & Screen Layers {Order of the stack currently
   working on}

--These data structures are not incorperated by the field agent, but rather 
how the data is stored and checked. 

  Screen 1: Allows the user to input a player name of their choosing 
      Key: Pebble ID
      Data: FA Struct
      
5) Set up a secondary window 

  Screen 2: Allows the user to input a team name of their choosing 
        (Team List)
        Key: Team Name
        DATA: Team Struct

6) The third window gives the user the option to neutralize, capture, request update or look at hints.

  NEUTRALIZE- Here the user is a able to enter a 4 digit hex code and check to see if the code trying to be neutralized is in the correct proximity and is being sent through OP codes

  CAPTURE- Here the user is able to enter a players id code if they are able to effectively send the right op code to the server, and once handled, this would allow a field agent to capture another person.

  REQUEST UPDATE- Updates common variables and allows users to get updates on a shorter time stamp reather than every 15 seconds.

  HINTS -  Just an animate window with the most up to date data from the server

                
7) Option 1: After the code is typed into a text box, the op code is sent to the game server and if validated, the message sent back will be parsed and will let me know the success value of my actions. 



-If not found return error message else return success
message, this occurs in my message handlers

-If already in inactive state return message 

Option 2:
If in the proximity of another player; Press Capture 

-Possibly Based on the location Data and the data from the (Team List), this
 would only work if this check is met

-Sends a signal to the Game Server to relay the ID of the player being captured; THis is no longer overwatched by a 60 second timer, but on the pebble a new window pops up indicating this informationa and new game state.

-Once captured Player is made Inactive(PlayerList)


    
-Checks on the proximity

-Checks on if the ID number entered is accurate

-Incorporation of a 60 second time limitation

-Check on if a player has already been captured

-Check on if all players of one team have been captured

7) Deinit all uses of layers, windows, and text layers

8) Buttons & Layered Fields

  May want to incorporate: 
    -basic_update_proc
    -config_provider
    -haptic feedback: backlit display & vibrations 
9) Check the connection between the Game Server and Game Agent 

  *Process/Display the Data Grams 
  *Incorporate this data into the application

10) Full check on exit status and shared data 



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

