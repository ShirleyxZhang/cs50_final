#include <pebble.h>
#include <mission.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  
#include <string.h>

//Deven Orie-CS50-Field Agent

//pebble install --serial /dev/cu.PebbleTime5B57-SerialPo
//pebble logs --serial /dev/cu.PebbleTime5B57-SerialPo
//pebble install --phone 10.31.117.204
//pebble logs --phone 10.31.117.204
//make install-emulator
//pebble install --phone 10.31.117.204 --logs



// enum {
//   AppKeyJSReady = 0,      // The JS environment on the companion smartphone is up and running!
//   AppKeySendMsg,          // Send a message over Bluetooth to the companion smartphone and then on to the Game Server
//   AppKeyRecvMsg,          // A message from the Game Server is available (arrived over Bluetooth)
//   AppKeyLocation,         // Request your GPS location from the companion smartphone
//   AppKeyPebbleId,         // Request your unique pebble ID from the companion smartphone
//   AppKeySendError         // Error: companion app can't connect to the Proxy (and ultimately the Game Server).
// };

//Windows for each screen of the app
Window *window, *choose_team_window, *window2, *neutralize_window, *capture_window;
Window *getting_captured;

//Allows for menu built on top of scroll view
MenuLayer *player_menu, *option_menu;
MenuLayer *neutralize_keyboard, *capture_keyboard, *team_name_keyboard;

//Theses are the text layers added to the navigation & capture windows
TextLayer *navigation_header, *navigation_input;
TextLayer *capture_header, *capture_input;
TextLayer *neutralize_code_text, *player_input;
TextLayer *team_header, *team_input;
TextLayer *capture_code_text;
TextLayer *capture_text;


//Array of player names
char *players[]={"Deven","Drew","Shirley","David","Sam"};

//Array of the neutralization codes
char *hex_test[]={"1234", "ABCD", "A1A2"};

//Array of the players codes
char *player_id[]={"1111", "AAAA", "BBBB"};

//Symbols for the key board
char *symbols[]={"SUBMIT CODE", "0","1","2","3","4","5","6","7","8","9","A","B","C","D","E", "F","G","H", "I", "J", "K", "L", "M", "N",
"O","P","Q","R","S", "T", "U", "V", "W", "X", "Y", "Z"};

//Player selected in screen one
char *chosenplayer="";

//Allows for concatination with the symbol chose and add it to buf
char *chosen_symbol="";
char *chosen_symbol_two="";
char *chosen_symbol_three="";
char *chosen_symbol_four="";

//The chose symbols are added to these character arrays 
char neutralization_code[10];
char capture_code[10];
char playerName[20];
char teamName[20];

//Prevents a code from being more than 4 characters
int incrementer=0;
int incrementer_two=0;
int player_letter_incrementer=0;
int team_letter_incrementer=0;

//Allows for string compare to occur once 
bool match=false;
bool match_two=false;

int pebbleId=0;
int gameId=0;

        
int lat=10;
int lon=10;
//request update = 1
//norequest = 0
int statusReq=0;


// Largest expected inbox and outbox message sizes
const uint32_t inbox_size = 64;
const uint32_t outbox_size = 256;


//*********************OP Codes****************************************//
//FA_LOCATION|gameId|pebbleId|teamName|playerName|lat|long|statusReq
void location_op(){
  //8k buffer
  char message[8191];
  char convert[8191];

  strcat(message, "FA_LOCATION|");
  snprintf(convert,8191,"%d",gameId);
  strcat(message,convert);
  strcat(message,"|");
  snprintf(convert,8191,"%d",pebbleId);
  strcat(message,convert);
  strcat(message,"|");
  strcat(message,teamName);
  strcat(message,"|");
  strcat(message,playerName);
  strcat(message,"|");
  snprintf(convert,8191,"%d",lat);
  strcat(message,convert);
  strcat(message,"|");
  snprintf(convert,8191, "%d", lon);
  strcat(message,convert);
  strcat(message,"|");
  snprintf(convert, 8191, "%d", statusReq);
  strcat(message,convert);
}

