
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

#include "usart/usart0.h"
#include "usart/usart1.h"

//--------------------------------------------------------------------------
// DEFINES
//--------------------------------------------------------------------------
// ESP8266 state dafines
#define ESP_TEST 0
#define AP_CONNECT 1
#define ESP_SSID_SET 2
#define ESP_IP_ADDRESS 3
#define DHCP_MODE 4
#define MUX_CONNECTION 5
#define SERVER_START 6

//--------------------------------------------------------------------------
// VARIABLES
//--------------------------------------------------------------------------
volatile uint8_t s1_flag = 1;

uint8_t ok_flag = 0;
uint8_t cmd_flag = 1;
uint8_t esp_config_flag = 1;
uint8_t config_state = 0;
uint16_t error_cnt = 0;
uint8_t first_cmd = 1;

char received_string[23];
char char_buffer;
uint8_t record_begin;
uint8_t string_control = 0;
char data_to_send[4];
uint8_t first = 1;

  
//--------------------------------------------------------------------------
// FUNCTIONS definitions
//--------------------------------------------------------------------------
uint8_t ok_check_f(void);
void esp_send_cmd(char command[]);

//--------------------------------------------------------------------------
// MAIN function
//--------------------------------------------------------------------------
int main (){
  sei(); // Global interrupt enable
  
  //----- USART initialization -----
  usart1_init(_UBRR);
  usart0_init(_UBRR);
  
  usart1_erase_screen();
  usart1_cursor_positioning(1,1);
  usart1_cursor_invisible();

  
  while(1) {
    //----- ESP8266 configuration -----
    if (esp_config_flag){
      switch (config_state){

	// Test ESP connection
      case ESP_TEST:
	if (first_cmd){
	  esp_send_cmd("AT");  // Send command to ESP
	  usart1_cursor_positioning(1,1);
	  uart1_puts("AT check");  // Send string to console
	  first_cmd = 0;  // Clear flag, command to ESP won't be send again
	}

	ok_flag = ok_check_f();  // ok_flag == 1 if ESP send 'OK'
	if (ok_flag){
	  usart1_cursor_positioning(2,1);
	  uart1_puts("ESP_TEST OK!");  // Send string to console
	  error_cnt = 0;  // Clear error counter
	  first_cmd = 1;  // Set flag to allow sending command to ESP
	  config_state = AP_CONNECT;
	}
	else error_cnt++;  // If ESP did't send response increase error couner

	if (error_cnt >= 40){
	  usart1_cursor_positioning(2,1);
	  uart1_puts("ESP_TEST ERROR!");
	  first_cmd = 1;  // If error counter is greater than 40 send the command again
	  error_cnt = 0;
	}

	_delay_ms(1);
	break;

	// Connect to AP
      case AP_CONNECT:
	if (first_cmd){
	  esp_send_cmd("AT+CWJAP=\"SSID\",\"PWD\"");
	  usart1_cursor_positioning(3,1);
	  uart1_puts("AT+CWJAP check");
	  first_cmd = 0;
	}

	ok_flag = ok_check_f();
	if (ok_flag){
	  usart1_cursor_positioning(4,1);
	  uart1_puts("AP_CONNECT OK!");
	  error_cnt = 0;
	  first_cmd = 1;
	  config_state = ESP_SSID_SET;
	}
	else error_cnt++;

	if (error_cnt >= 5000){
	  usart1_cursor_positioning(4,1);
	  uart1_puts("AP_CONNECT ERROR!");
	  first_cmd = 1;
	  error_cnt = 0;
	}

	_delay_ms(1);
	break;
	
	// ----- Configure as an Access Point -----
	// Set a SSID, PWD, channel id, enc, max count of stations, <hidden ssid>
      case ESP_SSID_SET:
	if (first_cmd){
	  esp_send_cmd("AT+CWSAP=\"ESP8266\",\"1234\",3,0");
	  usart1_cursor_positioning(5,1);
	  uart1_puts("AT+CWSAP check");
	  first_cmd = 0;
	}
	
	ok_flag = ok_check_f();
	if (ok_flag){
	  usart1_cursor_positioning(6,1);
	  uart1_puts("ESP_SSID_SET OK!");
	  error_cnt = 0;
	  first_cmd = 1;
	  config_state = ESP_IP_ADDRESS; 
	}
	else error_cnt++;
	// Set  IP address of ESP softAP
      case ESP_IP_ADDRESS:
	if (first_cmd){
	  esp_send_cmd("AT+CIPAP=\"192.168.0.101\"");
	  usart1_cursor_positioning(7,1);
	  uart1_puts("AT+CIPAP check");
	  first_cmd = 0;
	}

	ok_flag = ok_check_f();
	if(ok_flag){
	  usart1_cursor_positioning(8,1);
	  uart1_puts("ESP_IP_ADDRESS OK!");
	  error_cnt = 0;
	  first_cmd = 1;
	  config_state = DHCP_MODE;
	}
	else error_cnt++;

	if (error_cnt >= 40){
	  usart1_cursor_positioning(8,1);
	  uart1_puts("ESP_IP_ADDRESS ERROR!");
	  first_cmd = 1;
	  error_cnt = 0;
	}

	_delay_ms(1);
	break;
	
	// Enable DHCP
      case DHCP_MODE:
	if (first_cmd){
	  esp_send_cmd("AT+CWDHCP=0,1");
	  usart1_cursor_positioning(9,1);
	  uart1_puts("AT+CWDHCP check");
	  first_cmd = 0;
	}

	ok_flag = ok_check_f();
	if (ok_flag){
	  usart1_cursor_positioning(10,1);
	  uart1_puts("DHCP_MODE OK!");
	  error_cnt = 0;
	  first_cmd = 1;
	  config_state = MUX_CONNECTION;
	}
	else error_cnt++;

	if(error_cnt >= 40){
	  usart1_cursor_positioning(10,1);
	  uart1_puts("DHCP_MODE ERROR!");
	  first_cmd = 1;
	  error_cnt = 0;
	}

	_delay_ms(1);
	break;

	// Enable multiple connections
      case MUX_CONNECTION:
	if (first_cmd){
	  esp_send_cmd("AT+CIPMUX=1");
	  usart1_cursor_positioning(11,1);
	  uart1_puts("AT+CIPMUX check");
	  first_cmd = 0;
	}

	ok_flag = ok_check_f();
	if (ok_flag){
	  usart1_cursor_positioning(12,1);
	  uart1_puts("MUX_CONNECT OK!");
	  error_cnt = 0;
	  first_cmd = 1;
	  config_state = SERVER_START;
	}
	else error_cnt++;

	if (error_cnt >= 40){
	  usart1_cursor_positioning(12,1);
	  uart1_puts("MUX_CONNECT ERROR!");
	  first_cmd = 1;
	  error_cnt = 0;
	}

	_delay_ms(1);
	break;
	
	// Configure as server
      case SERVER_START:
	if (first_cmd){
	  esp_send_cmd("AT+CIPSERVER=1,80");
	  usart1_cursor_positioning(13,1);
	  uart1_puts("AT+CIPSERVER check");
          first_cmd = 0;
	}

	ok_flag = ok_check_f();
	if (ok_flag){
	  usart1_cursor_positioning(14,1);
	  uart1_puts("SERVER_START OK!");
	  error_cnt = 0;
	  esp_config_flag = 0;
	  uart1_puts("\r\nConfiguration finished\r\n");
	  _delay_ms(1000);
	  usart1_erase_screen();
	}
	else error_cnt++;

	if (error_cnt >= 40){
	  usart1_cursor_positioning(14,1);
	  uart1_puts("SERVER_START ERROR!");
	  first_cmd = 1;
	  error_cnt = 0;
	}

	_delay_ms(1);
	break;
      }
    }
    else{
      char_buffer = uart0_getc(); // Seve received data to buffer
      // Seach for 'T'. If found, start saving data to string.
      if (char_buffer == 'T') record_begin = 1; 
      // Search for 'C'. If found stop saving data to string and
      //   display received temperatures
      if (char_buffer == 'C') {
	record_begin = 0;
	// Send data to console, properly set
	if (received_string[1] == '0'){
	  if (received_string[4] == '0') usart1_cursor_positioning(4,4);
	  else if (received_string[4] == '1') usart1_cursor_positioning(4,16);
	}
	else if (received_string[1] == '1'){
	  if (received_string[4] == '0') usart1_cursor_positioning(8,4);
	  else if (received_string[4] == '1') usart1_cursor_positioning(8,16);
	}
	else if (received_string[1] == '2'){
	  if (received_string[4] == '0') usart1_cursor_positioning(12,4);
	  else if (received_string[4] == '1') usart1_cursor_positioning(12,16);
	}
	else usart1_cursor_positioning(4,4);
	
	for (uint8_t i = 0; i < 4; i++){
	  data_to_send[i] = received_string[i+5];
	}
	uart1_puts(data_to_send);
	string_control = 0;
      }
      if (record_begin){
	if (char_buffer != received_string[string_control])
	  received_string[string_control++] = char_buffer;
      }

      // If it was no data send to console before, clear console
      if (first){
	usart1_erase_screen();
	first = 0;
      }
    }
  }
}


//=========================================================================
// FUNCTIONS
//=========================================================================
//-------------------------------------------------------------------------
void esp_send_cmd(char command[]) {
  strcat(command, "\r\n");  // Concatenation of the command and ENTER
                            //  to send command to ESP 
  uart0_puts(command);  // Send command to esp
}

//-------------------------------------------------------------------------
// Function to check if ESP properly response
uint8_t ok_check_f(void){
  char receive_buffer;
  char tab = '\0';
  
  uint8_t ok_cnt = 0;

  receive_buffer = uart0_getc();
  
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
  
    receive_buffer = uart0_getc();
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

