#include <pebble.h>
//Deven Orie-CS50-Field Agent
//pebble clean
//pebble build
//pebble install --serial /dev/cu.PebbleTime5B57-SerialPo

//Windows for each screen of the app
Window *window, *window2, *neutralize_window, *capture_window;

//Allows for menu built on top of scroll view
MenuLayer *player_menu, *option_menu;
MenuLayer *neutralize_keyboard, *capture_keyboard;

//Theses are the text layers added to the navigation & capture windows
TextLayer *navigation_header, *navigation_input;
TextLayer *capture_header, *capture_input;
TextLayer *neutralize_code_text, *capture_code_text;


//Array of player names
char *players[]={"Deven","Drew","Shirley","David","Sam"};

//Array of the neutralization codes
char *hex_test[]={"1234", "ABCD", "A1A2"};

//Array of the players codes
char *player_id[]={"1111", "AAAA", "BBBB"};

//Symbols for the key board
char *symbols[]={"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E", "F","G","H", "I", "J", "K", "L", "M", "N",
"O","P","Q","R","S", "T", "U", "V", "W", "X", "Y", "Z"};

//Player selected in screen one
char *chosenplayer="";

//Allows for concatination with the symbol chose and add it to buf
char *chosen_symbol="";
char *chosen_symbol_two="";

//The chose symbols are added to these character arrays 
char buf[10];
char buf_two[10];

//Prevents a code from being more than 4 characters
int incrementer=0;
int incrementer_two=0;

//Allows for string compare to occur once 
bool match=false;
bool match_two=false;

