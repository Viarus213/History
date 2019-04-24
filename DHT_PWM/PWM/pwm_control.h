/*
 * pwm_control.h
 *
 *  Created on: 7 mar 2018
 *      Author: Viarus
 */

#ifndef PWM_PWM_CONTROL_H_
#define PWM_PWM_CONTROL_H_

//Pin configuration
#define RED   (1<<PD6)  //Red colour uP pin
#define GREEN (1<<PD5)  //Green colour uP pin
#define BLUE  (1<<PD4)  //Blue colour uP pin

#define PWM_DIR DDRD
#define PWM_PORT PORTD

extern volatile uint8_t pwm_red, pwm_green, pwm_blue; //RGB control sygnals

void init_pwm_ports(void);
void set_pwm(uint8_t bufor, uint8_t reference_temp);

#endif /* PWM_PWM_CONTROL_H_ */
