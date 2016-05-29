// Written by Farrell Farahbod
// Last revised on 2014-07-01
// This file is released into the public domain

/** Basic example:
 *
 *  int main() {
 *      printf("%d as a binary string is %s", 123, uint32_to_bin_string(123));  // "123 as a binary string is 0000 0000 0111 1011"
 *      printf("%d as a decimal string is %s", 123, uint32_to_dec_string(123)); // "123 as a decimal string is 0000000123"
 *      printf("%d as a decimal string is %s", 123, int16_to_dec_string(123));  // "123 as a decimal string is +00123"
 *      return 0;
 *  }
 *
 */

/**
 * Converts a uint32_t to a binary representation string. Spaces separate each group of four bits.
 *
 * @param num	The uint32_t to convert
 * @returns		A pointer to a static c-string. The string must be "used" before calling this function again.
 */
char* uint32_to_bin_string(uint32_t num);

/**
 * Converts a uint32_t to a decimal representation string.
 *
 * @param num	The uint32_t to convert
 * @returns		A pointer to a static c-string. The string must be "used" before calling this function again.
 */
char* uint32_to_dec_string(uint32_t num);

/**
 * Converts a int16_t to a decimal representation string.
 *
 * @param num	The int16_t to convert
 * @returns		A pointer to a static c-string. The string must be "used" before calling this function again.
 */
char* int16_to_dec_string(int16_t num);

char* fixed_point_number_to_string(uint8_t leading_places, uint8_t trailing_places, uint32_t num);