//FA_NEUTRALIZE|gameId|pebbleId|teamName|playerName|lat|long|codeId
void neutralize_op(){
  //8k buffer
  char message[8191];
  char convert[8191];

  strcat(message, "FA_NEUTRALIZE|");
  snprintf(convert,8191,"%d",gameId);
  strcat(message,convert);
  strcat(message,"|");
  snprintf(convert,8191,"%d",pebbleId);
  strcat(message,convert);
  strcat(message,"|");
  strcat(message,teamName);
  strcat(message,"|");
  strcat(message,playerName);
  strcat(message,"|");
  snprintf(convert,8191,"%d",lat);
  strcat(message,convert);
  strcat(message,"|");
  snprintf(convert,8191, "%d", lon);
  strcat(message,convert);
  strcat(message,"|");
  strcat(message,neutralization_code);
}

//FA_CAPTURE|gameId|pebbleId|teamName|playerName|captureId
void capture_op(){
  //8k buffer
  char message[8191];
  char convert[8191];

  strcat(message, "FA_NEUTRALIZE|");
  snprintf(convert,8191,"%d",gameId);
  strcat(message,convert);
  strcat(message,"|");
  snprintf(convert,8191,"%d",pebbleId);
  strcat(message,convert);
  strcat(message,"|");
  strcat(message,teamName);
  strcat(message,"|");
  strcat(message,playerName);
  strcat(message,"|");
  strcat(message,capture_code);
} 

//*************************PHONE CONNECTION CALLBACKS*******************************************//
static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
  // The message just sent has been successfully delivered
  LOG("MESSAGE SENT");

}  

static void outbox_failed_callback(DictionaryIterator *iter,
                                      AppMessageResult reason, void *context) {
  // The message just sent failed to be delivered
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message send failed. Reason: %d", (int)reason);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  // A message was received, but had to be dropped
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int)reason);
}


//Add Check to this later
static void in_received_handler(DictionaryIterator *iter, void *context) {
  // Is the location name inside this message?
  LOG("MESSAGE RECEIVED");

   Tuple *location_tuple = dict_find(iter, AppKeyLocation);
   if(location_tuple) {
  //   // This value was stored as JS String, which is stored here as a char string
     char *location_name = location_tuple->value->cstring;

  //   // Use a static buffer to store the string for display
     static char s_buffer[8191];
     snprintf(s_buffer, 8191, "Location: %s", location_name);

    // Display in the TextLayer
    text_layer_set_text(neutralize_code_text, s_buffer);
    LOG("Log Buffer");
    LOG(s_buffer);
  }
  //For Testing
  if (!location_tuple){
    LOG("Not True");
    text_layer_set_text(neutralize_code_text, "Not Found");
  }

}
//*************************SENT INT*********************************//
static void sendInt(int key, int value) {
    LOG("Initial Send");
    // Declare the dictionary's iterator
    DictionaryIterator *iter;

    // Prepare the outbox buffer for this message
    AppMessageResult result = app_message_outbox_begin(&iter);

    if(result == APP_MSG_OK) {
      // Construct & send the message.
      dict_write_int(iter, key, &value, sizeof(int), true);
      result = app_message_outbox_send();

      if(result != APP_MSG_OK) {
        LOG("ERROROne");
      }
    } 

    else {  
        LOG("ERRORTwo");
    }
}//end of the method

//*********************************************************************//


// static void sendMessage(){
//   // Declare the dictionary's iterator
//   DictionaryIterator *out_iter;
//   // Prepare the outbox buffer for this message
//   AppMessageResult result = app_message_outbox_begin(&out_iter);

//   if(result == APP_MSG_OK) {
//     // Add an item to ask for weather data
//     int value = 0;

//     dict_write_cstring(out_iter,0,"Hello");
//     //dict_write_end(dictionaryIterator);


//    // dict_write_int(out_iter, AppKeyRequestData, &value, sizeof(int), true);

//     // Send this message
//     result = app_message_outbox_send();
//     if(result != APP_MSG_OK) {
//       APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
//     }
//   } else {
//     // The outbox cannot be used right now
//     APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
//   }

// }//end of send method

