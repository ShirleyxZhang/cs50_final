## TESTING.md ##

Here you can find a link to a folder with all of the screens in the PEBBLE Applcation. For the Field Agent Extension we incoorperated a easy to use and well formatted UI for the every day person to play.(This is shown through allof the window handlers,layers, and pictures)


To test the game server we used the chatserver client provided in lectures. From the chat we sent to the game server appropriate messages to create a game using "fake" field and guide agents.
We created "fake" field agents and guide agents by sending the following messages to the game server through the chat:

**making fake players**
FA_LOCATION|0|fa2ID|TEAM1|FA2|43.7033825112305|-72.2885437011719|1  

FA_LOCATION|0|fa3ID|TEAM2|FA3|43.7033615112305|-72.2885437011719|1  

FA_LOCATION|0|fa4ID|TEAM2|FA4|43.7033643050646|-72.2885407136371|1  

FA_LOCATION|0|fa5ID|TEAM2|FA5|8|8|1  

FA_LOCATION|0|fa6ID|TEAM1|FA6|8|8|1  

FA_LOCATION|0|fa7ID|TEAM1|FA7|8|8|1  

FA_LOCATION|0|fa8ID|TEAM3|FA8|8|8|1  

FA_LOCATION|0|fa9ID|TEAM3|FA9|8|8|1  

FA_LOCATION|0|fa10ID|TEAM3|FA10|8|8|1  

FA_LOCATION|0|fa11ID|TEAM1|FA11|8|8|1


**making fake guide agents**  
GA_STATUS|0|ga1ID|TEAM1|GA1|1 
  
GA_STATUS|0|ga2ID|TEAM2|GA2|1

**Testing**  
Testing for different combinations of commands. 
 
* pebble announces presence  
pebble sending periodic updates to gameserver. Gamesever prints out the periodic updates. For example:  

		From port 48813: 'FA_LOCATION|0|0123456789abcdef0123456789abcdef|F|E|43.7226|-72.1342|1'
		From port 48813: 'FA_LOCATION|494E|0123456789abcdef0123456789abcdef|F|E|43.7226|-72.1342|1'  
		From port 48813: 'FA_LOCATION|494E|0123456789abcdef0123456789abcdef|F|E|43.7226|-72.1342|1'
		From port 48813: 'FA_LOCATION|494E|0123456789abcdef0123456789abcdef|F|E|43.7226|-72.1342|1'
		From port 48813: 'FA_LOCATION|494E|0123456789abcdef0123456789abcdef|F|E|43.7226|-72.1342|1'
		From port 48813: 'FA_LOCATION|494E|0123456789abcdef0123456789abcdef|F|E|43.7226|-72.1342|1'
		From port 48813: 'FA_LOCATION|494E|0123456789abcdef0123456789abcdef|F|E|43.7226|-72.1342|1'

* Pebble neutralizes code.
		
		From port 45352: 'FA_NEUTRALIZE|2D1A9F8|fa2ID|TEAM1|FA2|44.7088405090332|76.2873229980469|C2AO'
		success send: GS_RESPONSE|7443C58|MI_ERROR_INVALID_GAME_ID|invalid gameid, gameid should be 7443C58
		From port 45352: 'FA_NEUTRALIZE|7443C58|fa2ID|TEAM1|FA2|44.7088405090332|76.2873229980469|C2AO'
		success send: GS_RESPONSE|7443C58|MI_NEUTRALIZED|successfully neutralized a codedrop!  

When pebble neutralizes the same code again it fails

* Capturing player.
		
		32From port 59865: 'FA_CAPTURE|62D3811|fa2ID|TEAM1|FA2|0'
		success send: GS_CAPTURE_ID|62D3811|72A8
		58From port 59865: 'FA_CAPTURE|62D3811|fa2ID|TEAM1|FA2|72A8'
		success send: GS_RESPONSE|62D3811|MI_CAPTURE_SUCCESS|successfully captured!
		success send: GS_RESPONSE|62D3811|MI_CAPTURED|You have been captured!

* Guide agent announces presence and sends hints to its FA

		From port 45474: 'GA_HINT|A907239F|ga1ID|0|GA1|0123456789abcdef0123456789abcdef|hello'
		success send: GA_HINT|A907239F|ga1ID|0|GA1|0123456789abcdef0123456789abcdef|hello

		
		From port 38659: 'GA_HINT|D19D9|ga1ID|TEAM1|GA1|*|hello'
		success send: GA_HINT|D19D9|ga1ID|TEAM1|GA1|*|hello
		success send: GA_HINT|D19D9|ga1ID|TEAM1|GA1|*|hello

