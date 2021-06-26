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

/**
 * Simple function to disable interrupts.
 * This sets PRIMASK to 1, effectively disabling preemption
 */
void disable_irq();

/**
 * Simple function to reenable interrupts.
 * This sets PRIMASK to 0, effectively allowing preemption
 */
void enable_irq();
#endif