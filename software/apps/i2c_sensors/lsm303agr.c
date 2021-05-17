// LSM303AGR driver for Microbit_v2
//
// Initializes sensor and communicates over I2C
// Capable of reading temperature, acceleration, and magnetic field strength

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "lsm303agr.h"
#include "nrf_delay.h"

// Pointer to an initialized I2C instance to use for transactions
static const nrf_twi_mngr_t* i2c_manager = NULL;

// Helper function to perform a 1-byte I2C read of a given register
//
// i2c_addr - address of the device to read from
// reg_addr - address of the register within the device to read
//
// returns 8-bit read value
static uint8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr) {
  uint8_t rx_buf = 0;
  nrf_twi_mngr_transfer_t const read_transfer[] = {
    NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),
    NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0)
  };
  nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);

  return rx_buf;
}

// Helper function to perform a 1-byte I2C write of a given register
//
// i2c_addr - address of the device to write to
// reg_addr - address of the register within the device to write
static void i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data) {
  //TODO: implement me
  //Note: there should only be a single two-byte transfer to be performed
  //printf("reg address: %d\n", reg_addr);
  //printf("data: %d\n", data);
  uint8_t buffer[] = {reg_addr, data};
  //uint16_t full_data = ((uint16_t)reg_addr << 8) | (uint16_t)data; 
  //printf("full_data: %d\n", full_data);
  nrf_twi_mngr_transfer_t const write_transfer[] = {
    NRF_TWI_MNGR_WRITE(i2c_addr, &buffer, 2, 0)
  };

  nrf_twi_mngr_perform(i2c_manager, NULL, write_transfer, 1, NULL);
}

// Initialize and configure the LSM303AGR accelerometer/magnetometer
//
// i2c - pointer to already initialized and enabled twim instance
void lsm303agr_init(const nrf_twi_mngr_t* i2c) {
  i2c_manager = i2c;

  // ---Initialize Accelerometer---

  // Reboot acclerometer
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_CTRL_REG5, 0x80);
  nrf_delay_ms(100); // needs delay to wait for reboot

  // Enable Block Data Update
  // Only updates sensor data when both halves of the data has been read
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_CTRL_REG4, 0x80);

  // Configure accelerometer at 100Hz, normal mode (10-bit)
  // Enable x, y and z axes
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_CTRL_REG1, 0x57);

  // Read WHO AM I register
  // Always returns the same value if working
  uint8_t result = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_WHO_AM_I_REG);
  //TODO: check the result of the Accelerometer WHO AM I register
  printf("RESULTS of agr who am i %d\n", result);

  // ---Initialize Magnetometer---

  // Reboot magnetometer
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_CFG_REG_A, 0x40);
  nrf_delay_ms(100); // needs delay to wait for reboot

  // Enable Block Data Update
  // Only updates sensor data when both halves of the data has been read
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_CFG_REG_C, 0x10);

  // Configure magnetometer at 100Hz, continuous mode
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_CFG_REG_A, 0x0C);

  // Read WHO AM I register
  result = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_WHO_AM_I_REG);
  //TODO: check the result of the Magnetometer WHO AM I register
  printf("RESULTS of mag who am i %d\n", result);
  // ---Initialize Temperature---

  // Enable temperature sensor
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_TEMP_CFG_REG, 0xC0);
}

// Read the internal temperature sensor
//
// Return measurement as floating point value in degrees C
float lsm303agr_read_temperature(void) {
  //TODO: implement me
  uint8_t lsb_temp = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_TEMP_L);
  uint8_t msb_temp = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_TEMP_H);
  int16_t reading = ((uint16_t)msb_temp << 8) | (uint16_t)lsb_temp;
  //printf("READING: %d\n", reading);
  float sensitivity = 1.0/256.0;
  float bias = 25.0;
  float celcius = (float)reading*sensitivity + bias;
  return celcius;
}

lsm303agr_measurement_t lsm303agr_read_accelerometer(void) {
  //TODO: implement me
  uint8_t lsb_acc_x = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_X_L);
  uint8_t msb_acc_x = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_X_H);
  uint8_t lsb_acc_y = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_Y_L);
  uint8_t msb_acc_y = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_Y_H);
  uint8_t lsb_acc_z = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_Z_L);
  uint8_t msb_acc_z = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_Z_H);
  //printf("lsb:: %x\n", lsb_acc_z);
  //printf("msb: %x\n", msb_acc_z);
  int16_t x_reading = ((uint16_t)msb_acc_x << 8) | (uint16_t)lsb_acc_x;
  x_reading = x_reading >> 6;
  int16_t y_reading = ((uint16_t)msb_acc_y << 8) | (uint16_t)lsb_acc_y;
  y_reading = y_reading >> 6;
  int16_t z_reading = ((uint16_t)msb_acc_z << 8) | (uint16_t)lsb_acc_z;
  z_reading = z_reading >> 6;
  //printf("z after combining: %x\n", z_reading);
  float scalar = 3.9;
  float final_x = x_reading*scalar/1000;
  float final_y = y_reading*scalar/1000;
  float final_z = z_reading*scalar/1000;
  //printf("CHECKING AT END: %d\n", final_z);
  lsm303agr_measurement_t measurement = {final_x, final_y, final_z};
  return measurement;
}

lsm303agr_measurement_t lsm303agr_read_magnetometer(void) {
  //TODO: implement me
  uint8_t lsb_mag_x = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OFFSET_X_REG_L);
  uint8_t msb_mag_x = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OFFSET_X_REG_H);
  uint8_t lsb_mag_y = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OFFSET_Y_REG_L);
  uint8_t msb_mag_y = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OFFSET_Y_REG_H);
  uint8_t lsb_mag_z = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OFFSET_Z_REG_L);
  uint8_t msb_mag_z = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OFFSET_Z_REG_H);
  //printf("lsb: %d\n", lsb_mag_z);
  //printf("msb: %d\n", msb_mag_z);
  int16_t x_reading = ((uint16_t)msb_mag_x << 8) | (uint16_t)lsb_mag_x;
  int16_t y_reading = ((uint16_t)msb_mag_y << 8) | (uint16_t)lsb_mag_y;
  int16_t z_reading = ((uint16_t)msb_mag_z << 8) | (uint16_t)lsb_mag_z;
  float sensitivity = 1.5;
  float final_x = x_reading*sensitivity/10;
  float final_y = y_reading*sensitivity/10;
  float final_z = z_reading*sensitivity/10;
  lsm303agr_measurement_t measurement = {final_x, final_y, final_z};
  return measurement;
}

float calculate_tilt(void){
  lsm303agr_measurement_t acc_meas = lsm303agr_read_accelerometer();
  float phi_rads = atan(sqrt(acc_meas.x_axis*acc_meas.x_axis + acc_meas.y_axis*acc_meas.y_axis)/acc_meas.z_axis);
  float phi_deg = phi_rads*(180/M_PI);
  return phi_deg;
}

