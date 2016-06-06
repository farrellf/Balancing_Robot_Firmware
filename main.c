// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#include <stm32f0xx.h>
#include "f0lib/f0lib_mpu6050_hmc5883l.h"
#include "f0lib/f0lib_uart.h"
#include "f0lib/f0lib_timers.h"

#include "MadgwickAHRS.h"
#include <math.h>

void process_new_sensor_values(float gyro_x, float gyro_y, float gyro_z, float accel_x, float accel_y, float accel_z, float magn_x, float magn_y, float magn_z) {

	// sensor fusion with Madgwick's Filter
	// MadgwickAHRSupdate(gyro_x, gyro_y, gyro_z, accel_x, accel_y, accel_z, magn_x, magn_y, magn_z);
	MadgwickAHRSupdateIMU(gyro_x, gyro_y, gyro_z, accel_x, accel_y, accel_z);

	// calculate the pitch angle so that:    0 = vertical    -pi/2 = on its back    +pi/2 = on its face
	float pitch = acosf(q0 / sqrt(q0*q0 + q2*q2)) * 2.0f - 1.570796327f;

	// update the motor speeds. if we're far from vertical, stop the motors since there is no chance of success.
	int32_t motor_a_speed = 0;
	int32_t motor_b_speed = 0;

	if(pitch > -0.5f && pitch < 0.5f) {
		motor_a_speed = pitch * 4500.0f;
		motor_b_speed = pitch * 4500.0f;
	}

	timer_dual_hbridge_motor_speeds(motor_a_speed, motor_b_speed);

	uart_send_quat(q0, q1, q2, q3);

//	// generate strings for PC telemetry
//	uart_reset_tx_buffer();
//	uart_append_ascii_graph("X Acceleration   ", accel_x, "G", -4.0, 4.0);
//	uart_append_ascii_graph("Y Acceleration   ", accel_y, "G", -4.0, 4.0);
//	uart_append_ascii_graph("Z Acceleration   ", accel_z, "G", -4.0, 4.0);
//	uart_append_newline();
//	uart_append_ascii_graph("X Rotation       ", gyro_x, "Rad/s", -34.91, 34.91);
//	uart_append_ascii_graph("Y Rotation       ", gyro_y, "Rad/s", -34.91, 34.91);
//	uart_append_ascii_graph("Z Rotation       ", gyro_z, "Rad/s", -34.91, 34.91);
//	uart_append_newline();
//	uart_append_ascii_graph("X Magnetic Field ", magn_x, "Gs", -2.5, 2.5);
//	uart_append_ascii_graph("Y Magnetic Field ", magn_y, "Gs", -2.5, 2.5);
//	uart_append_ascii_graph("Z Magnetic Field ", magn_z, "Gs", -2.5, 2.5);
//	uart_append_newline();
//	uart_append_ascii_graph("Q0               ", q0, " ", -1.0, 1.0);
//	uart_append_ascii_graph("Q1               ", q1, " ", -1.0, 1.0);
//	uart_append_ascii_graph("Q2               ", q2, " ", -1.0, 1.0);
//	uart_append_ascii_graph("Q3               ", q3, " ", -1.0, 1.0);
//	uart_append_newline();
//	uart_append_ascii_graph("Pitch Angle      ", pitch, "Rad", -1.570796327, 1.570796327f);
//	uart_append_cursor_home();
//	uart_tx_via_dma();

}

void main(void) {

	// configure the UART
	uart_setup(PA9, 1000000);

	// configure the 9DOF
	mpu6050_hmc5883l_setup(PB8, PB9, PB7, &process_new_sensor_values);

	// configure the dual h-bridge PWM timer
	timer_dual_hbridge_setup(PA0, PA1, PA2, PA3);

	// everything else is handled by the ISR
	while(1);

}
