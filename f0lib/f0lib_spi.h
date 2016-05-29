// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

#include "stm32f0xx.h"
#include "f0lib_gpio.h"

/**
 * Possible GPIO usage:
 *
 * SPI1 CLK:	PA5 AF0		PB3 AF0
 * SPI1 MISO:	PA6 AF0		PB4 AF0
 * SPI1 MOSI:	PA7 AF0		PB5 AF0
 * 
 * SPI2 CLK:	PB13 AF0
 * SPI2 MISO:	PB14 AF0
 * SPI2 MOSI:	PB15 AF0
 */

#ifndef F0LIB_SPI
#define F0LIB_SPI
enum SPI_PRESCALER {PRESCALE_2 = 0,
					PRESCALE_4 = 1,
					PRESCALE_8 = 2,
					PRESCALE_16 = 3,
					PRESCALE_32 = 4,
					PRESCALE_64 = 5,
					PRESCALE_128 = 6,
					PRESCALE_256 = 7};

/**
 * Configures and enables SPI for 8bit mode, 4wire mode, with a prescaler based on PCLK.
 *
 * @param spi			Either SPI1 or SPI2
 * @param prescaler		Prescaler for the PCLK clock. By default PCLK = SYSCLK = 48MHz
 * @param clk			SPI clock pin
 * @param miso			SPI MISO pin
 * @param mosi			SPI MOSI pin
 */
void spi_setup(SPI_TypeDef *spi, enum SPI_PRESCALER prescaler, enum GPIO_PIN clk, enum GPIO_PIN miso, enum GPIO_PIN mosi);

/**
 * Writes one byte and returns the received byte
 * This function does NOT toggle the CS line -- do that manually.
 *
 * @param spi			Either SPI1 or SPI2
 * @param byte			Byte of data to send
 * @returns				Byte of data received
 */
uint8_t spi_write_byte(SPI_TypeDef *spi, uint8_t byte);

/**
 * Writes multiple bytes and returns the last received byte
 * This function does NOT toggle the CS line -- do that manually.
 *
 * @param spi			Either SPI1 or SPI2
 * @param byteCount		Number of bytes to write
 * @param firstByte		First byte of data to send
 * @param ...			Additional bytes
 * @returns				Last byte of data received
 */
uint8_t spi_write_bytes(SPI_TypeDef *spi, uint8_t byteCount, uint8_t firstByte, ...);
#endif
