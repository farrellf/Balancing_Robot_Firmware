// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

#include "stm32f0xx.h"
#include "f0lib_gpio.h"

/**
 * Configures the GPIOs and I2C, then writes to the gyro configuration registers.
 *
 * @param i2c_n		Either I2C1 or I2C2
 * @param sck		Serial Clock pin
 * @param sda		Serial Data pin
 * @param sa0		Serial Address LSB config pin
 * @param cs		Chip Select pin (used to select I2C mode)
 * @param drdy		Data Ready pin
 */
void gyro_l3gd20_setup(	I2C_TypeDef *i2c_n,
						enum GPIO_PIN sck,
						enum GPIO_PIN sda,
						/*enum GPIO_PIN sa0,
						enum GPIO_PIN cs,*/
						enum GPIO_PIN drdy);

/**
 * Reads from the X, Y and Z data registers.
 *
 * @param x		Pointer to an int16_t for X
 * @param y		Pointer to an int16_t for Y
 * @param z		Pointer to an int16_t for Z
 */
void gyro_l3gd20_get_xyz(int16_t *x, int16_t *y, int16_t *z);

/**
 * Reads the value of one register.
 *
 * @param reg	Register to read
 * @returns		Register value
 */
uint8_t gyro_l3gd20_read_register(uint8_t reg);

/**
 * Writes the value of one register.
 *
 * @param reg		Register to write
 * @param value		Register value
 */
void gyro_l3gd20_write_register(uint8_t reg, uint8_t value);
