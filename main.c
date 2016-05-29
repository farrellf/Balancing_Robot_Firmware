// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#include <stm32f0xx.h>
#include "f0lib/f0lib_i2c.h"
#include "f0lib/f0lib_gpio.h"
#include "f0lib/f0lib_exti.h"

// for semihosting
extern void initialise_monitor_handles(void);
#include <stdio.h>

// globals for current state
float accel_x_g;
float accel_y_g;
float accel_z_g;
float mpu_temp_c;
float gyro_x_rad;
float gyro_y_rad;
float gyro_z_rad;
float magn_x_gs;
float magn_y_gs;
float magn_z_gs;
float pressure_float;
float baro_temp_float;

// i2c device addresses
#define MPU6050_ADDRESS  0b1101000
#define HMC5883L_ADDRESS 0b0011110
#define MS5611_ADDRESS   0b1110111

/**
 * Draws a horizontal ASCII line graph to represent a numerical value, like this:
 *
 * X Acceleration     [                      *                                     ]    -0.985 G
 *
 * @param name    A c-string to show at the left of the graph
 * @param value   The value to be graphed and also shown at the right of the graph
 * @param unit    The unit to be shown at the right of the graph
 * @param min     Sets the scale of the graph
 * @param max     Sets the scale of the graph
 */
void print_graph(char name[], double value, char unit[], double min, double max) {

	#define GRAPH_LENGTH 20

	double percentage = (value - min) / (max - min);
	int dot_location = (GRAPH_LENGTH - 1.0) * percentage;
	printf("%s [", name);
	for (int i = 0; i < GRAPH_LENGTH; i++)
		if (i == dot_location)
			printf("*");
		else
			printf(" ");
	printf("] %+9.3f %s\n", value, unit);

}

void main(void) {

	// configure semihosting and disable autoflushing on \n
	initialise_monitor_handles();
	setvbuf(stdout, NULL, _IOFBF, BUFSIZ);

	// configure i2c
	i2c_setup(I2C1, FAST_MODE_400KHZ, PB8, PB9);

	// configure the MPU6050 (gyro/accelerometer)
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x6B, 0x00);                    // exit sleep
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x19, 109);                     // sample rate = 8kHz / 110 = 72.7Hz
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x1B, 0x18);                    // gyro full scale = +/- 2000dps
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x1C, 0x08);                    // accelerometer full scale = +/- 4g
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x38, 0x01);                    // enable INTA interrupt

	// configure the HMC5883L (magnetometer)
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x6A, 0x00);                    // disable i2c master mode
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x37, 0x02);                    // enable i2c master bypass mode
	i2c_write_register(I2C1, HMC5883L_ADDRESS, 0x00, 0x18);                    // sample rate = 75Hz
	i2c_write_register(I2C1, HMC5883L_ADDRESS, 0x01, 0x60);                    // full scale = +/- 2.5 Gauss
	i2c_write_register(I2C1, HMC5883L_ADDRESS, 0x02, 0x00);                    // continuous measurement mode
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x37, 0x00);                    // disable i2c master bypass mode
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x6A, 0x20);                    // enable i2c master mode

	// configure the MPU6050 to automatically read the magnetometer
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x25, HMC5883L_ADDRESS | 0x80); // slave 0 i2c address, read mode
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x26, 0x03);                    // slave 0 register = 0x03 (x axis)
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x27, 6 | 0x80);                // slave 0 transfer size = 6, enabled
	i2c_write_register(I2C1, MPU6050_ADDRESS,  0x67, 1);                       // enable slave 0 delay

//	// configure the MS5611 (barometer)
//	i2c_write_register(I2C1, MS5611_ADDRESS,   0x1E, 0x00);                    // reset
//	i2c_write_register(I2C1, MS5611_ADDRESS,   0x48, 0x00);                    // start conversion of the pressure sensor

	// configure an external interrupt for the MPU6050's active-high INTA signal
	gpio_setup(PB7, INPUT, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
	exti_setup(PB7, RISING_EDGE);

//	// configure an external interrupt for the MS5611's active-low DRDY signal
//	gpio_setup(PB6, INPUT, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
//	exti_setup(PB6, FALLING_EDGE);

	// enter an infinite loop that prints the sensor readings out to the PC
	while(1) {

		print_graph("X Acceleration   ", accel_x_g, "G", -4.0, 4.0);
		print_graph("Y Acceleration   ", accel_y_g, "G", -4.0, 4.0);
		print_graph("Z Acceleration   ", accel_z_g, "G", -4.0, 4.0);
		printf("\n");
		print_graph("MPU6050 Temp     ", mpu_temp_c, "C", -150.0, 150.0);
		printf("\n");
		print_graph("X Rotation       ", gyro_x_rad, "Rad/s", -34.91, 34.91);
		print_graph("Y Rotation       ", gyro_y_rad, "Rad/s", -34.91, 34.91);
		print_graph("Z Rotation       ", gyro_z_rad, "Rad/s", -34.91, 34.91);
		printf("\n");
		print_graph("X Magnetic Field ", magn_x_gs, "Gs", -2.5, 2.5);
		print_graph("Y Magnetic Field ", magn_y_gs, "Gs", -2.5, 2.5);
		print_graph("Z Magnetic Field ", magn_z_gs, "Gs", -2.5, 2.5);
		printf("\n");
//		print_graph("Pressure         ", pressure_float, " ", 0.0, 10000000.0);
//		print_graph("MS5611 Temp      ", baro_temp_float, " ", 0.0, 10000000.0);
		printf("\x1B[14A"); // VT100 escape code: move cursor up 14 lines
		fflush(stdout);

	}

}

