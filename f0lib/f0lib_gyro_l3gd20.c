// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

#include "f0lib_gyro_l3gd20.h"
#include "f0lib_exti.h"
#include "f0lib_i2c.h"

static I2C_TypeDef *i2c; // stores I2C1 or I2C2
#define GYRO_ADDRESS 0b1101011 // I2C device address

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
						enum GPIO_PIN drdy) {

	i2c = i2c_n;

//	gpio_setup(sck, AF, OPEN_DRAIN, FIFTY_MHZ, PULL_UP, AF1);
//	gpio_setup(sda, AF, OPEN_DRAIN, FIFTY_MHZ, PULL_UP, AF1);

	//gpio_setup(sa0, OUTPUT, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0); // sa0 determines LSB of i2c address
	//gpio_high(sa0); // gyo address = 0b1101011

	//gpio_setup(cs, OUTPUT, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
	//gpio_high(cs); // gyo i2c mode

	i2c_setup(i2c, FAST_MODE_400KHZ, sck, sda);

	// waste time to allow gyro to boot up
	volatile uint32_t i;
	for(i = 0; i < 100000; i++);

	// config for 190Hz, +/-250dps (1 count = 8.75 millidegrees per second)
//	i2c_write_registers(i2c, GYRO_ADDRESS, 5,	0x20,				// register address = 0x20, auto-increment
//												0b01111111,			// CTRL_REG1 value
//												0b00101001,			// CTRL_REG2 value
//												0b00001000,			// CTRL_REG3 value
//												0,					// CTRL_REG4 value
//												0);					// CTRL_REG5 value

	exti_setup(drdy, RISING_EDGE);
	exti_trigger(drdy);
}

/**
 * Reads from the X, Y and Z data registers.
 *
 * @param x		Pointer to an int16_t for X
 * @param y		Pointer to an int16_t for Y
 * @param z		Pointer to an int16_t for Z
 */
void gyro_l3gd20_get_xyz(int16_t *x, int16_t *y, int16_t *z) {
	uint8_t xl, xh, yl, yh, zl, zh;

//	i2c_read_registers(i2c, GYRO_ADDRESS, 6, 0x28, &xl, &xh, &yl, &yh, &zl, &zh);

	*x = (xh << 8) | xl;
	*y = (yh << 8) | yl;
	*z = (zh << 8) | zl;
}

/**
 * Reads the value of one register.
 *
 * @param reg	Register to read
 * @returns		Register value
 */
uint8_t gyro_l3gd20_read_register(uint8_t reg) {
	uint8_t value;
//	i2c_read_registers(i2c, GYRO_ADDRESS, 1, reg, &value);
	return value;
}

/**
 * Writes the value of one register.
 *
 * @param reg		Register to write
 * @param value		Register value
 */
void gyro_l3gd20_write_register(uint8_t reg, uint8_t value) {
//	i2c_write_registers(i2c, GYRO_ADDRESS, 2, reg, value);
}
