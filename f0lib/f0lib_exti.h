// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

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
 * IMPORTANT: The appropriate ISR must be defined, or code execution will halt as soon as an interrupt occurs!
 * EXTI0 can be pin 0 of any gpio port, EXTI1 can be pin 1 of any gpio port, etc.
 * You can not have interrupts for two pins with the same number but on different ports.
 *
 * @param pin		GPIO pin used as an external interrupt
 * @param edge		trigger on rising, falling, or both edges
 */
void exti_setup(enum GPIO_PIN pin, enum EXTI_EDGE edge);

/**
 * Manually trigger an external interrupt.
 *
 * @param pin		GPIO pin associated with the interrupt.
 */
void exti_trigger(enum GPIO_PIN pin);
