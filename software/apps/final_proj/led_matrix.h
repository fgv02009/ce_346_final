#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Initialize the LED matrix display
void led_matrix_init(void);

// You may need to add more functions here
void deal_with_cols(bool* cols_to_write);
void display();
void print_matrix();
void display_string(void* display_str);
void display_char(char* ch);
void next_letter();
void win();
void game_init();
void start_level();
void init_led_states();
uint32_t char_ind;

typedef struct displayConfig {
   char* display_str;
   bool  repeated;
} displayConfig;

