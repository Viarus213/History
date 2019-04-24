/*
 * usart0.h
 *
 *  Created on: 09/23/18
 *  Author: Viarus
 */

#ifndef USART_USART0_H_
#define USART_USART0_H_

#define UART_BAUD 115200
#define MYUBRR F_CPU/16/UART_BAUD-1
#define _UBRR ((F_CPU+UART_BAUD*8UL) / (16UL*UART_BAUD)-1)

#define UART0_RX_BUF_SIZE 256
#define UART0_RX_BUF_MASK (UART0_RX_BUF_SIZE - 1)

#define UART0_TX_BUF_SIZE 256
#define UART0_TX_BUF_MASK (UART0_TX_BUF_SIZE - 1)

void usart0_init(uint16_t ubrr);
void uart0_clear(void);
void uart0_putc(char data);
void uart0_puts(char *str);
void uart0_putint(int value);
char uart0_getc(void);

// Cursor handling
void usart0_cursor_positioning(uint8_t row, uint8_t column); // Home-positioning to X and Y coordinates (ANIS uses 1-1)
void usart0_cursor_up(uint8_t value);        // Moves the cursor up by value rows
void usart0_cursor_down(uint8_t value);      // Moves the cursor down by value rows
void usart0_cursor_forward(uint8_t value);   // Moves the cursor forward by value columns
void usart0_cursor_backward(uint8_t value);  // Moves the cursor backward by value columns
void usart0_cursor_save(void);      // Save current cursor position
void usart0_cursor_restore(void);   // Restore saved cursor position
void usart0_cursor_invisible(void); // Make cursor invisible
void usart0_cursor_visible(void);   // Make cursor visible

// Erasing text
void usart0_erase_line_cursor_end(void); // Erases from the current cursor position to the end of the current line
void usart0_erase_line_cursor_beginning(void); // Erases from the current cursor position to the start of the current line
void usart0_erase_line(void);    // Erases the entire current line
void usart0_erase_down(void);    // Erases the screen from the current line down to the bottom of the screen
void usart0_erase_up(void);      // Erases the screen from the current line up to the top of the screen
void usart0_erase_screen(void);  // Erases the screen with the background colour and moves the cursor to home

// General text attributes
void usart0_attr_reset_all(void);  // Reset all attributes
void usart0_attr_bright(void);     // Set "bright" attribute
void usart0_attr_dim(void);        // Set "dim" attribute
void usart0_attr_standout(void);   // Set "standout" attribute
void usart0_attr_underscore(void); // Set "underscore" attribute
void usart0_attr_blink(void);      // Set "blink" attribute
void usart0_attr_reverse(void);    // Set "reverse" attribute
void usart0_attr_hidden(void);     // Set "hidden" attribute


// Foreground colouring
void usart0_foreground_black(void);
void usart0_foreground_red(void);
void usart0_foreground_green (void);
void usart0_foreground_yellow(void);
void usart0_foreground_blue(void);
void usart0_foreground_magenta(void);
void usart0_foreground_cyan(void);
void usart0_foreground_white(void);
void usart0_foreground_set_default(void);

// Background colouring
void usart0_background_black(void);
void usart0_background_red (void);
void usart0_background_green(void);
void usart0_background_yellow(void);
void usart0_background_blue(void);
void usart0_background_magenta(void);
void usart0_background_cyan(void);
void usart0_background_white(void);
void usart0_background_set_default(void);

#endif /* USART_USART0_H_ */
