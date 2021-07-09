#ifndef ISR_H
#define ISR_H

/** Macro to convert IRQ number to exception number */
#define IRQN_TO_EXCEPTION(irq) (irq) + 16

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
 * Disable interrupt number "num" (in Nested vector interrupt controller).
 * Resets handler function.
 * @param num: Interrupt number to disable
 */
void disable_irq(uint32_t num);

/**
 * Enable interrupt number "num" (in Nested vector interrupt controller),
 * and install a handler function for it.
 * @param num: Interrupt number to enable
 * @param handler: Handler function. Will be called from interrupt context.
 */
void enable_irq(uint32_t num, void (*handler)(void));

#endif