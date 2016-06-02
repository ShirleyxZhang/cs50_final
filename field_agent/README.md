CS50-Field Agent ReadMe

To run the program, first run the top level make file


* make (Top Level Make)

* Fork a copy of the git repo onto the desktop of the user
* CD into the field agent file and run make
	* This would run pebble build
	* make install-emulator 
	* make install -phone
* Before playing the game sart the proxy
	* make start-proxy
	* Make sure that the right proxy settings are setup

* Set up game server
	* Turn on the game server 

***Assumptions***

* A player should not leave the game or use the back button when on the first three screens. Player should only ever choose user name or team name once. Using the same team name and player name is necessary for functionality. 

* After the game over screen the user must exit out of the game

* If being captured, do not back out of the popout that shows up; The popup will be maintained for 60 seconds.

* The codes sent from the game server to the pebble should always be in the right format and readable

* All connections should be set up before running the pebble

* We wanted all team names and player names to be chosen by hand and to be of hashcode symbols

* Do not click the request update button or any of the input symbol buttons rapidly



***Limitations***

* The pebble game runs well; When inputting into it, input at a reasonable pace

* In might take some seconds for the hints page to update or to see notifications show up.

***Citations****

For this lap I accessed resources from sample examples, cloud developersite sources and tutorial videos. I also spoke to some students and shared ideas.

James Chartouni, Raunak Bhojwani, Aaron Svendsen, Issa Sylla, Divya Kalidindi, https://www.youtube.com/watch?v=lBfyvXPpdzw, https://developer.pebble.com/docs/, C-D-Lewis Menu Implentations, dlee28 dictionary implementation 






