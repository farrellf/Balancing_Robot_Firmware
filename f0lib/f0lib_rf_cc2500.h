// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#pragma once

#include "stm32f0xx.h"
#include "f0lib_spi.h"
#include "f0lib_gpio.h"

/**
 * Gets the status byte.
 *
 * @returns    Status
 */
uint8_t cc2500_get_status(void);

/**
 * Sets the current channel.
 *
 * @param channel    Channel number
 */
void cc2500_set_channel(uint8_t channel);

/**
 * Gets the current channel.
 *
 * @returns    Channel number
 */
uint8_t cc2500_get_channel(void);

/**
 * Reads the RSSI value of the current channel.
 * The register contains the RSSI has a signed byte,
 * this function converts it to an unsigned integer in the 0 - 239 range. 0 = min power.
 *
 * @returns    RSSI
 */
uint8_t cc2500_get_rssi(void);

/**
 * Flushes the receiver FIFO.
 */
void cc2500_flush_rx_fifo(void);

/**
 * Enters receiver mode.
 */
void cc2500_enter_rx_mode(void);

/**
 * Flushes the transmit FIFO.
 */
void cc2500_flush_tx_fifo(void);

/**
 * Enters transmit mode.
 */
void cc2500_enter_tx_mode(void);

/**
 * Transmits a packet.
 *
 * @param bytes    The packet.
 */
void cc2500_transmit_packet(uint8_t bytes[]);

/**
 * Setup SPI, reset the CC2500, then write to registers that must be different from their reset values.
 * Register values were determined with the TI SmartRF program.
 *
 * The CC2500 supports a frequency range of 2400 - 2483.5 MHz, and it can be split up into 256 (or fewer) channels.
 * This function configures the CC2500 for 160 (0-159) channels.
 * Base frequency = 2411 MHz,  channel spacing = 405 kHz,  receiver filter bandwidth = 406 kHz.
 * Therefore: channel 0 = 2411 MHz ... channel 159 = 2475 MHz
 *
 * @param spi		        SPI1 or SPI2
 * @param clk		        Clock pin
 * @param miso		        MISO pin
 * @param mosi		        MOSI pin
 * @param cs		        Chip select pin
 * @param gdo0              GDO0 interrupt pin
 * @param packet_size       Number of bytes in a packet
 * @param packet_handler    Your function that will be called after successfully receiving a packet
 */
void cc2500_setup(SPI_TypeDef *spi, enum GPIO_PIN clk, enum GPIO_PIN miso, enum GPIO_PIN mosi, enum GPIO_PIN cs, enum GPIO_PIN gdo0, uint8_t packet_size, void (*packet_handler)(uint8_t byte_count, uint8_t bytes[]));