* Guide agent announces presence and sends hints to its FA
* 
		GAME_STATUS|D19D9|fa2ID,TEAM1,FA2,active,8.000000,8.000000,113:fa3ID,TEAM2,FA3,active,8.000000,8.000000,113:fa4ID,TEAM2,FA4,active,8.000000,8.000000,113:fa5ID,TEAM2,FA5,active,8.000000,8.000000,45:|: 68A6,43.7050819396973,-72.2949295043945,NONE: B3EB,43.7088928222656,-72.2842864990234,NONE: 2CA3,43.7064704895020,-72.2871017456055,NONE: 9231,43.7051391601562,-72.2881393432617,NONE: 053F,43.7024879455566,-72.2865524291992,NONE: 1AFE,43.7011871337891,-72.2873382568359,NONE: 4760,43.7034759521484,-72.2854690551758,NONE: 05AD,43.7030677795410,-72.2841186523438,NONE: 1819,43.7010498046875,-72.2879714965820,NONE: 002C,43.7007408142090,-72.2884368896484,NONE: 268B,43.7009239196777,-72.2893524169922,NONE: 6C8A,43.7012481689453,-72.2894134521484,NONE: 17E2,43.7023735046387,-72.2888946533203,NONE: AEA3,43.7021903991699,-72.2894821166992,NONE: 2C15,43.7031173706055,-72.2903671264648,NONE: 4C24,43.7026977539062,-72.2895965576172,NONE: 7895,43.7068099975586,-72.2872772216797,NONE: 39FC,43.7015380859375,-72.2894515991211,NONE: DECA,43.7051277160645,-72.2891616821289,NONE: 0462,43.7062950134277,-72.2892303466797,NONE: 5754,43.7126731872559,-72.2857742309570,NONE: 693F,43.7082405090332,-72.2873229980469,NONE: 36E5,43.7082405090332,-72.2873229980469,NONE: C2A0,43.7033615112305,-72.2885437011719,NONE
		
* check logfiles were created and documented to


**Sample run**  
Extra credit for calculating more accurate distance. the distance prints each time its calculated

The great circle distance, as expected.

		313From port 51033: 'FA_CAPTURE|286|0123456789abcdef0123456789abcdef|2345|0|0'
		the distance squared: 2.33success send: GS_CAPTURE_ID|286|3239


####Example of testing the different actions, and accompanying raw logfile
The gameID is: 5F
Ready at port 23383

sending update to field agent
		
		From port 35700: 'FA_LOCATION|5F|0123456789abcdef0123456789abcdef|0|0|43.7226|-72.1342|1'
		success send: GAME_STATUS|5F|no active guide|25|1|1

sending capture id to maybe-capture player

		From port 35700: 'FA_CAPTURE|5F|0123456789abcdef0123456789abcdef|0|0|0'
		success send: GS_ CAPTURE_ ID|5F|DDF6

sending captured message to captured player, and success message to capturing player

		From port 35700: 'FA_CAPTURE|5F|0123456789abcdef0123456789abcdef|0|0|DDF6'
		success send: GS_RESPONSE|5F|MI_CAPTURE_SUCCESS|successfully captured!
		FA_CAPTURE|5F|0123456789abcdef0123456789abcdef|0|0|ECA6

		success send: GS_RESPONSE|5F|MI_CAPTURED|You have been captured!

when neutralization fails, no response

	From port 35700: 'FA_NEUTRALIZE|5F|0123456789abcdef0123456789abcdef|0|0|43.7226|-72.1342|A9AB'

When successful, message is sent

	From port 35700: 'FA_NEUTRALIZE|5f|0123456789abcdef0123456789abcdef|0|0|43.7226|-72.1342|AAAA'
	success send: GS_RESPONSE|5F|MI_NEUTRALIZED|successfully neutralized a codedrop!


###The logfile
Below a snippits from the logfile documenting correct messages were sent. 

		Tue May 31 20:33:23 2016 
		received message: FA_CAPTURE|5F|0123456789abcdef0123456789abcdef|0|0|0 from
		Tue May 31 20:33:23 2016
		 **successfully sent datagram: GS_CAPTURE_ID|5F|DDF6 , to fa2ID **

