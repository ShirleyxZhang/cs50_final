##README for game_server

Compliling : Use makefile, call "make" in the top level (project-starter-kit) directory 

**Description**
The game server is the communication middleman between the pebble and the guide agent. It is responsible for directing the hints from GAs to appropriate FAs, giving games status info to fa and ga upon request, updateing all game state information including the last time of contact for all players, player id, player name, team names, neutralized number of codes per team etc. When agem ends either all codedrops have been neutralized or time runs out it sends game end info to all players.

**Example command line**  
./game_server mycodedrops 23383

**Assumptions:**
- switches must come before other arguments  
- the codedrop file follows the strict format in the example
- Any new agents to the game must initially request a status update as this give them the needed gameid
- There can be incorrect information, but there cannot be blanks in the datagrams or short datagrams ie. cannot be 'FA_LOCATION|0' 'GA_STATUS|34|||'

**Exit status**  
0- success  
1- wrong switches  
2- codedropfile is NULL  
3- codedropfile cant be opened or is not 
4- malloc failures, logfile cannot open  
5-socket problems

**limitations:** 
- multiple errors in one message will only notify one error
- In regards to the timer, the game eventually ends only after the timer expires AND another datagram comes in, else it seems to be running overtime
- The game status to guideid has slight formatting differences to protocol i.e. there will be an extra colon. But this codes not affect the way it communicates with guide agent.
- There can be incorrect information, but there cannot be blanks in the datagrams or short datagrams ie. cannot be 'FA_LOCATION|0' 'GA_STATUS|34|||'
- cannot validate whether port number is a valid one used by teams in the class, simply tries to bind

**Non-assumptions**
- can send in "nothing" ie new line and an error message wil be printed to terminal
- validates components of incoming messages to check whether the inflowing info matched the ones stored before given responses back
- validates command line arguments before proceeding


**Extensions**  
3 points: Compute the distance between two lat/long points with the proper equations, allowing the game to be played over much larger distances than our little campus.
