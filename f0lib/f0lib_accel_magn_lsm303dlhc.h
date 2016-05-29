// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

/** Basic example:
 *
 *  volatile int16_t accelX = 0;
 *  volatile int16_t accelY = 0;
 *  volatile int16_t accelZ = 0;
 *
 *  // setup the accelerometer during power up
 *  int main() {
 *      accel_lsm303dlhc_setup(I2C1, PB6, PB7, PB9); // sck, sda, drdy
 *  }
 *
 *  // ISR for External Interrupts 4 - 15
 *  void EXTI4_15_IRQHandler() {
 *      if((EXTI->PR & (1<<9)) != 0) {   // interrupt occured on EXTI 9
 *          EXTI->PR = (1<<9);			 // handling EXTI 9
 * 
 *          int16_t tempX, tempY, tempZ;
 *
 *          // sample the accelerometer and update globals
 *          accel_lsm303dlhc_get_xyz(&tempX, &tempY, &tempZ);
 *          accelX = tempX;
 *          accelY = tempY;
 *          accelZ = tempZ;
 *      }
 *  }
 *
 */

#include "stm32f0xx.h"
#include "f0lib_gpio.h"

/**
 * Configures the GPIOs and I2C, then writes to the gyro configuration registers.
 *
 * @param i2c_n		Either I2C1 or I2C2
 * @param sck		Serial Clock pin
 * @param sda		Serial Data pin
 * @param drdy		Data Ready pin
 */
void accel_lsm303dlhc_setup(I2C_TypeDef *i2c_n,
							enum GPIO_PIN sck,
							enum GPIO_PIN sda,
							enum GPIO_PIN drdy);

/**
 * Reads from the X, Y and Z data registers.
 *
 * @param x		Pointer to an int16_t for X
 * @param y		Pointer to an int16_t for Y
 * @param z		Pointer to an int16_t for Z
 */
void accel_lsm303dlhc_get_xyz(int16_t *x, int16_t *y, int16_t *z);

/**
 * Reads the value of one register.
 *
 * @param reg	Register to read
 * @returns		Register value
 */
uint8_t accel_lsm303dlhc_read_register(uint8_t reg);

/**
 * Writes the value of one register.
 *
 * @param reg		Register to write
 * @param value		Register value
 */
void accel_lsm303dlhc_write_register(uint8_t reg, uint8_t value);
