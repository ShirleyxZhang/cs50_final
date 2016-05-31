##README for game_server

Compliling : Use makefile, call "make" in the game_server directory 

**Example command line**  
./game_server mycodedrops 23383

**Assumptions:**
- switches must come before other arguments  
- the codedrop file follows the strict format in the example

**Exit status**  
0- success  
1- wrong switches  
2- codedropfile is NULL  
3- codedropfile cant be opened or is not 
4- malloc failures  
5-socket problems

**limitations:**  
- If an unknown FA sends in request to capture the message will be ignored  
- multiple errors in one message will only notify one error

**Extensions**  
3 points: Compute the distance between two lat/long points with the proper equations, allowing the game to be played over much larger distances than our little campus.
