// Written by Farrell Farahbod
// Last revised on 2014-07-01
// This file is released into the public domain

#include "stm32f0xx.h"
#include "f0lib_converters.h"

/**
 * Converts a uint32_t to a binary representation string. Spaces separate each group of four bits.
 *
 * @param num	The uint32_t to convert
 * @returns		A pointer to a static c-string. The string must be "used" before calling this function again.
 */
char* uint32_to_bin_string(uint32_t num) {
	static char numString[] = "0000 0000 0000 0000 0000 0000 0000 0000";

	int8_t i = 31; // current bit
	uint8_t j = 0; // current string index

	for(i = 31; i >= 0; i--) {
		if(num & (1 << i))
			numString[j] = '1';
		else
			numString[j] = '0';
		j++;
		if((i % 4) == 0) { // separate each 4bit block with a space
			numString[j] = ' ';
			j++;
		}
	}

	return numString;
}

/**
 * Converts a uint32_t to a decimal representation string.
 *
 * @param num	The uint32_t to convert
 * @returns		A pointer to a static c-string. The string must be "used" before calling this function again.
 */
char* uint32_to_dec_string(uint32_t num) {
	static char numString[11] = {0}; // 32bit unsigned integer can take up to 10 decimal places, and a null term

	if(num / 1000000000) numString[0] = num / 1000000000 + 48; else numString[0] = '0';
	if(num / 100000000 % 10) numString[1] = num / 100000000 % 10 + 48; else numString[1] = '0';
	if(num / 10000000 % 10) numString[2] = num / 10000000 % 10 + 48; else numString[2] = '0';
	if(num / 1000000 % 10) numString[3] = num / 1000000 % 10 + 48; else numString[3] = '0';
	if(num / 100000 % 10) numString[4] = num / 100000 % 10 + 48; else numString[4] = '0';
	if(num / 10000 % 10) numString[5] = num / 10000 % 10 + 48; else numString[5] = '0';
	if(num / 1000 % 10) numString[6] = num / 1000 % 10 + 48; else numString[6] = '0';
	if(num / 100 % 10) numString[7] = num / 100 % 10 + 48; else numString[7] = '0';
	if(num / 10 % 10) numString[8] = num / 10 % 10 + 48; else numString[8] = '0';
	numString[9] = num % 10 + 48;

	return numString;
}

/**
 * Converts a int16_t to a decimal representation string.
 *
 * @param num	The int16_t to convert
 * @returns		A pointer to a static c-string. The string must be "used" before calling this function again.
 */
char* int16_to_dec_string(int16_t num) {
	static char numString[7] = {0}; // 16bit signed integer can take up to 5 decimal places, a sign, and a null term

	if(num < 0) numString[0] = '-'; else numString[0] = '+';
	if(num < 0) num *= -1; // convert to signed

	if(num / 10000) numString[1] = num / 10000 + 48; else numString[1] = '0';
	if(num / 1000 % 10) numString[2] = num / 1000 % 10 + 48; else numString[2] = '0';
	if(num / 100 % 10) numString[3] = num / 100 % 10 + 48; else numString[3] = '0';
	if(num / 10 % 10) numString[4] = num / 10 % 10 + 48; else numString[4] = '0';
	numString[5] = num % 10 + 48;

	return numString;
}

char* fixed_point_number_to_string(uint8_t leading_places, uint8_t trailing_places, uint32_t num) {
	static char numString[30] = {0};
	for(uint8_t i = 0; i < 30; i++)
		numString[i] = 0;

	if(trailing_places > 0) {
		uint8_t index = leading_places + trailing_places;
		while(trailing_places > 0) {
			numString[index--] = (num % 10) + 48;
			num /= 10;
			trailing_places--;
		}
		numString[index--] = '.';
		if(leading_places > 0) {
			numString[index--] = (num % 10) + 48;
			num /= 10;
			leading_places--;
		}
		while(leading_places > 0) {
			if(num != 0)
				numString[index--] = (num % 10) + 48;
			else
				numString[index--] = ' ';
			num /= 10;
			leading_places--;
		}
	} else {
		uint8_t index = leading_places - 1;
		if(leading_places > 0) {
			numString[index--] = (num % 10) + 48;
			num /= 10;
			leading_places--;
		}
		while(leading_places > 0) {
			if(num != 0)
				numString[index--] = (num % 10) + 48;
			else
				numString[index--] = ' ';
			num /= 10;
			leading_places--;
		}
	}

	return numString;
}
