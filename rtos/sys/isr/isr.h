#ifndef ISR_H
#define ISR_H

#include <drivers/uart/uart.h>

/**
 * UART ISR registration function.
 * @param handler: UART ISR handler. Will be called with the UART peripheral 
 * number that generated an interrupt when one fires.
 */
void set_UART_isr(void(*handler)(UART_periph_t));

/**
 * Simple function to disable interrupts.
 * This sets PRIMASK to 1, effectively disabling preemption
 */
void mask_irq();

/**
 * Simple function to reenable interrupts.
 * This sets PRIMASK to 0, effectively allowing preemption
 */
void unmask_irq();

/**
 * Disable interrupt number "num"
 * @param num: Interrupt number to disable
 */
void disable_irq(uint32_t num);

/**
 * Enable interrupt number "num"
 * @param num: Interrupt number to enable
 */
void enable_irq(uint32_t num);


/**
 * System interrupt handler definitions. These should not be called, they
 * are defined for use in the NVIC table
 */

/**
 * Non maskable interrupt handler
 */
void NMI_irq();

/**
 * Hard fault interrupt handler
 */
void HardFault_irq();

/**
 * Memory management fault interrupt handler
 */
void MMFault_irq();

/**
 * Bus fault interrupt handler
 */
void BusFault_irq();

/**
 * Usage fault interrupt handler
 */
void UsageFault_irq();

/**
 * Debug monitor interrupt handler
 */
void DebugMonitor_irq();

/**
 * UART ISR handler. Handles IRQ numbers 37, 38, 39 and 70
 */
void UART_isr(void);
#endif