// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

#include "f0lib_i2c.h"
#include "stdarg.h"

/**
 * Configures the I2C peripheral.
 *
 * @param i2c     I2C1 or I2C2
 * @param speed   STANDARD_MODE_100KHZ or FAST_MODE_400KHZ or FAST_MODE_PLUS_1MHZ
 * @param sck     The pin used for the I2C clock signal
 * @param sda     The pin used for the I2C data signal
 */
void i2c_setup(I2C_TypeDef *i2c, enum I2C_SPEED speed, enum GPIO_PIN sck_pin, enum GPIO_PIN sda_pin) {

	// "unstick" any I2C slave devices that might be in a bad state by driving the clock a few times while SDA is high
	gpio_setup(sck_pin, OUTPUT, OPEN_DRAIN, FIFTY_MHZ, PULL_UP, AF1);
	gpio_setup(sda_pin, OUTPUT, OPEN_DRAIN, FIFTY_MHZ, PULL_UP, AF1);
	gpio_high(sda_pin);
	for(uint8_t i = 0; i < 10; i++) {
		gpio_low(sck_pin);
		for(volatile uint32_t j = 0; j < 1000; j++);
		gpio_high(sck_pin);
		for(volatile uint32_t j = 0; j < 1000; j++);
	}

	// configure the GPIOs
	gpio_setup(sck_pin, AF, OPEN_DRAIN, FIFTY_MHZ, PULL_UP, AF1);
	gpio_setup(sda_pin, AF, OPEN_DRAIN, FIFTY_MHZ, PULL_UP, AF1);

	if(i2c == I2C1) {

		// enable clock, then reset
		RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

		// use sysclock for i2c1 clock source
		RCC->CFGR3 |= RCC_CFGR3_I2C1SW;

	} else if(i2c == I2C2) {

		// enable clock, then reset
		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C2RST;

	}

	uint8_t presc = 15;
	uint8_t scldel = 1;
	uint8_t sdadel = 1;
	uint8_t sclh = 14;
	uint8_t scll = 14;

	// adjust the prescaler so one bit takes 30 cycles
	if(speed == STANDARD_MODE_100KHZ)
		presc = (SystemCoreClock / 3000000) - 1;  // 3MHz clock
	else if(speed == FAST_MODE_400KHZ)
		presc = (SystemCoreClock / 12000000) - 1; // 12MHz clock
	else if(speed == FAST_MODE_PLUS_1MHZ)
		presc = (SystemCoreClock / 30000000) - 1; // 30MHz clock

	// set timing
	i2c->TIMINGR = (scll << 0) | (sclh << 8) | (sdadel << 16) | (scldel << 20) | (presc << 28);

	// enable
	i2c->CR1 |= 1;

}

/**
 * Writes to one register of an I2C device
 *
 * @param i2c           I2C1 or I2C2
 * @param i2c_address   I2C device address
 * @param reg           Register being written to
 * @param value         Value for the register
 */
void i2c_write_register(I2C_TypeDef *i2c, uint8_t i2c_address, uint8_t reg, uint8_t value) {

	// write two bytes with a start bit and a stop bit
	i2c->CR2 = (i2c_address << 1) | I2C_CR2_START | I2C_CR2_AUTOEND | (2 << 16);
	while((i2c->ISR & I2C_ISR_TXIS) == 0);
	i2c->TXDR = reg;
	while((i2c->ISR & I2C_ISR_TXIS) == 0);
	i2c->TXDR = value;
	while(i2c->ISR & I2C_ISR_BUSY);

}

/**
 * Read the specified number of bytes from an I2C device
 *
 * @param i2c           I2C1 or I2C2
 * @param i2c_address   I2C device address
 * @param byte_count    Number of bytes to read
 * @param first_reg     First register to read from
 * @param rx_buffer     Pointer to an array of uint8_t's where values will be stored
 */
void i2c_read_registers(I2C_TypeDef *i2c, uint8_t i2c_address, uint8_t byte_count, uint8_t first_reg, uint8_t *rx_buffer) {

	// write one byte (the register number) with a start bit but no stop bit
	i2c->CR2 = (i2c_address << 1) | I2C_CR2_START | (1 << 16);
	while((i2c->ISR & I2C_ISR_TXIS) == 0);
	i2c->TXDR = first_reg;
	while((i2c->ISR & I2C_ISR_TC) == 0);

	// read the specified number of bytes with a start bit and a stop bit
	i2c->CR2 = (i2c_address << 1) | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_AUTOEND | (byte_count << 16);

	// wait for the bytes to arrive
	while(byte_count-- > 0) {
		while((i2c->ISR & I2C_ISR_RXNE) == 0);
		*rx_buffer++ = i2c->RXDR;
	}
}
