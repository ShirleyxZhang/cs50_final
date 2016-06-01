## README for mission incomputable

*Drew Waterman, Deven Orie, Shirley Zhang*
*Team lapis*
*June 2016*

### Mission incomputable
This is a multiplayer game involving teams of field agents gudied by guide agents. The communication between the agents happen through pebble watches and a game server. The setting of the game is as described on the cs50 mission incomputable page:
S&S has recently learned that some of its rogue agents have stolen copies of the source code for the Tiny Search Engine, and have distributed them at secret dead drop locations around campus. We anticipate that operatives from Dartmouth’s fiercest competitors will soon reach these locations. As you know, the TSE is Dartmouth’s most valuable technology, and we must retrieve every page before those operatives get their hands on it.

### Rules of the game
There are one or more teams.In a game with one team, the team’s goal is to find all code drops. No other team is in competition, and there is no opportunity to capture or be captured.In a game with two or more teams, each team’s goal is to find all code drops, while avoiding capture and (when possible) capture other teams’ players.

Structure of team: Teams can have one or more field agents, and at most one guide agent. It is possible for a team to have no guide agents and play while waiting for a potential guide agent to join.

Objective of game: There are two main components to the game. The first is having FAs neutralize code by being within 10 meters of the codedrop position. The second is to try and capture other players on other teams. Again, capturing can only occur within 10 meters.

### Compiling
Call make in the top level make file. This makes the libraries, game_server and guide agent. Install required programs and updates in order to run a proxy. Edit json files with the correct proxy and settings, make start-proxy in toplevel directory.


##Description of subdirectorys

### game_server directory
This directory contains the game_server.c file which will compile in to the game server program. It also contains sample coderops in a codedrops file. 
There is also a gstest.c which is the chatserver code seen in class. This program can be useful in making fake players to simulate the game. For more details of this program refer to the README in game _server directory.

### Guide_agent directory



### field_agent directory
This directory contains the pebble.c file, a json file, and multiple java script files. All of these work together in allowing the pebble application to 
connect with the right proxy and ports on a server and access data from using the keys stated in the mission.h file to send certain information to a certain destination. This program can be useful in allowing a user to have an interactive experience while player with others and sharing information with users.







