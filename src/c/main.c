/*
 * main.c
 * Creates a Window and TextLayer, then subscribes to wakeup events.
 The meaning – the literal meaning – of the word Vipassana is  “to look,” and the metaphorical meaning is  “to watch, to witness.”
 */

#include <pebble.h>
//#define aaa "Have you been aware of your Breathing? \nIf yes press Up, if no press Down."
#define WAKEUP_REASON 0
#define PERSIST_KEY_WAKEUP_ID 42

static Window *s_main_window;
static TextLayer *s_output_layer;
static WakeupId s_wakeup_id;



static void wakeup_handler(WakeupId id, int32_t reason) {
  // The app has woken!
  text_layer_set_text(s_output_layer, "Still aware? \nIf yes press Up, if no press Down."), //4th
  vibes_short_pulse(); 
 
 
  // Delete the ID
  persist_delete(PERSIST_KEY_WAKEUP_ID);
};

static void check_wakeup() {
  // Get the ID
  s_wakeup_id = persist_read_int(PERSIST_KEY_WAKEUP_ID);

  if (s_wakeup_id > 0) {
    // There is a wakeup scheduled soon
    time_t timestamp = 0;
    wakeup_query(s_wakeup_id, &timestamp);
    int seconds_remaining = timestamp - time(NULL);

    // Show how many seconds to go
    static char s_buffer[128];
    snprintf(s_buffer, sizeof(s_buffer), "You still have to be aware for %d seconds from now! Press Select to update remaining time.", seconds_remaining);
    text_layer_set_text(s_output_layer, s_buffer);
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //Check the event is not already scheduled
  if (!wakeup_query(s_wakeup_id, NULL)) {
    // Current time + 30 seconds
    time_t future_time = time(NULL) + 11;

    // Schedule wakeup event and keep the WakeupId
    s_wakeup_id = wakeup_schedule(future_time, WAKEUP_REASON, true);
    persist_write_int(PERSIST_KEY_WAKEUP_ID, s_wakeup_id);

    // Prepare for waking up later
    text_layer_set_text(s_output_layer, "This App will ask if you remained aware of your Breathing in 4 minutes.\n\nYou can close me!"); //2nd
  } else {
    // Check existing wakeup
    check_wakeup();
  }
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_output_layer, "Up");
}
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_output_layer, "Down");
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h));
  //text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  text_layer_set_text(s_output_layer, "Press SELECT and start being aware of your Breathing."); //1st
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
}

static void init(void) {
  // Create main Window
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  // Subscribe to Wakeup API
  wakeup_service_subscribe(wakeup_handler);

  // Was this a wakeup launch?
  if (launch_reason() == APP_LAUNCH_WAKEUP) {
    // The app was started by a wakeup
    WakeupId id = 0;
    int32_t reason = 0;

    // Get details and handle the wakeup
    wakeup_get_launch_event(&id, &reason);
    wakeup_handler(id, reason);
  } else {
    // Check whether a wakeup will occur soon
    check_wakeup();
  }
}

static void deinit(void) {
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
