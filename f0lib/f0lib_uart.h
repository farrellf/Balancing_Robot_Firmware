// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#include "f0lib_gpio.h"

/**
 * Possible GPIO usage:
 *
 * USART1 TX:	PA9	AF1		PB6 AF0
 * USART2 TX:	PA2 AF1		PA14 AF1
 */

/**
 * Setup one of the USARTs for TX-only communication via DMA.
 *
 * @param tx      TX pin
 * @param baud    The baud rate, such as 9600
 */
void uart_setup(enum GPIO_PIN tx_pin, uint32_t baud);

void uart_send_quat(float q0, float q1, float q2, float q3); //////////////////

/**
 * Effectively empties the TX buffer by placing a null character at position zero and resetting the pointer.
 */
void uart_reset_tx_buffer(void);

/**
 * Transmit the contents of uart_tx_buffer[] (until the null character) via DMA.
 */
void uart_tx_via_dma(void);

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
void uart_append_ascii_graph(char name[], float value, char unit[], float min, float max);

/**
 * Appends a \n\r\0 to uart_tx_buffer[].
 */
void uart_append_newline(void);

/**
 * Appends \x1B[*A\x1B[?25l to uart_tx_buffer[]. * is replaced with the actual number of lines.
 * This moves the cursor back up to the top, and hides the cursor.
 */
void uart_append_cursor_home(void);