documenting events and sending to the correct agents the appropriate capture messages

		Tue May 31 20:33:47 2016
		player fa2ID was captured by player 0123456789abcdef0123456789abcdef
		Tue May 31 20:33:47 2016
		successfully sent datagram: GS_RESPONSE|5F|MI_CAPTURE_SUCCESS|successfully captured! , to 0123456789abcdef0123456789abcdef

		Tue May 31 20:33:47 2016
		successfully sent datagram: GS_RESPONSE|5F|MI_CAPTURED|You have been captured! , to fa2ID

Documenting neutralizations

		Tue May 31 20:35:26 2016
		received message: FA_NEUTRALIZE|5F|0123456789abcdef0123456789abcdef|0|0|43.7226|-72.1342|AAAA from
		Tue May 31 20:35:26 2016
		0 successfully neutralized codedrop AAAA
		Tue May 31 20:35:26 2016
		successfully sent datagram: GS_RESPONSE|5F|MI_NEUTRALIZED|successfully neutralized a codedrop! , to 0123456789abcdef0123456789abcdef

###Sending game over message

	success send: GAME_OVER|C21D37E5|25|:TEAM2,1,0,0,0:lapis,2,0,0,0

	success send: GAME_OVER|C21D37E5|25|:TEAM2,1,0,0,0:lapis,2,0,0,0

	success send: GAME_OVER|C21D37E5|25|:TEAM2,1,0,0,0:lapis,2,0,0,0


* Note: the "success send" statements printed out to the terminal mean that the Game Server successfully sent a message to the Field or Guide agent.


# Testing Guide Agent
To test the Guide Agent, we used the assistance of the chatserver program provided in the CS50 lectures. We ran chatserver from a local directory on one of our Macs, and we ran Guide Agent on one of the CS50 servers and connected it to the chatserver. From the chatserver, we were able to print out messages sent by the Guide Agent, and we received "fake" messages formatted like messages from the Game Server.


## Testing invalid command-line parameters:

[waterman@moose ~/cs50/project/lapis/guide_agent]$ ./guide 1  
Error: incorrect number of arguments.  
Usage: ./guide [-v] teamName playerName GShost GSport  
[waterman@moose ~/cs50/project/lapis/guide_agent]$ ./guide 1 2  
Error: incorrect number of arguments.  
Usage: ./guide [-v] teamName playerName GShost GSport  
[waterman@moose ~/cs50/project/lapis/guide_agent]$ ./guide 1 2 3  
Error: incorrect number of arguments.  
Usage: ./guide [-v] teamName playerName GShost GSport  
[waterman@moose ~/cs50/project/lapis/guide_agent]$ ./guide 1 2 3 4  
What is the guide's ID?: 12341234  
Error sending startup message  
[waterman@moose ~/cs50/project/lapis/guide_agent]$ ./guide lapis 007 host port  
What is the guide's ID?: 12341234  
Port number can only consist of integers.  



## Using valid parameters to run the Guide Agent:
First, start the chatserver:

       marias-mbp-6:client-server-udp-select mariawaterman$ ./chatserver
       					     Ready at port 56605

Then use the port given to connect the Guide Agent to the chat server.
The program prompts the user for an ID.

         ./guide -v lapis 007 10.31.39.63 56605
	 	      What is the guide's ID?:
		      We enter a valid 8-digit hexadecimal ID, and the program takes us to an ASCII interface.
  
This is what pops up on the screen:

                             Unable to join a game.
		      Please enter 'quit' and try again.


