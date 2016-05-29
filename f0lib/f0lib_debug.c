// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

#include "f0lib_debug.h"

/** Set or clear a GPIO for debugging purposes
 *
 * @param pin		The GPIO pin
 * @param state		The new state, LOW or HIGH
 */
void debug_io(enum GPIO_PIN pin, enum PIN_STATE state) {
	if(state == LOW)
		gpio_low(pin);
	else
		gpio_high(pin);
}
