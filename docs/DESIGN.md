#Final Project- Design Spec
Team: Lapis
Shirley Zhang, Deven Orie, Drew Waterman

##Requirement spec
Refer to the requirements spec at: http://www.cs.dartmouth.edu/~cs50/Labs/Project/
And details about each program at:
 http://www.cs.dartmouth.edu/~cs50/Labs/Project/mission.html

##User Interface
###Field agent
The user interacts with the field agent app through the pebble

###Guide Agent
An example of the command-line interface for the guide agent is as follows:

       $ ./guide lapis flume.cs.dartmouth.edu 23383

###Gamer Server
An example of the command-line interface for the game server is as follows:

       $ ./gameserver [-v] mycodedrops 23383

##Inputs and Outputs
###Field Agent
Input:

* 4-digit hex code for capturing agents
* 4-digit hex code for neutralizing code
* messages from game server including:
  - notification this agent has been captured
    - hints from guide agent


Output:

* location of current agent, and request for updates (FA_LOCATION)
* Notification of capture (FA_CAPTURE)
* Notification of neutralization (FA_NEUTRALIZE)

###Guide Agent
Input:

* Command-line parameters, see User Interface above
* Updates of current status and location  of each player on Guide’s team
* Notifications of new players, players captured, and drops neutralized

Output:

* Hints for other field agents (GA_HINT)
* Request for update and request to join game (GA_STATUS)

###Game Server
Input:

* command-line parameters, see User Interface above
* FA’s request to capture a nearby agent
* 4-digit hex code from a capturing FA
* GA's requests to forward a hint to a given teammate
* request from new GA or FA to join game
* request from FA or GA for game updates
* request from FA to neutralize drop code

Output:

* hints from GA to FA
* game number response to each new player
* game status updates to GA including:
  * game statistics: elapsed time, number of active agents, number of active teams, number of code drops yet to neutralize.
    * location and status of all code drops.
      * location, name, team, and status of all field agents.

* game status updates to FA including:
  * gameId, the game number.
  * guideId is the identifier of this team’s Guide Agent, or 0 if not yet known.
  * numRemainingCodeDrops is number of remaining (active) code drops.
  * numFriendlyOperatives is the number of friendly operatives (i.e., FAs from your team) still active (i.e., excluding captured agents).
  * numFoeOperatives is the number of foe operatives (i.e., FAs from other teams) still active (i.e., excluding captured agents).

###Functional Decomposition into Modules
####Field Agent
We anticipate the following modules or functions:
1. main, which waits for the user to interact with the pebble
2. UpdateInfo, send location updates to GS
3. RequestInfo, send request to GS for updates
4. CaptureAgent, sends hexcode to capture
5. NeutralizeCode, sends hexcode to neutralize codedrop

####Guide Agent
We anticipate the following modules or functions:
1. main, which waits for datagrams from GS
2. writeLog, records messages to guideagent.log
3. RequestInfo, send request to GS for updates

####Game Server
We anticipate the following modules or functions:  
1. parse_ arguments - parses command line arguments, chooses log options  
2. check_ NULL- ensures no malloc failures  
3. load_ codedrops- uses codedrop file to load  
4. new_ location- creates location struct used for containing latitude and longitude  
5.process_ message- breaks incoming message into parts for processing  
6.validateOP- makes sure OP is valid  
7.validate_ hint- makes sure hint is 140 characters or less and printable  
8.generate_ hex- generates hexcode   
9.create_ player - creates fa  
10. create_ team - creates a team  
11. create_ guide - creates a ga  
12. new_ codedrop - makes a new codedrop   
13. socket_ setup - sets up socket   
14. capture_ player - attempts to capture a player by looking up the hashtable of maybe- capture players   
15. within_ radius - checks if two things are within 10 meters of each other  
16. within_ capture_ area- checks for all players that are close enough to be in maybe- capture  
17. get_ codedrop_ info - iterates through codedroplis to get information about codedrops to send to ga  
18. get_ fa_ info - iterates through playerlist to get info to send to ga  
19. send_ to_ fa- iterates through playerlist and send each a message
20. send_ to_ GA- iterates through guidelist and send each a message  
21. finish_ game- Helps create the string that contains the wanted info of all teams at the end of the game to send in the GAME_OVER message  
22. failure_ log - log failure to send
23. success_ log - log successful outbound messages
24. logfile - writes to logfile  
25. validate_ fa_ message  
26. validate_ ga_ message  
27. send_ message - sends a message, calls appropriate logs  
28. haversine - calculates more precise distance  
29. itemdelete_ fa - free memory  
30. itemdelete_ ga - free memory  
31. itemdelete_ team - free memory  
32. itemdelete_ cd - free memory

###Dataflow through modules
![Alt text](/Users/imac/Desktop/diagram.jpg)

* Game Server- Server is in working order and is connected to without any failure.
* Game server reads codedrop files and creates a struct for each codedrop. Puts them in a list with keyword as codedrop hex code and data as the struct. Have a count of the total codedrops
* GA joins the game. A GA struct is created and a team struct is created.
* FAs join the game: FA structs are created. corresponding team structs are updated with a list of pointers to FAs.
* FA neutralizes a code
  * Received the hexcode. Check the status of the codedrop.
  * Check distance of hexcode to player sending in hexcode
    * If all codedrops are neutralized, end the game
* FA captures another player
  * Check validity of the code, the time since code has been sent and whether the player is not already captured   
  * Go to the captured FA's team struct. Make a check to see if all players of one team have been captured. If so, change the team to inactive.
* Updates to GA: GS loops through the existing teams, and for every FA struct in every team, obtain appropriate info, including all player info, and codedrop status, and add the information to a string. send that string tp the guide agent
* Updates to FA: make string of appropriate info by looking at general game state information and information and information specific to that player and palyer's team, send to FA.
* Sending hints: GA send hint to GS. GS recoginizes its a valide hint. Checks the hint is going to a teammember of the GA and forwards the entire message.
* updating location of FA: update appropriate member of the fa struct

###Major data structures
* list of codedrops
* list of teams
* lists of FAs
* codedrop files, FA, GA, Team, location will each be a struct.
* location struct: longitude, latitude
* code drop struct: status, hex code, location, neutralized team
* FA struct: teamname,address,status(0 is active, 1 is maybe-capture, 2 is captured),pebbleid,name, location, capture start time, last time of contact, gameid;
* GA struct: teamname, address, last time of contact, guideid, playername
* team struct: teamname, field agents, guide agent, status, active players, total number of players ever present, number of neutralized codes, number of players captured, number of captured players

##Extensions
Extensions:
3 points: Compute the distance between two lat/long points with the proper equations, allowing the game to be played over much larger distances than our little campus.
