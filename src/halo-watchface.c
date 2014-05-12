
#include "pebble.h"

Window *window;

TextLayer *time_layer;
// Layer to show the month; 
TextLayer *month_layer;

// Layer to show the day
TextLayer *number_day;

// Logo image
GBitmap *background_image;
//  Layer to show the image
BitmapLayer *background_layer;

GBitmap *battery_image;
BitmapLayer *battery_layer;

// Layer to show the battery %
TextLayer *battery_text_layer;

GBitmap *bluetooth_image;
BitmapLayer *bluetooth_layer;

// Invert colors
InverterLayer *inverter_layer;

Layer *window_layer;
// Array with all the battery images 
const int IMAGE_BATTERY_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_BATTERY_EMPTY,
  RESOURCE_ID_IMAGE_BATTERY_LOW,
  RESOURCE_ID_IMAGE_BATTERY_25,
  RESOURCE_ID_IMAGE_BATTERY_50,
  RESOURCE_ID_IMAGE_BATTERY_75,
  RESOURCE_ID_IMAGE_BATTERY_100,
  RESOURCE_ID_IMAGE_BATTERY_CHARGE
};

// Change the battery image depending on it's state
static void handle_battery(BatteryChargeState charge_state) {
  
  static char battery_text[] = "100%";

  // Print the & of charge
  snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  // Stablish the text into the layer
  text_layer_set_text(battery_text_layer, battery_text);

  // Choose image 
  if(charge_state.is_charging){
    battery_image = gbitmap_create_with_resource(IMAGE_BATTERY_RESOURCE_IDS[6]);
  }else if(charge_state.charge_percent >0 && charge_state.charge_percent<=10){
    battery_image = gbitmap_create_with_resource(IMAGE_BATTERY_RESOURCE_IDS[0]);
  }else if(charge_state.charge_percent >15 && charge_state.charge_percent<=25){
    battery_image = gbitmap_create_with_resource(IMAGE_BATTERY_RESOURCE_IDS[1]);
  }else if(charge_state.charge_percent>=25 && charge_state.charge_percent<50){
    battery_image = gbitmap_create_with_resource(IMAGE_BATTERY_RESOURCE_IDS[3]);
  }else if(charge_state.charge_percent>=50 && charge_state.charge_percent<75){
    battery_image = gbitmap_create_with_resource(IMAGE_BATTERY_RESOURCE_IDS[4]);
  }else{
    battery_image = gbitmap_create_with_resource(IMAGE_BATTERY_RESOURCE_IDS[5]);
  }
  // Set the image
  bitmap_layer_set_bitmap(battery_layer, battery_image);
}


// Check time and date
static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {

  static char time_text[] = "00:00";
  static char time_date[] = "Monday 00 00";
  static char *month[] = {"January", 
                          "February",
                             "March",
                             "April",
                               "May",
                              "June",
                              "July",
                            "August",
                         "September",
                           "October",
                          "November",
                          "December"};
  // Must contain the maximum number of characters you want to show
  static char day[] = "00";

  // from 12 to 00  -> black background - white images/text
  // Else invert color
  if(tick_time->tm_hour >=00 && tick_time->tm_hour<12){
    layer_set_hidden(inverter_layer_get_layer(inverter_layer), false);
  }else{
    layer_set_hidden(inverter_layer_get_layer(inverter_layer), true);
  }

  // Get the hour and date
  strftime(time_text, sizeof(time_text), "%T", tick_time);
  // Get the day
  snprintf(day, sizeof(day), "%d%%", tick_time->tm_mday );

  text_layer_set_text(time_layer, time_text);
  text_layer_set_text(month_layer, month[tick_time->tm_mon]);
  text_layer_set_text(number_day, day);
}

// Check bluetooth connection. If not connected, image will be hidden
static void handle_bluetooth(bool connected) {
  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), !connected);
}