//*********************************************************************************//

//{The Player Menu}-Menu One
//Select Button for the Player Menu
void select_click_callback (MenuLayer *player_menu, MenuIndex *cell_index, void *callback_context) {
  int which = cell_index->row;
  //sendMessage();

  if (which!=0){
    //if the code is not greater than for characters 
    if (player_letter_incrementer!=16&&which>10){
      //symbol that is chosen from the keyboard
      chosen_symbol_three=symbols[which];
      //concatinate the buffer with the chose symbol to create a code (string)
      strcat(playerName, chosen_symbol_three);  
      //add the concatinated string to the navigation output showing what character have already been entered
      text_layer_set_text(player_input, playerName);
      //Sleep for 1/2 second 
      psleep(100);
      //Increment 
      player_letter_incrementer++;
    }
  }
    //Once we have 4 characters
    if (player_letter_incrementer>0&&which==0){
          //window_stack_pop(true);
          psleep(100);
          window_stack_push(choose_team_window,true);

          //timer = app_timer_register(200, getting_captured, 0);
    }
}

//{Option Menu}}-Menu Two
// This is the menu item draw callback where you specify what each item should look like
void draw_row_callback_two (GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {

    switch (cell_index->row) {
    case 0:
        menu_cell_basic_draw(ctx, cell_layer, "Neutralize Code", "Neutralize Hex Code", NULL);
        break;
    case 1:
        menu_cell_basic_draw(ctx, cell_layer, "Capture Player", "Input Player ID", NULL);
        break;
    case 2:
        menu_cell_basic_draw(ctx, cell_layer, "Hints", "My Hints", NULL);
        break; 
    case 3:
        menu_cell_basic_draw(ctx, cell_layer, "Steps", "Health", NULL);
        break;                 

    }
}
//{Option Menu}}-Menu Two
//Sets the amount of rows for the Second Menu
uint16_t num_rows_callback_two (MenuLayer *player_menu, uint16_t section_index, void *callback_context) {
  return 4;
}

//{Option Menu}}-Menu Two
//Select for the Second Menu
void select_click_callback_two (MenuLayer *player_menu, MenuIndex *cell_index, void *callback_context) {
  int option = cell_index->row;
  if (option==0){
    //Go the the neutralize screen
    window_stack_push(neutralize_window,true);
  } else{
    //Go the the capture screen
    window_stack_push(capture_window,true);
  }

}

//{Universal Key Board}}-Menu Three & Menu Four (Neutralize Menu & Capture Menu)
// This is the menu item draw callback where you specify what each item should look like
void draw_row_callback_three (GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    switch (cell_index->row) {
    case 0:
        menu_cell_basic_draw(ctx, cell_layer,symbols[0] , NULL, NULL);
        break;
    case 1:
        menu_cell_basic_draw(ctx, cell_layer, symbols[1], NULL, NULL);
        break;
    case 2:
        menu_cell_basic_draw(ctx, cell_layer, symbols[2], NULL, NULL);
        break;
    case 3:
        menu_cell_basic_draw(ctx, cell_layer, symbols[3], NULL, NULL);
        break;
    case 4:
        menu_cell_basic_draw(ctx, cell_layer, symbols[4], NULL, NULL);
        break;
    case 5:
        menu_cell_basic_draw(ctx, cell_layer, symbols[5], NULL, NULL);
        break;
    case 6:
        menu_cell_basic_draw(ctx, cell_layer, symbols[6], NULL, NULL);
        break;
    case 7:
        menu_cell_basic_draw(ctx, cell_layer, symbols[7], NULL, NULL);
        break;
    case 8:
        menu_cell_basic_draw(ctx, cell_layer, symbols[8], NULL, NULL);
        break;
    case 9:
        menu_cell_basic_draw(ctx, cell_layer, symbols[9], NULL, NULL);
        break;
    case 10:
        menu_cell_basic_draw(ctx, cell_layer, symbols[10], NULL, NULL);
        break;
    case 11:
        menu_cell_basic_draw(ctx, cell_layer, symbols[11], NULL, NULL);
        break;
    case 12:
        menu_cell_basic_draw(ctx, cell_layer, symbols[12], NULL, NULL);
        break;
    case 13:
        menu_cell_basic_draw(ctx, cell_layer, symbols[13], NULL, NULL);
        break;
    case 14:
        menu_cell_basic_draw(ctx, cell_layer, symbols[14], NULL, NULL);
        break;
    case 15:
        menu_cell_basic_draw(ctx, cell_layer, symbols[15], NULL, NULL);
        break;
    case 16:
        menu_cell_basic_draw(ctx, cell_layer, symbols[16], NULL, NULL);
        break;        
    
    }
}

//{Universal Key Board}}-Menu Three & Menu Four (Neutralize Menu & Capture Menu)
//Each section has a number of items;  we use a callback to specify this
//Sets number of rows in the menu
uint16_t num_rows_callback_three (MenuLayer *neutralize_keyboard, uint16_t section_index, void *callback_context) {
  return 17;
}

//{Key Board}}-Neutralize KeyBoard
//Select Button to input code to neutralize
void select_click_callback_three (MenuLayer *neutralize_keyboard, MenuIndex *cell_index, void *callback_context) {
  int which = cell_index->row;
  int index = sizeof(hex_test);

  if (which!=0){
    //if the code is not greater than for characters 
    if (incrementer!=4){
      //symbol that is chosen from the keyboard
      chosen_symbol=symbols[which];
      //concatinate the buffer with the chose symbol to create a code (string)
      strcat(neutralization_code, chosen_symbol);  
      //add the concatinated string to the navigation output showing what character have already been entered
      text_layer_set_text(navigation_input, neutralization_code);
      //Sleep for 1/2 second 
      psleep(100);
      //Increment 
      incrementer++;
    }
  }
    //Once we have 4 characters
    if (incrementer==4&&which==0){
      //Iterate though the array of hex_codes
      for (int i=0; i<index; i++){
        //Comapre the strings 
        if (strcmp(neutralization_code,hex_test[i]) == 0){
          //Set to true if there is a match
          match=true;
        }
      }//for loop

      //If a match is found print out the following else....
      if (match){
        text_layer_set_text(navigation_header, "Neutralized!");

      }else{
        text_layer_set_text(navigation_header, "Unable to Neutralize!");

      } 
      //Resets all of the booleans, integers, and buffers
      match=false;    
      memset(neutralization_code, 0, 10);
      incrementer=0;
      text_layer_set_text(navigation_input, neutralization_code);
    }//main if

    //If submitting before the code is at four characters
    // if (incrementer<4&&which==0){
    //   text_layer_set_text(navigation_header, "Must Be 4 Chars!");
    // }

}

//{Key Board}}-Capture KeyBoard
//Select Button to input to capture players
void select_click_callback_four (MenuLayer *capture_keyboard, MenuIndex *cell_index, void *callback_context) {
   int which = cell_index->row;
   int index = sizeof(hex_test);
  if (which!=0){

    //if the code is not greater than for characters 
    if (incrementer_two!=4){
      //symbol that is chosen from the keyboard
      chosen_symbol_two=symbols[which];
      //concatinate the buffer with the chose symbol to create a code (string)
      strcat(capture_code, chosen_symbol_two);  
      //add the concatinated string to the navigation output showing what character have already been entered
      text_layer_set_text(capture_input, capture_code);
      //Sleep for 1/2 second 
      psleep(100);
      //increment
      incrementer_two++;
    }
  }
  //Once we have 4 characters
  if (incrementer_two==4&&which==0){
    //Iterate though the array of player_codes
    for (int i=0; i<index; i++){
      //Comapre the strings 
      if (strcmp(capture_code,player_id[i]) == 0){
        //Set to true if there is a match
        match_two=true;
      }
    }//for loop

    //If a match is found print out the following else....
    if (match_two){
      text_layer_set_text(capture_header, "Player Captured!");

    }else{
      text_layer_set_text(capture_header, "Unable to Capture!");

    } 
    //Resets all of the booleans, integers, and buffers
    match_two=false;    
    memset(capture_code, 0, 10);
    incrementer_two=0;
    text_layer_set_text(capture_input, capture_code);
  }//main if

  //If submitting before the code is at four characters
  // if (incrementer_two<4&&which==0){
  //     text_layer_set_text(navigation_header, "Must Be 4 Chars!");
  // }
}

void select_click_callback_five (MenuLayer *player_menu, MenuIndex *cell_index, void *callback_context) {
  int which = cell_index->row;

  if (which!=0){
    //if the code is not greater than for characters 
    if (team_letter_incrementer!=16&&which>10){
      //symbol that is chosen from the keyboard
      chosen_symbol_four=symbols[which];
      //concatinate the buffer with the chose symbol to create a code (string)
      strcat(teamName, chosen_symbol_four);  
      //add the concatinated string to the navigation output showing what character have already been entered
      text_layer_set_text(team_input, teamName);
      //Sleep for 1/2 second 
      psleep(100);
      //Increment 
      team_letter_incrementer++;
    }
  }
    //Once we have 4 characters
    if (team_letter_incrementer>0&&which==0){
          //window_stack_pop(window_get_root_layer(window2));
          psleep(100);
          window_stack_push(window2,true);
          //window_stack_pop(window_get_root_layer(window2));
    }
}

//****************************ALL WINDOWS************************************//
//****************************WINDOW ONE-CHOOSE PLAYER***********************//
// This initializes the menu upon window load
void window_load (Window *window){
 

  //origin of the (x,y,width,height)
  //creates all of the menu layers
  player_menu = menu_layer_create(GRect(0, 80, 144, 90));

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(player_menu, window);
  

  // Create a text layer and set the text
  neutralize_code_text = text_layer_create(GRect(0, 0, 144, 30));
  text_layer_set_text(neutralize_code_text, "Input Player Name!"); 
  // Set the font and text alignment
  text_layer_set_font(neutralize_code_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(neutralize_code_text, GTextAlignmentCenter);


    // Create a text layer and set the text
  player_input = text_layer_create(GRect(0, 30, 144, 50));
  text_layer_set_text(player_input, "");
  // Set the font and text alignment
  text_layer_set_text_color(player_input, GColorBlue);
  text_layer_set_font(player_input, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(player_input, GTextAlignmentCenter);


  //These are all of the callbacks
  //Cal backs for each of the menu layers
  MenuLayerCallbacks callbacksthree = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback_three,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback_three,
    .select_click = (MenuLayerSelectCallback) select_click_callback_three
  };

  MenuLayerCallbacks callbacks = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback_three,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback_three,
    .select_click = (MenuLayerSelectCallback) select_click_callback
  };

  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(player_menu, NULL, callbacks);

  // Add it to the window for display
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(neutralize_code_text));
  layer_add_child(window_get_root_layer(window), menu_layer_get_layer(player_menu));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(player_input));


  //API & SERVER CONNECTIONS HERE

  // Register to be notified about outbox sent events
  app_message_register_outbox_sent(outbox_sent_callback);
  // Register to be notified about outbox failed events
  app_message_register_outbox_failed(outbox_failed_callback);

  app_message_register_inbox_received(in_received_handler);

  app_message_register_inbox_dropped(inbox_dropped_callback);


  sendInt(AppKeyLocation, 1);

}

