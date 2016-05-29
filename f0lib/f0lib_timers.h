// Written by Farrell Farahbod
// Last revised on 2014-08-03
// This file is released into the public domain

#include "stm32f0xx.h"
#include "stdarg.h"
#include "f0lib_gpio.h"

/*
ISRs:

void TIM1_BRK_UP_TRG_COM_IRQHandler(void);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM6_DAC_IRQHandler(void);
void TIM14_IRQHandler(void);
void TIM15_IRQHandler(void);
void TIM16_IRQHandler(void);
void TIM17_IRQHandler(void);


Don't forget to check for AND clear the update event flag:

if(timer->SR & TIM_SR_UIF != 0) { timer->SR &= ~TIM_SR_UIF; ... }
*/

/**
 * Possible GPIO usage:
 *
 * TIM1 CH1		= PA8  AF2
 * TIM1 CH1N	= PA7  AF2		PB13 AF2
 * TIM1 CH2		= PA9  AF2
 * TIM1 CH2N	= PB0  AF2		PB14 AF2
 * TIM1 CH3		= PA10 AF2
 * TIM1 CH3N	= PB1  AF2		PB15 AF2
 * TIM1 CH4		= PA11 AF2
 * TIM1 BKIN	= PA6  AF2		PB12 AF2
 * TIM1 ETR		= PA12 AF2
 *
 * TIM2 CH1ETR	= PA0  AF2		PA5  AF2		PA15 AF2
 * TIM2 CH2		= PA1  AF2		PB3  AF2
 * TIM2 CH3		= PA2  AF2		PB10 AF2
 * TIM2 CH4		= PA3  AF2		PB11 AF2
 *
 * TIM3 CH1		= PA6  AF1		PC6  AF			PB4  AF1
 * TIM3 CH2		= PA7  AF1		PC7  AF			PB5  AF1
 * TIM3 CH3		= PB0  AF1		PC8  AF
 * TIM3 CH4		= PB1  AF1		PC9  AF
 * TIM3 ETR		= PD2  AF
 *
 * TIM14 CH1	= PA4  AF4		PA7  AF4		PB1  AF0
 * 
 * TIM15 CH1	= PA2  AF0		PB14 AF1
 * TIM15 CH1N	= PB15 AF3
 * TIM15 CH2	= PA3  AF0		PB15 AF1
 * TIM15 BKIN	= PA9  AF0
 *
 * TIM16 CH1	= PA6  AF5		PB8  AF2
 * TIM16 CH1N	= PB6  AF2
 * TIM16 BKIN	= PB5  AF2
 * 
 * TIM17 CH1	= PA7  AF5		PB9  AF2
 * TIM17 CH1N	= PB7  AF2
 * TIM17 BKIN	= PA10 AF0
 */

#ifndef F0LIB_TIMERS
#define F0LIB_TIMERS
enum TIMER_CHANNELS {ONE_CHANNEL, TWO_CHANNELS, FOUR_CHANNELS};
enum TIMER_CHANNEL {CH1, CH2, CH3, CH4};
#endif

/**
 * Configure a timer for PWM output. All channels of the same timer share the same period.
 * Channel outputs default to a duty cycle of 0%.
 *
 * @param timer		TIM1, TIM2, TIM3, TIM14, TIM15, TIM16 or TIM17
 * @param prescaler	Perscaler for the 48MHz clock
 * @param arr		Auto-reload value, determines the period
 * @param channels	ONE_CHANNEL, TWO_CHANNELS or FOUR_CHANNELS
 * @param ch1		Pin for channel 1
 * @param ...		Pins for other channels if applicable
 */
void timer_pwm_setup(TIM_TypeDef *timer, uint32_t prescaler, uint32_t arr, enum TIMER_CHANNELS channels, enum GPIO_PIN ch1, ...);

/**
 * Change the period of a PWM timer.
 *
 * @param timer		TIM1, TIM2, TIM3, TIM14, TIM15, TIM16 or TIM17
 * @param period	Period in half microseconds
 */
void timer_pwm_period(TIM_TypeDef *timer, uint32_t period);

/**
 * Change the high-time of the PWM signal.
 *
 * @param timer		TIM1, TIM2, TIM3, TIM14, TIM15, TIM16 or TIM17
 * @param channel	CH1, CH2, CH3 or CH4
 * @param value		High time in half microseconds
 */
void timer_pwm_value(TIM_TypeDef *timer, enum TIMER_CHANNEL channel, uint32_t value);

/**
 * Configure a timer as a timebase.
 *
 * @param timer		TIM1, TIM2, TIM3, TIM14, TIM15, TIM16 or TIM17
 * @param prescaler	Perscaler for the 48MHz clock
 * @param arr		Auto-reload value, determines the period
 * @param interrupt	0=disable interrupt, 1=enable interrupt
 */
void timer_timebase_setup(TIM_TypeDef *timer, uint32_t prescaler, uint32_t arr, uint32_t interrupt);

/**
 * Configure a timer for one-pulse mode.
 *
 * @param timer		TIM1, TIM2, TIM3, TIM14, TIM15, TIM16 or TIM17
 * @param delay		Delay in milliseconds before the pulse.
 */
void timer_one_pulse_setup(TIM_TypeDef *timer, uint32_t delay);
