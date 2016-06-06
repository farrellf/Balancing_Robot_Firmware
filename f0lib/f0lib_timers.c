// Written by Farrell Farahbod
// Last revised on 2014-08-03
// This file is released into the public domain

#include "f0lib_timers.h"
#include "core_cm0.h"
#include "stm32f0xx.h"


static TIM_TypeDef *hbridge_timer;

/**
 * Configure one of the 4-channel timers for controlling dual h-bridges.
 *
 * @param ch1_pin   Pin for channel 1
 * @param ch2_pin   Pin for channel 2
 * @param ch3_pin   Pin for channel 3
 * @param ch4_pin   Pin for channel 4
 */
void timer_dual_hbridge_setup(enum GPIO_PIN ch1_pin, enum GPIO_PIN ch2_pin, enum GPIO_PIN ch3_pin, enum GPIO_PIN ch4_pin) {

	if(ch1_pin == PA8)
		hbridge_timer = TIM1;
	else if(ch1_pin == PA0 || ch1_pin == PA5 || ch1_pin == PA15)
		hbridge_timer = TIM2;
	else if(ch1_pin == PA6 || ch1_pin == PC6 || ch1_pin == PB4)
		hbridge_timer = TIM3;
	else
		return;

	// configure the GPIOs
	if(hbridge_timer == TIM1 || hbridge_timer == TIM2) {
		gpio_setup(ch1_pin, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
		gpio_setup(ch2_pin, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
		gpio_setup(ch3_pin, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
		gpio_setup(ch4_pin, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
	} else {
		gpio_setup(ch1_pin, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF1);
		gpio_setup(ch2_pin, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF1);
		gpio_setup(ch3_pin, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF1);
		gpio_setup(ch4_pin, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF1);
	}

	// enable timer clock and reset the timer
	if(hbridge_timer == TIM1) {
		RCC->APB2ENR  |=  RCC_APB2ENR_TIM1EN;
		RCC->APB2RSTR |=  RCC_APB2RSTR_TIM1RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM1RST;
	} else if(hbridge_timer == TIM2) {
		RCC->APB1ENR  |=  RCC_APB1ENR_TIM2EN;
		RCC->APB1RSTR |=  RCC_APB1RSTR_TIM2RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;
	} else if(hbridge_timer == TIM3) {
		RCC->APB1ENR  |=  RCC_APB1ENR_TIM3EN;
		RCC->APB1RSTR |=  RCC_APB1RSTR_TIM3RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM3RST;
	}

	// prescaler
	hbridge_timer->PSC = 1;

	// auto-reload
	hbridge_timer->ARR = 999;

	// auto-reload preload enable
	hbridge_timer->CR1 |= TIM_CR1_ARPE;

	// pwm mode for each channel
	hbridge_timer->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // OC1M = 110 for PWM Mode 1 output on ch1
	hbridge_timer->CCMR1 |= TIM_CCMR1_OC1PE;                     // Output 1 preload enable
	hbridge_timer->CCER  |= TIM_CCER_CC1E;                       // Enable output for ch1
	hbridge_timer->CCR1   = 0;                                   // 0% duty cycle for ch1

	hbridge_timer->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2; // OC2M = 110 for PWM Mode 1 output on ch2
	hbridge_timer->CCMR1 |= TIM_CCMR1_OC2PE;                     // Output 2 preload enable
	hbridge_timer->CCER  |= TIM_CCER_CC2E;                       // Enable output for ch2
	hbridge_timer->CCR2   = 0;                                   // 0% duty cycle for ch2

	hbridge_timer->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2; // OC3M = 110 for PWM Mode 1 output on ch3
	hbridge_timer->CCMR2 |= TIM_CCMR2_OC3PE;                     // Output 3 preload enable
	hbridge_timer->CCER  |= TIM_CCER_CC3E;                       // Enable output for ch3
	hbridge_timer->CCR3   = 0;                                   // 0% duty cycle for ch3

	hbridge_timer->CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2; // OC4M = 110 for PWM Mode 1 output on ch4
	hbridge_timer->CCMR2 |= TIM_CCMR2_OC4PE;                     // Output 4 preload enable
	hbridge_timer->CCER  |= TIM_CCER_CC4E;                       // Enable output for ch4
	hbridge_timer->CCR4   = 0;                                   // 0% duty cycle for ch4

	// force update, then clear the update flag
	hbridge_timer->EGR |= TIM_EGR_UG;
	hbridge_timer->SR &= ~TIM_SR_UIF;

	// disable interrupts
	if(hbridge_timer == TIM1)
		NVIC_DisableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
	else if(hbridge_timer == TIM2)
		NVIC_DisableIRQ(TIM2_IRQn);
	else if(hbridge_timer == TIM3)
		NVIC_DisableIRQ(TIM3_IRQn);

	// enable counter
	hbridge_timer->CR1 |= TIM_CR1_CEN;

	// main output enable is only needed for advanced control timers
	if(hbridge_timer == TIM1)
		hbridge_timer->BDTR |= TIM_BDTR_MOE;

}

/**
 * Updates the PWM values for the dual h-bridge.
 *
 * @param motor_a_speed   -1000 to +1000
 * @param motor_b_speed   -1000 to +1000
 */
void timer_dual_hbridge_motor_speeds(int32_t motor_a_speed, int32_t motor_b_speed) {

	// clip speeds to +/- 1000
	if(motor_a_speed >  1000) motor_a_speed =  1000;
	if(motor_b_speed >  1000) motor_b_speed =  1000;
	if(motor_a_speed < -1000) motor_a_speed = -1000;
	if(motor_b_speed < -1000) motor_b_speed = -1000;

	// update motor a
	if(motor_a_speed >= 0) {
		hbridge_timer->CCR1 = 0;
		hbridge_timer->CCR2 = motor_a_speed;
	} else {
		hbridge_timer->CCR1 = motor_a_speed * -1;
		hbridge_timer->CCR2 = 0;
	}

	// update motor b
	if(motor_b_speed >= 0) {
		hbridge_timer->CCR3 = motor_b_speed;
		hbridge_timer->CCR4 = 0;
	} else {
		hbridge_timer->CCR3 = 0;
		hbridge_timer->CCR4 = motor_b_speed * -1;
	}

}

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
void timer_pwm_setup(TIM_TypeDef *timer, uint32_t prescaler, uint32_t arr, enum TIMER_CHANNELS channels, enum GPIO_PIN ch1, ...) {
	va_list arglist;
	va_start(arglist, ch1);

	// enable timer clock and reset the timer
	if(timer == TIM1) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_TIM1RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM1RST;
	} else if(timer == TIM2) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;
	} else if(timer == TIM3) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_TIM3RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM3RST;
	} else if(timer == TIM14) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_TIM14RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM14RST;
	} else if(timer == TIM15) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_TIM15RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM15RST;
	} else if(timer == TIM16) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_TIM16RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM16RST;
	} else if(timer == TIM17) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_TIM17RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM17RST;
	}
	
	// prescaler = 23 (1 tick per half microsecond)
	timer->PSC = prescaler - 1;

	// auto-reload = pwm period in half microseconds
	timer->ARR = arr - 1;

	// auto-reload preload enable
	timer->CR1 |= TIM_CR1_ARPE;

	// pwm mode for each channel
	switch(channels) {
		case FOUR_CHANNELS:
			timer->CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;	// OC4M = 110 for PWM Mode 1 output on ch4
			timer->CCMR2 |= TIM_CCMR2_OC4PE;						// Output 4 preload enable
			timer->CCER |= TIM_CCER_CC4E;							// Enable output for ch4
			timer->CCR4 = 0;										// 0% duty cycle for ch4
			
			timer->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2;	// OC3M = 110 for PWM Mode 1 output on ch3
			timer->CCMR2 |= TIM_CCMR2_OC3PE;						// Output 3 preload enable
			timer->CCER |= TIM_CCER_CC3E;							// Enable output for ch3
			timer->CCR3 = 0;										// 0% duty cycle for ch3
			// fall through
		case TWO_CHANNELS:
			timer->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;	// OC2M = 110 for PWM Mode 1 output on ch2
			timer->CCMR1 |= TIM_CCMR1_OC2PE;						// Output 2 preload enable
			timer->CCER |= TIM_CCER_CC2E;							// Enable output for ch2
			timer->CCR2 = 0;										// 0% duty cycle for ch2
			// fall through
		case ONE_CHANNEL:
			timer->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;	// OC1M = 110 for PWM Mode 1 output on ch1
			timer->CCMR1 |= TIM_CCMR1_OC1PE;						// Output 1 preload enable
			timer->CCER |= TIM_CCER_CC1E;							// Enable output for ch1
			timer->CCR1 = 0;										// 0% duty cycle for ch1
		break;
	}

	// force update, then clear the update flag
	timer->EGR |= TIM_EGR_UG;
	timer->SR &= ~TIM_SR_UIF;

	// enable interrupt on update events
	timer->DIER |= TIM_DIER_UIE;
	if(timer == TIM1)
		NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
	else if(timer == TIM2)
		NVIC_EnableIRQ(TIM2_IRQn);
	else if(timer == TIM3)
		NVIC_EnableIRQ(TIM3_IRQn);
	else if(timer == TIM6)
		NVIC_EnableIRQ(TIM6_DAC_IRQn);
	else if(timer == TIM14)
		NVIC_EnableIRQ(TIM14_IRQn);
	else if(timer == TIM15)
		NVIC_EnableIRQ(TIM15_IRQn);
	else if(timer == TIM16)
		NVIC_EnableIRQ(TIM16_IRQn);
	else if(timer == TIM17)
		NVIC_EnableIRQ(TIM17_IRQn);

	// main output enable is only needed for advanced control timers
	if(timer == TIM1)
		timer->BDTR |= TIM_BDTR_MOE;

	// enable counter
	timer->CR1 |= TIM_CR1_CEN;

	// configure gpios
	if(timer == TIM1) {
		gpio_setup(ch1, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
		if(channels == TWO_CHANNELS || channels == FOUR_CHANNELS)
			gpio_setup(va_arg(arglist, int), AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
		if(channels == FOUR_CHANNELS) {
			gpio_setup(va_arg(arglist, int), AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
			gpio_setup(va_arg(arglist, int), AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
		}
	} else if(timer == TIM2) {
		gpio_setup(ch1, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
		if(channels == TWO_CHANNELS || channels == FOUR_CHANNELS)
			gpio_setup(va_arg(arglist, int), AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
		if(channels == FOUR_CHANNELS) {
			gpio_setup(va_arg(arglist, int), AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
			gpio_setup(va_arg(arglist, int), AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
		}
	} else if(timer == TIM3) {
		gpio_setup(ch1, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF1);
		if(channels == TWO_CHANNELS || channels == FOUR_CHANNELS)
			gpio_setup(va_arg(arglist, int), AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF1);
		if(channels == FOUR_CHANNELS) {
			gpio_setup(va_arg(arglist, int), AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF1);
			gpio_setup(va_arg(arglist, int), AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF1);
		}
	} else if(timer == TIM14) {
		if(ch1 == PA4 || ch1 == PA7)
			gpio_setup(ch1, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF4);
		else if(ch1 == PB1)
			gpio_setup(ch1, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
	} else if(timer == TIM15) {
		if(ch1 == PA2)
			gpio_setup(ch1, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
		else if(ch1 == PB14)
			gpio_setup(ch1, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF1);
		if(channels == TWO_CHANNELS) {
			enum GPIO_PIN temp = va_arg(arglist, int);
			if(temp == PA3)
				gpio_setup(temp, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
			else if(temp == PB15)
				gpio_setup(temp, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF1);
		}
	} else if(timer == TIM16) {
		if(ch1 == PA6)
			gpio_setup(ch1, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF5);
		else if(ch1 == PB8)
			gpio_setup(ch1, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
	} else if(timer == TIM17) {
		if(ch1 == PA7)
			gpio_setup(ch1, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF5);
		else if(ch1 == PB9)
			gpio_setup(ch1, AF, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF2);
	}
}

/**
 * Change the period of a PWM timer.
 *
 * @param timer		TIM1, TIM2, TIM3, TIM14, TIM15, TIM16 or TIM17
 * @param period	Period in half microseconds
 */
void timer_pwm_period(TIM_TypeDef *timer, uint32_t period) {
	// auto-reload = pwm period in half microseconds
	timer->ARR = period - 1;
}

/**
 * Change the high-time of the PWM signal.
 *
 * @param timer		TIM1, TIM2, TIM3, TIM14, TIM15, TIM16 or TIM17
 * @param channel	CH1, CH2, CH3 or CH4
 * @param value		High time in half microseconds
 */
void timer_pwm_value(TIM_TypeDef *timer, enum TIMER_CHANNEL channel, uint32_t value) {
	if(channel == CH1) {
		timer->CCR1 = value;
		// force update, then clear the update flag
		//timer->EGR |= TIM_EGR_UG;
		//timer->SR &= ~TIM_SR_UIF;
	} else if(channel == CH2) {
		timer->CCR2 = value;
		// force update, then clear the update flag
		//timer->EGR |= TIM_EGR_UG;
		//timer->SR &= ~TIM_SR_UIF;
	} else if(channel == CH3) {
		timer->CCR3 = value;
		// force update, then clear the update flag
		//timer->EGR |= TIM_EGR_UG;
		//timer->SR &= ~TIM_SR_UIF;
	} else if(channel == CH4) {
		timer->CCR4 = value;
		// force update, then clear the update flag
		//timer->EGR |= TIM_EGR_UG;
		//timer->SR &= ~TIM_SR_UIF;
	}
}

/**
 * Configure a timer as a timebase.
 *
 * @param timer		TIM1, TIM2, TIM3, TIM14, TIM15, TIM16 or TIM17
 * @param prescaler	Perscaler for the 48MHz clock
 * @param arr		Auto-reload value, determines the period
 * @param interrupt	0=disable interrupt, 1=enable interrupt
 */
void timer_timebase_setup(TIM_TypeDef *timer, uint32_t prescaler, uint32_t arr, uint32_t interrupt) {
	// enable timer clock and reset the timer
	if(timer == TIM1) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_TIM1RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM1RST;
	} else if(timer == TIM2) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;
	} else if(timer == TIM3) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_TIM3RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM3RST;
	} else if(timer == TIM14) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_TIM14RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM14RST;
	} else if(timer == TIM15) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_TIM15RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM15RST;
	} else if(timer == TIM16) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_TIM16RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM16RST;
	} else if(timer == TIM17) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_TIM17RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM17RST;
	}
	
	// prescaler
	timer->PSC = prescaler - 1;

	// auto-reload = number of clock ticks per event
	timer->ARR = arr - 1;

	// auto-reload preload enable
	timer->CR1 |= TIM_CR1_ARPE;

	// force update, then clear the update flag
	timer->EGR |= TIM_EGR_UG;
	timer->SR &= ~TIM_SR_UIF;

	// generate TRGO on update events
	timer->CR2 = TIM_CR2_MMS_1;

	// enable interrupt on update events
	if(interrupt == 1) {
		timer->DIER |= TIM_DIER_UIE;
		if(timer == TIM1)
			NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
		else if(timer == TIM2)
			NVIC_EnableIRQ(TIM2_IRQn);
		else if(timer == TIM3)
			NVIC_EnableIRQ(TIM3_IRQn);
		else if(timer == TIM6)
			NVIC_EnableIRQ(TIM6_DAC_IRQn);
		else if(timer == TIM14)
			NVIC_EnableIRQ(TIM14_IRQn);
		else if(timer == TIM15)
			NVIC_EnableIRQ(TIM15_IRQn);
		else if(timer == TIM16)
			NVIC_EnableIRQ(TIM16_IRQn);
		else if(timer == TIM17)
			NVIC_EnableIRQ(TIM17_IRQn);
	}

	// main output enable is only needed for advanced control timers
	if(timer == TIM1)
		timer->BDTR |= TIM_BDTR_MOE;

	// enable counter
	timer->CR1 |= TIM_CR1_CEN;
}

/**
 * Configure a timer for one-pulse mode.
 *
 * @param timer		TIM1, TIM2, TIM3, TIM14, TIM15, TIM16 or TIM17
 * @param delay		Delay in milliseconds before the pulse.
 */
void timer_one_pulse_setup(TIM_TypeDef *timer, uint32_t delay) {
	// enable timer clock and reset the timer
	if(timer == TIM1) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_TIM1RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM1RST;
	} else if(timer == TIM2) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;
	} else if(timer == TIM3) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_TIM3RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM3RST;
	} else if(timer == TIM14) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_TIM14RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM14RST;
	} else if(timer == TIM15) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_TIM15RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM15RST;
	} else if(timer == TIM16) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_TIM16RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM16RST;
	} else if(timer == TIM17) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_TIM17RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM17RST;
	}
	
	// prescaler
	timer->PSC = 48000 - 1;

	// auto-reload = number of clock ticks per event
	timer->ARR = delay - 1;

	// one-pulse mode
	timer->CR1 |= TIM_CR1_OPM;

	// force update, then clear the update flag
	timer->EGR |= TIM_EGR_UG;
	timer->SR &= ~TIM_SR_UIF;

	// enable interrupt on update events
	timer->DIER |= TIM_DIER_UIE;
	if(timer == TIM1)
		NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
	else if(timer == TIM2)
		NVIC_EnableIRQ(TIM2_IRQn);
	else if(timer == TIM3)
		NVIC_EnableIRQ(TIM3_IRQn);
	else if(timer == TIM6)
		NVIC_EnableIRQ(TIM6_DAC_IRQn);
	else if(timer == TIM14)
		NVIC_EnableIRQ(TIM14_IRQn);
	else if(timer == TIM15)
		NVIC_EnableIRQ(TIM15_IRQn);
	else if(timer == TIM16)
		NVIC_EnableIRQ(TIM16_IRQn);
	else if(timer == TIM17)
		NVIC_EnableIRQ(TIM17_IRQn);

	// main output enable is only needed for advanced control timers
	if(timer == TIM1)
		timer->BDTR |= TIM_BDTR_MOE;

	// enable counter
	timer->CR1 |= TIM_CR1_CEN;
}
