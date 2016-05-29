#include "stm32f0xx.h"
#include "f0lib_gpio.h"
#include "f0lib_spi.h"
#include "f0lib_lcd_ili9163.h"

// a few of the registers, not a complete list
#define NOP							0x00 // no operation
#define SOFT_RESET					0x01 // software reset
#define SLEEP_ON					0x10 // enter sleep mode (lowest power consumption)
#define SLEEP_OFF					0x11 // exit sleep mode (wait 5ms before next command)
#define PARTIAL_MODE_ON				0x12 // enter partial mode
#define NORMAL_MODE_ON   			0x13 // enter normal mode (exits partial mode or scroll mode)
#define INVERT_COLORS_OFF			0x20 // disable color inversion
#define INVERT_COLORS_ON			0x21 // enable color inversion
#define DISPLAY_OFF					0x28 // disable rendering
#define DISPLAY_ON					0x29 // enable rendering
#define SET_COLUMN_RANGE			0x2A // specify column starting and ending addresses
#define SET_PADE_RANGE				0x2B // specify page starting and ending addresses
#define WRITE_MEMORY				0x2C // start writing pixels to memory
#define SET_PARTIAL_REGION			0x30 // specify the partial mode's region
#define SET_SCROLL_REGION			0x33 // specify the scroll mode's region
#define SET_MEMORY_CONTROL			0x36 // specify row/column/page direction, refresh direction, and rgb/bgr mode.
#define SET_SCROLL_START			0x37 // specify the scroll mode starting address
#define IDLE_MODE_OFF				0x38 // exit idle mode
#define IDLE_MODE_ON				0x39 // enter idle mode
#define SET_PIXEL_FORMAT			0x3A // specify the pixel format (12/16/18 bits per pixel)
#define SET_FRAME_RATE_NORMAL		0xB1 // specify the frame rate for normal mode
#define SET_FRAME_RATE_IDLE			0xB2 // specify the frame rate for idle mode
#define SET_FRAME_RATE_PARTIAL		0xB3 // specify the frame rate for partial mode
#define SET_LINE_FRAME_DIRECTION	0xB4 // specify direction for lines and frames
#define SET_SOURCE_DIRECTION		0xB7 // specify source driver direction
#define SET_GATE_DIRECTION			0xB8 // specify gate driver direction
#define SET_POWER_CONTROL_1			0xC0 // specify gvdd/vci1 voltages
#define SET_POWER_CONTROL_2			0xC1 // specify avdd/vcl/vgh/vgl voltages
#define SET_POWER_CONTROL_3			0xC2 // specify normal mode op amp current and boost cycle rate
#define SET_POWER_CONTROL_4			0xC3 // specify idle mode op amp current and boost cycle rate
#define SET_POWER_CONTROL_5			0xC4 // specify partial mode op amp current and boost cycle rate
#define SET_VCOMH_VCOML_VOLTAGES	0xC5 // specify vcomh/vcoml voltages
#define SET_VCOMH_OFFSET			0xC7 // specify vcomh offset

extern const uint8_t font_8x16[223][16];  // see f0lib_lcd_font_8x16.c

static enum GPIO_PIN cn_d;
static enum GPIO_PIN cs;
static enum GPIO_PIN sclk;
static enum GPIO_PIN mosi;
static SPI_TypeDef *spi_periph;

/**
 * A helper function that writes the command portion of a command.
 *
 * @param reg        Register to access
 */
static inline __attribute__((always_inline)) void write_command(uint8_t reg) {

	gpio_low(cn_d);
	gpio_low(cs);
	spi_write_byte(spi_periph, reg);
	gpio_high(cs);

}

/**
 * A helper function that writes the data portion of a command.
 * The "data" is either an argument to a command, or is part of a pixel.
 *
 * @param data       Data to write
 */
static inline __attribute__((always_inline)) void write_data(uint8_t data) {

	gpio_high(cn_d);
	gpio_low(cs);
	spi_write_byte(spi_periph, data);
	gpio_high(cs);

}

/**
 * A helper function that writes a command with a 1-byte argument.
 *
 * @param reg        Register to access
 * @param data       Data to write into the register
 */
static inline __attribute__((always_inline)) void write_command_one_byte(uint8_t reg, uint8_t data) {

	write_command(reg);
	write_data(data);

}

/**
 * A helper function that writes a command with a 2-byte argument.
 *
 * @param reg        Register to access
 * @param data1      First byte to write into the register
 * @param data2      Second byte to write into the register
 */
