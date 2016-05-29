// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

#include "f0lib_rs232.h"
#include "f0lib_converters.h"

/**
 * Setup one of the USARTs for RS232 TX-only communication.
 *
 * @param usart		Either USART1 or USART2
 * @param tx		TX pin
 * @param baud		The baud rate, such as 9600
 */
void rs232_setup(USART_TypeDef *usart, enum GPIO_PIN tx, uint32_t baud) {

	if(tx == PB6)
		gpio_setup(tx, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
	else
		gpio_setup(tx, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF1);
	
	if(usart == USART1)
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN; // enable USART1 clock
	else if(usart == USART2)
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // enable USART2 clock
	
	usart->CR1 = 0;
	usart->CR2 = 0;
	usart->CR3 = 0;
	usart->BRR = 48000000L / baud;
	usart->GTPR = 0;
	usart->RTOR = 0;
	usart->RQR = 0;
	usart->CR1 |= USART_CR1_UE;
	usart->CR1 |= USART_CR1_TE;
}


/**
 * Transmit one string over the RS232 link.
 * This function does NOT send a newline or other formatting characters. Put that in the string if desired.
 *
 * @param usart		Either USART1 or USART2
 * @param string	The string to transmit
 */
void rs232_write_string(USART_TypeDef *usart, char string[]) {
	uint32_t i = 0;
	while(string[i] != 0) {
		while((usart->ISR & USART_ISR_TXE) == 0); // wait for TX buffer empty
		usart->TDR = string[i];
		i++;
	}
}

/**
 * Transmit one character over the RS232 link.
 *
 * @param usart		Either USART1 or USART2
 * @param c			The character to transmit
 */
void rs232_write_char(USART_TypeDef *usart, char c) {
	while((usart->ISR & USART_ISR_TXE) == 0); // wait for TX buffer empty
	usart->TDR = c;
}

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
void rs232_printf(USART_TypeDef *usart, char s[], ...) {
	va_list arglist;
	va_start(arglist, s);

	// parse the format string
	uint32_t i = 0;
	while(s[i] != 0) {
		if(s[i] != '%') { // normal char
			rs232_write_char(usart, s[i]);
			i++;
			continue;
		}
		
		i++; // skip past the % to the format specifier
		switch(s[i]) {
			case '%':
				rs232_write_char(usart, s[i]);
				i++;
				break;
			case 'd': // int16
				rs232_write_string(usart, int16_to_dec_string(va_arg(arglist, int)));
				i++;
				break;
			case 'u': // uint32
				rs232_write_string(usart, uint32_to_dec_string(va_arg(arglist, uint32_t)));
				i++;
				break;	
			case 'b':
				rs232_write_string(usart, uint32_to_bin_string(va_arg(arglist, uint32_t)));
				i++;
				break;
			case 's':
				rs232_write_string(usart, va_arg(arglist, char*));
				i++;
				break;
			default:
				return; // invalid format specifier
		}
	}
}

/**
 * Prints the numerical value and a horizontal bar graph representation.
 *
 * @param usart		Either USART1 or USART2
 * @param legend	Text to show at the beginning of the line
 * @param value		Integer to print numerically and in graph form
 */
void rs232_print_graph(USART_TypeDef *usart, char legend[], int16_t value) {
	uint8_t i;
	uint8_t marker = value/1640 + 20;
	char graph[41] = {0}; // 40 chars wide. current position marked with a *
	for(i = 0; i < 40; i++)
		if(i == marker) graph[i] = '*'; else graph[i] = ' ';
	rs232_printf(usart, "%s = %d  [%s]\n\r", legend, value, graph);
}
