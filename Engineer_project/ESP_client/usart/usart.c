/*
 * usart0.c
 *
 *  Created on: 09/23/18
 *  Author: Viarus
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
 
#include "usart.h"

volatile char UART_RxBuf[UART_RX_BUF_SIZE];
volatile uint8_t UART_RxHead;
volatile uint8_t UART_RxTail;

volatile char UART_TxBuf[UART_TX_BUF_SIZE];
volatile uint8_t UART_TxHead;
volatile uint8_t UART_TxTail;

void usart_init(uint16_t ubrr){
  // Set baud rate
  UBRRH = (uint8_t)(ubrr >> 8);
  UBRRL = (uint8_t)(ubrr);
  // Enable receiver and transmiter
  UCSRB = ((1 << RXEN) | (1 << TXEN)); // Receiver and Transmiter Enable
  UCSRB |= (1 << RXCIE); // RX Complete Inerrupt Enable
  UCSRC = ((1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0));
}

// USART Transmit
void uart_putc(char data){
  uint8_t tmp_head;
  
  tmp_head = (UART_TxHead + 1) & UART_TX_BUF_MASK;

  while (tmp_head == UART_TxTail) {}

  UART_TxBuf[tmp_head] = data;
  UART_TxHead = tmp_head;

  UCSRB |= (1 << UDRIE); // USART Date Register Empty Interrupt Enable
  
}

void uart_puts(char *str){
  register char sign;
  while ((sign = *str++)) uart_putc(sign);
}

void uart_putint(int value){
  char str[20];
  itoa(value, str, 10);
  uart_puts(str);
}

ISR(USART_UDRE_vect){
  if (UART_TxHead != UART_TxTail){
    UART_TxTail = (UART_TxTail + 1) & UART_TX_BUF_MASK;

    UDR = UART_TxBuf[UART_TxTail];
  }
  else {
    UCSRB &= ~(1 << UDRIE);
  }
}

// USART Receive
char uart_getc(void){
  if (UART_RxHead == UART_RxTail) return 0;

  UART_RxTail = (UART_RxTail + 1) & UART_RX_BUF_MASK;

  return UART_RxBuf[UART_RxTail];
}

ISR(USART_RXC_vect){
  uint8_t tmp_head;
  char data;

  data = UDR;
  tmp_head = (UART_RxHead + 1) & UART_RX_BUF_MASK;
  if (tmp_head == UART_RxTail){
    uart_puts("UART ERROR\r\n");
  }
  else {
    UART_RxHead = tmp_head;
    UART_RxBuf[tmp_head] = data;
  }
}

void uart_clear(void){
  UART_RxTail = UART_RxHead;
  UART_TxTail = UART_TxHead;
}
