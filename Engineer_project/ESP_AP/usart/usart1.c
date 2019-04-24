/*
 *  usart1.c
 *
 *  Created on: 09/23/18
 *  Author: Viarus
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "usart1.h"

volatile char UART1_RxBuf[UART1_RX_BUF_SIZE];
volatile uint8_t UART1_RxHead;
volatile uint8_t UART1_RxTail;

volatile char UART1_TxBuf[UART1_TX_BUF_SIZE];
volatile uint8_t UART1_TxHead;
volatile uint8_t UART1_TxTail;

void usart1_init(uint16_t ubrr){
  // Set baud rate
  UBRR1H = (uint8_t)(ubrr >> 8);
  UBRR1L = (uint8_t)(ubrr);
  UCSR1B |= ((1 << RXEN1) | (1 << TXEN1)); // Receiver and Transmiter Enable
  UCSR1B |= (1 << RXCIE1); // RX Complete Interrupt Enable
  uart1_puts("UART1 configuration finish!\r\n");
}

// USART1 Transmit
void uart1_putc(char data){
  uint8_t tmp_head;
  
  tmp_head = (UART1_TxHead + 1) & UART1_TX_BUF_MASK;

  while (tmp_head == UART1_TxTail) {}

  UART1_TxBuf[tmp_head] = data;
  UART1_TxHead = tmp_head;

  UCSR1B |= (1 << UDRIE1); // USART Date Register Empty Interrupt Enable
}

void uart1_puts(char *str){
  register char sign;
  while((sign = *str++)) uart1_putc(sign);
}

void uart1_putint(int value){
  char str[20];
  itoa(value, str, 10);
  uart1_puts(str);
}

ISR(USART1_UDRE_vect){
  if (UART1_TxHead != UART1_TxTail){
    UART1_TxTail = (UART1_TxTail + 1) & UART1_TX_BUF_MASK;
    UDR1 = UART1_TxBuf[UART1_TxTail];
  }
  else{
    UCSR1B &= ~(1 << UDRIE1);
  }
}

// USART1 Receive
char uart1_getc(void){
  if (UART1_RxHead == UART1_RxTail) return 0;

  UART1_RxTail = (UART1_RxTail + 1) & UART1_RX_BUF_MASK;

  return UART1_RxBuf[UART1_RxTail];
}

ISR(USART1_RX_vect){
  uint8_t tmp_head;
  char data;
  data = UDR1;
  tmp_head = (UART1_RxHead + 1) & UART1_RX_BUF_MASK;

  if (tmp_head == UART1_RxTail){
    uart1_puts("UART1 RECEIVE ERROR\r\n");
  }
  else {
    UART1_RxHead = tmp_head;
    UART1_RxBuf[tmp_head] = data;
  }
}

//----------------------------------------------------------------------------------------------------
// Cursor handling
//----------------------------------------------------------------------------------------------------
void usart1_cursor_positioning(uint8_t row, uint8_t column){
  uart1_putc(0x1b);
  uart1_putc('[');
  uart1_putint(row);
  uart1_putc(';');
  uart1_putint(column);
  uart1_putc('H');
}

void usart1_cursor_up(uint8_t value){
  uart1_putc(0x1b);
  uart1_putint(value);
  uart1_putc('A');
}

void usart1_cursor_down(uint8_t value){
  uart1_putc(0x1b);
  uart1_putint(value);
  uart1_putc('B');
}

void usart1_cursor_forward(uint8_t value){
  uart1_putc(0x1b);
  uart1_putint(value);
  uart1_putc('C');
}

void usart1_cursor_backward(uint8_t value){
  uart1_putc(0x1b);
  uart1_putint(value);
  uart1_putc('D');
}

void usart1_cursor_save(void){
  uart1_putc(0x1b);
  uart1_putint(7);
}

void usart1_cursor_restore(void){
  uart1_putc(0x1b);
  uart1_putint(8);
}

void usart1_cursor_invisible(void){
  uart1_putc(0x1b);
  uart1_putc('[');
  uart1_putc('?');
  uart1_putint(25);
  uart1_putc('l');
}

void usart1_cursor_visible(void){
  uart1_putc(0x1b);
  uart1_putc('[');
  uart1_putc('?');
  uart1_putint(25);
  uart1_putc('h');
}

//----------------------------------------------------------------------------------------------------
// Erasing text
//----------------------------------------------------------------------------------------------------
void erase_line_usart1_cursor_end(void){
  uart1_putc(0x1b);
  uart1_puts("[0K");
}

void usart1_erase_line_beginning_cursor(void){
  uart1_putc(0x1b);
  uart1_puts("[1K");
}

void usart1_erase_line(void){
  uart1_putc(0x1b);
  uart1_puts("[2K");
}

void usart1_erase_down(void){
  uart1_putc(0x1b);
  uart1_puts("[0J");
}

void usart1_erase_up(void){
  uart1_putc(0x1b);
  uart1_puts("[1J");
}

void usart1_erase_screen(void){
  uart1_putc(0x1b);
  uart1_puts("[2J");
}

//----------------------------------------------------------------------------------------------------
// General text attributes
//----------------------------------------------------------------------------------------------------
void usart1_attr_reset_all(void){
  uart1_putc(0x1b);
  uart1_puts("[0m");
}

void usart1_attr_bright(void){
  uart1_putc(0x1b);
  uart1_puts("[1m");
}

void usart1_attr_dim(void){
  uart1_putc(0x1b);
  uart1_puts("[2m");
}

void usart1_attr_standout(void){
  uart1_putc(0x1b);
  uart1_puts("[3m");
}

void usart1_attr_underscore(void){
  uart1_putc(0x1b);
  uart1_puts("[4m");
}

void usart1_attr_blink(void){
  uart1_putc(0x1b);
  uart1_puts("[5m");
}

void usart1_attr_reverse(void){
  uart1_putc(0x1b);
  uart1_puts("[7m");
}

void usart1_attr_hidden(void){
  uart1_putc(0x1b);
  uart1_puts("[8m");
}

//----------------------------------------------------------------------------------------------------
// Foreground colouring
//----------------------------------------------------------------------------------------------------
void usart1_foreground_black(void){
  uart1_putc(0x1b);
  uart1_puts("[30m");
}

void usart1_foreground_red(void){
  uart1_putc(0x1b);
  uart1_puts("[31m");
}

void usart1_foreground_green(void){
  uart1_putc(0x1b);
  uart1_puts("[32m");
}

void usart1_foreground_yellow(void){
  uart1_putc(0x1b);
  uart1_puts("[33m");
}

void usart1_foreground_blue(void){
  uart1_putc(0x1b);
  uart1_puts("[34m");
}

void usart1_foreground_magenta(void){
  uart1_putc(0x1b);
  uart1_puts("[35m");
}

void usart1_foreground_cyan(void){
  uart1_putc(0x1b);
  uart1_puts("[36m");
}

void usart1_foreground_white(void){
  uart1_putc(0x1b);
  uart1_puts("[37m");
}

void usart1_foreground_set_default(void){
  uart1_putc(0x1b);
  uart1_puts("[39m");
}
//----------------------------------------------------------------------------------------------------
// Background colouring
//----------------------------------------------------------------------------------------------------
void usart1_background_black(void){
  uart1_putc(0x1b);
  uart1_puts("[40m");
}

void usart1_background_red(void){
  uart1_putc(0x1b);
  uart1_puts("[41m");
}

void usart1_background_green(void){
  uart1_putc(0x1b);
  uart1_puts("[42m");
}

void usart1_background_yellow(void){
  uart1_putc(0x1b);
  uart1_puts("[43m");
}

void usart1_background_blue(void){
  uart1_putc(0x1b);
  uart1_puts("[44m");
}

void usart1_background_magenta(void){
  uart1_putc(0x1b);
  uart1_puts("[45m");
}

void usart1_background_cyan(void){
  uart1_putc(0x1b);
  uart1_puts("[46m");
}

void usart1_background_white(void){
  uart1_putc(0x1b);
  uart1_puts("[47m");
}

void usart1_background_set_default(void){
  uart1_putc(0x1b);
  uart1_puts("[49m");
}
