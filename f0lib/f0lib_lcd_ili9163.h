#pragma once
#include "f0lib_gpio.h"

// pinout, looking from the front of the LCD, with the PFC coming out of the bottom of the LCD, and pin 1 being on the left:
//
// 1  = VDDIO   = 3V3
// 2  = VDDA    = 3V3
// 3  = MOSI    = PA7
// 4  = Cn/D    = PA0
// 5  = CS      = PA1
// 6  = SCLK    = PA5
// 7  = GND     = GND
// 8  = LCD_ID1 = NC
// 9  = RST     = 3V3
// 10 = GND     = GND
// 11 = LED+    = 3V3
// 12 = LED-    = GND
// 13 = LCD_ID2 = NC

// a few colors, in RGB565 format
#define RED     0b1111100000000000
#define GREEN   0b0000011111100000
#define BLUE    0b0000000000011111
#define WHITE   0b1111111111111111
#define BLACK   0b0000000000000000

/**
 * Configure the LCD controller and draw a black screen.
 *
 * @param cn_d_pin   Command (active low) / Data (active high) pin
 * @param cs_pin     Chip select pin
 * @param sclk_pin   Serial clock pin
 * @param mosi_pin   Master out slave in pin
 */
void lcd_ili9163_initialize(enum GPIO_PIN cn_d_pin, enum GPIO_PIN cs_pin, enum GPIO_PIN sclk_pin, enum GPIO_PIN mosi_pin);

/**
 * Draw a string on the screen. Lines will automatically wrap around, but stop at the end of the screen.
 *
 * @param col        Starting column (0 = first column)
 * @param row        Starting row (0 = first row)
 * @param fg_color   Foreground color, the color used for text
 * @param bg_color   Background color
 * @param string     Text to draw on screen
 */
void lcd_ili9163_write_string(uint8_t col, uint8_t row, uint16_t fg_color, uint16_t bg_color, char string[]);

/**
 * Draw one character on the screen.
 *
 * @param col        Column (0 = first column)
 * @param row        Row (0 = first row)
 * @param fg_color   Foreground color, the color used for text
 * @param bg_color   Background color
 * @param c          Character to draw on screen
 */
void lcd_ili9163_write_char(uint8_t col, uint8_t row, uint16_t fg_color, uint16_t bg_color, char c);

/**
 * Overwrite the entire screen with a new color.
 *
 * @param color      New color
 */
void lcd_ili9163_fill_screen(uint16_t color);

/**
 * Draws a box with a single outline around it, like this:
 *  ┌────────────┐
 *  │            │
 *  └────────────┘
 * @param x1         X coordinate of the top-left corner
 * @param y1         Y coordinate of the top-left corner
 * @param x2         X coordinate of the bottom-right corner
 * @param y2         Y coordinate of the bottom-right corner
 */
void lcd_ili9163_draw_box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t fg_color, uint16_t bg_color);

/**
 * Draws a box with a double outline around it, like this:
 *  ╔════════════╗
 *  ║            ║
 *  ╚════════════╝
 * @param x1         X coordinate of the top-left corner
 * @param y1         Y coordinate of the top-left corner
 * @param x2         X coordinate of the bottom-right corner
 * @param y2         Y coordinate of the bottom-right corner
 */
void lcd_ili9163_draw_double_box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t fg_color, uint16_t bg_color);
