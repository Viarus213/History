/*
 * usart0.c
 *
 *  Created on: 09/23/18
 *  Author: Viarus
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
 
#include "usart0.h"
#include "usart1.h"

volatile char UART0_RxBuf[UART0_RX_BUF_SIZE];
volatile uint8_t UART0_RxHead;
volatile uint8_t UART0_RxTail;

volatile char UART0_TxBuf[UART0_TX_BUF_SIZE];
volatile uint8_t UART0_TxHead;
volatile uint8_t UART0_TxTail;

void usart0_init(uint16_t ubrr){
  // Set baud rate
  UBRR0H = (uint8_t)(ubrr >> 8);
  UBRR0L = (uint8_t)(ubrr);
  // Enable receiver and transmiter
  UCSR0B |= ((1 << RXEN0) | (1 << TXEN0)); // Receiver and Transmiter Enable
  UCSR0B |= (1 << RXCIE0); // RX Complete Inerrupt Enable
  uart1_puts("UART0 configuration finish!\r\n");
}

void uart0_clear(void){
  UART0_RxTail = UART0_RxHead;
  UART0_TxTail = UART0_TxHead;
}

// USART0 Transmit
void uart0_putc(char data){
  uint8_t tmp_head;
  
  tmp_head = (UART0_TxHead + 1) & UART0_TX_BUF_MASK;

  while (tmp_head == UART0_TxTail) {}

  UART0_TxBuf[tmp_head] = data;
  UART0_TxHead = tmp_head;

  UCSR0B |= (1 << UDRIE0); // USART Date Register Empty Interrupt Enable
}

void uart0_puts(char *str){
  register char sign;
  while ((sign = *str++)) uart0_putc(sign);
}

void uart0_putint(int value){
  char str[20];
  itoa(value, str, 10);
  uart0_puts(str);
}

ISR(USART0_UDRE_vect){
  if (UART0_TxHead != UART0_TxTail){
    UART0_TxTail = (UART0_TxTail + 1) & UART0_TX_BUF_MASK;

    UDR0 = UART0_TxBuf[UART0_TxTail];
  }
  else {
    UCSR0B &= ~(1 << UDRIE0);
  }
}

// USART0 Receive
char uart0_getc(void){
  if (UART0_RxHead == UART0_RxTail) return 0;

  UART0_RxTail = (UART0_RxTail + 1) & UART0_RX_BUF_MASK;

  return UART0_RxBuf[UART0_RxTail];
}

ISR(USART0_RX_vect){
  uint8_t tmp_head;
  char data;

  data = UDR0;
  tmp_head = (UART0_RxHead + 1) & UART0_RX_BUF_MASK;
  if (tmp_head == UART0_RxTail){
    uart1_puts("UART0 RECEIVE ERROR\r\n");
  }
  else {
    UART0_RxHead = tmp_head;
    UART0_RxBuf[tmp_head] = data;
  }
}

//------------------------------------------------------------------------------
// Cursor handling
//------------------------------------------------------------------------------
void usart0_cursor_positioning(uint8_t row, uint8_t column){
  uart0_putc(0x1b);
  uart0_putc('[');
  uart0_putint(row);
  uart0_putc(';');
  uart0_putint(column);
  uart0_putc('H');
}

void usart0_cursor_up(uint8_t value){
  uart0_putc(0x1b);
  uart0_putint(value);
  uart0_putc('A');
}

void usart0_cursor_down(uint8_t value){
  uart0_putc(0x1b);
  uart0_putint(value);
  uart0_putc('B');
}

void usart0_cursor_forward(uint8_t value){
  uart0_putc(0x1b);
  uart0_putint(value);
  uart0_putc('C');
}

void usart0_cursor_backward(uint8_t value){
  uart0_putc(0x1b);
  uart0_putint(value);
  uart0_putc('D');
}

void usart0_cursor_save(void){
  uart0_putc(0x1b);
  uart0_putint(7);
}

void usart0_cursor_restore(void){
  uart0_putc(0x1b);
  uart0_putint(8);
}

void usart0_cursor_invisible(void){
  uart0_putc(0x1b);
  uart0_putc('[');
  uart0_putc('?');
  uart0_putint(25);
  uart0_putc('l');
}

void usart0_cursor_visible(void){
  uart0_putc(0x1b);
  uart0_putc('[');
  uart0_putc('?');
  uart0_putint(25);
  uart0_putc('h');
}

//------------------------------------------------------------------------------
// Erasing text
//------------------------------------------------------------------------------
void usart0_erase_line_usart0_cursor_end(void){
  uart0_putc(0x1b);
  uart0_puts("[0K");
}

void usart0_erase_line_beginning_cursor(void){
  uart0_putc(0x1b);
  uart0_puts("[1K");
}

void usart0_erase_line(void){
  uart0_putc(0x1b);
  uart0_puts("[2K");
}

void usart0_erase_down(void){
  uart0_putc(0x1b);
  uart0_puts("[0J");
}

void usart0_erase_up(void){
  uart0_putc(0x1b);
  uart0_puts("[1J");
}

void usart0_erase_screen(void){
  uart0_putc(0x1b);
  uart0_puts("[2J");
}

//------------------------------------------------------------------------------
// General text attributes
//------------------------------------------------------------------------------
void usart0_attrreset_all(void){
  uart0_putc(0x1b);
  uart0_puts("[0m");
}

void usart0_attrbright(void){
  uart0_putc(0x1b);
  uart0_puts("[1m");
}

void usart0_attrdim(void){
  uart0_putc(0x1b);
  uart0_puts("[2m");
}

void usart0_attrstandout(void){
  uart0_putc(0x1b);
  uart0_puts("[3m");
}

void usart0_attrunderscore(void){
  uart0_putc(0x1b);
  uart0_puts("[4m");
}

void usart0_attrblink(void){
  uart0_putc(0x1b);
  uart0_puts("[5m");
}

void usart0_attrreverse(void){
  uart0_putc(0x1b);
  uart0_puts("[7m");
}

void usart0_attrhidden(void){
  uart0_putc(0x1b);
  uart0_puts("[8m");
}

//------------------------------------------------------------------------------
// Foreground colouring
//------------------------------------------------------------------------------
void usart0_foreground_black(void){
  uart0_putc(0x1b);
  uart0_puts("[30m");
}

void usart0_foreground_red(void){
  uart0_putc(0x1b);
  uart0_puts("[31m");
}

void usart0_foreground_green(void){
  uart0_putc(0x1b);
  uart0_puts("[32m");
}

void usart0_foreground_yellow(void){
  uart0_putc(0x1b);
  uart0_puts("[33m");
}

void usart0_foreground_blue(void){
  uart0_putc(0x1b);
  uart0_puts("[34m");
}

void usart0_foreground_magenta(void){
  uart0_putc(0x1b);
  uart0_puts("[35m");
}

void usart0_foreground_cyan(void){
  uart0_putc(0x1b);
  uart0_puts("[36m");
}

void usart0_foreground_white(void){
  uart0_putc(0x1b);
  uart0_puts("[37m");
}

void usart0_foreground_set_default(void){
  uart0_putc(0x1b);
  uart0_puts("[39m");
}
//------------------------------------------------------------------------------
// Usart0_Background colouring
//------------------------------------------------------------------------------
void usart0_background_black(void){
  uart0_putc(0x1b);
  uart0_puts("[40m");
}

void usart0_background_red(void){
  uart0_putc(0x1b);
  uart0_puts("[41m");
}

void usart0_background_green(void){
  uart0_putc(0x1b);
  uart0_puts("[42m");
}

void usart0_background_yellow(void){
  uart0_putc(0x1b);
  uart0_puts("[43m");
}

void usart0_background_blue(void){
  uart0_putc(0x1b);
  uart0_puts("[44m");
}

void usart0_background_magenta(void){
  uart0_putc(0x1b);
  uart0_puts("[45m");
}

void usart0_background_cyan(void){
  uart0_putc(0x1b);
  uart0_puts("[46m");
}

void usart0_background_white(void){
  uart0_putc(0x1b);
  uart0_puts("[47m");
}

void usart0_background_set_default(void){
  uart0_putc(0x1b);
  uart0_puts("[49m");
}
