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

typedef enum {
    MSICLK_4MHz,
    MSICLK_32MHz,
} msiclk_speed_t;

// Local functions
static void enable_msiclk(msiclk_speed_t spd);
static void update_flash_acr(sysclock_src_t src);

/**
 * Selects and initializes the system clock
 * @param src: System clock to utilize
 */
void select_sysclock(sysclock_src_t src) {
    switch (src) {
    case CLK_MSI_32MHz:
        // Enable the MSI clock at 32MHz
        enable_msiclk(MSICLK_32MHz);
        /**
         * Update the flash acr register wait states
         * (see section 3.3.3 of technical reference manual)
         */
        if (current_clk < src) {
            update_flash_acr(src);
        }
        /**
         * Select MSI as the system clock source by clearing the
         * SW bits in RCC_CFGR
         */
        CLEARBITS(RCC->CFGR, RCC_CFGR_SW);
        // Wait for the system clock to set
        while (READBITS(RCC->CFGR, RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_MSI)
            ;
        // System clock is slowing, we need to update flash acr as last step
        if (current_clk > src) {
            update_flash_acr(src);
        }
        break;
    case CLK_PLL_80MHz:
        // First disable the PLL to configure it
        CLEARBITS(RCC->CR, RCC_CR_PLLON);
        // Wait for the PLL to shutdown
        while (READBITS(RCC->CR, RCC_CR_PLLRDY))
            ;
        /**
         * Now that PLL is off, configure it. To get an 80MHz clock we must
         * chain the PLLSAI1 clock to the VCO input frequency, multiply the
         * VCO output frequency, then divide the VCO output frequency to
         * generate PLLCLK
         */
        /* First, we must enable the MSI clock at 4MHz: */
        enable_msiclk(MSICLK_4MHz);
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
        /**
         * This is our fastest clock (cpu speed will always be increasing)
         * so update ACR first
         */
        update_flash_acr(src);
        // Now switch the system clock to the PLL
        CLEARBITS(RCC->CFGR, RCC_CFGR_SW);
        SETBITS(RCC->CFGR, RCC_CFGR_SW_PLL);
        // Wait for the system clock to set
        while (READBITS(RCC->CFGR, RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL)
            ;
        break;
    case CLK_MSI_4MHz:
        // This is the default option, so the cases are combined
    default:
        src = CLK_MSI_4MHz;
        /*
         * Although these steps are not required at boot, this function
         * should be able to switch the clock back the the MSI 4MHz source as
         * well
         */
        // Enable the msi clock running at 4MHz
        enable_msiclk(MSICLK_4MHz);
        /**
         * Select MSI as the system clock source by clearing the
         * SW bits in RCC_CFGR
         */
        CLEARBITS(RCC->CFGR, RCC_CFGR_SW);
        // Wait for the system clock to set
        while (READBITS(RCC->CFGR, RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_MSI)
            ;
        /**
         * This is our slowest clock (cpu speed will always be increasing)
         * so update ACR last
         */
        update_flash_acr(src);
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
    case CLK_MSI_32MHz:
        increment = 32000UL;
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

/**
 * Enables the MSI clock, runnining at the selected speed
 * @param spd: Selected speed for MSI clock.
 */
static void enable_msiclk(msiclk_speed_t spd) {
    // Ensure the clock is enabled
    SETBITS(RCC->CR, RCC_CR_MSION);
    // Wait for the MSI clock to be ready by polling MSIRDY bit
    while (READBITS(RCC->CR, RCC_CR_MSIRDY) == 0)
        ;
    // Now select range in the MSIRANGE register
    CLEARBITS(RCC->CR, RCC_CR_MSIRANGE_Msk);
    switch (spd) {
    case MSICLK_4MHz:
        SETBITS(RCC->CR, RCC_CR_MSIRANGE_6);
        break;
    case MSICLK_32MHz:
        SETBITS(RCC->CR, RCC_CR_MSIRANGE_10);
        break;
    }
    /**
     * Now, switch the selection register for MSI range to MSIRANGE
     * The selection register at startup only allows up to 8MHz
     */
    SETBITS(RCC->CR, RCC_CR_MSIRGSEL);
    // Wait for the MSI clock to be ready by polling MSIRDY bit
    while (READBITS(RCC->CR, RCC_CR_MSIRDY) == 0)
        ;
}

static void update_flash_acr(sysclock_src_t src) {
    /**
     * When the system clock frequency changes, we must update the
     * access control wait state for the flash to match. We assume
     * we are in vcore range 1 (high performance mode)
     */
    CLEARBITS(FLASH->ACR, FLASH_ACR_LATENCY_Msk);
    switch (src) {
    case CLK_MSI_4MHz:
        // 0 wait states. This corresponds to 0b000 (already set)
        // Wait for the wait state to set
        while (READBITS(FLASH->ACR, FLASH_ACR_LATENCY_Msk) !=
               FLASH_ACR_LATENCY_0WS)
            ;
        break;
    case CLK_MSI_32MHz:
        // 1 wait state
        SETBITS(FLASH->ACR, FLASH_ACR_LATENCY_2WS);
        // Wait for the wait state to set
        while (READBITS(FLASH->ACR, FLASH_ACR_LATENCY_Msk) !=
               FLASH_ACR_LATENCY_2WS)
            ;
        break;
    case CLK_PLL_80MHz:
        // 4 wait states
        SETBITS(FLASH->ACR, FLASH_ACR_LATENCY_4WS);
        // Wait for the wait state to set
        while (READBITS(FLASH->ACR, FLASH_ACR_LATENCY_Msk) !=
               FLASH_ACR_LATENCY_4WS)
            ;
        break;
    }
}