When we press enter, it takes us to a blank screen.
Let's enter a GAME_STATUS message through the chatserver to the Guide Agent. The Guide Agent gets the message, displays the following, and sends periodic status requests to the Game Server every 15 seconds.

     Enter a hint to send to a field agent, or enter 'quit' to quit.
     Game ID is D19D9
     Locations of active agents:
     Agent FA1: team lapis: 8.000000,8.000000
     Agent FA2: team TEAM1: 8.000000,8.000000
     Agent FA5: team TEAM2: 8.000000,8.000000
     Locations of active code drops:
     Code drop 68A6: 43.705,-72.29;
     Code drop B3EB: 43.708,-72.28;
     Code drop 2CA3: 43.706,-72.28;
     Code drop 9231: 43.705,-72.28;
     Code drop 053F: 43.702,-72.28;
     Code drop 1AFE: 43.701,-72.28;
     Code drop 4760: 43.703,-72.28;
     Code drop 05AD: 43.703,-72.28;
     Code drop 1819: 43.701,-72.28;
     Code drop 002C: 43.700,-72.28;
     Code drop 268B: 43.700,-72.28;
     Code drop 6C8A: 43.701,-72.28;
     Code drop 17E2: 43.702,-72.28;
     Code drop AEA3: 43.702,-72.28;
     Code drop 2C15: 43.703,-72.29;
     Code drop 4C24: 43.702,-72.28;


##Sending a hint:
When the user starts typing, the screen clears so the user can see what they are typing in their hint. When we enter "Agent FA1, good job!" as a hint to send, the following appears:

     Agent FA1, good job!
     Send hint to which field agent?
     Your active agents have the following ID's:
     You may also enter '*' to send to all agents on your team.
     fa1ID


Then the user types in fa1ID:


     Agent FA1, good     job!
     Send hint to which field agent?
     Your active agents have the following ID's:
     You may also enter '*' to send to all agents on your team.
     fa1ID

     fa1ID



After pressing enter:


                    Your hint was sent to the game server.



Suppose that at some point, the field agent on the user's team gets captured. Agent FA1 on team lapis is no longer in the game:

     Enter a hint to send to a field agent, or enter 'quit' to quit.
     Game ID is D19D9
     Locations of active agents:
     Agent FA2: team TEAM1: 8.000000,8.000000
     Agent FA5: team TEAM2: 8.000000,8.000000
     Locations of active code drops:
     Code drop 68A6: 43.705,-72.29;
     Code drop B3EB: 43.708,-72.28;
     Code drop 2CA3: 43.706,-72.28;
     Code drop 9231: 43.705,-72.28;
     Code drop 053F: 43.702,-72.28;
     Code drop 1AFE: 43.701,-72.28;
     Code drop 4760: 43.703,-72.28;
     Code drop 05AD: 43.703,-72.28;
     Code drop 1819: 43.701,-72.28;
     Code drop 002C: 43.700,-72.28;
     Code drop 268B: 43.700,-72.28;
     Code drop 6C8A: 43.701,-72.28;
     Code drop 17E2: 43.702,-72.28;
     Code drop AEA3: 43.702,-72.28;
     Code drop 2C15: 43.703,-72.29;
     Code drop 4C24: 43.702,-72.28;


Now, when the user tries to send a hint (they typed "hint" as their hint):

     hint
     Send hint to which field agent?
     Your active agents have the following ID's:
     You may also enter '*' to send to all agents on your team.
     Oops! You have no active agents to send hints to!


The program correctly does not let the user send a hint, since they have no active agents to send hints to.



**Throughout this testing, the following is what we can see on the chatserver side:**


GA\_STATUS|0|12345678|lapis|007|1
*--sent by the Guide Agent, received by the Game Server (approximately every 15 seconds)*

     : GAME\_STATUS|D19D9|fa1ID, lapis, FA1, active, 8.000000,8.000000,113:fa2ID,TEAM1,FA2,active,8.000000,8.0
     0000,1\
     13:fa5ID,TEAM2,FA5,active,8.000000,8.000000,45:|: 68A6,43.7050819396973,-72.2949295043945,NONE: B3EB,43.7
     88928\
     222656,-72.2842864990234,NONE: 2CA3,43.7064704895020,-72.2871017456055,NONE: 9231,43.7051391601562,-72.28
     13934\
     32617,NONE: 053F,43.7024879455566,-72.2865524291992,NONE: 1AFE,43.7011871337891,-72.2873382568359,NONE: 4
     60,43\
     .7034759521484,-72.2854690551758,NONE: 05AD,43.7030677795410,-72.2841186523438,NONE: 1819,43.701049804687
     ,-72.\
     2879714965820,NONE: 002C,43.7007408142090,-72.2884368896484,NONE: 268B,43.7009239196777,-72.2893524169922
     NONE:\
     6C8A,43.7012481689453,-72.2894134521484,NONE: 17E2,43.7023735046387,-72.2888946533203,NONE: AEA3,43.70219
     3991\
     699,-72.2894821166992,NONE: 2C15,43.7031173706055,-72.2903671264648,NONE: 4C24,43.7026977539062,-72.28959
     55761\
     72,NONE:  

