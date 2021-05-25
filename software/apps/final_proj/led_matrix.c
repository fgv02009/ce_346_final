// LED Matrix Driver
// Displays characters on the LED matrix

#include <stdbool.h>
#include <stdio.h>

#include "nrf_gpio.h"
#include "app_timer.h"
#include "nrf_delay.h"

#include "led_matrix.h"
#include "font.h"
#include "microbit_v2.h"

APP_TIMER_DEF(my_timer_1);
APP_TIMER_DEF(my_timer_2);
APP_TIMER_DEF(left_timer);
APP_TIMER_DEF(right_timer);
APP_TIMER_DEF(up_timer);
APP_TIMER_DEF(down_timer);
APP_TIMER_DEF(start_timer);
APP_TIMER_DEF(display_timer);
APP_TIMER_DEF(display_string_timer);
APP_TIMER_DEF(move_char_timer);

//APP_TIMER_DEF(my_timer_3);
//enum state{Waiting, Playing, Between};
//enum state game_state = Waiting;
volatile bool led_states[5][5] = {false};
uint32_t curr_row = 1;
uint32_t led_rows[] = {0, LED_ROW1, LED_ROW2, LED_ROW3, LED_ROW4, LED_ROW5};
uint32_t led_cols[] = {0, LED_COL1, LED_COL2, LED_COL3, LED_COL4, LED_COL5};
bool led_states_x[5][5] = {{true, false, false, false, true},
                           {false, true, false, true, false},
			   {false, false, true, false, false},
			   {false, true, false, true, false},
			   {true, false, false, false, true}};

bool reset = false;
uint32_t players_location[2] = {0,0};
uint32_t lose_location[2] = {4,4};
uint32_t level = 1;
uint32_t char_ind = 0;
uint8_t seconds_per_level = 10;

void game_init(){
  //pick random place for player and lose led
  game_state = Waiting;
  init_led_states();
  app_timer_init();
  start_level();
  printf("initing game\n");
  app_timer_create(&display_timer, APP_TIMER_MODE_REPEATED, display);
  app_timer_start(display_timer, 40, NULL);
}

void win(){
 printf("10 seconds passed, you beat level %d\n", level);
 
 if(level==3){
    printf("you won the whole game\n");
    char *win_str = "You win!";
    //this may need to be a timer -- one for display_str and one for display_char
    app_timer_create(&display_string_timer, APP_TIMER_MODE_REPEATED, display_string);
    app_timer_create(&move_char_timer, APP_TIMER_MODE_REPEATED, update_char_pointer);
    app_timer_start(display_string_timer, 40, win_str);
    app_timer_start(move_char_timer, 32768, NULL);
    display_string(win_str);
    
    game_state = Waiting;
    //win_game();
  } else {
    level = level + 1;
    game_state = Between;
    //wait a few seconds;
    nrf_delay_ms(3000);
    
    start_level();
  }
}

void start_level(){
  printf("starting level %d\n", level);
  //todo: reset played led and lose led
  app_timer_create(&start_timer, APP_TIMER_MODE_SINGLE_SHOT, win);
  app_timer_start(start_timer, 32768*seconds_per_level, NULL);
  game_state = Playing;
}

void init_led_states(){
  led_states[players_location[0]][players_location[1]] = true;
  //also turn on lose led here
  led_states[lose_location[0]][lose_location[1]] = true;
  //print_matrix();
}

static void display_x(){
  printf("in displayx");
  //deal with prev row
  uint32_t prev_row = curr_row == 1 ? 5 : curr_row - 1;
  nrf_gpio_pin_write(led_rows[prev_row], false);
  //change col pin states
  bool* cols_to_write = led_states_x[curr_row-1];
  deal_with_cols(cols_to_write);
  //deal with next row
  nrf_gpio_pin_write(led_rows[curr_row], true);
  curr_row = curr_row < 5 ? curr_row + 1 : 1;
}

void clear_leds(){
  nrf_gpio_pin_clear(LED_ROW1);
  nrf_gpio_pin_clear(LED_ROW2);
  nrf_gpio_pin_clear(LED_ROW3);
  nrf_gpio_pin_clear(LED_ROW4);
  nrf_gpio_pin_clear(LED_ROW5);

  nrf_gpio_pin_set(LED_COL1);
  nrf_gpio_pin_set(LED_COL2);
  nrf_gpio_pin_set(LED_COL3);
  nrf_gpio_pin_set(LED_COL4);
  nrf_gpio_pin_set(LED_COL5);
}