void mpu6050_read_sensors(void) {

	// read the sensor values
	uint8_t rx_buffer[20];
	i2c_read_registers(I2C1, MPU6050_ADDRESS, 20, 0x3B, rx_buffer);

	// extract the raw values
	int16_t  accel_x  = rx_buffer[0]  << 8 | rx_buffer[1];
	int16_t  accel_y  = rx_buffer[2]  << 8 | rx_buffer[3];
	int16_t  accel_z  = rx_buffer[4]  << 8 | rx_buffer[5];
	int16_t  mpu_temp = rx_buffer[6]  << 8 | rx_buffer[7];
	int16_t  gyro_x   = rx_buffer[8]  << 8 | rx_buffer[9];
	int16_t  gyro_y   = rx_buffer[10] << 8 | rx_buffer[11];
	int16_t  gyro_z   = rx_buffer[12] << 8 | rx_buffer[13];
	int16_t  magn_x   = rx_buffer[14] << 8 | rx_buffer[15];
	int16_t  magn_y   = rx_buffer[16] << 8 | rx_buffer[17];
	int16_t  magn_z   = rx_buffer[18] << 8 | rx_buffer[19];

	// convert accelerometer readings into G's
	accel_x_g = accel_x / 8192.0f;
	accel_y_g = accel_y / 8192.0f;
	accel_z_g = accel_z / 8192.0f;

	// convert temperature reading into degrees Celsius
	mpu_temp_c = mpu_temp / 340.0f + 36.53f;

	// convert gyro readings into Radians per second
	gyro_x_rad = gyro_x / 939.650784f;
	gyro_y_rad = gyro_y / 939.650784f;
	gyro_z_rad = gyro_z / 939.650784f;

	// convert magnetometer readings into Gauss's
	magn_x_gs = magn_x / 660.0f;
	magn_y_gs = magn_y / 660.0f;
	magn_z_gs = magn_z / 660.0f;

}

void ms5611_read_sensors(void) {

	static enum {READ_PRESSURE, READ_TEMPERATURE} state = READ_PRESSURE;

	uint8_t rx_buffer[3];

	if(state == READ_PRESSURE) {

		// read the pressure
		i2c_read_registers(I2C1, MS5611_ADDRESS, 3, 0x00, rx_buffer);

		// extract the raw value
		uint32_t pressure  = rx_buffer[0] << 16 | rx_buffer[1] << 8 | rx_buffer[2];

		// convert the pressure reading
		pressure_float = (float) pressure;

		// start conversion of the temperature sensor
		i2c_write_register(I2C1, MS5611_ADDRESS, 0x58, 0x00);

		// at the next interrupt, read the temperature
		state = READ_TEMPERATURE;

	} else if (state == READ_TEMPERATURE) {

		// read the temperature
		i2c_read_registers(I2C1, MS5611_ADDRESS, 3, 0x00, rx_buffer);

		// extract the raw value
		uint32_t temperature  = rx_buffer[0] << 16 | rx_buffer[1] << 8 | rx_buffer[2];

		// convert the temperature reading
		baro_temp_float = (float) temperature;

		// start conversion of the pressure sensor
		i2c_write_register(I2C1, MS5611_ADDRESS, 0x48, 0x00);

		// at the next interrupt, read the pressure
		state = READ_PRESSURE;

	}

}

/**
 * Event handler for EXTI 7 (MPU6050 INTA signal) and EXTI 6 (MS5611 DRDY signal.)
 * Reads all registers and formats the readings as floating point numbers.
 */
void EXTI4_15_IRQHandler(void) {

	if(EXTI->PR & EXTI_PR_PR7) {

		// clear and service the interrupt
		EXTI->PR = EXTI_PR_PR7;
		mpu6050_read_sensors();

	}

	if(EXTI->PR & EXTI_PR_PR6) {

		// clear and service the interrupt
		EXTI->PR = EXTI_PR_PR6;
		ms5611_read_sensors();

	}

}
