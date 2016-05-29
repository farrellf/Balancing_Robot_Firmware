// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#include "f0lib_gpio.h"

/**
 * Configure an MPU6050 and HMC5883L sensor.
 *
 * @param sck_pin   I2C clock pin
 * @param sda_pin   I2C data pin
 * @param int_pin   MPU6050 interrupt pin
 * @param handler   Pointer to an event handler that will be called after new sensor readings have been processed
 */
void mpu6050_hmc5883l_setup(enum GPIO_PIN sck_pin, enum GPIO_PIN sda_pin, enum GPIO_PIN int_pin, void (*handler)(float gyro_x, float gyro_y, float gyro_z, float accel_x, float accel_y, float accel_z, float magn_x, float magn_y, float magn_z));