*-- sent by the Game Server, received by the Guide Agent*


     GA_HINT|D19D9|12345678|lapis|007|fa1ID|Agent FA1, good job! -- sent by Guide Agent, received by Game Server
       
*-- send by the Guide Agent, received by the Game Server*

     GAME_STATUS|D19D9|fa1ID, lapis, FA1, captured, 8.000000,8.000000,113:fa2ID,TEAM1,FA2,active,8.000000,8.000000,1\  
     13:fa5ID,TEAM2,FA5,active,8.000000,8.000000,45:|: 68A6,43.7050819396973,-72.2949295043945,NONE: B3EB,43.7088928\
     222656,-72.2842864990234,NONE: 2CA3,43.7064704895020,-72.2871017456055,NONE: 9231,43.7051391601562,-72.28813934\
     32617,NONE: 053F,43.7024879455566,-72.2865524291992,NONE: 1AFE,43.7011871337891,-72.2873382568359,NONE: 4760,43\
     .7034759521484,-72.2854690551758,NONE: 05AD,43.7030677795410,-72.2841186523438,NONE: 1819,43.7010498046875,-72.\
     2879714965820,NONE: 002C,43.7007408142090,-72.2884368896484,NONE: 268B,43.7009239196777,-72.2893524169922,NONE:\
     6C8A,43.7012481689453,-72.2894134521484,NONE: 17E2,43.7023735046387,-72.2888946533203,NONE: AEA3,43.7021903991\
     699,-72.2894821166992,NONE: 2C15,43.7031173706055,-72.2903671264648,NONE: 4C24,43.7026977539062,-72.28959655761\
     72,NONE:

  
*-- sent by Game Server, received by Guide Agent*


### Invalid messages to the Guide Agent:

**Message with bad OPCODE:**

	  GAME|D19D9|fa1ID, lapis, FA1, captured, 8.000000,8.000000,113:fa2ID,TEAM1,FA2,active,8.000000,8.000000,113:fa5ID,TEAM2,FA5,active,8.000000,8.000000,45:|: 68A6,43.7050819396973,-72.2949295043945,NONE

**Message with no Game ID:**

	  GAME\_STATUS||fa1ID, lapis, FA1, captured, 8.000000,8.000000,113:fa2ID,TEAM1,FA2,active,8.000000,8.000000,113:fa5ID,TEAM2,FA5,active,8.000000,8.000000,45:|: 68A6,43.7050819396973,-72.2949295043945,NONE:

**Messages with missing field agent information:**

	   GAME\_STATUS|D19D9|fa1ID, lapis, FA1, captured, 8.000000,8.000000,|: 68A6,43.7050819396973,-72.2949295043945,NONE: B3EB,43.7088928222656,-72.2842864990234,NONE:

	   GAME\_STATUS|D19D9|fa1ID, lapis, FA1, captured, 8.000000,113|: 68A6,43.7050819396973,-72.2949295043945,NONE: B3EB,43.7088928222656,-72.2842864990234,NONE:

     : GAME\_STATUS|D19D9|fa1ID, lapis,captured, 8.000000,8.000000,|: 68A6,43.7050819396973,-72.2949295043945,NONE: B3EB,43.7088928222656,-72.2842864990234,NONE:


**Messages with too much field agent information:**

	   GAME_STATUS|D19D9|fa1ID, fa1, lapis, FA1, captured, 8.000000,8.000000,|: 68A6,43.7050819396973,-72.2949295043945,NONE: B3EB,43.7088928222656,-72.2842864990234,NONE:


	   GAME_STATUS|D19D9|fa1ID, fa1, lapis, lapis,  FA1, captured, 8.000000,8.000000,|: 68A6,43.7050819396973,-72.2949295043945,NONE: B3EB,43.7088928222656,-72.2842864990234,NONE:


**Messages with missing code drop information:**

	   GAME_STATUS|D19D9|fa1ID, fa1, lapis, lapis,  FA1, captured, 8.000000,8.000000,|: 68A6,43.7050s819396973,-72.2949295043945,

	   GAME_STATUS|D19D9|fa1ID, fa1, lapis, lapis,  FA1, captured, 8.000000,8.000000,|:3.7050819396973,-72.2949295043945,NONE: B3EB,43.7088928222656,-72.2842864990234,NONE:


