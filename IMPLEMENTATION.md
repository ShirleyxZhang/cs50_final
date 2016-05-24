#Final Project- Impelmentation Spec
Team: Lapis
Shirley Zhang, Deven Orie, Drew Waterman

**Field Agent**

1) Check the validity/active state of the game server & game agent

2) Checks that the data structure implementations are not null/exist
Exit Status: exit(1)
	
3) Setup the SDK & Set WatchFace to False

4) Set up Text Layers, Windows, & Screen Layers {Order of the stack currently working on}

	Screen 1: Choose player name from the (player list)
			Key: Pebble ID
			Data: FA Struct
			
5) Set up a secondary window 

	Screen 2: Display the amount of teams in the game/names & Players on my team
			 	(Team List)
				Key: Team Name
				DATA: Team Struct
6) Set up a third window/screen layer

	Screen 3: Two Options
				Print out how many codes have been neutralized/total amount.
			Option 1: Neutralize Code	
			Option 2: Capture Player
								
Option 1: After the code is typed into a text box, we search to see if it can be found in the (List of Code Drops)
			
			Key: Hex Code
			DATA: Code Drop Struct & Pointer to Location Struct

-The State of this code drop struct would have to be
changed once found

-If not found return error message else return success
message

-If already in inactive state return message 

Option 2:
If in the proximity of another player; Press Capture 

-Possibly Based on the location Data and the data from the (Team List), this would only work if this check is met

-Sends a signal to the Game Server to relay the ID of the player being captured

Once captured Player is made Inactive(PlayerList)

			Key: Pebble ID
			DATA: FA Struct
		
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

**Game Server**

**Guide Agent**
