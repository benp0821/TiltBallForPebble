#include <pebble.h>

Window *my_window;
static Layer *s_canvas_layer, *text_layer;

GRect bounds;

GPoint player_point;
GPoint objective_point;

const int16_t STARTING_RADIUS = 5;
int16_t player_radius;
const int16_t OBJECTIVE_RADIUS = 10;
int16_t xvelocity = 0;
int16_t yvelocity = 0;
int16_t hit_counter = 0;
bool game_over = true;
bool game_started = false;

bool enemy_exists = false;
struct {
  int16_t startingSide;
  int16_t startingX;
  int16_t startingY;
  int16_t radius;
  int16_t speed;
} enemy_object;

/*
* Calculates the square root of a number.
*/
float my_sqrt(const float num) {
  const uint MAX_STEPS = 40;
  const float MAX_ERROR = 0.001;
  float answer = num;
  float ans_sqr = answer * answer;
  uint step = 0;
  while((ans_sqr - num > MAX_ERROR) && (step++ < MAX_STEPS)) {
    answer = (answer + (num / answer)) / 2;
    ans_sqr = answer * answer;
  }
  return answer;
}


static void accel_data_handler(AccelData *data, uint32_t num_samples){
  // Read sample 0's x, y, and z values
  int16_t x = data[0].x;
  int16_t y = data[0].y;
  
  if(!game_over) {
    
    if (x > 0)
      xvelocity++;
    else
      xvelocity--;
    
    if (y > 0)
      yvelocity--;
    else
      yvelocity++;
    
    if (xvelocity > 5){
      xvelocity = 5;
    }
    if (yvelocity > 5){
      yvelocity = 5;
    }
    if (xvelocity < -5){
      xvelocity = -5;
    }
    if (yvelocity < -5){
      yvelocity = -5;
    }
    
    player_point.x = player_point.x + xvelocity;
    player_point.y = player_point.y + yvelocity;
    
    int16_t dx = player_point.x - objective_point.x;
    int16_t dy = player_point.y - objective_point.y;
    double distance = my_sqrt(dx * dx + dy * dy);
    
    if (distance < player_radius + OBJECTIVE_RADIUS){
      objective_point = GPoint(rand()%(bounds.size.w - 2*OBJECTIVE_RADIUS) + OBJECTIVE_RADIUS, 
                               rand()%(bounds.size.h - 2*OBJECTIVE_RADIUS) + OBJECTIVE_RADIUS);
      hit_counter++;
      if (hit_counter % 5 == 0){
        player_radius = player_radius + 1;
      }
    }
    
    
    int16_t dx2 = player_point.x - enemy_object.startingX;
    int16_t dy2 = player_point.y - enemy_object.startingY;
    double distance2 = my_sqrt(dx2 * dx2 + dy2 * dy2);
    
    if (distance2 < player_radius + enemy_object.radius){
      game_over = true;
      enemy_object.startingX = 500;
      enemy_object.startingY = 500;
    }
    
    
    if (player_point.x + player_radius > bounds.size.w || player_point.x - player_radius < 0 || 
       player_point.y + player_radius > bounds.size.h || player_point.y - player_radius < 0){
      player_point.x = bounds.size.w / 2;
      player_point.y = bounds.size.h / 2;
      player_radius = STARTING_RADIUS;
      xvelocity = 0;
      yvelocity = 0;
      objective_point = GPoint(rand()%(bounds.size.w - 2*OBJECTIVE_RADIUS) + OBJECTIVE_RADIUS, 
                               rand()%(bounds.size.h - 2*OBJECTIVE_RADIUS) + OBJECTIVE_RADIUS);
      game_over = true;
    }
    
    
    if (game_started && !enemy_exists && rand() % 500 > 200){
      enemy_object.startingSide = rand() % 4;
      
      if (enemy_object.startingSide == 0){
        enemy_object.startingX = rand() % 144;
        enemy_object.startingY = -5;
      }else if (enemy_object.startingSide == 1){
        enemy_object.startingY = rand() % 168;
        enemy_object.startingX = -5;
      }else if (enemy_object.startingSide == 2){
        enemy_object.startingX = rand() % 144;
        enemy_object.startingY = 173;
      }else{
        enemy_object.startingY = rand() % 168;
        enemy_object.startingX = 149;
      }
      
      enemy_object.radius = 3;
      enemy_object.speed = rand()%4 + 4;
      enemy_exists = true;
    }
    
  } 
  
  
  
  layer_mark_dirty(s_canvas_layer);
}