static inline __attribute__((always_inline)) void write_command_two_bytes(uint8_t reg, uint8_t data1, uint8_t data2) {

	write_command(reg);
	write_data(data1);
	write_data(data2);

}

/**
 * A helper function that writes a command with a 2-word argument.
 *
 * @param reg        Register to access
 * @param data1      First 16bit word to write into the register
 * @param data2      Second 16bit word to write into the register
 */
static inline __attribute__((always_inline)) void write_command_two_words(uint8_t reg, uint16_t data1, uint16_t data2) {

	write_command(reg);
	write_data(data1 >> 8);
	write_data(data1);
	write_data(data2 >> 8);
	write_data(data2);

}

/**
 * A helper function that tells the LCD what region (inclusive) pixels should be drawn in.
 * After this function is called, send_pixel() can be repeatedly called to draw in the region.
 *
 * @param x1         Starting x position
 * @param x2         Ending x position
 * @param y1         Starting y position
 * @param y2         Ending y position
 */
static inline __attribute__((always_inline)) void set_region(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2) {

	write_command_two_words(SET_COLUMN_RANGE, x1, x2);
	write_command_two_words(SET_PADE_RANGE, y1, y2);
	write_command(WRITE_MEMORY);

}

/**
 * A helper function that writes pixels to the screen.
 * This function should be called after using set_region().
 *
 * @param reg        Register to access
 * @param data1      First byte to write into the register
 * @param data2      Second byte to write into the register
 */
static inline __attribute__((always_inline)) void send_pixel(uint16_t pixel) {

	write_data(pixel >> 8);
	write_data(pixel & 0xFF);

}

/**
 * Configure the LCD controller and draw a black screen.
 *
 * @param cn_d_pin   Command (active low) / Data (active high) pin
 * @param cs_pin     Chip select pin
 * @param sclk_pin   Serial clock pin
 * @param mosi_pin   Master out slave in pin
 */
