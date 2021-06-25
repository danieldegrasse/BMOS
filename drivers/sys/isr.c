/**
 * @file isr.c
 * Implements peripheral and system interrupt handlers.
 * All peripheral interrupt handlers provide a means to register a
 * handler function, and simply serve as a wrapper around it
 */

#include <stdlib.h>

#include <device/device.h>
#include <uart/uart.h>
#include <util/util.h>

/* Structure of registered IRQ handlers */
static struct { void (*uart_handler)(UART_periph_t); } irq_handlers = {0};


/**
 * Simple function to disable interrupts.
 * This sets PRIMASK to 1, effectively disabling preemption
 */
inline void disable_irq() {
    asm("MOV r4, #1 \n\t"
        "MSR primask, r4");
}

/**
 * Simple function to reenable interrupts.
 * This sets PRIMASK to 0, effectively allowing preemption
 */
inline void enable_irq() {
    asm("MOV r4, #0 \n\t"
        "MSR primask, r4");
}


/**
 * UART ISR registration function.
 * @param handler: UART ISR handler. Will be called with the UART peripheral
 * number that generated an interrupt when one fires.
 */
void set_UART_isr(void (*handler)(UART_periph_t)) {
    irq_handlers.uart_handler = handler;
}

/**
 * UART ISR handler. Handles IRQ numbers 37, 38, 39 and 70
 */
void UART_isr(void) {
    if (irq_handlers.uart_handler == NULL) {
        // No way to handle interrupt. Return here.
        return;
    }
    /**
     * First check the IRQ number in the ICSR to determine which peripheral
     * fired an interrupt
     */
    switch (READBITS(SCB->ICSR, SCB_ICSR_VECTACTIVE_Msk)) {
    /* Call the uart_handler function with the relevant UART device */
    case USART1_IRQn:
        irq_handlers.uart_handler(USART_1);
        break;
    case USART2_IRQn:
        irq_handlers.uart_handler(USART_2);
        break;
    case USART3_IRQn:
        irq_handlers.uart_handler(USART_3);
        break;
    case LPUART1_IRQn:
        irq_handlers.uart_handler(LPUART_1);
        break;
    default:
        /**
         * Spin here. We want to stop processor as we
         * should not be handling this exception.
         */
        while (1) {
            // Spin
        }
        break;
    }
}