void lose(){
  printf("YOU LOST\n");
  app_timer_stop(start_timer);
  //id like to display x here but not working
  app_timer_create(&my_timer_1, APP_TIMER_MODE_REPEATED, display_x);
  app_timer_start(my_timer_1, 40, NULL);
  nrf_delay_ms(2000);
  /////
  clear_leds();
  //app_timer_stop(my_timer_1);
}

void display_string(void* display_str){
  char* cPtr;
  cPtr = (char*)display_str;
  //if(char_ind >= strlen(cPtr)) char_ind = 0;
  if(char_ind >= strlen(cPtr)){
    //app_timer_stop(my_timer_1);
    //app_timer_stop(my_timer_2);
    if(!reset) {
      led_matrix_init();
      reset = true;
    }
    app_timer_stop(display_string_timer);
    app_timer_stop(move_char_timer);
    return;
  }
  if(reset) reset = false;
  cPtr += char_ind;
  display_char(cPtr);
}

void display_char(char* ch){
  int ascii_i = *ch;
  uint8_t* font_row = font[ascii_i];
  for(int i = 0; i < 5; i++){
    uint8_t row = font_row[i];
    //printf("font[%d][%d] = %#2x\n",ascii_i, i, row); 
    for(int col = 0; col < 5; col++){
      uint8_t flag = ((row>>col)&1);
      //printf("row: %d, col: %d, flag: %d\n", row, col, flag);
      led_states[i][col] = flag;
    }
  }
  display(); 
}

void display(){
  if(players_location[0] == lose_location[0] && players_location[1] == lose_location[1]){
    app_timer_stop(display_timer);
    lose();
    return;
  }
  //deal with prev row
  uint32_t prev_row = curr_row == 1 ? 5 : curr_row - 1;
  nrf_gpio_pin_write(led_rows[prev_row], false);
  //change col pin states
  bool* cols_to_write = led_states[curr_row-1];
  deal_with_cols(cols_to_write);
  //deal with next row
  nrf_gpio_pin_write(led_rows[curr_row], true);
  curr_row = curr_row < 5 ? curr_row + 1 : 1;
}

void update_char_pointer(){
  char_ind++;
}

void print_matrix(){
  printf("led_states: \n");
  for(int i = 0; i < 5; i++){
    for(int j = 0; j < 5; j++){
      printf("%d", led_states[i][j]);
    }
    printf("\n");
  }
}

void move_left(){
  if(players_location[1] != 0){
    led_states[players_location[0]][players_location[1]] = false;
    players_location[1] = players_location[1] - 1;
    led_states[players_location[0]][players_location[1]] = true;
  }
}

void move_right(){
  if(players_location[1] != 4){
    led_states[players_location[0]][players_location[1]] = false;
    players_location[1] = players_location[1] +1;
    led_states[players_location[0]][players_location[1]] = true;
  }
}

void move_up(){
  if(players_location[0] != 0){
    led_states[players_location[0]][players_location[1]] = false; 
    players_location[0] = players_location[0] - 1;
    led_states[players_location[0]][players_location[1]] = true;
  }
}

void move_down(){
  if(players_location [0] != 4){
    led_states[players_location[0]][players_location[1]] = false;
    players_location[0] = players_location[0] + 1;
    led_states[players_location[0]][players_location[1]] = true;
  }
}

void deal_with_cols(bool* cols_to_write){
  //not sure ill be able to get cols in an array like this?
   for(int i = 0; i < 5; i++){
    nrf_gpio_pin_write(led_cols[i+1], !cols_to_write[i]);
  }
}

void led_matrix_init(void) {
  printf("in led matrix init\n");
  // initialize row pins
  nrf_gpio_pin_dir_set(LED_ROW1, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW2, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW3, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW4, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW5, NRF_GPIO_PIN_DIR_OUTPUT);
  
  // initialize col pins
  nrf_gpio_pin_dir_set(LED_COL1, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_COL2, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_COL3, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_COL4, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_COL5, NRF_GPIO_PIN_DIR_OUTPUT);

  // set default values for pins
  clear_leds();
};


