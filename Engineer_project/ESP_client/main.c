//==========================================================================
// main.c
//
// Created by: Viarus
// Date: 10/21/2018
// Description: Use the ESP8266 to receive data (temperature) from slaves.
//               The master is the ATmega128 and it is a server
//==========================================================================

//--------------------------------------------------------------------------
// INCLUDES
//--------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "usart/usart.h"
#include "1Wire/ds18x20.h"

//--------------------------------------------------------------------------
// DEFINES
//--------------------------------------------------------------------------
// ESP8266 state dafines
#define ESP_TEST 0
#define AP_CONNECT 1
#define TCP_CONNECTION 2
#define SEND_DATA 3

//--------------------------------------------------------------------------
// VARIABLES
//--------------------------------------------------------------------------
// ESP8266
uint8_t esp_config_flag = 1;  // Does ESP module need to be configured
uint8_t config_state = 0;     // Whan part of configuration is now needed
uint16_t error_cnt = 0;       // Indicate that OK didn't occure after command
uint8_t first_cmd = 1;        // Command needs to be send to ESP just once
uint8_t prepare_to_send = 1;

// DS18B20
uint8_t sensor_number;     // Number of sensors
volatile uint8_t s1_flag;  // 1 second passed
volatile uint8_t seconds;  // Seconds counter
uint8_t temp_sign[3];      // Negative or positive temperature
uint8_t temp_total[3];     // Temperature total part
uint8_t temp_frac[3];      // Temperature fration part

//--------------------------------------------------------------------------
// FUNCTIONS definitions
//--------------------------------------------------------------------------
uint8_t ok_check_f(void);
//void esp_send_cmd(char command[]);

