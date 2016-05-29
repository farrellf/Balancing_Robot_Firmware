// Written by Farrell Farahbod
// Last revised on 2014-07-31
// This file is released into the public domain

/** Basic example:
 *
 *  volatile int16_t adc_ch1 = 0;
 *
 *  // setup the ADC during power up
 *  int main() {
 *      adc_setup(1, PB0);
 *      return 0;
 *  }
 *
 *  // ISR for ADC updates the global variable
 *  void ADC1_COMP_IRQHandler() {
 *      adc_ch1 = ADC1->DR;
 *  }
 * 
 */

#include "stm32f0xx.h"
#include "stdarg.h"
#include "f0lib_gpio.h"

/**
 * Possible GPIO usage:
 *
 * PA0		ADC_IN0
 * PA1		ADC_IN1
 * PA2		ADC_IN2
 * PA3		ADC_IN3
 * PA4		ADC_IN4
 * PA5		ADC_IN5
 * PA6		ADC_IN6
 * PA7		ADC_IN7
 * 
 * PB0		ADC_IN8
 * PB1		ADC_IN9
 * 
 * PC0		ADC_IN10
 * PC1		ADC_IN11
 * PC2		ADC_IN12
 * PC3		ADC_IN13
 * PC4		ADC_IN14
 * PC5		ADC_IN15
 */
 
#ifndef F0LIB_ADC
#define F0LIB_ADC
enum ADC_CLOCK_SOURCE {CLOCK_HSI14, CLOCK_TIM1, CLOCK_TIM2, CLOCK_TIM3, CLOCK_TIM15};
#endif
 
/**
 * Setup the ADC with one or more channels.
 * The ADC ISR must be defined and used to read the acquired samples.
 *
 * @param channels	number of channels
 * @param ch1		first channel
 * @param ...		additional channels
 */
void adc_setup(enum ADC_CLOCK_SOURCE clockSource, uint8_t channels, enum GPIO_PIN ch1, ...);
