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
* User interacts with the game mainly through an ASCII interface. 
*
* Usage:
*    ./guide [-v] teamName playerName GShost GSport
*
* Upon receipt of valid command line parameters, Guide Agent sets up a socket.
* It joins a game by sending a message to the Game Server, and then it listens
* for input from stdin and the socket and uses the ncurses library to present
* an ASCII interface. Input from the socket includes game updates from the
* Game Server and "Game Over" notifications from the Game Sever. Input from
* stdin includes hints that the Guide Agent enters to send to one of its Field
* Agents. It sends these hints via formatted messages to the Game Server. 
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
  6 - recvfrom() error
  7 - error with select()



Assumptions:
- Upon starting, if the program has correct and valid arguments, the program
  should prompt the user for a guide ID.
- The Guide Agent should only be advised to send hints to active agents on the
  user's team. However, if they enter a pebble ID that does not belong to one
  of their field agents anyways, it is the Game Server's job to ignore that
  message.
- In order to send a hint to a Field Agent, the user should first type in the
  message that they want to send. If the Guide Agent has no active team members
  in the game, the program should tell them that and not allow them to send a
  message. Otherwise, it should print out a list of the user's team members'
  pebble ID's to choose from. The user then types in the pebble ID of the Field
  Agent that they want to send the hint to.
- Every 15 seconds of inactivity, the Guide Agent program should send a status
  request message to the Game Server. This way, the Guide Agent is updated
  about changes to the game relatively frequently.
- If the Guide Agent receives a message from the Game Server with an empty
  agent field, it takes this message as invalid. If a Game Server wants to
  relay to the Guide Agent that no Field Agents have joined the game yet, the
  FA field in the message should say "NO_AGENTS"
- If there is a word where an integer should be, or vice-versa, in a message
  that the Guide Agent receives from the Game Server, the program doesn't care.
  For example, say that the Guide Agent receives a GAME_OVER message that looks
  like the following:
  "GAME_OVER|12345678|seven|t1:t2:...:tK" or
  "GAME_OVER|12345678|hello|t1:t2:...:tK".
  While numRemainingCodeDrops should be an integer such as "7", the Guide Agent
  does not mind and prints out that the number of remaining code drops is
  "seven" or "hello"



Limitations:
- guide.c does not really work as a standalone program. It is used in
  conjunction with field_agent.c and game_server.c to create a game.
- The program has a limited buffer size of 8000 when handling information from
  stdin or the socket. Anything bigger than this, while very unlikely, could
  cause the program to crash or other undefined responses.
- If a game server repeatedly sends invalid messages, the Guide Agent does not
  alert them or respond in any way to the invalid messages. It just ignores the
  messages.
- The program has no way of alerting the user to an invalid message.
- The program works using UDP, which is not as reliable as TCP.
- When sending a hint, the Guide Agent chooses from a list of pebble ID's to
  send hints to. However, the pebble ID's can get long, so typing in the pebble
  ID of the Field Agent that the user wants to send a hint to can be a long and
  annoying task, increasing the chance of a typo, making it so the user may not
  know that they messed up when trying to send a hint to their Field Agent.
- This programs mallocs a lot of different memory for structs and modules such
  as bags and list
- The interface window clears when the user starts typing a hint, so they can
  not see the game statistics until the Game Server sends them another update.
  However, the Guide Agent will request a status update for every 15 seconds of
  inactivity, so the Guide Agent should not have to go long without seeing any
  updates.
- Because the Guide Agent only gets an update about every 15 seconds, it is
  possible that the Guide Agent could have some unapplicable data. They may not
  know of a capture or a code neutralization that happened split seconds after
  they received an update from the Game Server, so the Guide Agent could
  potentially send an uninformed hint to one of their Field Agents.
- The Guide Agent cannot join the game until after a Game Server starts up. If
  the Guide Agent starts up before the Game Server, it will never join the
  game, even if the Game Server starts up while Guide Agent is running.
- There will hardly ever be enough active agents in the game to reach the end
  of the Guide Agent interface screen when printing. If there are, it does not
  really matter, since the Guide Agent will have enough information to work
  with in assisting their Field Agents, and can ignore the information that
  does not get printed.
- The text on the Guide Agent interface goes away when the user tries to
  scroll. However, it comes back upon new input from the socket or when the
  user enters another hint to a Field Agent.