#include <pebble.h>
#include <mission.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  
#include <string.h>
#include <pebble_strtok.h>
//Desktop
//Team Lapis-CS50-Field Agent
//make install-emulator -logs
//pebble install --phone 10.31.240.237 --logs
//make start-proxy
//./game_server codedrops 23383

//Windows for each screen of the app
static Window *window, *choose_team_window, *window2, *neutralize_window, *capture_window;
static Window *getting_captured;
static Window *hint_window, *alert_window;


//Allows for menu built on top of scroll view
static MenuLayer *player_menu, *option_menu;
static MenuLayer *neutralize_keyboard, *capture_keyboard, *team_name_keyboard;

//Theses are the text layers added to the navigation & capture windows
static TextLayer *navigation_header, *navigation_input;
static TextLayer *capture_header, *capture_input;
static TextLayer *neutralize_code_text, *player_input;
static TextLayer *team_header, *team_input;
static TextLayer *capture_code_text;
static TextLayer *capture_text, *hint_text_input;
static TextLayer *alert_text_input;
static TextLayer *received_capture_code;

//Variables to update from the status sent by the game server
static char player_name[20]="0";
static char game_id[30]="0";
static char pebble_id[35]="0";
static char team_name[20]="0";
static char capture_id[10]="0";
static char guide_id[30];
static char code_drops_left[30];
static char friends[100];
static char foes[100];
static char game_over_message[8191];
static char status_req[3]="1";
static char location[90]="0";

//Second place for a FA_CAPTURE CODE for initialize Capture
static char secondary[8191];
static char code_neutral[10]="0";

//Symbols for the key board
static char *symbols[]={"SUBMIT CODE", "0","1","2","3","4","5","6","7","8","9","A","B","C","D","E", "F","G","H", "I", "J", "K", "L", "M", "N",
"O","P","Q","R","S", "T", "U", "V", "W", "X", "Y", "Z"};

//Allows for concatination with the symbol chose and add it to buf
static char *chosen_symbol="";
static char *chosen_symbol_two="";
static char *chosen_symbol_three="";
static char *chosen_symbol_four="";

//The chose symbols are added to these character arrays 
static char neutralization_code[10];
static char capture_code[10];
static char playerName[20];
static char teamName[20];

static char resp_Code_message[500];
static char hint_message[500]="HINT";
static char message[300];
static char neutralize_message[8191];
static char capture_message[8191];

static char *game_status_key_check="GAME_STATUS";
static char *game_status_capture_key_check="GS_CAPTURE_ID";
static char *game_status_game_over_key_check="GAME_OVER";
static char *game_status_gs_response_key_check="GS_RESPONSE";
static char *game_status_ga_hint_key_check="GA_HINT";


const char delim[2] = "|";

//Prevents a code from being more than 4 characters
static int incrementer=0;
static int incrementer_two=0;
static int player_letter_incrementer=0;
static int team_letter_incrementer=0;

//Allows for string compare to occur once 
static bool match=false;
static bool match_two=false;

//static bool player_registered=false;
static bool team_registered=false;

//*********************OP Codes****************************************//
//FA_LOCATION|gameId|pebbleId|teamName|playerName|lat|long|statusReq
//FA_NEUTRALIZE|gameId|pebbleId|teamName|playerName|lat|long|codeId
//FA_CAPTURE|gameId|pebbleId|teamName|playerName|captureId
//*********************Send Message****************************************//
static void sendMessage(int key, char* value) {

    // Declare the dictionary's iterator
    DictionaryIterator *iter;

    // Prepare the outbox buffer for this message
    AppMessageResult result = app_message_outbox_begin(&iter);

    if(result == APP_MSG_OK) {
      // Construct & send the message.
      dict_write_cstring(iter, key, value);
      result = app_message_outbox_send();

      //Check on if the app message
      if(result != APP_MSG_OK) {
        LOG("APP MESSAGE NOT OKAY");
      }
    } else {
        LOG("APP MESSAGE NOT OKAY");
    }
}

//*************************Message CONNECTION CALLBACKS*******************************************//
//Callback for the timer
static void app_callback(void *data){
  window_stack_pop(true);
}

