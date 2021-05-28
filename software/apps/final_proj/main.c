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

// Global variables
NRF_TWI_MNGR_DEF(twi_mngr_instance, 1, 0);
uint32_t lr_distance = 0;
uint32_t ud_distance = 0;
uint8_t new_coords[2];

//void pin_event_handler(void) {
  // Clear interrupt event
//  NRF_GPIOTE->EVENTS_IN[0] = 0;

  // Implement me
//  printf("button pressed in interrupt\n");
//}


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
  
  
// try interrupt with drivers instead -- this is messing with everything - leave out for now
//  ret_code_t err_code;
//
//  if (!nrf_drv_gpiote_is_init())
//  {
//    err_code = nrf_drv_gpiote_init();
//    APP_ERROR_CHECK(err_code);
//  }
//  
//  nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
//  err_code = nrf_drv_gpiote_in_init(14, &config, pin_event_handler);
//  APP_ERROR_CHECK(err_code);
//  nrf_drv_gpiote_in_event_enable(14, true);

  led_matrix_init();
  app_timer_init();
  pre_game_setup();
  game_init();
  nrf_delay_ms(2000);
  
  //not sure if this is a good idea?
  //lsm303agr_tilt_measurement_t prev_meas = {0.0,0.0,0.0};
  uint8_t prev_lr = players_location[0];
  uint8_t prev_ud = players_location[1];
  // Loop forever
  while (1) {
    // Print output
    
    //calculate tilt
    if(game_state == Playing){  
      lsm303agr_tilt_measurement_t meas = calculate_tilt();
      if((meas.x_tilt > 20.0)){ //&& (meas.x_tilt > prev_meas.x_tilt)){
        if(lr_distance < 40) lr_distance++;
	uint8_t new_lr = map_player_pos()[0];
	if(prev_lr != new_lr) move_right();
        //if(lr_distance % 10 == 0) move_right();
      }else if((meas.x_tilt < -20.0)){ //&& (meas.x_tilt < prev_meas.x_tilt)) {
        if(lr_distance > 0) lr_distance--;
	uint8_t new_lr = map_player_pos()[0];
	if(prev_lr != new_lr) move_left();
	//if(lr_distance % 10 == 0) move_left();
      }else if((meas.y_tilt < -20.0)){ //&& (meas.y_tilt < prev_meas.y_tilt)){
        if(ud_distance > 0) ud_distance--;
	uint8_t new_ud = map_player_pos()[1];
	if(prev_ud != new_ud) move_down();
	//if(ud_distance % 10 == 0) move_down();
      }else if((meas.y_tilt > 20.0)){ //&& (meas.y_tilt > prev_meas.y_tilt)){
        if(ud_distance < 40) ud_distance++;
	uint8_t new_ud = map_player_pos()[1];
	if(prev_ud != new_ud) move_up();
	//if(ud_distance % 10 == 0) move_up();
      }
      prev_lr = players_location[0];
      prev_ud = players_location[1];
    }
  }
}