//--------------------------------------------------------------------------
// MAIN function
//--------------------------------------------------------------------------
int main (){
  DDRD |= (1 << PD7);
  PORTD &= ~(1 << PD7);
  //=========================================================================
  //----- TIMER/COUNTER1 configuration -----
  //=========================================================================
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler: clk/1024
  OCR1A = 36;             // Output Compare Register 1 A
  TIMSK |= (1 << OCIE1A); // Output Compare A Match Interrupt Enable

  sei(); // Global interrupt enable

  //=========================================================================
  //----- USART initialization -----
  //=========================================================================
  usart_init(_UBRR);

  //=========================================================================
  //----- DS18B20 configuration -----
  //=========================================================================
  sensor_number = search_sensors();  // Check the number of the sensors

  DS18X20_start_meas(DS18X20_POWER_EXTERN, NULL); //Start measurment for all
                                                  //sensors, NULL - sensor's
                                                  //id in the rom-code
  _delay_ms(750);  //Temp conversion time for 12-bit resolution

  // Read temperature of the 1st sensor
  if (DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &temp_sign[0],
                                      &temp_total[0], &temp_frac[0]));
  else PORTD |= (1 << PD7);
  // Read temperature of the 1st sensor
  if (DS18X20_OK == DS18X20_read_meas(gSensorIDs[1], &temp_sign[1],
                                      &temp_total[1], &temp_frac[1]));
  else PORTD |= (1 << PD7);
  // Read temperature of the 1st sensor
  if (DS18X20_OK == DS18X20_read_meas(gSensorIDs[2], &temp_sign[2],
                                      &temp_total[2], &temp_frac[2]));
  else PORTD |= (1 << PD7);

  
  
  while(1) {
    //=========================================================================
    //----- ESP8266 configuration -----
    //=========================================================================
    if (esp_config_flag){ // Flag that is set when ESP is not already
                          //   configured
      switch (config_state){
        // Test ESP connection
      case ESP_TEST:
        if (first_cmd){
          uart_puts("AT\r\n");
          first_cmd = 0;
        }
  
        if (ok_check_f()){
          error_cnt = 0;
          first_cmd = 1;
          uart_clear();
          config_state = AP_CONNECT;
        }
        else error_cnt++;
       
        if (error_cnt >= 40){
          first_cmd = 1;
          error_cnt = 0;
        }
       
        _delay_us(500);
        break;
       
        // Connect to AP
      case AP_CONNECT:
        if (first_cmd){
          uart_puts("AT+CWJAP=\"ESP8266\",\"1234\"\r\n");
          first_cmd = 0;
        }
  
        if (ok_check_f()){
          error_cnt = 0;
          first_cmd = 1;
          config_state = TCP_CONNECTION;
          uart_clear();
        }
        else error_cnt++;
       
        if (error_cnt >= 5000){
          first_cmd = 1;
          error_cnt = 0;
        }
       
        _delay_ms(1);
        break;

        // Establish TCP connection and start connection
      case TCP_CONNECTION:
        if (first_cmd){
          uart_puts("AT+CIPSTART=\"TCP\",\"192.168.0.101\",80\r\n");
          first_cmd = 0;
        }
  
        if (ok_check_f()){
          error_cnt = 0;
          esp_config_flag = 0;
          uart_clear();
        }
        else error_cnt++;
       
        if (error_cnt >= 200){
          first_cmd = 1;
          error_cnt = 0;
        }
       
        _delay_ms(1);
        break;
      }
    }
    
    //=========================================================================
    //----- ESP8266 configuration finish -----
    //=========================================================================
    else{
      /* for (uint8_t i = 0; i < 10; i++){ */
      /*  if (prepare_to_send == 1){ */
      /*    uart_puts("AT+CIPSEND=1\r\n"); */
      /*    _delay_ms(4); */
      /*    uart_clear(); */
      /*    uart_putint(i); */
      /*    uart_puts("\r\n"); */
      /*  } */
      /*  if (ok_check_f()){ */
      /*    prepare_to_send = 1; */
      /*    i++; */
      /*  } */
      /*  _delay_ms(1000); */
      /*  uart_clear(); */
      /* } */
      /* uart_puts("AT+CIPCLOSE\r\n"); */

      if (s1_flag){
        // Make the actions when 1 second get passed
        // Check the number of sensors on the bus
        if ((seconds&5) == 0) sensor_number = search_sensors();
        // Send an order to DS18B20 to start a measure
        if ((seconds%5) == 1) DS18X20_start_meas(DS18X20_POWER_EXTERN, NULL);
        // Read temperature form the 1st sensor and send the result to the
        //   master via WiFi
        if (((seconds%5) == 2)){
          if (DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &temp_sign[0],
                                              &temp_total[0], &temp_frac[0])){
            uart_puts("AT+CIPSEND=12\r\n");
            _delay_ms(4);
            uart_clear();
            uart_puts("T0: ");
            uart_putc('1');
            uart_putint(temp_total[0]);
            uart_putc(',');
            uart_putint(temp_frac[0]);
            uart_putc('C');
            uart_puts("\r\n");
            _delay_ms(10);
            uart_clear();
          }
          else PORTD |= (1 << PD7);
        }
        // Read temperature form the 2nd sensor and send the result to the
        //   master via WiFi
        if ((seconds%5) == 3){
          //_delay_ms(50);
          if (DS18X20_OK == DS18X20_read_meas(gSensorIDs[1], &temp_sign[1],
                                              &temp_total[1], &temp_frac[1])){
            uart_puts("AT+CIPSEND=12\r\n");
            _delay_ms(4);
            uart_clear();
            uart_puts("T1: ");
            uart_putc('1');
            uart_putint(temp_total[1]);
            uart_putc(',');
            uart_putint(temp_frac[1]);
            uart_putc('C');
            uart_puts("\r\n");
            _delay_ms(10);
            uart_clear();
          }
          else PORTD |= (1 << PD7);
        }
        // Read temperature form the 3rd sensor and send the result to the
        //   master via WiFi
        if ((seconds%5) == 4){
          //_delay_ms(50);
          if (DS18X20_OK == DS18X20_read_meas(gSensorIDs[2], &temp_sign[2],
                                              &temp_total[2], &temp_frac[2])){
            uart_puts("AT+CIPSEND=12\r\n");
            _delay_ms(4);
            uart_clear();
            uart_puts("T2: ");
            uart_putc('1');
            uart_putint(temp_total[2]);
            uart_putc(',');
            uart_putint(temp_frac[2]);
            uart_putc('C');
            uart_puts("\r\n");
            _delay_ms(10);
            uart_clear();
          }
          else PORTD |= (1 << PD7);
        }
        s1_flag = 0;
      }
    }
    
  }
}

//=========================================================================
// FUNCTIONS
//=========================================================================
//-------------------------------------------------------------------------
//void esp_send_cmd(char command[]) {
// strcat(command, "\r\n");
// uart_puts(command);
//}

//-------------------------------------------------------------------------
uint8_t ok_check_f(void){
  char receive_buffer;
  char tab = '\0';
   
  uint8_t ok_cnt = 0;
   
  receive_buffer = uart_getc();
   
  while (receive_buffer){
    if (receive_buffer == 'O') {
      tab = receive_buffer;
      ok_cnt = 0;
    }
    else if ((receive_buffer == 'K') && (tab == 'O')){
      tab = receive_buffer ;
      ok_cnt++;
    }
    else if ((receive_buffer == '\r') && (tab == 'K')){
      tab = '\0';
      ok_cnt++;
    }
   
    receive_buffer = uart_getc();
  }
  if (ok_cnt == 2){
    ok_cnt = 0;
    return 1;
  }
  else{
    ok_cnt = 0;
    return 0;
  }
}

//=========================================================================
// INTERRUPTS
//=========================================================================
ISR(TIMER1_COMPA_vect){
  static uint8_t cnt = 0;
  
  if (++cnt > 99){  // After 1second set the s1_flag and increase
                    //   seconds counter
    s1_flag = 1;
    seconds++;

    if (seconds > 59) seconds = 0;

    cnt = 0;
  }
}
