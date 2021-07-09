/**
 * @file isr.c
 * Implements peripheral and system interrupt handlers.
 * All peripheral interrupt handlers provide a means to register a
 * handler function, and simply serve as a wrapper around it
 */

#include <stdlib.h>

#include <drivers/device/device.h>
#include <sys/task/task.h>
#include <util/bitmask.h>

#include "isr.h"

/** Provided in init.c */
extern void system_init(void);
/** Provided by linker */
extern unsigned char _stack_ptr;

// Dynamic array of exception handlers
static void (*exception_handlers[84])(void) = { 0 };

/**
 * System interrupt handler definitions. These should not be called, they
 * are implemented for use in the NVIC table
 */

/**
 * Default Handler for an ISR.
 */
static void DefaultISRHandler(void) {
    /** Read ICSR to determine exception number */
    uint8_t vecactive = (READBITS(SCB->ICSR, SCB_ICSR_VECTACTIVE_Msk) - 16);
    // Check if a handler is installed for this function
    if (exception_handlers[vecactive] != NULL) {
        exception_handlers[vecactive]();
    }

}

/**
 * Non maskable interrupt handler
 */
void NMI_irq() {
    while (1)
        ;
}

/**
 * Hard fault interrupt handler
 */
void HardFault_irq() {
    while (1)
        ;
}

/**
 * Memory management fault interrupt handler
 */
void MMFault_irq() {
    while (1)
        ;
}

/**
 * Bus fault interrupt handler
 */
void BusFault_irq() {
    while (1)
        ;
}

/**
 * Usage fault interrupt handler
 */
void UsageFault_irq() {
    while (1)
        ;
}

/**
 * SVCall interrupt handler
 */
void SVCall_irq() {
    while (1)
        ;
}

/**
 * Debug monitor interrupt handler
 */
void DebugMonitor_irq() {
    while (1)
        ;
}

/**
 * PendSV interrupt handler
 */
void PendSV_irq() {
    while (1)
        ;
}

/**
 * SysTick interrupt handler
 */
void SysTick_irq() {
    while (1)
        ;
}

/*
 * Exception Vector Table. See page 321 of Datasheet for list.
 * Reset vector is required for code to run.
 */
