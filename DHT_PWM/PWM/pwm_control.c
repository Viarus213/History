/*
 * pwm_control.c
 *
 *  Created on: 7 mar 2018
 *      Author: Viarus
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "pwm_control.h"

volatile uint8_t pwm_red, pwm_green, pwm_blue;

void init_pwm_ports(){
	PWM_DIR |= RED | GREEN | BLUE;
	PWM_PORT &= ~(RED | GREEN | BLUE);
}

void set_pwm(uint8_t bufor, uint8_t reference_temp){
	if(bufor < (reference_temp - 1)){
		pwm_red = 255 - 255*bufor/reference_temp;
		if(pwm_red >= 255){
			pwm_red = 255;
		}
		pwm_blue = 0;
		pwm_green = 0;
	}
	else if(bufor > (reference_temp + 1)){
		pwm_blue = 255*bufor/reference_temp - 255;
		if(pwm_blue >= 255){
			pwm_blue = 255;
		}
		pwm_red = 0;
		pwm_green = 0;
	}
	else{
		pwm_red = 0;
		pwm_green = 0;
		pwm_blue = 0;
	}
}
