##Implementation Spec
###Game Server
1. Program parses command line arguments using parse_arguments function. Checks for wrong number of arguments, unreadable codedrop filepath and invalid port number.

2. Uses load_codedrops function to read from the codedrop file and create a codedrop struct for each. In each code drop there is also a pointer to location struct which stores the float values of longitude and latitude of the codedrop. Each struct is stored in a list of codedrops. List is a module written in lab 3 with key as a string and data as void* , which in this case is the codedrop struct. Also create a count of unneutralized codedrops.

3. Initialize playerlist(list that contains pebbleid as key, and FA struct as data), guidelist(list that contains guideid as key and GA struct as data), teamlist(list that contains teamName as key and team struct as data), hashtable for keeping track of maybe-capture agents(key is hexcode, data is pointer to maybe-capture FA struct).

4. Sets up sockets using createSocket function. A while loop will continue to run until the game ends and it will listen for datagrams.

5. For each message, process it by breaking it down into the parts separated by pipes using function process_message.

6. Validate opcode, see whether message is from GA or FA.

7. If message if from FA, validate gameId, pebbleId, teamName, and playerName, and then check what the opcode is , process appropriately using switches.

  - FA_ LOCATION: If FA is new to the game, use create_player function to initialize a new FA with information from message. Then, either add to the given team’s existing FA list or create a new team, add to new team’s FA list and and insert new team into team list. Check the status token and give updates if requested

  - FA_ NEUTRALIZE: list_ find the codeid that came in the message. If NULL is not returned then the code is valid. Then check this code is not already neutralized by looking into the corresponding codedrop structs teamname field. If teamname is NULL and the absolute difference between the codedrop location and position of the FA is less than 10m, then this is a valid neutralization. Change the code’s status to neutralized and decrement the counter for remaining codedrops. Send success message back.

  - FA_ CAPTURE:
    - * If captureID is 0, loop through player list to find all players within 10m radius and not on this FA’s team. Send each a random hexcode using helper function generate_ random_ hex. Use time() to get start time for capturing. Give this start time to each maybe-capture FA’s capture_ start field. Insert in a hashtable with key as the random hex and data as pointer to each maybe-capture player struct.
     - * If request to capture a player, lookup hashtable to find player corresponding to the hexcode. check capture start time and find difference with current time, if less than 60, change player status to inactive. Look into the captured player’s teamname. Find corresponding team struct in team list, check status of all FAs in the team, if all are captured, team is inactive. Send message to the capturing FA and captured FA using sendto.

8. If message is from GA, validate the gameId, guideId, teamName, and playerName, and then check what the opcode, process appropriately using switches.
 - GA_ STATUS: If GA doesn’t exist, create a new GA and add to existing team or create new team. If GA does exist, check the team name sent in matches the team name of the GA already stored in the guidelist. If GA requested update loop through player list to obtain update appropriate information and send to GA.

 - GA_HINT: Lookup the pebbleid in playerlist and send the message to that FA using sendto and the stored address of the FA in the FA struct.
 9. Send out updates every 45 seconds. Send update to each FA by going to team list and looping through the FA list in each team struct and update info. Send this updated info, along with codedrop info to each FA in that team. Repeat for all teams. After, send information about the whole game to each GA by going through guidelist.
 10. End game and clean up memory appropriately and close socket when all codedrops have been neutralized, send the ending message to all the players by looping through playerlist and GA list.
 