//OutBox Callback//These three were used for testing purposes
static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
  // The message just sent has been successfully delivered
  //LOG("MESSAGE SENT");
}  

//Failure Outbox
static void outbox_failed_callback(DictionaryIterator *iter,
                                      AppMessageResult reason, void *context) {
  // The message just sent failed to be delivered
  //APP_LOG(APP_LOG_LEVEL_ERROR, "Message send failed. Reason: %d", (int)reason);
}

//Dropping a Callback Inbox
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  // A message was received, but had to be dropped
  //APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int)reason);
}

//Handles the messages Received
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Is the location name inside this message?
  LOG("MESSAGE RECEIVED");

    //PEBBLE ID
    Tuple *ready_tuple = dict_find(iter, AppKeyJSReady);
    if(ready_tuple) {
        // Log the value sent as part of the received message.
        char *ready_str = ready_tuple->value->cstring;
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Got AppKeyJSReady: %s", ready_str);
        strcpy(pebble_id,ready_str);
        sendMessage(AppKeyLocation,"1");
    }

    //Receiving A Message- Server
    Tuple *received_message = dict_find(iter, AppKeyRecvMsg);
    if (received_message){
      char *hold_message = received_message->value->cstring;

      //Parsing of the message sent by the game server
      LOG(hold_message);
      char token[500];
      strcpy(token ,pebble_strtok(hold_message, delim));
      //If the token/key before the first pipe is valid
      if(token!=NULL){
          //Parces through the message and sets the variables appropiately
          //GAME_STATUS|gameId|guideId|numRemainingCodeDrops|numFriendlyOperatives|numFoeOperatives
          if(strcmp(token,game_status_key_check)==0){
            strcpy(game_id, pebble_strtok(NULL, delim));
            strcpy(guide_id,pebble_strtok(NULL, delim));
            strcpy(code_drops_left,pebble_strtok(NULL, delim));
            strcpy(friends,pebble_strtok(NULL, delim));
            strcpy(foes,pebble_strtok(NULL, delim));
          }

          //GS_CAPTURE_ID|gameId|captureId
          if(strcmp(token,game_status_capture_key_check)==0){
            strcpy(game_id, pebble_strtok(NULL, delim));
            strcpy(capture_id, pebble_strtok(NULL, delim));
             window_stack_push(getting_captured, true);
             app_timer_register(60000, app_callback, NULL);
          }

          //GAME_OVER|gameId|numRemainingCodeDrops|t1:t2:...:tK
          if(strcmp(token,game_status_game_over_key_check)==0){
            pebble_strtok(NULL, delim);
            strcpy(code_drops_left, pebble_strtok(NULL, delim));
            strcpy(game_over_message, pebble_strtok(NULL, delim));
            //window_stack_push(getting_captured, true);
            //app_timer_register(60000, app_callback, NULL);
          }

          //GS_RESPONSE|gameId|respCode|message
          if(strcmp(token,game_status_gs_response_key_check)==0){
            strcpy(game_id,pebble_strtok(NULL, delim));
            pebble_strtok(NULL, delim);
            strcpy(resp_Code_message, pebble_strtok(NULL, delim));
            LOG(resp_Code_message);
          }
          //GA_HINT|gameId|guideId|teamName|playerName|pebbleId|message
          if(strcmp(token,game_status_ga_hint_key_check)==0){
            pebble_strtok(NULL, delim);
            pebble_strtok(NULL, delim);
            pebble_strtok(NULL, delim);
            pebble_strtok(NULL, delim);
            pebble_strtok(NULL, delim);
            strcpy(hint_message, pebble_strtok(NULL, delim));
            LOG(hint_message);
          }
      }
    }
    //Location Tuple
   Tuple *location_tuple = dict_find(iter, AppKeyLocation);
   if(location_tuple) {
     // This value was stored as JS String, which is stored here as a char string
     char *location_name = location_tuple->value->cstring;
     // Use a static buffer to store the string for display
     strcpy(location,location_name);
     //LOG("HERE IS THE LOCATION %s", s_buffer);
    }
}

