/**
 * @file init.c
 * Init code for STM32L433xx Contains interrupt vectors and base init.
 * function.
 */
#include <stdint.h>
#include <stdlib.h>

#include <drivers/clock/clock.h>
#include <sys/isr/isr.h>
#include <sys/task/task.h>

// Variables declared in linker script
extern unsigned char _srcdata;
extern unsigned char _sdata;
extern unsigned char _edata;
extern unsigned char _sbss;
extern unsigned char _ebss;
extern unsigned char _stack_ptr;

// Function prototypes
static void system_init(void);
static void init_data_bss(void);
static void DefaultISRHandler(void);

// External functions
extern void __libc_init_array(void); // Provided by newlib
extern int main(void); // Assume that the user will provide a main function

/*
 * Exception Vector Table. See page 321 of Datasheet for list.
 * Reset vector is required for code to run.
 */
const uint32_t exception_vectors[] __attribute__((section(".vectors"))) = {
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
    (uint32_t)SVCallHandler,     /*!< -5 SVCall */
    (uint32_t)DebugMonitor_irq,  /*!< -4 Debug Monitor */
    (uint32_t)0,                 /*!< -3 Reserved */
    (uint32_t)PendSVHandler,     /*!< -2 PendSV */
    (uint32_t)SysTickHandler,    /*!< -1 Systick */
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
    (uint32_t)UART_isr,          /*!< 37 USART1 global Interrupt */
    (uint32_t)UART_isr,          /*!< 38 USART2 global Interrupt */
    (uint32_t)UART_isr,          /*!< 39 USART3 global Interrupt */
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
    (uint32_t)UART_isr,          /*!< 70 LP UART1 interrupt */
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
 * System reset handler. Sets up data and bss, initializes clocks to reset
 * values, and sets up libc. Then, calls main code entry point.
 */
static void system_init(void) {
    int ret;
    // First initialize global variables
    init_data_bss();
    // Now that data and BSS segments are populated, initialize clocks
    reset_clocks();
    // Init libs
    __libc_init_array();
    // init is done. Call the main entry point.
    ret = main();
    // exit with the return value of main
    exit(ret);
}

/**
 * Default Handler for an ISR.
 */
static void DefaultISRHandler(void) {
    // Spin.
    while (1)
        ;
}

/**
 * Core init function for MCU. Sets up global variables by copying data from
 * flash to ram, and zeroing BSS
 *
 * Reference: http://eleceng.dit.ie/frank/arm/BareMetalTILM4F/index.html
 */
static void init_data_bss(void) {
    unsigned char *src;
    unsigned char *dst;
    unsigned int len;
    // Copy all data values into the correct starting location.
    src = &_srcdata;
    dst = &_sdata;
    len = &_edata - &_sdata;
    // Copy each byte of src to dst.
    while (len--) {
        *dst++ = *src++;
    }
    // Zero out all bss values.
    dst = &_sbss;
    len = &_ebss - &_sbss;
    while (len--) {
        *dst++ = 0;
    }
}