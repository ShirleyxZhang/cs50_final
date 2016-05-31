README for guide.c
Team Lapis (Drew Waterman, Deven Orie, Shirley Zhang), May 2016

Compiling:
	make


Usage:
/*
* guide.c - One of three parts for the game "Mission Imcomputable"
* Works with a Game Server and Field Application to play the game.
* Receives status updates from the Game Server and allows the person
* playing as the Guide Agent to send hints to Field Agents through the
* Game Server.
*
* Usage:
*    ./guide [-v] teamName playerName GShost GSport
*
* Upon receipt of valid command line parameters, Guide Agent sets up a socket.
* It joins a game by sending a message to the Game Server, and then it listens
* for input from stdin and the socket. Input from the socket includes game
* updates from the Game Server and "Game Over" notifications from the Game
* Sever. Input from stdin includes hints that the Guide Agent enters to send
* to one of its Field Agents. It sends these hints via formatted messages to
* the Game Server.
*
* Team Lapis (Drew Waterman, Deven Orie, Shirley Zhang), May 2016
*
* code for socket setup (createSocket), handling stdin (handleStdin), and
* handling information sent by the socket (handleSocket) based on Professor
* Kotz's chatclient.c (5/18/16)
*/


Example command lines:
	./guide team1 agent1 flume.cs.dartmouth.edu 23383
	./guide -v team2 agent2 moose.cs.dartmouth.edu 22383
	./guide lapis drew [IP address] [Port number]


Exit status:
  0 - success
  1 - incorrect arguments/incorrect number of arguments
  2 - gethostbyname() error
  3 - memory allocation error
  4 - socket() error
  5 - sendto() error
  9 - error with select()
//  stat returned by sendStatusRequest




Assumptions:
  - Upon starting, if the program has correct and valid arguments, the program should
    prompt the user for a guide ID.
  - The Guide Agent should only be advised to send hints to active agents on the user's
    team. However, if they enter a pebble ID that does not belong to one of their field
    agents anyways, it is the Game Server's job to ignore that message.
  - In order to send a hint to a Field Agent, the user should first type in the message
    that they want to send. If the Guide Agent has no active team members in the game,
    the program should tell them that and not allow them to send a message. Otherwise,
    it should print out a list of the user's team members' pebble ID's to choose from.
    The user then types in the pebble ID of the Field Agent that they want to send the
    hint to.
  - Every 15 seconds of inactivity, the Guide Agent program should send a
    status request message to the Game Server. This way, the Guide Agent is
    updated about changes to the game relatively frequently.
  - If the Guide Agent receives a message from the Game Server with an empty
    agent field, it takes this message as invalid. Therefore, a Guide Agent
    cannot join a game until 1 or more field agents have joined. This is not a
    problem since the main job of the Guide Agent is to help Field Agents. The
    Guide Agent program will keep requesting status updates every 15 seconds
    anyways, so they will be able to join the game shortly after a Field Agent
    joins.
  - If there is a word where an integer should be, or vice-versa, in a message that the
    Guide Agent receives from the Game Server, the program doesn't care. For example, say
    that the Guide Agent receives a GAME_OVER message that looks like the following:
    "GAME_OVER|12345678|seven|t1:t2:...:tK" or "GAME_OVER|12345678|hello|t1:t2:...:tK".
    While numRemainingCodeDrops should be an integer such as "7", the Guide Agent does
    not mind and prints out that the number of remaining code drops is "seven" or "hello"
  - 