//Unloads the window
void window_unload (Window *window) {

  text_layer_destroy(neutralize_code_text);
  text_layer_destroy(player_input);
  menu_layer_destroy(player_menu);
}

//******************************CAPTURE WINDOW****************************************//

void window_load_capture_player (Window *window) {

  capture_keyboard = menu_layer_create(GRect(0, 80, 144, 90));
  menu_layer_set_click_config_onto_window(capture_keyboard, capture_window);

  //Capture Player
  // Create a text layer and set the text
  capture_header = text_layer_create(GRect(0, 0, 144, 30));
  text_layer_set_text(capture_header, "Type in Player Code");
  // Set the font and text alignment
  text_layer_set_font(capture_header, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(capture_header, GTextAlignmentCenter);


  // Create a text layer and set the text
  capture_input = text_layer_create(GRect(0, 30, 144, 50));
  text_layer_set_text(capture_input, "");
  // Set the font and text alignment
  text_layer_set_text_color(capture_input, GColorRed);
  text_layer_set_font(capture_input, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(capture_input, GTextAlignmentCenter);


  MenuLayerCallbacks callbacksthree = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback_three,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback_three,
    .select_click = (MenuLayerSelectCallback) select_click_callback_three

  };

  MenuLayerCallbacks callbacksfour = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback_three,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback_three,
    .select_click = (MenuLayerSelectCallback) select_click_callback_four
  };


  menu_layer_set_callbacks(capture_keyboard, NULL, callbacksfour);

  layer_add_child(window_get_root_layer(capture_window), menu_layer_get_layer(capture_keyboard));
  layer_add_child(window_get_root_layer(capture_window), text_layer_get_layer(capture_header));
  layer_add_child(window_get_root_layer(capture_window), text_layer_get_layer(capture_input));

}