static void init(void) {

  // Create window
  window = window_create();
  window_stack_push(window, true);

  // Set background as black
  window_set_background_color(window, GColorBlack);

  window_layer = window_get_root_layer(window);
 
  // Get the frame of the window and its bounds
  GRect frame = layer_get_frame(window_layer);
  GRect bounds = layer_get_bounds(window_layer);

  // Set the image as bluetooth.png from resources folder and defined in appinfo 
  bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
  // Create the layer to host the image
  bluetooth_layer = bitmap_layer_create(GRect(0,0,40,31));
  // Set the image into the layer
  bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_image);

  battery_text_layer = text_layer_create(GRect(70,3,30,25));
  text_layer_set_text_color(battery_text_layer, GColorWhite);
  text_layer_set_background_color(battery_text_layer, GColorClear);
  text_layer_set_text_alignment(battery_text_layer, GTextAlignmentCenter);
  text_layer_set_font(battery_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

  battery_image = gbitmap_create_with_resource(IMAGE_BATTERY_RESOURCE_IDS[0]);
  battery_layer = bitmap_layer_create(GRect(100,0,40,31));
  bitmap_layer_set_bitmap(battery_layer, battery_image);


  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  background_layer = bitmap_layer_create(GRect(0,30,frame.size.w,51));
  bitmap_layer_set_alignment(background_layer, GAlignCenter); 
  bitmap_layer_set_bitmap(background_layer, background_image);


  number_day = text_layer_create(GRect(00,80,frame.size.w,114));
  text_layer_set_text_color(number_day, GColorWhite);
  text_layer_set_background_color(number_day, GColorClear);
  text_layer_set_text_alignment(number_day,GTextAlignmentCenter);
  text_layer_set_font(number_day,
      fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HALO_SUBSET_28)));


  month_layer = text_layer_create(GRect(00, 105, frame.size.w ,114));
  text_layer_set_text_color(month_layer, GColorWhite);
  text_layer_set_background_color(month_layer, GColorClear);
  text_layer_set_text_alignment(month_layer,GTextAlignmentCenter);
  // Font size = 22
  text_layer_set_font(month_layer,
      fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HALO_SUBSET_22)));


  // Create layer in position (x,y)=(00,130) with size (width, height)= width of the window, 114px height
  time_layer = text_layer_create(GRect(00 /* posicion  x */,
                                       130 /* posicion y*/,
                                       frame.size.w /* ancho */,
                                       114 /* alto */));
  // Set the color of the text as white
  text_layer_set_text_color(time_layer, GColorWhite);
  // Set the background as transparent
  text_layer_set_background_color(time_layer, GColorClear);
  // Align the text in center
  text_layer_set_text_alignment(time_layer,GTextAlignmentCenter);

  // Set the font of the text as a custom one found in the resources folder and defined in appinfo.json
  // with size = 28
  text_layer_set_font(time_layer,
      fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HALO_SUBSET_28)));


  // Layer that inverts the color of everything above
  inverter_layer = inverter_layer_create(bounds);

  // Get current time
  time_t now = time(NULL);  
  struct tm *current_time = localtime(&now);

  //Subscribe to services. Refresh time every minute
  handle_second_tick(current_time, MINUTE_UNIT);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_second_tick);
  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  handle_bluetooth(bluetooth_connection_service_peek());
  handle_battery(battery_state_service_peek());

  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  layer_add_child(window_layer, text_layer_get_layer(month_layer));
  layer_add_child(window_layer, text_layer_get_layer(number_day));
  layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_layer));
  layer_add_child(window_layer, text_layer_get_layer(battery_text_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_layer));
  layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer));
}
// Destructor
// Destroy every element of the watchface and unsubscribe the services used
static void deinit(void){

  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  text_layer_destroy(time_layer);
  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);
  gbitmap_destroy(background_image);
  layer_remove_from_parent(bitmap_layer_get_layer(bluetooth_layer));
  bitmap_layer_destroy(bluetooth_layer);
  gbitmap_destroy(bluetooth_image);
  text_layer_destroy(battery_text_layer);
  layer_remove_from_parent(bitmap_layer_get_layer(battery_layer));
  bitmap_layer_destroy(battery_layer);
  gbitmap_destroy(battery_image);
  inverter_layer_destroy(inverter_layer);

  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
