// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

/**
 * At the moment there is only a wrapper function for toggling a GPIO.
 * This enables easy removal of debug code by commenting out the debug_io() function definition,
 * instead of having to go through all of your code or litter your code with a bunch of #ifdef's
 */

#include "f0lib_gpio.h"

#ifndef F0LIB_DEBUG
#define F0LIB_DEBUG
enum PIN_STATE {LOW, HIGH};
#endif

/** Set or clear a GPIO for debugging purposes
 *
 * @param pin		The GPIO pin
 * @param state		The new state, LOW or HIGH
 */
void debug_io(enum GPIO_PIN pin, enum PIN_STATE state);