void window_unload_capture_player (Window *window) {
  text_layer_destroy(capture_input);
  text_layer_destroy(capture_header);
  menu_layer_destroy(capture_keyboard);

}

//***********************GETTING CAPTURED WINDOW***********************************************//
void window_load_getting_captured (Window *window){

  // Create a text layer and set the text
  capture_text = text_layer_create(GRect(0, 40, 144, 90));
  text_layer_set_text(capture_text, "You are being Captured");
  // Set the font and text alignment
  text_layer_set_font(capture_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(capture_text, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(getting_captured), text_layer_get_layer(capture_text));
}

void window_unload_getting_captured (Window *window) {
    text_layer_destroy(capture_text);
}

//**************************OPTIONS WINDOW********************************************//
void window_load_choose_option (Window *window){
  option_menu = menu_layer_create(GRect(0, 30, 144, 152));

  menu_layer_set_click_config_onto_window(option_menu, window2);

  // Create a text layer and set the text
  capture_code_text = text_layer_create(GRect(0, 0, 144, 30));
  text_layer_set_text(capture_code_text, "Choose An Option");
  // Set the font and text alignment
  text_layer_set_font(capture_code_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(capture_code_text, GTextAlignmentCenter);


  MenuLayerCallbacks callbackstwo = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback_two,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback_two,
    .select_click = (MenuLayerSelectCallback) select_click_callback_two
  };


  menu_layer_set_callbacks(option_menu, NULL, callbackstwo);

  layer_add_child(window_get_root_layer(window2), text_layer_get_layer(capture_code_text));
  layer_add_child(window_get_root_layer(window2), menu_layer_get_layer(option_menu));

}

void window_unload_choose_option (Window *window) {
  text_layer_destroy(capture_code_text);
  menu_layer_destroy(option_menu);
}

//***************************CHOOSE TEAM WINDOW*******************************************//
void window_load_team_input (Window *window){

  team_name_keyboard= menu_layer_create(GRect(0, 80, 144, 90));

  menu_layer_set_click_config_onto_window(team_name_keyboard, choose_team_window);

  // Create a text layer and set the text
  team_header = text_layer_create(GRect(0, 0, 144, 30));
  text_layer_set_text(team_header, "Input Team Name");
  // Set the font and text alignment
  text_layer_set_font(team_header, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(team_header, GTextAlignmentCenter);

  // Create a text layer and set the text
  team_input = text_layer_create(GRect(0, 30, 144, 50));
  text_layer_set_text(team_input, "");
  // Set the font and text alignment
  text_layer_set_text_color(team_input, GColorGreen);
  text_layer_set_font(team_input, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(team_input, GTextAlignmentCenter);


  MenuLayerCallbacks callbacksthree = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback_three,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback_three,
    .select_click = (MenuLayerSelectCallback) select_click_callback_three
  };

  MenuLayerCallbacks callbacksfive = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback_three,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback_three,
    .select_click = (MenuLayerSelectCallback) select_click_callback_five
  };

  menu_layer_set_callbacks(team_name_keyboard, NULL, callbacksfive);

  layer_add_child(window_get_root_layer(choose_team_window), menu_layer_get_layer(team_name_keyboard));
  layer_add_child(window_get_root_layer(choose_team_window), text_layer_get_layer(team_header));
  layer_add_child(window_get_root_layer(choose_team_window), text_layer_get_layer(team_input));


}

void window_unload_team_input (Window *window){

  text_layer_destroy(team_input);
  text_layer_destroy(team_header);
  menu_layer_destroy(team_name_keyboard);

}

//*******************************NEUTRALIZE CODE***************************************//
void window_load_neutralize_code (Window *window){

  neutralize_keyboard = menu_layer_create(GRect(0, 80, 144, 90));
  menu_layer_set_click_config_onto_window(neutralize_keyboard, neutralize_window);

  //Neutralize Code
  // Create a text layer and set the text
  navigation_header = text_layer_create(GRect(0, 0, 144, 30));
  text_layer_set_text(navigation_header, "Type in Code");
  // Set the font and text alignment
  text_layer_set_font(navigation_header, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(navigation_header, GTextAlignmentCenter);


  // Create a text layer and set the text
  navigation_input = text_layer_create(GRect(0, 30, 144, 50));
  text_layer_set_text(navigation_input, "");
  // Set the font and text alignment
  text_layer_set_text_color(navigation_input, GColorRed);
  text_layer_set_font(navigation_input, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(navigation_input, GTextAlignmentCenter);


  MenuLayerCallbacks callbacksthree = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback_three,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback_three,
    .select_click = (MenuLayerSelectCallback) select_click_callback_three
  };

  menu_layer_set_callbacks(neutralize_keyboard, NULL, callbacksthree);

  layer_add_child(window_get_root_layer(neutralize_window), menu_layer_get_layer(neutralize_keyboard));
  layer_add_child(window_get_root_layer(neutralize_window), text_layer_get_layer(navigation_header));
  layer_add_child(window_get_root_layer(neutralize_window), text_layer_get_layer(navigation_input));

}

void window_unload_neutralize_code (Window *window){

  text_layer_destroy(navigation_header);
  text_layer_destroy(navigation_input);
  menu_layer_destroy(neutralize_keyboard);
}

//**********************************************************************//


//Initialize Method
void init() {

  //Creates all of the windows
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  //app_message_open(inbox_size, outbox_size);
  window = window_create();
  choose_team_window=window_create();
  window2= window_create();
  neutralize_window= window_create();
  capture_window =window_create();
  getting_captured=window_create();

  Layer *window_layer = window_get_root_layer(window);


  // Setup the window handlers
  WindowHandlers handlers = {
    .load = window_load,
    .unload = window_unload
  };

  WindowHandlers handlers_two = {
    .load = window_load_getting_captured,
    .unload = window_unload_getting_captured
  };

  WindowHandlers handlers_three = {
    .load = window_load_team_input,
    .unload = window_unload_team_input
  };

  WindowHandlers handlers_four = {
    .load = window_load_choose_option,
    .unload = window_unload_choose_option
  };

  WindowHandlers handlers_five = {
    .load = window_load_neutralize_code,
    .unload = window_unload_neutralize_code
  };

  WindowHandlers handlers_six = {
    .load = window_load_capture_player,
    .unload = window_unload_capture_player
  };


  window_set_window_handlers (capture_window, (WindowHandlers) handlers_six);
  window_set_window_handlers (neutralize_window, (WindowHandlers) handlers_five);
  window_set_window_handlers (window2, (WindowHandlers) handlers_four);
  window_set_window_handlers (choose_team_window, (WindowHandlers) handlers_three);
  window_set_window_handlers (getting_captured, (WindowHandlers) handlers_two);
  window_set_window_handlers (window, (WindowHandlers) handlers);
  window_stack_push (window, true);
}


//Deinitialize Method
void deinit() {
  //Destroys all of the windows
  window_destroy(window);
  window_destroy(window2);
  window_destroy(neutralize_window);
  window_destroy(capture_window);
  window_destroy(choose_team_window);
  window_destroy(getting_captured);

}

//Main Method Runs the application 
int main(void) {
  init();
  app_event_loop();
  deinit();
}



