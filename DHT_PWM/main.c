/*
 * main.c
 *
 *  Created on: 7 mar 2018
 *      Author: Viarus
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "LCD/lcd44780.h"
#include "dht/dht.h"
#include "PWM/pwm_control.h"

uint8_t button_pressed(uint8_t button);   //Reduce switch bouncing function


/*
 * LCD variables
 */
const uint8_t sign_s[] PROGMEM    = {2, 4, 14, 16, 14, 1, 30, 0};   //Pattern of œ in FLASH
const uint8_t sign_c[] PROGMEM    = {2, 4, 14, 17, 16, 17, 14, 0};  //Pattern of æ in FLASH
const uint8_t sign_deg[] PROGMEM  = {7, 5, 7, 0, 0, 0, 0, 0};       //Pattern of ° in FLASH

/*
 * Data variables
 */
int8_t temperature  = 25;
int8_t humidity   = 0;

volatile int8_t reference_temp    = 25;  //Referenced temp available to set while set_new_temp = 1
volatile uint8_t set_new_temp   = 1;     //0 - view temp and control it; 1 - change reference_temp
volatile uint8_t button_inter_flag  = 0; //Clear LCD after change menu

/*
 * Flags
 */
uint8_t temp_error_flag = 0;  //Wrong temperature measurment
uint8_t hum_error_flag  = 0;  //Wrong humidity measurment

volatile uint8_t ref_temp_menu_cnt    = 0;   //If ref temp setting menu >= 5s -> switch menu level
volatile uint8_t button_pressed_flag  = 0;   //If button pressed -> 1
volatile uint8_t button_pressed_cnt   = 0;   //Counter causing delay
volatile uint8_t button_pressed_flag_OK = 0; //If delay counter count the proper value this flag is set