void lcd_ili9163_initialize(enum GPIO_PIN cn_d_pin, enum GPIO_PIN cs_pin, enum GPIO_PIN sclk_pin, enum GPIO_PIN mosi_pin) {

	// store pins and periph for future use
	cn_d       = cn_d_pin;
	cs         = cs_pin;
	sclk       = sclk_pin;
	mosi       = mosi_pin;
	spi_periph = (sclk == PA5 || sclk == PB3) ? SPI1 : SPI2;

	// configure the GPIOs and SPI periph
	gpio_setup(cs, OUTPUT, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
	gpio_high(cs);

	gpio_setup(cn_d, OUTPUT, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
	gpio_low(cn_d);

	spi_setup(spi_periph, PRESCALE_2, sclk, mosi, mosi);

	// configure the LCD controller
	write_command(SOFT_RESET);
	for(volatile uint32_t i = 0; i < 10000; i++); // wait >5ms

	write_command(SLEEP_OFF);
	write_command_one_byte(SET_PIXEL_FORMAT, 0x05);
	write_command(NORMAL_MODE_ON);
	write_command_two_bytes(SET_FRAME_RATE_NORMAL, 0x08, 0x02);
	write_command_one_byte(SET_LINE_FRAME_DIRECTION, 0x07);
	write_command_two_bytes(SET_POWER_CONTROL_1, 0x0A, 0x02);
	write_command_one_byte(SET_POWER_CONTROL_2, 0x02);
	write_command_two_bytes(SET_VCOMH_VCOML_VOLTAGES, 0x50, 0x63);
	write_command_one_byte(SET_VCOMH_OFFSET, 0x00);
	write_command_two_words(SET_SCROLL_REGION, 32, 128);
	write_command_one_byte(SET_MEMORY_CONTROL, 0x08);
	write_command(DISPLAY_ON);

	lcd_ili9163_fill_screen(BLACK);

}

/**
 * Draw a string on the screen. Lines will automatically wrap around, but stop at the end of the screen.
 *
 * @param col        Starting column (0 = first column)
 * @param row        Starting row (0 = first row)
 * @param fg_color   Foreground color, the color used for text
 * @param bg_color   Background color
 * @param string     Text to draw on screen
 */
void lcd_ili9163_write_string(uint8_t col, uint8_t row, uint16_t fg_color, uint16_t bg_color, char string[]){

	if(col > 15 || row > 7)
		return;

	uint8_t i = 0;
	while(string[i] != 0) {
		lcd_ili9163_write_char(col++, row, fg_color, bg_color, string[i]);
		i++;

		if(col > 15) {
			row++;
			col = 0;
		}

		if(row > 7)
			return;
	}

}

/**
 * Draw one character on the screen.
 *
 * @param col        Column (0 = first column)
 * @param row        Row (0 = first row)
 * @param fg_color   Foreground color, the color used for text
 * @param bg_color   Background color
 * @param c          Character to draw on screen
 */
void lcd_ili9163_write_char(uint8_t col, uint8_t row, uint16_t fg_color, uint16_t bg_color, char c) {

	uint16_t xoff = col*8;
	uint16_t yoff = row*16;
	set_region(xoff, xoff+7, yoff, yoff+15);

	for(uint8_t i = 0; i < 16; i++) {

		font_8x16[c-32][i] & (1 << 7) ? send_pixel(fg_color) : send_pixel(bg_color);
		font_8x16[c-32][i] & (1 << 6) ? send_pixel(fg_color) : send_pixel(bg_color);
		font_8x16[c-32][i] & (1 << 5) ? send_pixel(fg_color) : send_pixel(bg_color);
		font_8x16[c-32][i] & (1 << 4) ? send_pixel(fg_color) : send_pixel(bg_color);
		font_8x16[c-32][i] & (1 << 3) ? send_pixel(fg_color) : send_pixel(bg_color);
		font_8x16[c-32][i] & (1 << 2) ? send_pixel(fg_color) : send_pixel(bg_color);
		font_8x16[c-32][i] & (1 << 1) ? send_pixel(fg_color) : send_pixel(bg_color);
		font_8x16[c-32][i] & (1 << 0) ? send_pixel(fg_color) : send_pixel(bg_color);

	}

}

/**
 * Overwrite the entire screen with a new color.
 *
 * @param color      New color
 */
void lcd_ili9163_fill_screen(uint16_t color) {
	set_region(0, 128, 0, 128);
	for(uint32_t i = 0; i < 128*128; i++)
		send_pixel(color);
}

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
void lcd_ili9163_draw_box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t fg_color, uint16_t bg_color) {

	// draw the four corners
	lcd_ili9163_write_char(x1, y1, fg_color, bg_color, 0xDA); // ┌
	lcd_ili9163_write_char(x2, y1, fg_color, bg_color, 0xBF); // ┐
	lcd_ili9163_write_char(x1, y2, fg_color, bg_color, 0xC0); // └
	lcd_ili9163_write_char(x2, y2, fg_color, bg_color, 0xD9); // ┘

	// draw the top and bottom edges
	uint8_t x = x1 + 1;
	while(x < x2) {
		lcd_ili9163_write_char(x, y1, fg_color, bg_color, 0xC4); // ─
		lcd_ili9163_write_char(x, y2, fg_color, bg_color, 0xC4); // ─
		x++;
	}

	// draw the left and right edges
	uint8_t y = y1 + 1;
	while(y < y2) {
		lcd_ili9163_write_char(x1, y, fg_color, bg_color, 0xB3); // │
		lcd_ili9163_write_char(x2, y, fg_color, bg_color, 0xB3); // │
		y++;
	}

}

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
void lcd_ili9163_draw_double_box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t fg_color, uint16_t bg_color) {

	// draw the four corners
	lcd_ili9163_write_char(x1, y1, fg_color, bg_color, 0xC9); // ╔
	lcd_ili9163_write_char(x2, y1, fg_color, bg_color, 0xBB); // ╗
	lcd_ili9163_write_char(x1, y2, fg_color, bg_color, 0xC8); // ╚
	lcd_ili9163_write_char(x2, y2, fg_color, bg_color, 0xBC); // ╝

	// draw the top and bottom edges
	uint8_t x = x1 + 1;
	while(x < x2) {
		lcd_ili9163_write_char(x, y1, fg_color, bg_color, 0xCD); // ═
		lcd_ili9163_write_char(x, y2, fg_color, bg_color, 0xCD); // ═
		x++;
	}

	// draw the left and right edges
	uint8_t y = y1 + 1;
	while(y < y2) {
		lcd_ili9163_write_char(x1, y, fg_color, bg_color, 0xBA); // ║
		lcd_ili9163_write_char(x2, y, fg_color, bg_color, 0xBA); // ║
		y++;
	}

}
