// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

#include "f0lib_spi.h"
#include "stdarg.h"

/**
 * Configures and enables SPI for 8bit mode, 4wire mode, with a prescaler based on PCLK.
 *
 * @param spi			Either SPI1 or SPI2
 * @param prescaler		Prescaler for the PCLK clock. By default PCLK = SYSCLK = 48MHz
 * @param clk			SPI clock pin
 * @param miso			SPI MISO pin
 * @param mosi			SPI MOSI pin
 */
void spi_setup(SPI_TypeDef *spi, enum SPI_PRESCALER prescaler, enum GPIO_PIN clk, enum GPIO_PIN miso, enum GPIO_PIN mosi) {
	// reset and enable spi
	if(spi == SPI1) {
		RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;
		RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	} else if(spi == SPI2) {
		RCC->APB1RSTR |= RCC_APB1RSTR_SPI2RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI2RST;
		RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	}

	// setup registers
	spi->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_FRXTH;
	spi->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | (prescaler << 3) | SPI_CR1_SPE;

	// setup gpios
	gpio_setup(clk, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
	gpio_setup(miso, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
	gpio_setup(mosi, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
}


/**
 * Writes one byte and returns the received byte
 * This function does NOT toggle the CS line -- do that manually.
 *
 * @param spi			Either SPI1 or SPI2
 * @param byte			Byte of data to send
 * @returns				Byte of data received
 */
uint8_t spi_write_byte(SPI_TypeDef *spi, uint8_t byte) {
	uint32_t spi_dr = (uint32_t) spi;
	spi_dr += 0x0C;

	while((spi->SR & SPI_SR_TXE) == 0);				// wait for empty TX buffer
	*(volatile uint8_t*) spi_dr = byte;				// send one byte of data
	while((spi->SR & SPI_SR_RXNE) == 0);			// wait for RX buffer contents
	return *(volatile uint8_t*) spi_dr;				// return the received byte
}


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
uint8_t spi_write_bytes(SPI_TypeDef *spi, uint8_t byteCount, uint8_t firstByte, ...) {
	va_list arglist;
	va_start(arglist, firstByte);
	uint8_t returnValue = 0;
	
	uint32_t spi_dr = (uint32_t) spi;
	spi_dr += 0x0C;

	while((spi->SR & SPI_SR_TXE) == 0);				// wait for empty TX buffer
	*(volatile uint8_t*) spi_dr = firstByte;		// send first byte of data
	while((spi->SR & SPI_SR_RXNE) == 0);			// wait for RX buffer contents
	returnValue = *(volatile uint8_t*) spi_dr;		// read the received byte
	byteCount--;

	while(byteCount > 0) {
		while((spi->SR & SPI_SR_TXE) == 0);						// wait for empty TX buffer
		*(volatile uint8_t*) spi_dr = va_arg(arglist, int);		// send next byte of data
		while((spi->SR & SPI_SR_RXNE) == 0);					// wait for RX buffer contents
		returnValue = *(volatile uint8_t*) spi_dr;				// read the received byte
		byteCount--;
	}

	return returnValue;
}