//****************************Tick_Handler*****************************//
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  
  //Prevents Overlap of sending messages
  if (tick_time->tm_sec == 6){
    sendMessage(AppKeyLocation,"1");
  }

  if((tick_time->tm_sec % 15 == 0) &&(team_registered)){
    memset(message, 0, 300);
    snprintf(message, 300, "FA_LOCATION|%s|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, location, status_req);
    LOG(message);
    strcpy(status_req, "1");
    sendMessage(AppKeySendMsg, message);
  }
}

//********************************MENU ONE *******************************//
//{The Player Menu}-Menu One
//Select Button for the Player Menu
void select_click_callback (MenuLayer *player_menu, MenuIndex *cell_index, void *callback_context) {
  int which = cell_index->row;
  //sendMessage();

  if (which!=0){
    //if the code is not greater than for characters 
    if (player_letter_incrementer!=16){
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

          strcpy(player_name,playerName);
          psleep(100);
          window_stack_push(choose_team_window,true);
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
        menu_cell_basic_draw(ctx, cell_layer, "Request Update", "Health", NULL);
        break;                 
    }
}

//{Option Menu}}-Menu Two
//Sets the amount of rows for the Second Menu
uint16_t num_rows_callback_two (MenuLayer *option_menu, uint16_t section_index, void *callback_context) {
  return 4;
}

//{Option Menu}}-Menu Two
//Select for the Second Menu
void select_click_callback_two (MenuLayer *option_menu, MenuIndex *cell_index, void *callback_context) {
  int option = cell_index->row;

  if (option==0){
    //Go the the neutralize screen
    window_stack_push(neutralize_window,true);
  } 
  if (option==1){
    //Go the the capture screen
    strcpy(capture_id,"0");
    snprintf(secondary, 8191, "FA_CAPTURE|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, capture_id);
    sendMessage(AppKeySendMsg, secondary);
    window_stack_push(capture_window,true);
  }

  if (option==2){
    //Go to check out the hint
    window_stack_push(hint_window,true);
  }

  if (option==3){
    //Requests a hint from the game server
    sendMessage(AppKeySendMsg,message);
    psleep(100);
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
      strcpy(code_neutral,neutralization_code);

      snprintf(neutralize_message, 8191, "FA_NEUTRALIZE|%s|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, location, code_neutral);
      LOG(neutralize_message);
      sendMessage(AppKeySendMsg,neutralize_message);
      psleep(2000);

      //Resets all of the booleans, integers, and buffers
      match=false;    
      memset(neutralization_code, 0, 10);
      incrementer=0;
      text_layer_set_text(navigation_input, neutralization_code);
    }//main if
}

//{Key Board}}-Capture KeyBoard
//Select Button to input to capture players
void select_click_callback_four (MenuLayer *capture_keyboard, MenuIndex *cell_index, void *callback_context) {
   int which = cell_index->row;
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
    strcpy(capture_id,capture_code);
    snprintf(capture_message, 8191, "FA_CAPTURE|%s|%s|%s|%s|%s", game_id, pebble_id, team_name, player_name, capture_id);

    //Sends Message After Capture
    sendMessage(AppKeySendMsg,capture_message);
    LOG(capture_message);
    psleep(200);

    //Resets all of the booleans, integers, and buffers
    match_two=false;    
    memset(capture_code, 0, 10);
    incrementer_two=0;
    text_layer_set_text(capture_input, capture_code);
  }//main if
}
//Typing in a team name;
void select_click_callback_five (MenuLayer *team_name_keyboard, MenuIndex *cell_index, void *callback_context) {
  int which = cell_index->row;

  if (which!=0){
    //if the code is not greater than for characters 
    if (team_letter_incrementer!=16){
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
          strcpy(team_name,teamName);
          team_registered=true;
          psleep(100);
          window_stack_push(window2,true);
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


  //Callbacks
  MenuLayerCallbacks callbacksfour = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback_three,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback_three,
    .select_click = (MenuLayerSelectCallback) select_click_callback_four
  };

  //Layers Added and Children Added
  menu_layer_set_callbacks(capture_keyboard, NULL, callbacksfour);

  layer_add_child(window_get_root_layer(capture_window), menu_layer_get_layer(capture_keyboard));
  layer_add_child(window_get_root_layer(capture_window), text_layer_get_layer(capture_header));
  layer_add_child(window_get_root_layer(capture_window), text_layer_get_layer(capture_input));

}

//Unload the Capture Window
void window_unload_capture_player (Window *window) {
  text_layer_destroy(capture_input);
  text_layer_destroy(capture_header);
  menu_layer_destroy(capture_keyboard);

}

//***********************GETTING CAPTURED WINDOW***********************************************//
void window_load_getting_captured (Window *window){

  // Create a text layer and set the text
  capture_text = text_layer_create(GRect(0, 40, 144, 90));
  text_layer_set_text(capture_text, "Wait Being Captured! Code Below!");
  // Set the font and text alignment
  text_layer_set_font(capture_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(capture_text, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(getting_captured), text_layer_get_layer(capture_text));

  // Create a text layer and set the text
  received_capture_code = text_layer_create(GRect(0, 90, 144, 20));
  text_layer_set_text(received_capture_code, capture_id);
  // Set the font and text alignment
  text_layer_set_font(received_capture_code, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(received_capture_code, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(getting_captured), text_layer_get_layer(received_capture_code));
}

void window_unload_getting_captured (Window *window) {
    text_layer_destroy(capture_text);
    text_layer_destroy(received_capture_code);
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

  //Menu Layers
  menu_layer_set_callbacks(option_menu, NULL, callbackstwo);

  layer_add_child(window_get_root_layer(window2), text_layer_get_layer(capture_code_text));
  layer_add_child(window_get_root_layer(window2), menu_layer_get_layer(option_menu));

}
//Unload Uptions 
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


//************************HINTS WINDOW*****************************//
void window_load_hints (Window *window){
    // Create a text layer and set the text
  hint_text_input = text_layer_create(GRect(0, 40, 144, 90));
  text_layer_set_text(hint_text_input, hint_message);
  // Set the font and text alignment
  text_layer_set_font(hint_text_input, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(hint_text_input, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(hint_window), text_layer_get_layer(hint_text_input));

}

void window_unload_hints (Window *window){
      text_layer_destroy(hint_text_input);
}

//************************Alerts WINDOW*****************************//
void window_load_alert(Window *window){
  // Create a text layer and set the text
  alert_text_input = text_layer_create(GRect(0, 40, 144, 90));
  text_layer_set_text(alert_text_input, hint_message);
  // Set the font and text alignment
  text_layer_set_font(alert_text_input, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(alert_text_input, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(alert_window), text_layer_get_layer(alert_text_input));

}

void window_unload_alert (Window *window){
    layer_add_child(window_get_root_layer(alert_window), text_layer_get_layer(alert_text_input));
}

//***********************INITIALIZE & DENIT*****************************//
//Initialize Method
void init() {

  // Register to be notified about outbox sent events
  app_message_register_outbox_sent(outbox_sent_callback);
  // Register to be notified about outbox failed events
  app_message_register_outbox_failed(outbox_failed_callback);

  app_message_register_inbox_received(inbox_received_handler);

  app_message_register_inbox_dropped(inbox_dropped_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  //Creates all of the windows
  window = window_create();
  choose_team_window=window_create();
  window2= window_create();
  neutralize_window= window_create();
  capture_window =window_create();
  getting_captured=window_create();
  alert_window=window_create();
  hint_window=window_create();

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

  WindowHandlers handlers_seven = {
    .load = window_load_alert,
    .unload = window_unload_alert
  };

  WindowHandlers handlers_eight = {
    .load = window_load_hints,
    .unload = window_unload_hints
  };


  //Window Handlers for all of the windows 
  window_set_window_handlers (hint_window, (WindowHandlers) handlers_eight);
  window_set_window_handlers (alert_window, (WindowHandlers) handlers_seven);
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
  window_destroy(hint_window);
  window_destroy(alert_window);
}

//Main Method Runs the application 
int main(void) {
  init();
  app_event_loop();
  deinit();
}

