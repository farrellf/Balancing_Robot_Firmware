// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

#include "stm32f0xx.h"
#include "stdarg.h"
#include "f0lib_gpio.h"

/**
 * The RS232 lib is currently a VERY simple TX-only setup without flow control.
 * This code might be furthur developed later on if I have a need for more.
 *
 * Possible GPIO usage:
 *
 * USART1 TX:	PA9	AF1		PB6 AF0
 * USART1 RX:	PA10 AF1	PB7 AF0
 * USART1 CTS:	PA11 AF1
 * USART1 RTS:	PA12 AF1
 * USART1 CK:	PA8 AF1
 *
 * USART2 TX:	PA2 AF1		PA14 AF1
 * USART2 RX:	PA3 AF1		PA15 AF1
 * USART2 CTS:	PA0 AF1
 * USART2 RTS:	PA1 AF1
 * USART2 CK:	PA4 AF1
 */

/**
 * Setup one of the USARTs for RS232 TX-only communication.
 *
 * @param usart		Either USART1 or USART2
 * @param tx		TX pin
 * @param baud		The baud rate, such as 9600
 */
void rs232_setup(USART_TypeDef *usart, enum GPIO_PIN tx, uint32_t baud);

/**
 * Transmit one string over the RS232 link.
 * This function does NOT send a newline or other formatting characters. Put that in the string if desired.
 *
 * @param usart		Either USART1 or USART2
 * @param string	The string to transmit
 */
void rs232_write_string(USART_TypeDef *usart, char string[]);

/**
 * Transmit one character over the RS232 link.
 *
 * @param usart		Either USART1 or USART2
 * @param c			The character to transmit
 */
void rs232_write_char(USART_TypeDef *usart, char c);

/**
 * A crude but useful version of printf. Only three format specifiers are supported:
 * 		%d	int16_t as decimal, always padded with zeros, always with a leading + or -
 * 		%b	uint32_t as binary, with a space between each group of four bits
 * 		%s	string
 *
 * @param usart		Either USART1 or USART2
 * @param s			Format string
 * @param ...		Variables to be formatted
 */
void rs232_printf(USART_TypeDef *usart, char s[], ...);

/**
 * Prints the numerical value and a horizontal bar graph representation.
 *
 * @param usart		Either USART1 or USART2
 * @param legend	Text to show at the beginning of the line
 * @param value		Integer to print numerically and in graph form
 */
void rs232_print_graph(USART_TypeDef *usart, char legend[], int16_t value);
