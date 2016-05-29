// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#include "f0lib_gpio.h"

#ifndef F0LIB_EXTI
#define F0LIB_EXTI
enum EXTI_EDGE {RISING_EDGE, FALLING_EDGE, BOTH_EDGES};
#endif

/**
 * EXTI ISRs:
 * void EXTI0_1_IRQHandler()  // for pins 0 and 1
 * void EXTI2_3_IRQHandler()  // for pins 2 and 3
 * void EXTI4_15_IRQHandler() // for pins 4 - 15
 */

/**
 * Configures an external interrupt.
 * EXTI0 can be pin 0 of any gpio port, EXTI1 can be pin 1 of any gpio port, etc.
 * You can not have interrupts for two pins with the same number but on different ports.
 *
 * @param pin       GPIO pin used as an external interrupt
 * @param pull      NO_PULL or PULL_UP or PULL_DOWN
 * @param edge      RISING_EDGE or FALLING_EDGE or BOTH_EDGES
 * @param handler   Pointer to an event handler function
 */
void exti_setup(enum GPIO_PIN pin, enum GPIO_PULL pull, enum EXTI_EDGE edge, void (*handler)(void));

/**
 * Manually trigger an external interrupt.
 *
 * @param pin		GPIO pin associated with the interrupt.
 */
void exti_trigger(enum GPIO_PIN pin);
