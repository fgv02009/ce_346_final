// I2C sensors app
//
// Read from I2C accelerometer/magnetometer on the Microbit

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "nrf_delay.h"
#include "nrf_twi_mngr.h"

#include "microbit_v2.h"
#include "lsm303agr.h"
#include "led_matrix.h"

// Global variables
NRF_TWI_MNGR_DEF(twi_mngr_instance, 1, 0);

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

  //init led matrix
  led_matrix_init();
  //game_state = Waiting;
  game_init();
  nrf_delay_ms(3000);
  
  //not sure if this is a good idea?
  lsm303agr_tilt_measurement_t prev_meas = {0.0,0.0,0.0};
  // Loop forever
  while (1) {
    // Print output
    //calculate tilt
    if(game_state == Playing){  
      lsm303agr_tilt_measurement_t meas = calculate_tilt();
      if((meas.x_tilt > 20.0) && (meas.x_tilt > prev_meas.x_tilt)){
        move_right();
      }else if((meas.x_tilt < -20.0) && (meas.x_tilt < prev_meas.x_tilt)) {
        move_left();
      }else if((meas.y_tilt < -20.0) && (meas.y_tilt < prev_meas.y_tilt)){
        move_down();
      }else if((meas.y_tilt > 20.0) && (meas.y_tilt > prev_meas.y_tilt)){
        move_up();
      }
      prev_meas = meas;
    }
  }
  nrf_delay_ms(500);
}

