

Implementation stuffs:
-v





Assumptions:
  - Upon starting, if the program has correct and valid arguments, the program should
    prompt the user for a guide ID.
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