int main(){
  //==========================================//
  //=============== LCD config ===============//
  //==========================================//
    DDRA |= (1<<PA0);   //Set LCD backlight as output
    PORTA &= ~(1<<PA0); //Turn LCD backlight on

    lcd_init(); //LCD pins configuration
    lcd_cls();  //LCD display clear

    lcd_defchar_P(0x80, sign_s);   //Definition of symbol œ, pattern in FLASH
    lcd_defchar_P(0x81, sign_c);   //Definition of symbol æ, pattern in FLASH
    lcd_defchar_P(0x82, sign_deg); //Definition of symbol °, pattern in FLASH

  //==========================================//
  //=============== PWM config ===============//
  //==========================================//
    init_pwm_ports();

  //===========================================//
  //============== TIMER0 config ==============//
  //=============== PWM control ===============//
  //===========================================//
    TCCR0 |= (1<<WGM01); //CTC mode
    TCCR0 |= (1<<CS01);  //8 prescaling
    OCR0 = 50;           //Output compare register - freq = 20k
    TIMSK |= (1<<OCIE0); //Compare match interrupt enable

  //===========================================//
  //============== TIMER2 config ==============//
  //=============== DHT control ===============//
  //===========================================//
    TCCR2 |= (1<<WGM21);                          //CTC mode
    TCCR2 |= ((1<<CS22) | (1<<CS21) | (1<<CS20)); //1024 prescaling
    OCR2 = 78;                                    //Compared register t=10ms
    TIMSK |= (1<<OCIE2);                          //Compare Match Interrupt Enable

  //===========================================//
  //=============== INT0 config ===============//
  //========= referenced_temp change ==========//
  //===========================================//
    DDRD &= ~(1<<PD2); //Set pin INT0 as output - insert/regulation
    PORTD |= (1<<PD2); //Enable pull-up resistor

    MCUCR |= ((1<<ISC01) | (1<<ISC00)); //Rising edge of INT0 generates interrupt
    GICR |= (1<<INT0);                  //INT0 interrupt enable

  //==========================================//
  //============= BUTTONS config =============//
  //==========================================//
    DDRD &= ~((1<<PD0) | (1<<PD1)); //Set pins buttons as output
    PORTD |= ((1<<PD0) | (1<<PD1)); //Enable pull-up resistors


  sei(); //Global interrupt enable
  s1_flag = 0;

  while(1){
    /*
     * Temperature and humidity measurment process
     */
    if((s1_flag) && (seconds%2 == 0)){
      if(dht_gettemperaturehumidity(&temperature, &humidity) != 1){ //Correct measurment
        temp_error_flag = 0;
        hum_error_flag  = 0;
      }
      else{ //Wrong measurment
        temp_error_flag = 1;
        hum_error_flag  = 1;
      }
      s1_flag = 0;
    }

    /*
     * RGB diode control
     * Error: signal it
     * Correct: temperature control
     */
    if(temp_error_flag){
      pwm_red   = 10;
      pwm_green = 10;
      pwm_blue  = 10;
    }
    else{
      set_pwm(temperature, reference_temp);
    }

    /*
     * Menu:
     * case 0: display temperature and humidity
     * case 1: display and set ref temperature
     */
    switch(set_new_temp){
      case 0:
        if(button_inter_flag){ //Menu level has been switched -> lcd clear
          lcd_cls();
          button_inter_flag = 0;
        }

        /*
         * Temperature display
         */
        if(temp_error_flag){ //Wrong temperature measurment -> display error
          lcd_locate(0,0);
          lcd_str("Temp:");
          lcd_str("error");
        }
        else{ //Correct temperature measurment
          lcd_locate(0,0);
          lcd_str("Temp: ");
          lcd_int(temperature);
          lcd_str("\x82""C");
        }

        /*
         * Humidity display
         */
        if(hum_error_flag){ //Wrong humidity measurment -> display error
          lcd_locate(1,0);
          lcd_str("Wilgotno\x80\x81:");
          lcd_str("error");
        }
        else{ //Correct humidity measurment
          lcd_locate(1,0);
          lcd_str("Wilgotno\x80\x81: ");
          lcd_int(humidity);
          lcd_str("%");
        }

      break;

      case 1:
        if(button_inter_flag){ //Menu level has been switched -> lcd clear
          lcd_cls();
          button_inter_flag = 0;
          ref_temp_menu_cnt = 0;
        }
        lcd_locate(0,0);
        lcd_str("Temperatura ref");
        lcd_locate(1,5);
        lcd_int(reference_temp);
        lcd_str(".0""\x82""C");
        if(button_pressed(1<<PD0)) reference_temp--;
        if(button_pressed(1<<PD1)) reference_temp++;
      break;

      default:
        set_new_temp = 1;
      break;
    }
  }
}

uint8_t button_pressed(uint8_t button){
  if(!(PIND & button)){
    button_pressed_flag = 1; //Button has been pressed
    ref_temp_menu_cnt = 0;
    if(!(PIND & button) && (button_pressed_flag_OK)){
      button_pressed_flag_OK = 0;
      return 1;
    }
  }
  return 0;
}


ISR(TIMER0_COMP_vect){
  static uint8_t cnt = 0; //PWM counter

  if(cnt >= pwm_red)    PWM_PORT &= ~(RED);   else PWM_PORT |= RED;
  if(cnt >= pwm_green)  PWM_PORT &= ~(GREEN); else PWM_PORT |= GREEN;
  if(cnt >= pwm_blue)   PWM_PORT &= ~(BLUE);  else PWM_PORT |= BLUE;

  cnt++;
}

ISR(TIMER2_COMP_vect){
  if(++ms10_counter>99){ //if 1second...
    s1_flag = 1;
    seconds++;

    if(seconds == 59){
      seconds = 0;
    }
    /*
     * Control temp ref setting menu
     */
    if(++ref_temp_menu_cnt >= 5){ //After 5seconds...
      button_inter_flag = 1;
      set_new_temp      = 0; //Change menu level to display temp and hum
      ref_temp_menu_cnt = 0;
    }

    ms10_counter = 0;
  }

  if(button_pressed_flag){         //Button has been pressed
    if(++button_pressed_cnt > 15){ //Delay time get passed
      button_pressed_flag_OK  = 1;
      button_pressed_flag   = 0;
      button_pressed_cnt    = 0;
    }
  }
}

ISR(INT0_vect){ //Menu level changing
  button_inter_flag = 1;
  set_new_temp ^= 1;
}
