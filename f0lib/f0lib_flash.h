// Written by Farrell Farahbod
// Last revised on 2014-07-11
// This file is released into the public domain

#include "stm32f0xx.h"

uint8_t flash_erase_page(uint8_t pageNum);

uint8_t flash_write_page(uint8_t pageNum, void* data, uint32_t byteCount);

// flash write testing /////////////////////////////////////////////////////////////////
/*
struct persistent_data {
	uint16_t item1;
	uint32_t item2;
	int16_t item3;
};

struct persistent_data calib_constants;
calib_constants.item1 = 345;
calib_constants.item2 = 456;
calib_constants.item3 = 567;

if((GPIOB->IDR & (1<<5)) == 0) { // if PB5=low (if up button pressed)

	// erase flash page 63 (last page)
	lcd_printf(0, 0, "Attempting page erase...");
	if(flash_erase_page(63) == 1)
		lcd_printf(0, 1, "success!");
	else
		lcd_printf(0, 1, "failed!");

	// write to page 63
	lcd_printf(0, 2, "Attempting to write...");
	if(flash_write_page(63, &calib_constants, sizeof(calib_constants)) == 1)
		lcd_printf(0, 3, "success!");
	else
		lcd_printf(0, 3, "failed!");

	while(1); // stop
} else {
	struct persistent_data* stuff = (struct persistent_data*) 0x0800FC00;
	lcd_printf(0, 0, "1 = %u", stuff->item1);
	lcd_printf(0, 1, "2 = %u", stuff->item2);
	lcd_printf(0, 2, "3 = %d", stuff->item3);
	while(1);
}
*/
// flash write testing /////////////////////////////////////////////////////////////////