**Messages with too much code drop information:**

	   GAME_STATUS|D19D9|fa1ID, fa1, lapis, lapis,  FA1, captured, 8.000000,8.000000,|:3.7050819396973,-72.2949295043945,NONE, lapis:


**Other messages with bad syntax:**

	GAME_STATUS|D19D9||fa1ID, fa1, lapis, lapis,  FA1, captured, 8.000000,8.000000,|:3.7050819396973,-72.2949295043945,NONE

	GAME_STATUS|D19D9|fa1ID.fa1, lapis, lapis,  FA1, captured, 8.000000,8.000000,|:3.7050819396973,-72.2949295043945,NONE



#### None of these messages caused the program to crash. The screen still displays the following from the last time we tried to send a hint.


     Enter a hint to send to a field agent, or enter 'quit' to quit.
     Send hint to which field agent?
     Your active agents have the following ID's:
     You may also enter '*' to send to all agents on your team.
     Oops! You have no active agents to send hints to!


### Error message from the Game Server:

    GS\_RESPONSE|D19D9|MI\_ERROR\_INVALID\_GAME\_ID| bad game ID


### Game Over message:
Upon receipt of this message:

     GAME\_OVER|D19D9|7|TEAM1, 1, 2, 3, 4:TEAM2, 5, 6, 7, 8:TEAM3, 1, 2, 3, 4

The interface displays the following:

    GAME OVER! Here are the records for this game:
    Number of remaining unneutralized code drops is 7
    Team TEAM1:
    Total number of active players: 1
    Captured 2 players from other teams.
    3 players on this team were captured.
    Neutralized 4 code drops
    Team TEAM2:
    Total number of active players: 5
    Captured 6 players from other teams.
    7 players on this team were captured.
    Neutralized 8 code drops
    Team TEAM3:
    Total number of active players: 1
    Captured 2 players from other teams.
    3 players on this team were captured.
    Neutralized 4 code drops
    Good game, everybody!


**The user enters 'quit' and exits the game.**


### Valgrind:
*Running the above process with valgrind gives us:*

	 ==18050== LEAK SUMMARY:
	 ==18050==    definitely lost: 0 bytes in 0 blocks
	 ==18050==    indirectly lost: 0 bytes in 0 blocks
	 ==18050==      possibly lost: 0 bytes in 0 blocks
	 ==18050==    still reachable: 65,708 bytes in 105 blocks
	 ==18050==         suppressed: 0 bytes in 0 blocks

*The "still reachable" memory is all from the ncurses library, not the other Guide Agent code*



### Logs:
* As the game went on, the program logged activity to a file called "guideagent.log"
* Since we are logging in the verbose [-v] mode, the log file has time stamps for every message that was received from the Game Server. Here is any example of one message that was logged during that game:

  Wed Jun  1 14:35:50 2016
  Received the following message from the game server: GAME_STATUS||fa1ID, lapis, FA1, captured, 8.000000,8.0000\
  00,113:fa2ID,TEAM1,FA2,active,8.000000,8.000000,113:fa5ID,TEAM2,FA5,active,8.000000,8.000000,45:|: 68A6,43.7050\
  819396973,-72.2949295043945,NONE:
  Wed Jun  1 14:35:50 2016
  Received an invalid message and ignored it.


The program received the message and wrote down exactly what that message was. Then it realized that the message was invalid, so it wrote that down as well with the same time stamp.

Another example: when the Game Server sent the GS_RESPONSE message:

	Wed Jun  1 14:37:09 2016
	Received the following message from the game server: GS_RESPONSE|D19D9|MI_ERROR_INVALID_GAME_ID| bad game ID
	Wed Jun  1 14:37:09 2016
	MI_ERROR_INVALID_GAME_ID: Guide agent sent an invalid game ID to the game server.

***Communications Are Open & Set***

