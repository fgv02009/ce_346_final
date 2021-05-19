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

  led_matrix_init();



  // Loop forever
  while (1) {
    // Print output
    printf("celcius reading: %.2f\n", lsm303agr_read_temperature());

    //acc readings
    lsm303agr_measurement_t acc_meas = lsm303agr_read_accelerometer();
    printf("accel readings: %.2f, %.2f, %.2f\n", acc_meas.x_axis, acc_meas.y_axis, acc_meas.z_axis);
  
    //mag readings
    //lsm303agr_measurement_t mag_meas = lsm303agr_read_magnetometer();
    //printf("mag readings: %.2f, %.2f, %.2f\n", mag_meas.x_axis, mag_meas.y_axis, mag_meas.z_axis);
    
    //calculate tilt
    float z_tilt_deg = calculate_tilt();
    printf("z tilt reading: %.2f\n", z_tilt_deg);
    nrf_delay_ms(1000);
  }
}

