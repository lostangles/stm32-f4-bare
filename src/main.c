/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f429xx.h"
#include <stdint.h>
#include "lib_gpio.h"
#include "lib_i2c.h"
#include "mpu6050.h"
#include "lib_uart.h"
#include "lib_time.h"

extern I2C_TypeDef *i2c;


void process_new_sensor_values(float gyro_x, float gyro_y, float gyro_z, float accel_x, float accel_y, float accel_z, float magn_x, float magn_y, float magn_z) {

	// sensor fusion with Madgwick's Filter
	// MadgwickAHRSupdate(gyro_z, gyro_y, -gyro_x, accel_z, accel_y, -accel_x, magn_z, magn_y, -magn_x);
	uart_send_csv_floats(3,
		accel_x,accel_y, accel_z);
	return;
}


int main(void)
{

	SystemCoreClockUpdate();
	EnableCycles();
	gpio_setup(PB7, OUTPUT, PUSH_PULL, FIFTY_MHZ, NO_PULL, AF0);
	mpu6050_hmc5883l_setup(PF1, PF0, PF2, &process_new_sensor_values);
	uart_setup(PD8, 115200);

	while (1)
	{
	  gpio_high(PB7);
	  sleepMs(1000);
	  gpio_low(PB7);
	  sleepMs(1000);
	}
}

