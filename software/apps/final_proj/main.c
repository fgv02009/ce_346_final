// I2C sensors app
//
// Read from I2C accelerometer/magnetometer on the Microbit

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_twi_mngr.h"
#include "nrf_drv_gpiote.h"
#include "nrfx_gpiote.h"

#include "microbit_v2.h"
#include "lsm303agr.h"
#include "led_matrix.h"
#include "nrfx_pwm.h"

// Global variables
NRF_TWI_MNGR_DEF(twi_mngr_instance, 1, 0);
uint32_t lr_distance = 0;
uint32_t ud_distance = 0;
uint8_t new_coords[2];


void pin_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
  //this is working
  //check if waiting, and start game
  printf("in button interrupt\n");
  if(game_state == Waiting){
    printf("in here\n");
    game_init();
  }
}

uint8_t* map_player_pos(){
  
  //uint8_t new_coords[2];
  if(lr_distance <= 10) new_coords[0] = 0;
  else if(lr_distance <= 20) new_coords[0] = 1;
  else if(lr_distance <= 30) new_coords[0] = 2;
  else if(lr_distance <= 40) new_coords[0] = 3;
  else if(lr_distance <= 50) new_coords[0] = 4;

  if(ud_distance <= 10) new_coords[1] = 0;
  else if(ud_distance <= 20) new_coords[1] = 1;
  else if(ud_distance <= 30) new_coords[1] = 2;
  else if(ud_distance <= 40) new_coords[1] = 3;
  else if(ud_distance <= 50) new_coords[1] = 4;

  return new_coords;
}

static void pwm_init(void) {
  // Initialize the PWM
  // SPEAKER_OUT is the output pin, mark the others as NRFX_PWM_PIN_NOT_USED
  // Set the clock to 500 kHz, count mode to Up, and load mode to Common
  // The Countertop value doesn't matter for now. We'll set it in play_tone()
  // TODO
  nrfx_pwm_config_t config = {
    .output_pins = {SPEAKER_OUT, NRFX_PWM_PIN_NOT_USED, NRFX_PWM_PIN_NOT_USED, NRFX_PWM_PIN_NOT_USED},
    .irq_priority = 4,
    .base_clock = NRF_PWM_CLK_500kHz,
    .count_mode = NRF_PWM_MODE_UP,
    .top_value = NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE,
    .load_mode = NRF_PWM_LOAD_COMMON,
    .step_mode = NRF_PWM_STEP_AUTO,
  };
  nrfx_pwm_init(&PWM_INST, &config, NULL);
}

int main(void) {
  printf("Board started!\n");

  // Initialize I2C peripheral and driver
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = I2C_SCL;
  i2c_config.sda = I2C_SDA;
  i2c_config.frequency = NRF_TWIM_FREQ_100K;
  nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);

  // Initialize the LSM303AGR accelerometer/magnetometer sensor
  lsm303agr_init(&twi_mngr_instance);
  
  //pwm
  pwm_init();

  led_matrix_init();
  app_timer_init();
  pre_game_setup();
  game_init();
  nrf_delay_ms(2000);
  
  uint8_t prev_lr = players_location[0];
  uint8_t prev_ud = players_location[1];
  float tilt_degrees = 20.0;
  // Loop forever
  while (1) {
    // Print output
    
    //calculate tilt
    if(game_state == Playing){  
      lsm303agr_tilt_measurement_t meas = calculate_tilt();
      if((meas.x_tilt > tilt_degrees)){ //&& (meas.x_tilt > prev_meas.x_tilt)){
        if(lr_distance < 40) lr_distance++;
	uint8_t new_lr = map_player_pos()[0];
	if(prev_lr != new_lr) move_right();
      }else if((meas.x_tilt < (-1*tilt_degrees))){ //&& (meas.x_tilt < prev_meas.x_tilt)) {
        if(lr_distance > 0) lr_distance--;
	uint8_t new_lr = map_player_pos()[0];
	if(prev_lr != new_lr) move_left();
      }else if((meas.y_tilt < (-1*tilt_degrees))){ //&& (meas.y_tilt < prev_meas.y_tilt)){
        if(ud_distance > 0) ud_distance--;
	uint8_t new_ud = map_player_pos()[1];
	if(prev_ud != new_ud) move_down();
      }else if((meas.y_tilt > tilt_degrees)){ //&& (meas.y_tilt > prev_meas.y_tilt)){
        if(ud_distance < 40) ud_distance++;
	uint8_t new_ud = map_player_pos()[1];
	if(prev_ud != new_ud) move_up();
      }
      prev_lr = players_location[0];
      prev_ud = players_location[1];
    }
  }
}

