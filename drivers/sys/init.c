/**
 * @file init.c
 * Init code for STM32L433xx Contains interrupt vectors and base init.
 * function.
 */
#include <stdint.h>

// Variables declared in linker script
extern unsigned char _srcdata;
extern unsigned char _sdata;
extern unsigned char _edata;
extern unsigned char _sbss;
extern unsigned char _ebss;
extern unsigned char _stack_ptr;

// Function prototypes
void init(void);
void DefaultISRHandler(void);
extern void main(void); // Assume that the user will provide a main function
/*
 * Exception Vector Table. See page 321 of Datasheet for list.
 * Reset vector is required for code to run.
 */
const uint32_t exception_vectors[] __attribute__((section(".vectors"))) = {
    (uint32_t)&_stack_ptr,      /* address for top of stack */
    (uint32_t)init,              /* Reset handler */
    (uint32_t)DefaultISRHandler, /* NMI */
    (uint32_t)DefaultISRHandler, /* Hard fault */
    (uint32_t)DefaultISRHandler, /* Bus fault */
    (uint32_t)DefaultISRHandler, /* Memory management fault */
    (uint32_t)DefaultISRHandler, /* Usage fault */
    (uint32_t)0,                 /* Reserved */
    (uint32_t)DefaultISRHandler, /* SVCall */
    (uint32_t)DefaultISRHandler, /* Debug Monitor */
    (uint32_t)0,                 /* Reserved */
    (uint32_t)DefaultISRHandler, /* PendSV */
    (uint32_t)DefaultISRHandler  /* SysTick */
};

/**
 * Default Handler for an ISR.
 */
void DefaultISRHandler(void) {
    // Spin.
    while (1) {
    }
}

/**
 * Core init function for MCU. Sets up global variables.
 *
 * Reference: http://eleceng.dit.ie/frank/arm/BareMetalTILM4F/index.html
 */
void init(void) {
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
    // init is done. Call the main entry point.
    main();

    // Should not reach this point unless main returns.
    while (1);
}
