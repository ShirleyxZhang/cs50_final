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

   	   $ ./guide lapis 129.170.214.115 23383

###Gamer Server
An example of the command-line interface for the game server is as follows:

   	   $ ./gameserver mycodedrops 23383

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
* request from FA or FA for game updates
* request from FA to neutralize drop code

Output:

* hints from GA to FA
* game number response to each new player
* game status updates to GA including:
  * game statistics: elapsed time, number of active agents, number of active teams, number of code drops yet to neutralize.
    * location and status of all code drops.
      * location, name, team, and status of all field agents.

* game status updates to FA including:
  * status about the team’s guide (‘pending’, ‘active’, and time since last contact).
  status of all Field Agents on this team.
  	 * game statistics: elapsed time, number of active agents, number of active teams, number of code drops yet to neutralize.
	 * notification to FA that have been captured
	 * a textual (or graphical) summary of game status on the terminal

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
1. GiveHint, function to send hint from a GA to appropriate FA
2. validateMessage, function to validate components of incoming messages
3. newGA, creates new GA struct
4. newFA, creates new FA struct
5. SendUpdate, sends updates to GA or FA
6. NotifyCapture, sends captured message to captured agent
7. Neutralize, received request to neutralize codedrop and attempts to neutralize it
8. NotifyNewFA, give notice to corresponding GA about new player


###Dataflow through modules
![Alt text](/Users/imac/Desktop/diagram.jpg)

* Game Server- Server is in working order and is connected to without any failure.
* Game server reads codedrop files and creates a struct for each codedrop. Puts them in a hashtable with keyword as codedrop hex code and data as the struct. Have a count of the total codedrops
* GA joins the game. A GA struct is created and a team struct is created.
* FAs join the game: FA structs are created. corresponding team structs are updated with a list of pointers to FAs.
* FA neutralizes a code
  * Received the hexcode. Check the status of the codedrop in hashtable.
    * If all codedrops are neutralized, end the game
    * FA captures another player
      * Checks the data structure in which all of that player's neutralized codes are and adds in the player in "capturing" mode data structure.
      	* Go to the captured FA's team struct. Make a check to see if all players of one team have been captured. If so, change the team to inactive.
	* Updates to GA: GS loops through the existing teams, and for every FA struct in every team, obtain needed info and write to a file in the appropriate format. Send the info of the file to all the GAs by going through each team again and getting the GAs.
	* Updates to FA: Go through each team,an obtain appropriate info of each member in a team. Write info file and send to each FA.
	* Sending hints: GA send hint to GS. GS recoginizes its a hint. Pulls the needed info from datagram and sends to the specific FA.
	* updating location of FA: Go to team of the FA that sent in update. Find the agent in the FA list and update the location of the FA struct.

###Major data structures
Hashtable of codedrops
Hashtable of teams
lists of FAs
codedrop files, FA, GA, Team will each be a struct.

code drop struct: status, hex code, location, neutralized team

FA struct: Location, capture status, team, pebbleID, last time of contact, neutralized code, sockfd

GA struct: teamname, status, sockfd

team struct: teamname, field agents, guide agent, status

##  #Extensions
Extensions:(25 points for now)
10 points: Game server provides a graphical game summary, e.g., displaying a marker for each code drop and each player, overlaid on a campus map.
   --- What the Guide Agent Sees (Uses its own data)

5 points: Game server outputs an ascii-art game summary, that is, filling the terminal window with regular (ASCII) characters that somehow represent the game status.
  --- What the Guide Agent Sees (Uses its own data)

10 points: Guide Agent provides a graphical game summary, e.g., displaying a marker for each code drop and each player, overlaid on a campus map.
   --- What the Field Agent Sees (Guide Agent Inputs)

5 points: Use the Pebble’s accelerometer API to detect when the Field Agent is stationary or moving, sending less-frequent location updates to the Game Server when stationary. The purpose is to save energy. The Pebble should report at least once every minute so Guide does not worry.

-------Extra Points Potentially-------(16 Points EXTRA)
5 points: Use the Pebble’s accelerometer API to detect when the Field Agent is stationary or moving, sending less-frequent location updates to the Game Server when stationary. The purpose is to save energy. The Pebble should report at least once every minute so Guide does not worry.

5 points: Use the Pebble’s compass API to send information about the Field Agent’s heading to the Guide, who can display and use that information in planning. Caveat: we are unsure about the accuracy of Pebble’s magnetometer.

4 points: Use the Pebble API to collect statistics about step-count and calories-burned, reporting that information to Game Server periodically.

2 points: Run games for a given duration; the game server’s status updates inform all players about time remaining in the game. When time runs out, all players are notified of the game-end and final statistics.
