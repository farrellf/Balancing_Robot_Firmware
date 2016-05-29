// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

#include "stm32f0xx.h"
#include "f0lib_gpio.h"

enum I2C_SPEED {STANDARD_MODE_100KHZ, FAST_MODE_400KHZ, FAST_MODE_PLUS_1MHZ};

/**
 * Possible GPIO usage:
 *
 * I2C1 CLK:	PB6 AF1		PB8 AF1
 * I2C1 SDA:	PB7 AF1		PB9 AF1
 * I2C2 CLK:	PB10 AF1	PF6 AF
 * I2C2 SDA:	PB11 AF1	PF7 AF
 */

/**
 * Configures the I2C peripheral.
 *
 * @param i2c     I2C1 or I2C2
 * @param speed   STANDARD_MODE_100KHZ or FAST_MODE_400KHZ or FAST_MODE_PLUS_1MHZ
 * @param sck     The pin used for the I2C clock signal
 * @param sda     The pin used for the I2C data signal
 */
void i2c_setup(I2C_TypeDef *i2c, enum I2C_SPEED speed, enum GPIO_PIN sck_pin, enum GPIO_PIN sda_pin);

/**
 * Writes to one register of an I2C device
 *
 * @param i2c           I2C1 or I2C2
 * @param i2c_address   I2C device address
 * @param reg           Register being written to
 * @param value         Value for the register
 */
void i2c_write_register(I2C_TypeDef *i2c, uint8_t i2c_address, uint8_t reg, uint8_t value);

/**
 * Read the specified number of bytes from an I2C device
 *
 * @param i2c           I2C1 or I2C2
 * @param i2c_address   I2C device address
 * @param byte_count    Number of bytes to read
 * @param first_reg     First register to read from
 * @param rx_buffer     Pointer to an array of uint8_t's where values will be stored
 */
void i2c_read_registers(I2C_TypeDef *i2c, uint8_t i2c_address, uint8_t byte_count, uint8_t first_reg, uint8_t *rx_buffer);
