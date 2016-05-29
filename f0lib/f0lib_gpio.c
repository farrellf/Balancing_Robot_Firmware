// Written by Farrell Farahbod
// Last revised on 2014-07-01
// This file is released into the public domain

#include "f0lib_gpio.h"

/**
 * Prepares a pin for use. This enables the appropriate GPIO clock and sets all the pin attributes.
 *
 * Example: gpio_setup(PA13, OUTPUT, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
 *
 * @param	pin		pin being configured
 * @param	mode	input, output, alternate function or analog
 * @param	type	push-pull or open-drain
 * @param	speed	50MHz, 10MHz or 2MHz
 * @param	pull	pull-up, pull-down or neither
 * @param	af		alternate function (must be specified, but only has an effect when mode is set to alternate function)
 */
void gpio_setup(enum GPIO_PIN	pin,
				enum GPIO_MODE	mode,
				enum GPIO_TYPE	type,
				enum GPIO_SPEED	speed,
				enum GPIO_PULL	pull,
				enum GPIO_AF	af) {

	uint8_t port = pin / 16;
	uint8_t pinNum = pin % 16;
	uint32_t *baseAddr = 0;
	uint32_t regValue = 0;

	// sanity check
	if(port == 4) return;												// no port e
	if(port == 3 && pinNum != 2) return;								// port d only has a pin 2
	if(port == 5 && (pinNum == 2 || pinNum == 3 || pinNum > 7)) return;	// port f has no pins 2,3,8+
	

	// Enable appropriate GPIO clock and determine GPIO base address

	switch(port) {
		case 0: // A
			RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
			baseAddr = (uint32_t *) 0x48000000;
			break;
		case 1: // B
			RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
			baseAddr = (uint32_t *) 0x48000400;
			break;
		case 2: // C
			RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
			baseAddr = (uint32_t *) 0x48000800;
			break;
		case 3: // D
			RCC->AHBENR |= RCC_AHBENR_GPIODEN;
			baseAddr = (uint32_t *) 0x48000C00;
			break;
		case 5: // F
			RCC->AHBENR |= RCC_AHBENR_GPIOFEN;
			baseAddr = (uint32_t *) 0x48001400;
			break;
	}


	// config mode register

	regValue = *(baseAddr + 0);

	switch(mode) {
		case INPUT:
			regValue &= ~(1 << pinNum*2);
			regValue &= ~(1 << pinNum*2 + 1);
			break;
		case OUTPUT:
			regValue |= (1 << pinNum*2);
			regValue &= ~(1 << pinNum*2 + 1);
			break;
		case AF:
			regValue &= ~(1 << pinNum*2);
			regValue |= (1 << pinNum*2 + 1);
			break;
		case ANALOG:
			regValue |= (1 << pinNum*2);
			regValue |= (1 << pinNum*2 + 1);
			break;
    }
    
	*(baseAddr + 0) = regValue;


	// config type register
	
	regValue = *(baseAddr + 1);

	switch(type) {
		case PUSH_PULL:
			regValue &= ~(1 << pinNum);
			break;
		case OPEN_DRAIN:
			regValue |= (1 << pinNum);
			break;
    }
    
	*(baseAddr + 1) = regValue;


	// config speed register
	
	regValue = *(baseAddr + 2);

	switch(speed) {
		case TWO_MHZ:
			regValue &= ~(1 << pinNum*2);
			regValue &= ~(1 << pinNum*2 + 1);
			break;
		case TEN_MHZ:
			regValue |= (1 << pinNum*2);
			regValue &= ~(1 << pinNum*2 + 1);
			break;
		case FIFTY_MHZ:
			regValue |= (1 << pinNum*2);
			regValue |= (1 << pinNum*2 + 1);
			break;
    }
    
	*(baseAddr + 2) = regValue;


	// config pull-up/pull-down register

	regValue = *(baseAddr + 3);

	switch(pull) {
		case NO_PULL:
			regValue &= ~(1 << pinNum*2);
			regValue &= ~(1 << pinNum*2 + 1);
			break;
		case PULL_UP:
			regValue |= (1 << pinNum*2);
			regValue &= ~(1 << pinNum*2 + 1);
			break;
		case PULL_DOWN:
			regValue &= ~(1 << pinNum*2);
			regValue |= (1 << pinNum*2 + 1);
			break;
    }

	*(baseAddr + 3) = regValue;


	// config alternate function registers

	if(port != 0 && port != 1)
		return; // no need to set AFx for ports C, D, F

	if(pinNum < 8)
		regValue = *(baseAddr + 8);
	else
		regValue = *(baseAddr + 9);

	switch(af) {
		case AF0:
			regValue &= ~(1 << (pinNum % 8)*4);
			regValue &= ~(1 << (pinNum % 8)*4 + 1);
			regValue &= ~(1 << (pinNum % 8)*4 + 2);
			regValue &= ~(1 << (pinNum % 8)*4 + 3);
			break;
		case AF1:
			regValue |= (1 << (pinNum % 8)*4);
			regValue &= ~(1 << (pinNum % 8)*4 + 1);
			regValue &= ~(1 << (pinNum % 8)*4 + 2);
			regValue &= ~(1 << (pinNum % 8)*4 + 3);
			break;
		case AF2:
			regValue &= ~(1 << (pinNum % 8)*4);
			regValue |= (1 << (pinNum % 8)*4 + 1);
			regValue &= ~(1 << (pinNum % 8)*4 + 2);
			regValue &= ~(1 << (pinNum % 8)*4 + 3);
			break;
		case AF3:
			regValue |= (1 << (pinNum % 8)*4);
			regValue |= (1 << (pinNum % 8)*4 + 1);
			regValue &= ~(1 << (pinNum % 8)*4 + 2);
			regValue &= ~(1 << (pinNum % 8)*4 + 3);
			break;
		case AF4:
			regValue &= ~(1 << (pinNum % 8)*4);
			regValue &= ~(1 << (pinNum % 8)*4 + 1);
			regValue |= (1 << (pinNum % 8)*4 + 2);
			regValue &= ~(1 << (pinNum % 8)*4 + 3);
			break;
		case AF5:
			regValue |= (1 << (pinNum % 8)*4);
			regValue &= ~(1 << (pinNum % 8)*4 + 1);
			regValue |= (1 << (pinNum % 8)*4 + 2);
			regValue &= ~(1 << (pinNum % 8)*4 + 3);
			break;
		case AF6:
			regValue &= ~(1 << (pinNum % 8)*4);
			regValue |= (1 << (pinNum % 8)*4 + 1);
			regValue |= (1 << (pinNum % 8)*4 + 2);
			regValue &= ~(1 << (pinNum % 8)*4 + 3);
			break;
		case AF7:
			regValue |= (1 << (pinNum % 8)*4);
			regValue |= (1 << (pinNum % 8)*4 + 1);
			regValue |= (1 << (pinNum % 8)*4 + 2);
			regValue &= ~(1 << (pinNum % 8)*4 + 3);
			break;
	}

	if(pinNum < 8)
		*(baseAddr + 8) = regValue;
	else
		*(baseAddr + 9) = regValue;
}

