// Written by Farrell Farahbod
// Last revised on 2014-07-11
// This file is released into the public domain

#include "stm32f0xx.h"

uint8_t flash_erase_page(uint8_t pageNum) {
	uint8_t returnValue = 0;
	
	while(FLASH->SR & FLASH_SR_BSY);	// wait for flash to not be busy
	if(FLASH->CR & FLASH_CR_LOCK) {		// unlock flash if locked
		FLASH->KEYR = 0x45670123;		// unlock codes
		FLASH->KEYR = 0xCDEF89AB;
	}
	FLASH->CR |= FLASH_CR_PER;			// set "page erase" bit
	FLASH->AR = 0x08000000 + (0x400 * pageNum);	// select page
	FLASH->CR |= FLASH_CR_STRT;			// start page erase
	while(FLASH->SR & FLASH_SR_BSY);	// wait for flash to not be busy
	if(FLASH->SR & FLASH_SR_EOP) {		// success?
		FLASH->SR &= ~FLASH_SR_EOP;		// succeeded, clear the flag
		FLASH->CR &= ~FLASH_CR_PER;		// clear page erase bit
		returnValue = 1;
	} else {
		returnValue = 0;				// failed to erase page
	}

	return returnValue;
}

uint8_t flash_write_page(uint8_t pageNum, void* data, uint32_t byteCount) {
	uint8_t returnValue = 0;
	
	while(FLASH->SR & FLASH_SR_BSY);	// wait for flash to not be busy
	FLASH->CR |= FLASH_CR_PG;			// set "programming" bit

	uint16_t* ptr = (uint16_t*) data;
	for(uint32_t addr = 0x08000000 + (0x400 * pageNum); addr < 0x08000000 + (0x400 * pageNum) + byteCount; addr += 2) {
		*(volatile uint16_t*) addr = *ptr;
		ptr++;
	}
	while(FLASH->SR & FLASH_SR_BSY);	// wait for flash to not be busy
	if(FLASH->SR & FLASH_SR_EOP) {		// success?
		FLASH->SR &= ~FLASH_SR_EOP;		// succeeded, clear the flag
		FLASH->CR &= ~FLASH_CR_PG;		// clear "programming" bit
		while(FLASH->SR & FLASH_SR_BSY);// wait for flash to not be busy
		returnValue = 1;
	} else {
		returnValue = 0;				// failed to write
	}

	return returnValue;
}
