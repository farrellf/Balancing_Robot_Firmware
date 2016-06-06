// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#include "f0lib_uart.h"
#include <stdio.h>
#include <string.h>

static USART_TypeDef *usart;
static char uart_tx_buffer[1024] = {0};
static uint32_t i = 0;

/**
 * Setup one of the USARTs for TX-only communication via DMA.
 *
 * @param tx      TX pin
 * @param baud    The baud rate, such as 9600
 */
void uart_setup(enum GPIO_PIN tx_pin, uint32_t baud) {

	// determine which USART to use
	if(tx_pin == PA9 || tx_pin == PB6)
		usart = USART1;
	else if(tx_pin == PA2 || tx_pin == PA14)
		usart = USART2;
	else
		return;

	// configure the GPIO
	if(tx_pin == PB6)
		gpio_setup(tx_pin, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
	else
		gpio_setup(tx_pin, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF1);

	// enable the clock, then reset
	if(usart == USART1) {

		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;

	} else if(usart == USART2) {

		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
		RCC->APB2RSTR |= RCC_APB1RSTR_USART2RST;
		RCC->APB2RSTR &= ~RCC_APB1RSTR_USART2RST;

	}

	// enable DMA for TX
	usart->CR3 = USART_CR3_DMAT;

	// set the baud rate prescaler
	usart->BRR = SystemCoreClock / baud;

	// enable the UART and TX
	usart->CR1 = USART_CR1_UE | USART_CR1_TE;

	// enable the DMA clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;

}

void uart_send_quat(float q0, float q1, float q2, float q3) { ///////////////////////

	sprintf(uart_tx_buffer, "%2.7f %2.7f %2.7f %2.7f\n\r", q0, q1, q2, q3);
	i = strlen(uart_tx_buffer);
	uart_tx_via_dma();

}


/**
 * Effectively empties the TX buffer by placing a null character at position zero and resetting the pointer.
 */
void uart_reset_tx_buffer(void) {

	uart_tx_buffer[0] = 0;
	i = 0;

}

/**
 * Transmit the contents of uart_tx_buffer[] (until the null character) via DMA.
 */
void uart_tx_via_dma(void) {

	// wait for previous DMA transfer to finish
	while((usart->ISR & USART_ISR_TC) == 0);

	// determine which DMA channel to use
	DMA_Channel_TypeDef *dma_channel;
	if(usart == USART1)
		dma_channel = DMA1_Channel2;
	else if(usart == USART2)
		dma_channel = DMA1_Channel4;
	else
		return;

	dma_channel->CCR &= !DMA_CCR_EN;                            // disable the channel
	dma_channel->CNDTR = i;                                     // bytes to transfer
	dma_channel->CPAR = (uint32_t) &usart->TDR;                 // peripheral address
	dma_channel->CMAR = (uint32_t) uart_tx_buffer;              // memory address
	dma_channel->CCR = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_EN; // increment memory address, read from memory, enable the channel

}

/**
 * Appends a horizontal ASCII line graph to uart_tx_buffer[]. The graph looks like this:
 *
 * X Acceleration     [          *                   ]    -0.985 G
 *
 * @param name    A text to show at the left of the graph
 * @param value   The value to be graphed and also shown at the right of the graph
 * @param unit    The text to be shown at the right of the graph
 * @param min     Sets the scale of the graph
 * @param max     Sets the scale of the graph
 */
void uart_append_ascii_graph(char name[], float value, char unit[], float min, float max) {

	#define GRAPH_LENGTH 30

	// remove the existing null character
	if(i > 0)
		i--;

	uint32_t j = 0;

	// append the name
	j = 0;
	while(name[j])
		uart_tx_buffer[i++] = name[j++];

	// append the ASCII line graph
	float percentage = (value - min) / (max - min);
	int dot_location = (GRAPH_LENGTH - 1.0) * percentage;

	uart_tx_buffer[i++] = ' ';
	uart_tx_buffer[i++] = '[';
	for(j = 0; j < GRAPH_LENGTH; j++) {
		if (j == dot_location)
			uart_tx_buffer[i++] = '*';
		else
			uart_tx_buffer[i++] = ' ';
	}
	uart_tx_buffer[i++] = ']';
	uart_tx_buffer[i++] = ' ';

	// append the value
	if(value >= 0.0f) {
		uart_tx_buffer[i++] = '+';
	} else {
		uart_tx_buffer[i++] = '-';
		value *= -1.0f;
	}
	char temp[7] = {0};
	sprintf(temp, "%02.3f", value);
	j = 0;
	while(temp[j])
		uart_tx_buffer[i++] = temp[j++];

	// append the unit
	uart_tx_buffer[i++] = ' ';
	j = 0;
	while(unit[j])
		uart_tx_buffer[i++] = unit[j++];

	// append a \n\r and null character
	uart_tx_buffer[i++] = '\n';
	uart_tx_buffer[i++] = '\r';
	uart_tx_buffer[i++] = 0;

}

/**
 * Appends a \n\r\0 to uart_tx_buffer[].
 */
void uart_append_newline(void) {

	// remove the existing null character
	if(i > 0)
		i--;

	uart_tx_buffer[i++] = '\n';
	uart_tx_buffer[i++] = '\r';
	uart_tx_buffer[i++] = 0;

}

/**
 * Appends \x1B[*A\x1B[?25l to uart_tx_buffer[]. * is replaced with the actual number of lines.
 * This moves the cursor back up to the top, and hides the cursor.
 */
void uart_append_cursor_home(void) {

	uint32_t j = 0;
	uint32_t n = 0;

	// remove the existing null character
	if(i > 0)
		i--;

	// count the \n's
	while(uart_tx_buffer[j]) {
		if(uart_tx_buffer[j] == '\n')
			n++;
		j++;
	}

	char temp[16] = {0};
	sprintf(temp, "\x1B[%dA\x1B[?25l", n);

	// append the text
	j = 0;
	while(temp[j])
		uart_tx_buffer[i++] = temp[j++];

	uart_tx_buffer[i++] = 0;

}
