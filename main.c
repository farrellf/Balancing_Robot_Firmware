// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#include <stm32f0xx.h>
#include "f0lib/f0lib_mpu6050_hmc5883l.h"
#include "f0lib/f0lib_uart.h"
#include "f0lib/f0lib_timers.h"
#include "f0lib/f0lib_rf_cc2500.h"
#include "f0lib/f0lib_gpio.h"

#include "MadgwickAHRS.h"
#include <math.h>
#include <stdio.h>

// variables written to by the CC2500 packet received handler
volatile float gimbalX = 0;
volatile float gimbalY = 0;
volatile float knobLeft = 0;
volatile float knobMiddle = 0;
volatile float knobRight = 0;

void process_new_sensor_values(float gyro_x, float gyro_y, float gyro_z, float accel_x, float accel_y, float accel_z, float magn_x, float magn_y, float magn_z) {

	// sensor fusion with Madgwick's Filter
	// MadgwickAHRSupdate(gyro_z, gyro_y, -gyro_x, accel_z, accel_y, -accel_x, magn_z, magn_y, -magn_x);
	MadgwickAHRSupdateIMU(gyro_z, gyro_y, -gyro_x, accel_z, accel_y, -accel_x);

	// calculate the pitch angle so that:    0 = vertical    -pi/2 = on its back    +pi/2 = on its face
	float pitch = asinf(-2.0f * (q1*q3 - q0*q2));

	// calculate the set point (desired angle) and error (difference between the current angle and desired angle)
	// since there are no wheel encoders, only throttle affects the set point
	// mapping throttle to an angle so that:  0 = no throttle    -pi/10 = full speed reverse    +pi/10 = full speed forward
	float set_point = (float) gimbalY / 1400.0f * 0.314159265f;
	float error = pitch - set_point;

	// calculate the proportional component (current error * p scalar)
	float p_scalar = 12000.0f + (knobLeft - 2048.0f) * 5.90f;
	if(p_scalar < 0) p_scalar = 0;
	float proportional = error * p_scalar;

	// calculate the integral component (summation of past errors * i scalar)
	float i_scalar = 500.0f + (knobMiddle - 2048.0f) * 0.27f;
	if(i_scalar < 0) i_scalar = 0;
	static float integral = 0;
	integral += error * i_scalar;
	if(integral >  1000) integral = 1000; // limit wind-up
	if(integral < -1000) integral = -1000;

	// calculate the derivative component (change since previous error * d scalar)
	static float previous_error = 0;
	float d_scalar = 16000.0f + (knobRight - 2048.0f) * 7.85f;
	if(d_scalar < 0) d_scalar = 0;
	float derivative = (error - previous_error) * d_scalar;
	previous_error = error;

	int32_t motor_a_speed = proportional + integral + derivative;
	int32_t motor_b_speed = proportional + integral + derivative;

	// apply steering
	motor_a_speed += gimbalX / 2;
	motor_b_speed -= gimbalX / 2;

	// stop the motors if we're far from vertical since there is no chance of success
	if(pitch < -0.7f || pitch > 0.7f) {
		motor_a_speed = 0;
		motor_b_speed = 0;
	}

	timer_dual_hbridge_motor_speeds(motor_a_speed, motor_b_speed);

	uart_send_bin_floats(27,
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
						 knobRight,
						 set_point,
						 error,
						 p_scalar,
						 proportional,
						 i_scalar,
						 integral,
						 d_scalar,
						 derivative);

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