App install succeeded.
[20:44:50] essage_outbox.c:51> app_message_open() called with app_message_outbox_size_maximum().
[20:44:50] essage_outbox.c:54> This consumes 8200 bytes of heap memory, potentially more in the future!
[20:44:50] message_inbox.c:13> app_message_open() called with app_message_inbox_size_maximum().
[20:44:50] message_inbox.c:16> This consumes 8200 bytes of heap memory, potentially more in the future!
[20:44:50] javascript> PebbleKit JS Environment ready!
[20:44:50] field_agent.c:155> MESSAGE RECEIVED
[20:44:50] field_agent.c:162> Got AppKeyJSReady: 0123456789abcdef0123456789abcdef
[20:44:50] javascript> BT-RX: AppMessage from Pebble received: {"3":"1","AppKeyLocation":"1"} 
[20:44:50] javascript> BT-TX: sendAppMessage success: '{AppKeyJSReady : 0123456789abcdef0123456789abcdef}'
[20:44:50] javascript> WEBSOCKET: connected to ws://flume.cs.dartmouth.edu:36023
[20:44:50] field_agent.c:155> MESSAGE RECEIVED
[20:44:50] javascript> BT-TX: sendAppMessage success: '{AppKeyLocation : 43.7226|-72.1342}'
[20:45:07] javascript> BT-RX: AppMessage from Pebble received: {"3":"1","AppKeyLocation":"1"} 
[20:45:07] field_agent.c:155> MESSAGE RECEIVED
[20:45:07] javascript> BT-TX: sendAppMessage success: '{AppKeyLocation : 43.7226|-72.1342}'
[20:45:16] field_agent.c:246> FA_LOCATION|0|0123456789abcdef0123456789abcdef|5678|0|43.7226|-72.1342|1
[20:45:16] javascript> WEBSOCKET-TX: FA_LOCATION|0|0123456789abcdef0123456789abcdef|5678|0|43.7226|-72.1342|1
[20:45:16] javascript> BT-RX: AppMessage from Pebble received: {"1":"FA_LOCATION|0|0123456789abcdef0123456789abcdef|5678|0|43.7226|-72.1342|1","AppKeySendMsg":"FA_LOCATION|0|0123456789abcdef0123456789abcdef|5678|0|43.7226|-72.1342|1"} 

***Field Agent Neutralize*** (LINE 426)

* Shows that I successfully sent an OP Code to the server to neutralize a code

[20:49:35] field_agent.c:153> MESSAGE RECEIVED
[20:49:35] javascript> BT-TX: sendAppMessage success: '{AppKeyLocation : 43.7226|-72.1342}'
[20:49:52] field_agent.c:427> FA_NEUTRALIZE|0|0123456789abcdef0123456789abcdef|5FE|2|43.7226|-72.1342|0122
[20:49:52] javascript> WEBSOCKET-TX: FA_NEUTRALIZE|0|0123456789abcdef0123456789abcdef|5FE|2|43.7226|-72.1342|0122

* Shows that the game server sucessfully received my message to neutralize

From port 60000: 'FA_NEUTRALIZE|0|0123456789abcdef0123456789abcdef|5FE|2|43.7226|-72.1342|0122'


***Field Agent Capture*** (LINE 462)

* Successfully Sending a Capture code to the game server

[20:58:51] field_agent.c:466> FA_CAPTURE|0|0123456789abcdef0123456789abcdef|01234|2345|0123

* On the server side, the server knows that I have successfully initialized the capture ability.

From port 55508: 'FA_CAPTURE|0|0123456789abcdef0123456789abcdef|01234|2345|0'

* The server sees my capture code and then goes on to do its taks of checking it and sending back a message

From port 55508: 'FA_CAPTURE|0|0123456789abcdef0123456789abcdef|01234|2345|0123'

***Communications***

* For testing my group and I used dummy agents at times and pre-set location configurations in order to test if a code was actually being neutralized from a location that meets the requirements

* This was the same test we conducted when we were sending hints from the gameagent to the server and then to me and as well as the capturing of players

* Location is updated every 6 seconds (LINE 236)

  if (tick_time->tm_sec == 6){
    sendMessage(AppKeyLocation,"1");
  }

* Location is updated every 15 seconds

   if((tick_time->tm_sec % 15 == 0) &&(team_registered)){
    memset(message, 0, 300);
    snprintf(message, 300, "FA_LOCATION|%s|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, location, status_req);
    LOG(message);
    strcpy(status_req, "1");
    sendMessage(AppKeySendMsg, message);
  }

* We tested by constantly looking at server output as I sent messages and output from the game server and looked to see if there were screen updates and coorelations which there were. 
