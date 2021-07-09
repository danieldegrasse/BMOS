/**
 * @file init.c
 * Init code for STM32L433xx. Contains base initialization functions
 */
#include <stdint.h>
#include <stdlib.h>

#include <drivers/clock/clock.h>

// Variables declared in linker script
extern unsigned char _srcdata;
extern unsigned char _sdata;
extern unsigned char _edata;
extern unsigned char _sbss;
extern unsigned char _ebss;
extern unsigned char _stack_ptr;

// Function prototypes
static void init_data_bss(void);

// External functions
extern void __libc_init_array(void); // Provided by newlib
extern int main(void); // Assume that the user will provide a main function

void system_init(void);

/**
 * System reset handler. Sets up data and bss, initializes clocks to reset
 * values, and sets up libc. Then, calls main code entry point.
 * Exception vector table is implemented in isr.c
 */
void system_init(void) {
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