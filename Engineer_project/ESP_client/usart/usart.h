/*
 * usart.h
 *
 *  Created on: 11/18/18
 *  Author: Viarus
 */

#ifndef USART_USART_H_
#define USART_USART_H_

#define UART_BAUD 115200
#define MYUBRR F_CPU/16/UART_BAUD-1
#define _UBRR ((F_CPU+UART_BAUD*8UL) / (16UL*UART_BAUD)-1)

#define UART_RX_BUF_SIZE 256
#define UART_RX_BUF_MASK (UART_RX_BUF_SIZE - 1)

#define UART_TX_BUF_SIZE 256
#define UART_TX_BUF_MASK (UART_TX_BUF_SIZE - 1)

void usart_init(uint16_t ubrr);
void uart_putc(char data);
void uart_puts(char *str);
void uart_putint(int value);
char uart_getc(void);
void uart_clear(void);
#endif /* USART_USART_H_ */
