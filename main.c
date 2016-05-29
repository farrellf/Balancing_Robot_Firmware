// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#include <stm32f0xx.h>
#include "f0lib/f0lib_mpu6050_hmc5883l.h"

// for semihosting
extern void initialise_monitor_handles(void);
#include <stdio.h>

// globals for telemetry
float accel_x_g;
float accel_y_g;
float accel_z_g;
float gyro_x_rad;
float gyro_y_rad;
float gyro_z_rad;
float magn_x_gs;
float magn_y_gs;
float magn_z_gs;

void process_new_sensor_values(float gyro_x, float gyro_y, float gyro_z, float accel_x, float accel_y, float accel_z, float magn_x, float magn_y, float magn_z) {

	gyro_x_rad = gyro_x;
	gyro_y_rad = gyro_y;
	gyro_z_rad = gyro_z;
	accel_x_g = accel_x;
	accel_y_g = accel_y;
	accel_z_g = accel_z;
	magn_x_gs = magn_x;
	magn_y_gs = magn_y;
	magn_z_gs = magn_z;

}

void main(void) {

	// configure semihosting and disable autoflushing on \n
	initialise_monitor_handles();
	setvbuf(stdout, NULL, _IOFBF, BUFSIZ);

	// configure the 9DOF
	mpu6050_hmc5883l_setup(PB8, PB9, PB7, &process_new_sensor_values);

	// enter an infinite loop that prints data to the PC
	while(1) {

		printf("X Acc  %+1.3f G\n", accel_x_g);
		printf("Y Acc  %+1.3f G\n", accel_y_g);
		printf("Z Acc  %+1.3f G\n", accel_z_g);
		printf("\n");
		printf("X Rot  %+1.3f Rad/s\n", gyro_x_rad);
		printf("Y Rot  %+1.3f Rad/s\n", gyro_y_rad);
		printf("Z Rot  %+1.3f Rad/s\n", gyro_z_rad);
		printf("\n");
		printf("X Mag  %+1.3f Gs\n", magn_x_gs);
		printf("Y Mag  %+1.3f Gs\n", magn_y_gs);
		printf("Z Mag  %+1.3f Gs\n", magn_z_gs);
		printf("\n");
		printf("\x1B[12A"); // VT100 escape code: move cursor up 12 lines
		fflush(stdout);

	}

}
