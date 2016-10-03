// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#include <stm32f0xx.h>
#include "f0lib/f0lib_mpu6050_hmc5883l.h"
#include "f0lib/f0lib_uart.h"
#include "f0lib/f0lib_timers.h"
#include "f0lib/f0lib_rf_cc2500.h"

#include "MadgwickAHRS.h"
#include <math.h>

// variables written to by the CC2500 packet received handler
float gimbalX = 0;
float gimbalY = 0;
float knobLeft = 0;
float knobMiddle = 0;
float knobRight = 0;

void process_new_sensor_values(float gyro_x, float gyro_y, float gyro_z, float accel_x, float accel_y, float accel_z, float magn_x, float magn_y, float magn_z) {

	// sensor fusion with Madgwick's Filter
	// MadgwickAHRSupdate(gyro_z, gyro_y, -gyro_x, accel_z, accel_y, -accel_x, magn_z, magn_y, -magn_x);
	MadgwickAHRSupdateIMU(gyro_z, gyro_y, -gyro_x, accel_z, accel_y, -accel_x);

	// calculate the pitch angle so that:    0 = vertical    -pi/2 = on its back    +pi/2 = on its face
	float pitch = asinf(-2.0f * (q1*q3 - q0*q2));

	// update the motor speeds. if we're far from vertical, stop the motors since there is no chance of success.
	int32_t motor_a_speed = 0;
	int32_t motor_b_speed = 0;

	if(pitch > -0.6f && pitch < 0.6f) {

		// proportional component
		motor_a_speed = pitch * 4500.0f;
		motor_b_speed = pitch * 4500.0f;

		// apply throttle
		motor_a_speed -= gimbalY;
		motor_b_speed -= gimbalY;

		// apply steering
		motor_a_speed += gimbalX / 8;
		motor_b_speed -= gimbalX / 8;

	}

	timer_dual_hbridge_motor_speeds(motor_a_speed, motor_b_speed);

	uart_send_csv_floats(19,
	                     accel_x, // G
						 accel_y, // G
						 accel_z, // G
						 gyro_x,  // Rad/s
						 gyro_y,  // Rad/s
						 gyro_z,  // Rad/s
						 magn_x,  // Gs
						 magn_y,  // Gs
						 magn_z,  // Gs
						 pitch,   // Rad
						 q0,      // Quaternion
						 q1,      // Quaternion
						 q2,      // Quaternion
						 q3,      // Quaternion
	                     gimbalX,
						 gimbalY,
						 knobLeft,
						 knobMiddle,
						 knobRight);

}


void process_new_packet(uint8_t byte_count, uint8_t bytes[]) {

	int16_t gimX = (bytes[1] << 8) | bytes[0];
	int16_t gimY = (bytes[3] << 8) | bytes[2];
	int16_t knoL = (bytes[5] << 8) | bytes[4];
	int16_t knoM = (bytes[7] << 8) | bytes[6];
	int16_t knoR = (bytes[9] << 8) | bytes[8];
	// ignore byte10: currently unused

	gimbalX    = (float) gimX;
	gimbalY    = (float) gimY;
	knobLeft   = (float) knoL;
	knobMiddle = (float) knoM;
	knobRight  = (float) knoR;

}

void main(void) {

	// configure the UART
	uart_setup(PA9, 921600);

	// configure the 9DOF
	mpu6050_hmc5883l_setup(PB8, PB9, PB7, &process_new_sensor_values);

	// configure the dual h-bridge PWM timer
	timer_dual_hbridge_setup(PA0, PA1, PA2, PA3);

	// configure the RF module
	cc2500_setup(SPI1, PB3, PB4, PB5, PD2, PC12, 11, &process_new_packet);
	cc2500_enter_rx_mode();

	// everything else is handled by the ISR
	while(1);

}
