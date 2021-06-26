/**
 * @file clock.c
 * Implements system clock support, including setting the system clock
 * source and delaying by a defined number of milliseconds
 */

#include <device/device.h>
#include <util/util.h>

#include "clock.h"

// Records the current clock source
static sysclock_src_t current_clk = CLK_MSI_4MHz;

/**
 * Selects and initializes the system clock
 * @param src: System clock to utilize
 */
void select_sysclock(sysclock_src_t src) {
    switch (src) {
    case CLK_MSI_48MHz:
        // Ensure the clock is enabled
        SETBITS(RCC->CR, RCC_CR_MSION);
        // Wait for the MSI clock to be ready by polling MSIRDY bit
        while (READBITS(RCC->CR, RCC_CR_MSIRDY)) {
        }
        /**
         * Now, switch the selection register for MSI range to MSIRANGE
         * The selection register at startup only allows up to 8MHz
         */
        SETBITS(RCC->CR, RCC_CR_MSIRGSEL);
        // Now select 48MHz in the MSIRANGE register
        SETBITS(RCC->CR, RCC_CR_MSIRANGE_11);
        /**
         * Select MSI as the system clock source by clearing the
         * SW bits in RCC_CFGR
         */
        CLEARBITS(RCC->CFGR, RCC_CFGR_SW);
        break;
    case CLK_PLL_80MHz:
        // First disable the PLL to configure it
        CLEARBITS(RCC->CR, RCC_CR_PLLON);
        // Wait for the PLL to shutdown
        while (READBITS(RCC->CR, RCC_CR_PLLRDY)) {
        }
        /**
         * Now that PLL is off, configure it. To get an 80MHz clock we must
         * chain the PLLSAI1 clock to the VCO input frequency, multiply the
         * VCO output frequency, then divide the VCO output frequency to
         * generate PLLCLK
         */
        /* First, we must enable the MSI clock at 4MHz: */
        // Ensure the clock is enabled
        SETBITS(RCC->CR, RCC_CR_MSION);
        // Wait for the MSI clock to be ready by polling MSIRDY bit
        while (READBITS(RCC->CR, RCC_CR_MSIRDY)) {
        }
        /**
         * Now, switch the selection register for MSI range to MSIRANGE
         * The selection register at startup only allows up to 8MHz
         */
        SETBITS(RCC->CR, RCC_CR_MSIRGSEL);
        // Now select 4MHz in the MSIRANGE register
        SETBITS(RCC->CR, RCC_CR_MSIRANGE_6);
        /* Now we can set the PLLSAI1 clock to use the MSI clock */
        CLEARBITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC);
        SETBITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC_MSI);
        /* Set the VCO input frequency division to 1. VCO input will be 4MHz */
        CLEARBITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLM);
        /* Set the VCO frequency multiplier to be 40, so VCO output is 160MHz */
        CLEARBITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLN_5 | RCC_PLLCFGR_PLLN_3);
        /* Ensure the division factor for PLLCLK is 2, giving 80MHz */
        CLEARBITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLR);
        /* Enable PLLCLK. It does not appear PLLSAI1 needs to be enabled */
        SETBITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLREN);
        /* Reenable the PLL */
        SETBITS(RCC->CR, RCC_CR_PLLON);
        // Wait for the PLL to stabilize
        while (READBITS(RCC->CR, RCC_CR_PLLRDY) == 0) {
        }
        // Now switch the system clock to the PLL
        CLEARBITS(RCC->CFGR, RCC_CFGR_SW);
        SETBITS(RCC->CFGR, RCC_CFGR_SW_PLL);
        break;
    case CLK_MSI_4MHz:
        // This is the default option, so the cases are combined
    default:
        /*
         * Although these steps are not required at boot, this function
         * should be able to switch the clock back the the MSI 4MHz source as
         * well
         */
        // Ensure the clock is enabled
        SETBITS(RCC->CR, RCC_CR_MSION);
        src = CLK_MSI_4MHz;
        // Wait for the MSI clock to be ready by polling MSIRDY bit
        while (READBITS(RCC->CR, RCC_CR_MSIRDY)) {
        }
        /**
         * Now, switch the selection register for MSI range to MSIRANGE
         * The selection register at startup only allows up to 8MHz
         */
        SETBITS(RCC->CR, RCC_CR_MSIRGSEL);
        // Now select 4MHz in the MSIRANGE register
        SETBITS(RCC->CR, RCC_CR_MSIRANGE_6);
        /**
         * Select MSI as the system clock source by clearing the
         * SW bits in RCC_CFGR
         */
        CLEARBITS(RCC->CFGR, RCC_CFGR_SW);
        break;
    }
    // Record the new clock speed
    current_clk = src;
}

/**
 * Delays the system by a given number of milliseconds.
 * This function simply spins the processor during this delay
 * @param delay: length to delay in ms
 */
void delay_ms(uint32_t delay) {
    unsigned long target, increment;
    switch (current_clk) {
    case CLK_MSI_48MHz:
        increment = 48000UL;
        break;
    case CLK_MSI_4MHz:
        increment = 4000UL;
        break;
    case CLK_PLL_80MHz:
        increment = 80000UL;
        break;
    default:
        // Should not occur, but in this case just assume default 4MHz CLK
        increment = 4000UL;
        break;
    }
    target = increment * delay;
    /**
     * To approximate the correct delay well, we will use a for loop with a
     * large increment to minimize the impact of the ADD instruction
     */
    for (unsigned long i = 0UL; i < target; i += increment) {
        // Spin here
    }
}