//{The Player Menu}-Menu One
// This is the menu item draw callback where you specify what each item should look like
void draw_row_callback (GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {

    switch (cell_index->row) {
    case 0:
        menu_cell_basic_draw(ctx, cell_layer, players[0], "Team: Lapis", NULL);
        break;
    case 1:
        menu_cell_basic_draw(ctx, cell_layer, players[1], "Team: Drone", NULL);
        break;
    case 2:
        menu_cell_basic_draw(ctx, cell_layer, players[2], "Team: Fish", NULL);
        break;
    case 3:
        menu_cell_basic_draw(ctx, cell_layer, players[3], NULL, NULL);
        break;
    case 4:
        menu_cell_basic_draw(ctx, cell_layer, players[4], NULL, NULL);
        break;
    }
}
//The Player Menu}-Menu One
// Each section has a number of items;  we use a callback to specify this
uint16_t num_rows_callback (MenuLayer *player_menu, uint16_t section_index, void *callback_context) {
  return 5;
}

//{The Player Menu}-Menu One
//Select Button for the Player Menu
void select_click_callback (MenuLayer *player_menu, MenuIndex *cell_index, void *callback_context) {
  int which = cell_index->row;
  chosenplayer=players[which];
  //window_stack_remove(window_stack_get_top_window());
  //vibes_enqueue_custom_pattern(pattern);
  window_stack_push(window2,true);
}

//{Option Menu}}-Menu Two
// This is the menu item draw callback where you specify what each item should look like
void draw_row_callback_two (GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {

    switch (cell_index->row) {
    case 0:
        menu_cell_basic_draw(ctx, cell_layer, "Neutralize Code", NULL, NULL);
        break;
    case 1:
        menu_cell_basic_draw(ctx, cell_layer, "Capture Player", NULL, NULL);
        break;

    }
}
//{Option Menu}}-Menu Two
//Sets the amount of rows for the Second Menu
uint16_t num_rows_callback_two (MenuLayer *player_menu, uint16_t section_index, void *callback_context) {
  return 2;
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
    case 17:
        menu_cell_basic_draw(ctx, cell_layer, symbols[17], NULL, NULL);
        break;
    case 18:
        menu_cell_basic_draw(ctx, cell_layer, symbols[18], NULL, NULL);
        break;
    case 19:
        menu_cell_basic_draw(ctx, cell_layer, symbols[19], NULL, NULL);
        break;
    case 20:
        menu_cell_basic_draw(ctx, cell_layer, symbols[20], NULL, NULL);
        break;
    case 21:
        menu_cell_basic_draw(ctx, cell_layer, symbols[21], NULL, NULL);
        break;
    case 22:
        menu_cell_basic_draw(ctx, cell_layer, symbols[22], NULL, NULL);
        break;
    case 23:
        menu_cell_basic_draw(ctx, cell_layer, symbols[23], NULL, NULL);
        break;
    case 24:
        menu_cell_basic_draw(ctx, cell_layer, symbols[24], NULL, NULL);
        break;
    case 25:
        menu_cell_basic_draw(ctx, cell_layer, symbols[25], NULL, NULL);
        break;
    case 26:
        menu_cell_basic_draw(ctx, cell_layer, symbols[26], NULL, NULL);
        break;
    case 27:
        menu_cell_basic_draw(ctx, cell_layer, symbols[27], NULL, NULL);
        break;
    case 28:
        menu_cell_basic_draw(ctx, cell_layer, symbols[28], NULL, NULL);
        break;
    case 29:
        menu_cell_basic_draw(ctx, cell_layer, symbols[29], NULL, NULL);
        break;
    case 30:
        menu_cell_basic_draw(ctx, cell_layer, symbols[30], NULL, NULL);
        break;
    case 31:
        menu_cell_basic_draw(ctx, cell_layer, symbols[31], NULL, NULL);
        break;
    case 32:
        menu_cell_basic_draw(ctx, cell_layer, symbols[32], NULL, NULL);
        break;
    case 33:
        menu_cell_basic_draw(ctx, cell_layer, symbols[33], NULL, NULL);
        break;
    case 34:
        menu_cell_basic_draw(ctx, cell_layer, symbols[34], NULL, NULL);
        break;
    case 35:
        menu_cell_basic_draw(ctx, cell_layer, symbols[35], NULL, NULL);
        break;
    }
}

//{Universal Key Board}}-Menu Three & Menu Four (Neutralize Menu & Capture Menu)
//Each section has a number of items;  we use a callback to specify this
//Sets number of rows in the menu
uint16_t num_rows_callback_three (MenuLayer *neutralize_keyboard, uint16_t section_index, void *callback_context) {
  return 36;
}

//{Key Board}}-Neutralize KeyBoard
//Select Button to input code to neutralize
void select_click_callback_three (MenuLayer *neutralize_keyboard, MenuIndex *cell_index, void *callback_context) {
  int which = cell_index->row;
  int index = sizeof(hex_test);

  //if the code is not greater than for characters 
  if (incrementer!=4){
    //symbol that is chosen from the keyboard
    chosen_symbol=symbols[which];
    //concatinate the buffer with the chose symbol to create a code (string)
    strcat(buf, chosen_symbol);  
    //add the concatinated string to the navigation output showing what character have already been entered
    text_layer_set_text(navigation_input, buf);
    //Sleep for 1/2 second 
    psleep(500);
    //Increment 
    incrementer++;
  }

  //Once we have 4 characters
  if (incrementer==4){
    //Iterate though the array of hex_codes
    for (int i=0; i<index; i++){
      //Comapre the strings 
      if (strcmp(buf,hex_test[i]) == 0){
        //Set to true if there is a match
        match=true;
      }
    }//for loop

    //If a match is found print out the following else....
    if (match){
      text_layer_set_text(navigation_header, "Neutralized!");
      psleep(1000);

    }else{
      text_layer_set_text(navigation_header, "Unable to Neutralize!");
      psleep(1000);

    } 
    //Resets all of the booleans, integers, and buffers
    match=false;    
    //char buf[10];

    // for(int i = 0; i < 10; i++)
    // {
    //   buf[i]='\0';
    // }
    //strcpy(buf, "");
    memset(buf, 0, sizeof(buf));
    incrementer=0;
    text_layer_set_text(navigation_input, buf);
  }//main if

}

//{Key Board}}-Capture KeyBoard
//Select Button to input to capture players
void select_click_callback_four (MenuLayer *capture_keyboard, MenuIndex *cell_index, void *callback_context) {
   int which = cell_index->row;
   int index = sizeof(hex_test);

  //if the code is not greater than for characters 
  if (incrementer_two!=4){
    //symbol that is chosen from the keyboard
    chosen_symbol_two=symbols[which];
    //concatinate the buffer with the chose symbol to create a code (string)
    strcat(buf_two, chosen_symbol_two);  
    //add the concatinated string to the navigation output showing what character have already been entered
    text_layer_set_text(capture_input, buf_two);
    //Sleep for 1/2 second 
    psleep(500);
    //increment
    incrementer_two++;
  }
  //Once we have 4 characters
  if (incrementer_two==4){
    //Iterate though the array of player_codes
    for (int i=0; i<index; i++){
      //Comapre the strings 
      if (strcmp(buf_two,player_id[i]) == 0){
        //Set to true if there is a match
        match_two=true;
      }
    }//for loop

    //If a match is found print out the following else....
    if (match_two){
      text_layer_set_text(capture_header, "Player Captured!");
      psleep(1000);

    }else{
      text_layer_set_text(capture_header, "Unable to Capture!");
      psleep(1000);

    } 
    //Resets all of the booleans, integers, and buffers

    match_two=false;    
    //char buf[10];

    // for(int i = 0; i < 10; i++)
    // {
    //   buf[i]='\0';
    // }
    //strcpy(buf, "");
    memset(buf_two, 0, sizeof(buf_two));
    incrementer_two=0;
    text_layer_set_text(capture_input, buf_two);
  }//main if

}


// This initializes the menu upon window load
void window_load (Window *window){

  //origin of the (x,y,width,height)
  //creates all of the menu layers
  player_menu = menu_layer_create(GRect(0, 30, 144, 152));
  option_menu = menu_layer_create(GRect(0, 30, 144, 152));
  neutralize_keyboard = menu_layer_create(GRect(0, 80, 144, 90));
  capture_keyboard = menu_layer_create(GRect(0, 80, 144, 90));

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(player_menu, window);
  menu_layer_set_click_config_onto_window(option_menu, window2);
  menu_layer_set_click_config_onto_window(neutralize_keyboard, neutralize_window);
  menu_layer_set_click_config_onto_window(capture_keyboard, capture_window);



  // Create a text layer and set the text
  neutralize_code_text = text_layer_create(GRect(0, 0, 144, 30));
  text_layer_set_text(neutralize_code_text, "Pick A Player!"); 
  // Set the font and text alignment
  text_layer_set_font(neutralize_code_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(neutralize_code_text, GTextAlignmentCenter);


  // Create a text layer and set the text
  capture_code_text = text_layer_create(GRect(0, 0, 144, 30));
  text_layer_set_text(capture_code_text, "Choose An Option");
  // Set the font and text alignment
  text_layer_set_font(capture_code_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(capture_code_text, GTextAlignmentCenter);


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

  //These are all of the callbacks
  //Cal backs for each of the menu layers
  MenuLayerCallbacks callbacks = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback,
    .select_click = (MenuLayerSelectCallback) select_click_callback
  };

  MenuLayerCallbacks callbackstwo = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback_two,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback_two,
    .select_click = (MenuLayerSelectCallback) select_click_callback_two
  };

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

  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(player_menu, NULL, callbacks);
  menu_layer_set_callbacks(option_menu, NULL, callbackstwo);
  menu_layer_set_callbacks(neutralize_keyboard, NULL, callbacksthree);
  menu_layer_set_callbacks(capture_keyboard, NULL, callbacksfour);


  // Add it to the window for display
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(neutralize_code_text));
  layer_add_child(window_get_root_layer(window), menu_layer_get_layer(player_menu));
  layer_add_child(window_get_root_layer(window2), text_layer_get_layer(capture_code_text));
  layer_add_child(window_get_root_layer(window2), menu_layer_get_layer(option_menu));
  layer_add_child(window_get_root_layer(neutralize_window), menu_layer_get_layer(neutralize_keyboard));
  layer_add_child(window_get_root_layer(neutralize_window), text_layer_get_layer(navigation_header));
  layer_add_child(window_get_root_layer(neutralize_window), text_layer_get_layer(navigation_input));
  layer_add_child(window_get_root_layer(capture_window), menu_layer_get_layer(capture_keyboard));
  layer_add_child(window_get_root_layer(capture_window), text_layer_get_layer(capture_header));
  layer_add_child(window_get_root_layer(capture_window), text_layer_get_layer(capture_input));
}

//Unloads the window
void window_unload (Window *window) {

  //Destroy the text layers
  text_layer_destroy(neutralize_code_text);
  text_layer_destroy(capture_code_text);
  text_layer_destroy(navigation_header);
  text_layer_destroy(navigation_input);
  text_layer_destroy(capture_input);
  text_layer_destroy(capture_header);

  //Destroy the menu layers
  menu_layer_destroy(player_menu);
  menu_layer_destroy(option_menu);
  menu_layer_destroy(neutralize_keyboard);
  menu_layer_destroy(capture_keyboard);


}

//Initialize Method
void init() {
  //Creates all of the windows
  window = window_create();
  window2= window_create();
  neutralize_window= window_create();
  capture_window =window_create();

  Layer *window_layer = window_get_root_layer(window);


  // Setup the window handlers
  WindowHandlers handlers = {
    .load = window_load,
    .unload = window_unload
  };
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

}

//Main Method Runs the application 
int main(void) {
  init();
  app_event_loop();
  deinit();
}

