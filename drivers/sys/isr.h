#ifndef ISR_H
#define ISR_H

#include <uart/uart.h>

/**
 * UART ISR registration function.
 * @param handler: UART ISR handler. Will be called with the UART peripheral 
 * number that generated an interrupt when one fires.
 */
void set_UART_isr(void(*handler)(UART_periph_t));

/**
 * UART ISR handler. Handles IRQ numbers 37, 38, 39 and 70
 */
void UART_isr(void);

#endif