void draw_custom(Layer *this_layer, GContext *ctx){
  char hit_string[10];
  snprintf(hit_string, sizeof(hit_string), "%d", hit_counter);
  graphics_context_set_text_color(ctx, GColorBlack);
  
  if (!game_over || !game_started){
    graphics_draw_text(ctx, hit_string, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
      GRect(bounds.size.w-45,-2,40,10), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
    if (!game_started){
      graphics_context_set_stroke_width(ctx, 5);
      graphics_draw_line(ctx, GPoint(bounds.size.w - 40, bounds.size.h/2), GPoint(bounds.size.w - 5, bounds.size.h/2));
      graphics_draw_line(ctx, GPoint(bounds.size.w - 20, bounds.size.h/2 + 15), GPoint(bounds.size.w - 5, bounds.size.h/2));
      graphics_draw_line(ctx, GPoint(bounds.size.w - 20, bounds.size.h/2 - 15), GPoint(bounds.size.w - 5, bounds.size.h/2));
    }
  }else{
    enemy_exists = false;
    
    char final_score_str[100];
    snprintf(final_score_str, sizeof(final_score_str), "%s%s", "Final Score: \n", hit_string);
    graphics_draw_text(ctx, final_score_str, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
      GRect(0,bounds.size.h/4,bounds.size.w,75), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx){
  if (!game_over || !game_started){
    graphics_context_set_fill_color(ctx, GColorBlue);
    graphics_fill_circle(ctx, player_point, player_radius);
  
    graphics_context_set_fill_color(ctx, GColorJaegerGreen);
    graphics_fill_circle(ctx, objective_point, OBJECTIVE_RADIUS);
    
    if (enemy_exists){
      graphics_context_set_fill_color(ctx, GColorRed);
      if (enemy_object.startingSide == 0){
        enemy_object.startingY += enemy_object.speed; 
        if (enemy_object.startingY > 168){
          enemy_exists = false;
        }
      }else if (enemy_object.startingSide == 1){
        enemy_object.startingX += enemy_object.speed; //144
        if (enemy_object.startingX > 144){
          enemy_exists = false;
        }
      }else if (enemy_object.startingSide == 2){
        enemy_object.startingY -= enemy_object.speed; //168
        if (enemy_object.startingY < 0){
          enemy_exists = false;
        }
      }else{
        enemy_object.startingX -= enemy_object.speed; //144
        if (enemy_object.startingX < 0){
          enemy_exists = false;
        }
      }
      graphics_fill_circle(ctx, GPoint(enemy_object.startingX, enemy_object.startingY), enemy_object.radius);
    }
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context){
  if (game_over){
    game_over = false;
    game_started = true;
    hit_counter = 0;
  }
}

static void click_config_provider(void *context){
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

void handle_init(void) {
  srand(time(NULL));
  player_point = GPoint(70,80);
  player_radius = STARTING_RADIUS;
  
  my_window = window_create();
  bounds = layer_get_bounds(window_get_root_layer(my_window));
  s_canvas_layer = layer_create(bounds);
  text_layer = layer_create(bounds);
  
  objective_point = GPoint(rand()%(bounds.size.w - 2*OBJECTIVE_RADIUS) + OBJECTIVE_RADIUS, 
                               rand()%(bounds.size.h - 2*OBJECTIVE_RADIUS) + OBJECTIVE_RADIUS);
  
  uint32_t num_samples = 3;  // Number of samples per batch/callback
  // Subscribe to batched data events
  accel_data_service_subscribe(num_samples, accel_data_handler);
  
  // Assign the custom drawing procedure
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_set_update_proc(text_layer, draw_custom);
  
  window_set_click_config_provider(my_window, click_config_provider);

  // Add to Window
  layer_add_child(window_get_root_layer(my_window), s_canvas_layer);
  layer_add_child(window_get_root_layer(my_window), text_layer);
  
  window_stack_push(my_window, true);
}

void handle_deinit(void) {
  accel_data_service_unsubscribe();
  layer_destroy(s_canvas_layer);
  layer_destroy(text_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