__attribute__((section(".vectors"))) const uint32_t exception_vectors[] = {
    (uint32_t)&_stack_ptr,       /*!< -16 address for top of stack */
    (uint32_t)system_init,       /*!< -15 Reset handler */
    (uint32_t)NMI_irq,           /*!< -14 NMI */
    (uint32_t)HardFault_irq,     /*!< -13 Hard fault */
    (uint32_t)MMFault_irq,       /*!< -12 Memory management fault */
    (uint32_t)BusFault_irq,      /*!< -11 Bus fault */
    (uint32_t)UsageFault_irq,    /*!< -10 Usage fault */
    (uint32_t)0,                 /*!< -9 Reserved */
    (uint32_t)0,                 /*!< -8 Reserved */
    (uint32_t)0,                 /*!< -7 Reserved */
    (uint32_t)0,                 /*!< -6 Reserved */
    (uint32_t)SVCallHandler,        /*!< -5 SVCall */
    (uint32_t)DebugMonitor_irq,  /*!< -4 Debug Monitor */
    (uint32_t)0,                 /*!< -3 Reserved */
    (uint32_t)PendSVHandler,        /*!< -2 PendSV */
    (uint32_t)SysTickHandler,       /*!< -1 Systick */
    (uint32_t)DefaultISRHandler, /*!< 0 Window Watchdog */
    (uint32_t)DefaultISRHandler, /*!< 1 PVD/PVM1 thru EXTI */
    (uint32_t)DefaultISRHandler, /*!< 2 RTC Tamper or timestamp */
    (uint32_t)DefaultISRHandler, /*!< 3 RTC wakeup timer via EXTI */
    (uint32_t)DefaultISRHandler, /*!< 4 Flash global interrupt */
    (uint32_t)DefaultISRHandler, /*!< 5 RCC global interrupt */
    (uint32_t)DefaultISRHandler, /*!< 6 EXTI Line 0 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 7 EXTI Line 1 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 8 EXTI Line 2 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 9 EXTI Line 3 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 10 EXTI Line 4 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 11 DMA1 channel 1 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 12 DMA1 channel 2 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 13 DMA1 channel 3 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 14 DMA1 channel 4 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 15 DMA1 channel 5 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 16 DMA1 channel 6 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 17 DMA1 channel 7 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 18 ADC1 and ADC2 global interrupt */
    (uint32_t)DefaultISRHandler, /*!< 19 CAN1_TX interrupts */
    (uint32_t)DefaultISRHandler, /*!< 20 CAN1_RX0 interrupts */
    (uint32_t)DefaultISRHandler, /*!< 21 CAN1_RX1 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 22 CAN1_SCE interrupt */
    (uint32_t)DefaultISRHandler, /*!< 23 External Line[9:5] Interrupts */
    (uint32_t)DefaultISRHandler, /*!< 24 TIM1 Break interrupt and TIM15 global
                                    interrupt */
    (uint32_t)DefaultISRHandler, /*!< 25 TIM1 Update Interrupt and TIM16
                                    globalinterrupt*/
    (uint32_t)
        DefaultISRHandler, /*!< 26 TIM1 Trigger and Commutation Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 27 TIM1 Capture Compare Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 28 TIM2 global Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 29 TIM3 global Interrupt */
    0,                           /*!< 30 Reserved */
    (uint32_t)DefaultISRHandler, /*!< 31 I2C1 Event Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 32 I2C1 Error Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 33 I2C2 Event Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 34 I2C2 Error Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 35 SPI1 global Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 36 SPI2 global Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 37 USART1 global Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 38 USART2 global Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 39 USART3 global Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 40 External Line[15:10] Interrupts */
    (uint32_t)DefaultISRHandler, /*!< 41 RTC Alarm (A and B) through EXTI Line
                                    Interrupt */
    0,                           /*!< 42 Reserved */
    0,                           /*!< 43 Reserved */
    0,                           /*!< 44 Reserved */
    0,                           /*!< 45 Reserved */
    0,                           /*!< 46 Reserved */
    0,                           /*!< 47 Reserved */
    0,                           /*!< 48 Reserved */
    (uint32_t)DefaultISRHandler, /*!< 49 SDMMC1 global Interrupt */
    0,                           /*!< 50 Reserved */
    (uint32_t)DefaultISRHandler, /*!< 51 SPI3 global Interrupt */
    0,                           /*!< 52 Not supported on STM32L433 */
    0,                           /*!< 53 Reserved */
    (uint32_t)DefaultISRHandler, /*!< 54 TIM6 global and DAC1&2 underrun error
                                    interrupts */
    (uint32_t)DefaultISRHandler, /*!< 55 TIM7 global interrupt */
    (uint32_t)DefaultISRHandler, /*!< 56 DMA2 Channel 1 global Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 57 DMA2 Channel 2 global Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 58 DMA2 Channel 3 global Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 59 DMA2 Channel 4 global Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 60 DMA2 Channel 5 global Interrupt */
    0,                           /*!< 61 Not supported on STM32L433 */
    0,                           /*!< 62 Not supported on STM32L433 */
    0,                           /*!< 63 Reserved */
    (uint32_t)DefaultISRHandler, /*!< 64 COMP1 and COMP2 Interrupts */
    (uint32_t)DefaultISRHandler, /*!< 65 LP TIM1 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 66 LP TIM2 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 67 USB event Interrupt */
    (uint32_t)DefaultISRHandler, /*!< 68 DMA2 Channel 6 global interrupt */
    (uint32_t)DefaultISRHandler, /*!< 69 DMA2 Channel 7 global interrupt */
    (uint32_t)DefaultISRHandler, /*!< 70 LP UART1 interrupt */
    (uint32_t)DefaultISRHandler, /*!< 71 Quad SPI global interrupt */
    (uint32_t)DefaultISRHandler, /*!< 72 I2C3 event interrupt */
    (uint32_t)DefaultISRHandler, /*!< 73 I2C3 error interrupt */
    (uint32_t)
        DefaultISRHandler, /*!< 74 Serial Audio Interface 1 global interrupt */
    0,                     /*!< 75 Reserved */
    (uint32_t)
        DefaultISRHandler, /*!< 76 Serial Wire Interface 1 global interrupt */
    (uint32_t)
        DefaultISRHandler, /*!< 77 Touch Sense Controller global interrupt */
    (uint32_t)DefaultISRHandler, /*!< 78 LCD global interrupt */
    (uint32_t)DefaultISRHandler, /*!< 80 RNG global interrupt */
    (uint32_t)DefaultISRHandler, /*!< 81 FPU global interrupt */
    (uint32_t)DefaultISRHandler  /*!< 82 CRS global interrupt  */
};

/**
 * Enable interrupt number "num" (in Nested vector interrupt controller),
 * and install a handler function for it.
 * @param num: Interrupt number to enable
 * @param handler: Handler function. Will be called from interrupt context.
 */
void enable_irq(uint32_t num, void (*handler)(void)) {
    uint32_t reg_sel;
    // Install exception handler
    exception_handlers[num] = handler;
    // divide "num" by 32 to get interrupt set/enable register # to use
    reg_sel = num >> 5;
    // Subtract "reg_sel" times 32 to get the offset within the set/enable reg
    num = num - (reg_sel << 5);
    SETFIELD(NVIC->ISER[reg_sel], 1UL, num);
}

/**
 * Disable interrupt number "num" (in Nested vector interrupt controller).
 * Resets handler function.
 * @param num: Interrupt number to disable
 */
void disable_irq(uint32_t num) {
    uint32_t reg_sel;
    // Reset exception handler
    exception_handlers[num] = DefaultISRHandler;
    // divide "num" by 32 to get interrupt set/enable register # to use
    reg_sel = num >> 5;
    // Subtract "reg_sel" times 32 to get the offset within the set/enable reg
    num = num - (reg_sel << 5);
    CLEARFIELD(NVIC->ISER[reg_sel], 1UL, num);
}

/**
 *
 * Simple function to disable interrupts.
 * This sets PRIMASK to 1, effectively disabling preemption
 */
void mask_irq() { asm volatile("CPSID i"); }

/**
 * Simple function to reenable interrupts.
 * This sets PRIMASK to 0, effectively allowing preemption
 */
void unmask_irq() { asm volatile("CPSIE i"); }
