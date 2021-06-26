/**
 * @file clock.h
 * Implements system clock support, including setting the system clock
 * source and delaying by a defined number of milliseconds
 */

#include <sys/err.h>

/** 
 * System clock source/speed options. 
 * Note this code does not support HSE Clocks.
 */
typedef enum {
    CLK_MSI_48MHz,   /*!< Will change speed of MSI source to 48MHz */
    CLK_MSI_4MHz,    /*!< Default source, starts at 4MHz */
    CLK_PLL_80MHz,   /*!< Uses the PLL to multiply MSI frequency to 80MHz */
} sysclock_src_t;

/**
 * Selects and initializes the system clock
 * @param src: System clock to utilize
 */
void select_sysclock(sysclock_src_t src);

/**
 * Delays the system by a given number of milliseconds.
 * This function simply spins the processor during this delay
 * @param delay: length to delay in ms
 */
void delay_ms(uint32_t delay);