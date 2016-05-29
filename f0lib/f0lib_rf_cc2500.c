// Functions for interfacing with the CC2500 2.4GHz RF module

#include "stm32f0xx.h"
#include "f0lib_rf_cc2500.h"
#include "f0lib_spi.h"
#include "f0lib_gpio.h"

SPI_TypeDef *SPIx;
enum GPIO_PIN cs_pin;

/**
 * Access a configuration register on the CC2500
 * NOT used for multi-byte registers or command strobes
 *
 * @returns status byte for writes, or value for reads
 */
uint8_t cc2500_write_register(uint8_t reg, uint8_t val) {
	uint8_t status = 0;
	
	gpio_low(cs_pin);
	spi_write_byte(SPIx, reg);
	status = spi_write_byte(SPIx, val);
	gpio_high(cs_pin);
	
	return status;
}

/**
 * Send a command strobe (one-byte register access that initiates an action)
 *
 * @returns status byte
 */
uint8_t cc2500_send_strobe(uint8_t reg) {
	uint8_t status = 0;
	
	gpio_low(cs_pin);
	status = spi_write_byte(SPIx, reg);
	gpio_high(cs_pin);
	
	return status;
}

/**
 * Setup SPI, reset the CC2500, then write to registers that must be different from their reset values.
 * Register values were determined with the TI SmartRF program.
 *
 * The CC2500 supports a frequency range of 2400 - 2483.5 MHz, and it can be split up into 256 (or fewer) channels.
 * This function configures the CC2500 for 160 (0-159) channels.
 * Base frequency = 2411 MHz,  channel spacing = 405 kHz,  receiver filter bandwidth = 406 kHz.
 * Therefore: channel 0 = 2411 MHz ... channel 159 = 2475 MHz
 *
 * @param spi		SPI1 or SPI2
 * @param clk		clock pin
 * @param miso		miso pin
 * @param mosi		mosi pin
 * @param cs		chip select pin
 */
void cc2500_setup(SPI_TypeDef *spi, enum GPIO_PIN clk, enum GPIO_PIN miso, enum GPIO_PIN mosi, enum GPIO_PIN cs) {
	SPIx = spi;
	cs_pin = cs;
	
	spi_setup(spi, PRESCALE_8, clk, miso, mosi);
	gpio_setup(cs, OUTPUT, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
	gpio_high(cs_pin);
	
	// ensure cc2500 registers contain their reset values
	cc2500_send_strobe(SRES);

	// waste time for reset to complete
	volatile uint32_t i;
	for(i = 0; i < 99999; i++);

	// write to the registers that need different values
	cc2500_write_register(IOCFG0,	0x06);	// GDO0 as interrupt: asserts on sync, deasserts on end of packet
	cc2500_write_register(SYNC1,	0xBE);	// Sync word: 0xBEEF
	cc2500_write_register(SYNC0,	0xEF);	// Sync word: 0xBEEF
	cc2500_write_register(PKTLEN,	0x0C);	// Packet length: 12 bytes
	cc2500_write_register(PKTCTRL1,	0x0D);	// Packet control: CRC autoflush, append status bytes, strict address check
	cc2500_write_register(PKTCTRL0,	0x44);	// Packet control: data whitening, CRC enabled, fixed packet length mode
	cc2500_write_register(ADDR,		0x69);	// Address: 0x69
	cc2500_write_register(CHANNR,	0x7F);	// Channel: 127
	cc2500_write_register(FSCTRL1,	0x0C);	//
	cc2500_write_register(FREQ2,	0x5C);	//
	cc2500_write_register(FREQ1,	0xF6);	//
	cc2500_write_register(FREQ0,	0x27);	//
	cc2500_write_register(MDMCFG4,	0x0E);	//
	cc2500_write_register(MDMCFG3,	0x3B);	//
	cc2500_write_register(MDMCFG2,	0x73);	// Modem configuration: MSK, 30/32 sync bits detected
	cc2500_write_register(MDMCFG1,	0xC2);	// Modem configuration: FEC, 8 byte minimum preamble
	cc2500_write_register(DEVIATN,	0x00);	//
	cc2500_write_register(MCSM1,	0x0E);	// Radio state machine: stay in TX mode after sending a packet, stay in RX mode after receiving
	cc2500_write_register(MCSM0,	0x18);	//
	cc2500_write_register(FOCCFG,	0x1D);	//
	cc2500_write_register(BSCFG,	0x1C);	//
	cc2500_write_register(AGCCTRL2,	0xC7);	//
	cc2500_write_register(AGCCTRL0,	0xB0);	//
	cc2500_write_register(FREND1,	0xB6);	//
	cc2500_write_register(FSCAL3,	0xEA);	//
	cc2500_write_register(FSCAL1,	0x00);	//
	cc2500_write_register(FSCAL0,	0x19);	//
	cc2500_write_register(PATABLE,	0xFF);	// Output power: +1dBm (the maximum possible)
}

/**
 * Gets the status byte
 *
 * @returns		status
 */
uint8_t cc2500_get_status() {
	return cc2500_send_strobe(SNOP); // nop
}

/**
 * Sets the current channel.
 *
 * @param channel	channel number
 */
void cc2500_set_channel(uint8_t channel) {
	cc2500_write_register(CHANNR, channel);
}

/**
 * Gets the current channel.
 *
 * @returns 		channel number
 */
uint8_t cc2500_get_channel() {
	return cc2500_write_register(CHANNR | READ_BYTE, 0x00);
}

/**
 * Reads the RSSI value of the current channel.
 * The register contains the RSSI has a signed byte,
 * this function converts it to an unsigned integer in the 0 - 239 range. 0 = min power.
 *
 * @returns			RSSI value
 */
uint8_t cc2500_get_rssi() {
	int8_t rawRssi = cc2500_write_register(RSSI, 0x00);
	uint8_t normalized = rawRssi + 108; // (-128 to 127) -> (0 to 255) -> -20 to remove some offset
	return normalized;
}

/**
 * Flushes the receiver FIFO
 */
void cc2500_flush_rx_fifo() {
	cc2500_send_strobe(SFRX);
}

/**
 * Enters receiver mode
 */
void cc2500_enter_rx_mode() {
	cc2500_send_strobe(SRX);
}
