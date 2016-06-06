// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#include "f0lib_mpu6050_hmc5883l.h"
#include "f0lib_i2c.h"
#include "f0lib_exti.h"

// i2c device addresses
#define MPU6050_ADDRESS  0b1101000
#define HMC5883L_ADDRESS 0b0011110

// offsets calculated at power up
static int16_t gyro_x_offset = 0;
static int16_t gyro_y_offset = 0;
static int16_t gyro_z_offset = 0;
static uint32_t samples = 0;

I2C_TypeDef *i2c;
void (*event_handler)(float gyro_x, float gyro_y, float gyro_z, float accel_x, float accel_y, float accel_z, float magn_x, float magn_y, float magn_z);

static void mpu6050_hmc5883l_read_sensors(void) {

	// read the sensor values
	uint8_t rx_buffer[20];
	i2c_read_registers(i2c, MPU6050_ADDRESS, 20, 0x3B, rx_buffer);

	// extract the raw values
	int16_t  accel_x_raw  = rx_buffer[0]  << 8 | rx_buffer[1];
	int16_t  accel_y_raw  = rx_buffer[2]  << 8 | rx_buffer[3];
	int16_t  accel_z_raw  = rx_buffer[4]  << 8 | rx_buffer[5];
	int16_t  mpu_temp_raw = rx_buffer[6]  << 8 | rx_buffer[7];
	int16_t  gyro_x_raw   = rx_buffer[8]  << 8 | rx_buffer[9];
	int16_t  gyro_y_raw   = rx_buffer[10] << 8 | rx_buffer[11];
	int16_t  gyro_z_raw   = rx_buffer[12] << 8 | rx_buffer[13];
	int16_t  magn_x_raw   = rx_buffer[14] << 8 | rx_buffer[15];
	int16_t  magn_y_raw   = rx_buffer[16] << 8 | rx_buffer[17];
	int16_t  magn_z_raw   = rx_buffer[18] << 8 | rx_buffer[19];

	// calculate the offsets at power up
	if(samples < 64) {
		samples++;
		return;
	} else if(samples < 128) {
		gyro_x_offset += gyro_x_raw;
		gyro_y_offset += gyro_y_raw;
		gyro_z_offset += gyro_z_raw;
		samples++;
		return;
	} else if(samples == 128) {
		gyro_x_offset /= 64;
		gyro_y_offset /= 64;
		gyro_z_offset /= 64;
		samples++;
	} else {
		gyro_x_raw -= gyro_x_offset;
		gyro_y_raw -= gyro_y_offset;
		gyro_z_raw -= gyro_z_offset;
	}

	// convert accelerometer readings into G's
	float accel_x = accel_x_raw / 8192.0f;
	float accel_y = accel_y_raw / 8192.0f;
	float accel_z = accel_z_raw / 8192.0f;

	// convert temperature reading into degrees Celsius
	float mpu_temp = mpu_temp_raw / 340.0f + 36.53f;

	// convert gyro readings into Radians per second
	float gyro_x = gyro_x_raw / 939.650784f;
	float gyro_y = gyro_y_raw / 939.650784f;
	float gyro_z = gyro_z_raw / 939.650784f;

	// convert magnetometer readings into Gauss's
	float magn_x = magn_x_raw / 660.0f;
	float magn_y = magn_y_raw / 660.0f;
	float magn_z = magn_z_raw / 660.0f;

	// give the event handler the sensor readings
	event_handler(gyro_x, gyro_y, gyro_z, accel_x, accel_y, accel_z, magn_x, magn_y, magn_z);

}

/**
 * Configure an MPU6050 and HMC5883L sensor.
 *
 * @param sck_pin   I2C clock pin
 * @param sda_pin   I2C data pin
 * @param int_pin   MPU6050 interrupt pin
 * @param handler   Pointer to an event handler that will be called after new sensor readings have been processed
 */
void mpu6050_hmc5883l_setup(enum GPIO_PIN sck_pin, enum GPIO_PIN sda_pin, enum GPIO_PIN int_pin, void (*handler)(float gyro_x, float gyro_y, float gyro_z, float accel_x, float accel_y, float accel_z, float magn_x, float magn_y, float magn_z)) {

	// determine which i2c peripheral to use
	if(sck_pin == PB6 && sda_pin == PB7)
		i2c = I2C1;
	else if(sck_pin == PB8 && sda_pin == PB9)
		i2c = I2C1;
	else if(sck_pin == PB10 && sda_pin == PB11)
		i2c = I2C2;
	else if(sck_pin == PF6 && sda_pin == PF7)
		i2c = I2C2;
	else
		return;

	// assign the event handler pointer
	event_handler = handler;

	// configure i2c
	i2c_setup(i2c, FAST_MODE_400KHZ, sck_pin, sda_pin);

	// configure the MPU6050 (gyro/accelerometer)
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x6B, 0x00);                    // exit sleep
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x19, 109);                     // sample rate = 8kHz / 110 = 72.7Hz
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x1B, 0x18);                    // gyro full scale = +/- 2000dps
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x1C, 0x08);                    // accelerometer full scale = +/- 4g
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x38, 0x01);                    // enable INTA interrupt

	// configure the HMC5883L (magnetometer)
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x6A, 0x00);                    // disable i2c master mode
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x37, 0x02);                    // enable i2c master bypass mode
	i2c_write_register(i2c, HMC5883L_ADDRESS, 0x00, 0x18);                    // sample rate = 75Hz
	i2c_write_register(i2c, HMC5883L_ADDRESS, 0x01, 0x60);                    // full scale = +/- 2.5 Gauss
	i2c_write_register(i2c, HMC5883L_ADDRESS, 0x02, 0x00);                    // continuous measurement mode
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x37, 0x00);                    // disable i2c master bypass mode
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x6A, 0x20);                    // enable i2c master mode

	// configure the MPU6050 to automatically read the magnetometer
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x25, HMC5883L_ADDRESS | 0x80); // slave 0 i2c address, read mode
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x26, 0x03);                    // slave 0 register = 0x03 (x axis)
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x27, 6 | 0x80);                // slave 0 transfer size = 6, enabled
	i2c_write_register(i2c, MPU6050_ADDRESS,  0x67, 1);                       // enable slave 0 delay

	// configure an external interrupt for the MPU6050's active-high INTA signal
	exti_setup(PB7, NO_PULL, RISING_EDGE, &mpu6050_hmc5883l_read_sensors);

}