/**
 * Prepares an entire GPIO port for use. This enables the appropriate GPIO clock and sets all the pin attributes.
 *
 * Example: gpio_port_setup(PORT_A, OUTPUT, PUSH_PULL, FIFTY_MHZ, NO_PULL);
 *
 * @param	port	port being configured
 * @param	mode	input, output, alternate function or analog
 * @param	type	push-pull or open-drain
 * @param	speed	50MHz, 10MHz or 2MHz
 * @param	pull	pull-up, pull-down or neither
 */
void gpio_port_setup(	GPIO_TypeDef *port,
						enum GPIO_MODE	mode,
						enum GPIO_TYPE	type,
						enum GPIO_SPEED	speed,
						enum GPIO_PULL	pull) {
	enum GPIO_PIN firstPin;
	if(port == GPIOA)
			firstPin = PA0;
	else if(port == GPIOB)
			firstPin = PB0;
	else if(port == GPIOC)
			firstPin = PC0;
	else if(port == GPIOD)
			firstPin = PD0;
	else if(port == GPIOF)
			firstPin = PF0;

	for(uint8_t i = 0; i < 16; i++) {
		gpio_setup(firstPin++, mode, type, speed, pull, AF0);
	}
}

/* unoptimized version:
void gpio_low(enum GPIO_PIN pin) {
	uint8_t port = pin / 16;
	uint8_t pinNum = pin % 16;

	switch(port) {
		case 0: // A
			GPIOA->BRR = (1 << pinNum);
			break;
		case 1: // B
			GPIOB->BRR = (1 << pinNum);
			break;
		case 2: // C
			GPIOC->BRR = (1 << pinNum);
			break;
		case 3: // D
			GPIOD->BRR = (1 << pinNum);
			break;
		case 5: // F
			GPIOF->BRR = (1 << pinNum);
			break;
	}
 }
*/

/* unoptimized version:
void gpio_high(enum GPIO_PIN pin) {
	uint8_t port = pin / 16;
	uint8_t pinNum = pin % 16;

	switch(port) {
		case 0: // A
			GPIOA->BSRR = (1 << pinNum);
			break;
		case 1: // B
			GPIOB->BSRR = (1 << pinNum);
			break;
		case 2: // C
			GPIOC->BSRR = (1 << pinNum);
			break;
		case 3: // D
			GPIOD->BSRR = (1 << pinNum);
			break;
		case 5: // F
			GPIOF->BSRR = (1 << pinNum);
			break;
	}
}
*/
