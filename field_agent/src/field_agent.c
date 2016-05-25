#include <pebble.h>
#include <mission.h>

Window *window, *window2, *neutralize_window, *capture_window;
MenuLayer *menu_layer, *menu_layer_two;
TextLayer *s_text_layer, *s_text_layer_two;

//Array of player names
char *players[]={"Deven","Drew","Shirley","David","Sam"};

char *chosenplayer="";
// This is the menu item draw callback where you specify what each item should look like
void draw_row_callback (GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    // Which row is it?
    switch (cell_index->row) {
    case 0:
        menu_cell_basic_draw(ctx, cell_layer, players[0], NULL, NULL);
        break;
    case 1:
        menu_cell_basic_draw(ctx, cell_layer, players[1], NULL, NULL);
        break;
    case 2:
        menu_cell_basic_draw(ctx, cell_layer, players[2], NULL, NULL);
        break;
    case 3:
        menu_cell_basic_draw(ctx, cell_layer, players[3], NULL, NULL);
        break;
    case 4:
        menu_cell_basic_draw(ctx, cell_layer, players[4], NULL, NULL);
        break;
    }
}

// Each section has a number of items;  we use a callback to specify this
// You can also dynamically add and remove items using this
uint16_t num_rows_callback (MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
  return 5;
}


void draw_row_callback_two (GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    // Which row is it?
    switch (cell_index->row) {
    case 0:
        menu_cell_basic_draw(ctx, cell_layer, "Neutralize Code", NULL, NULL);
        break;
    case 1:
        menu_cell_basic_draw(ctx, cell_layer, "Capture Player", NULL, NULL);
        break;

    }
}

uint16_t num_rows_callback_two (MenuLayer *menu_layer_two, uint16_t section_index, void *callback_context) {
  return 2;
}


//Select Button 
void select_click_callback (MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  int which = cell_index->row;
  chosenplayer=players[which];
  //window_stack_remove(window_stack_get_top_window());

  uint32_t segments[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  for (int i = 0; i <= which; ++i) {
    segments[2*i] = 200;
    segments[(2*i)+1] = 100;
  }//for loop
  VibePattern pattern = {
    .durations = segments,
    .num_segments = 16
  };
  vibes_enqueue_custom_pattern(pattern);
  window_stack_push(window2,true);
}

void select_click_callback_two (MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  int option = cell_index->row;
  if (option==0){
    window_stack_push(neutralize_window,true);
  } else{
    window_stack_push(capture_window,true);
  }

}

// This initializes the menu upon window load
void window_load (Window *window){

  //origin of the (x,y,width,height)
  menu_layer = menu_layer_create(GRect(0, 30, 144, 152));
  menu_layer_two = menu_layer_create(GRect(0, 30, 144, 152));


  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(menu_layer, window);

  menu_layer_set_click_config_onto_window(menu_layer_two, window2);



  // Create a text layer and set the text
  s_text_layer = text_layer_create(GRect(0, 0, 144, 30));
  text_layer_set_text(s_text_layer, "Pick A Player!");
  
  // Set the font and text alignment
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);



  // Create a text layer and set the text
  s_text_layer_two = text_layer_create(GRect(0, 10, 144, 30));
  text_layer_set_text(s_text_layer_two, "Pick!");
  
  // Set the font and text alignment
  text_layer_set_font(s_text_layer_two, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_text_layer_two, GTextAlignmentCenter);



  //These are all of the callbacks
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
  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(menu_layer, NULL, callbacks);
  menu_layer_set_callbacks(menu_layer_two, NULL, callbackstwo);

  // Add it to the window for display
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_text_layer));
  layer_add_child(window_get_root_layer(window), menu_layer_get_layer(menu_layer));
  //layer_add_child(window_get_root_layer(window2), menu_layer_get_layer(s_text_layer_two));
  layer_add_child(window_get_root_layer(window2), menu_layer_get_layer(menu_layer_two));

}

//Unloads the window
void window_unload (Window *window) {
  //Destroy the layers
  text_layer_destroy(s_text_layer);
  menu_layer_destroy(menu_layer);
  menu_layer_destroy(menu_layer_two);
  text_layer_destroy(s_text_layer_two);

}


void init() {
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


void deinit() {
  window_destroy(window);
  window_destroy(window2);
  window_destroy(neutralize_window);
  window_destroy(capture_window);


}

int main(void) {
  init();
  app_event_loop();
  deinit();
}