// Authored by Farrell Farahbod, last revised on 2014-05-20
// This file is released into the public domain

#include "stm32f0xx.h"
#include "f0lib_exti.h"

/**
 * Configures an external interrupt.
 * IMPORTANT: The appropriate ISR must be defined, or code execution will halt as soon as an interrupt occurs!
 * EXTI0 can be pin 0 of any gpio port, EXTI1 can be pin 1 of any gpio port, etc.
 * You can not have interrupts for two pins with the same number but on different ports.
 *
 * @param pin		GPIO pin used as an external interrupt
 * @param edge		trigger on rising, falling, or both edges
 */
void exti_setup(enum GPIO_PIN pin, enum EXTI_EDGE edge) {
	uint8_t pinNumer = pin % 16;
	char port = (pin / 16); // 0 means GPIOA, etc.
	
	EXTI->IMR |= (1 << pinNumer); // unmask exti

	if((edge == RISING_EDGE) || (edge == BOTH_EDGES))	EXTI->RTSR |= (1 << pinNumer); // trigger exti on rising edges
	if((edge == FALLING_EDGE) || (edge == BOTH_EDGES))	EXTI->FTSR |= (1 << pinNumer); // trigger exti on falling edges

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	// specify which port the exti is connected to
	uint32_t temp = SYSCFG->EXTICR[pinNumer/4];
	switch(port) {
		case 0: // A
			// bits = 0000
			temp &= ~(1 << (pinNumer%4*4) + 3);
			temp &= ~(1 << (pinNumer%4*4) + 2);
			temp &= ~(1 << (pinNumer%4*4) + 1);
			temp &= ~(1 << (pinNumer%4*4) + 0);
			break;
		case 1: // B
			// bits = 0001
			temp &= ~(1 << (pinNumer%4*4) + 3);
			temp &= ~(1 << (pinNumer%4*4) + 2);
			temp &= ~(1 << (pinNumer%4*4) + 1);
			temp |=  (1 << (pinNumer%4*4) + 0);
			break;
		case 2: // C
			// bits = 0010
			temp &= ~(1 << (pinNumer%4*4) + 3);
			temp &= ~(1 << (pinNumer%4*4) + 2);
			temp |=  (1 << (pinNumer%4*4) + 1);
			temp &= ~(1 << (pinNumer%4*4) + 0);
			break;
		case 3: // D
			// bits = 0011
			temp &= ~(1 << (pinNumer%4*4) + 3);
			temp &= ~(1 << (pinNumer%4*4) + 2);
			temp |=  (1 << (pinNumer%4*4) + 1);
			temp |=  (1 << (pinNumer%4*4) + 0);
			break;
		case 5: // F
			// bits = 0101
			temp &= ~(1 << (pinNumer%4*4) + 3);
			temp |=  (1 << (pinNumer%4*4) + 2);
			temp &= ~(1 << (pinNumer%4*4) + 1);
			temp |=  (1 << (pinNumer%4*4) + 0);
			break;
	}
	SYSCFG->EXTICR[pinNumer/4] = temp; // EXTI0=PA0, EXTI1=PA1, EXTI2=PA2, EXTI3=PA3
	
	if(pinNumer < 2)
		NVIC_EnableIRQ(EXTI0_1_IRQn); // irq for pins 0,1
	else if(pinNumer < 4)
		NVIC_EnableIRQ(EXTI2_3_IRQn); // irq for pins 2,3
	else
		NVIC_EnableIRQ(EXTI4_15_IRQn); // irq for pins 4-15
}

/**
 * Manually trigger an external interrupt.
 *
 * @param pin		GPIO pin associated with the interrupt.
 */
void exti_trigger(enum GPIO_PIN pin) {
	EXTI->SWIER = (1 << (pin%16)); // trigger exti
}
