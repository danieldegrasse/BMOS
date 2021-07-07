/**
 * @file clock.c
 * Implements system clock support, including setting the system clock
 * source and delaying by a defined number of milliseconds
 */
#include <stdlib.h>

#include <drivers/device/device.h>
#include <util/bitmask.h>

#include "clock.h"

#define REG_VERIFY_TIMEOUT 10 // How many times to check register before timeout

// Static variables to record current clock frequencies and states
static sysclock_src_t system_clk_src = CLK_MSI; // Default clock is MSI
static uint64_t sysclk_freq = MSI_freq_4MHz;    // Default frequency is 4 MHz
static MSI_Freq_t msi_freq = MSI_freq_4MHz;     // MSI clock defaults to 4 MHz
static uint64_t pll_freq = PLL_freq_disabled;   // PLL is disabled at boot
static uint16_t plln = 0;                       // PLL multiplication factor
static PLLR_div_t pllr = PLLR_2;                // PLL division factor
static HSI16_Freq_t hsi16_freq =
    HSI16_freq_disabled;                          // HSI16 is disabled at boot
static LSI_Freq_t lsi32_freq = LSI_freq_disabled; // LSI is disabled at boot
static uint64_t apb_freq = MSI_freq_4MHz;  // src is sysclock divided by 1
static uint64_t apb1_freq = MSI_freq_4MHz; // src is sysclock divided by 1
static uint64_t apb2_freq = MSI_freq_4MHz; // src is sysclock divided by 1

// Local functions
static syserr_t update_flash_ws(uint64_t new_freq);
static syserr_t msiclk_init(clock_cfg_t *cfg);
static syserr_t pllclk_init(clock_cfg_t *cfg);
static inline syserr_t verify_reg(uint32_t reg, uint32_t msk, uint32_t expect);

/**
 * Initializes device clocks. This function should be called at boot
 * @return SYS_OK on successful configuration, or ERR_BADPARAM if a clock
 * parameter is invalid
 */
syserr_t clock_init(clock_cfg_t *cfg) {
    syserr_t ret;
    uint64_t new_apb_freq, new_sysclock_freq;
    uint32_t SW, apb_scale;
    // Check parameters
    if (cfg == NULL) {
        return ERR_BADPARAM;
    }
    /* ------- Configure the MSI clock --------- */
    /**
     * If the PLL is the system clock and is sourced from MSI, or MSI is the
     * system clock, we must update the flash wait state
     * This is because modifying the MSI clock will change the system clock,
     * which requires changing the flash wait state.
     */
    if (system_clk_src == CLK_MSI ||
        (READBITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC_MSI) ==
             RCC_PLLCFGR_PLLSRC_MSI &&
         system_clk_src == CLK_PLL)) {
        if (READBITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC_MSI) ==
            RCC_PLLCFGR_PLLSRC_MSI) {
            // PLL frequency should be updated
            pll_freq = (cfg->MSI_freq / msi_freq) * pll_freq;
        }
        // Flash wait state must be updated
        // Calculate expected HCLK (apb) frequency
        switch (system_clk_src) {
        case CLK_PLL:
            if (READBITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC_MSI) ==
                RCC_PLLCFGR_PLLSRC_MSI) {
                // PLL frequency should be updated
                // new HCLK frequency derives from PLL, which derives from MSI
                new_apb_freq = (cfg->MSI_freq / msi_freq) * apb_freq;
                // system frequency must also be updated
                sysclk_freq = (cfg->MSI_freq / msi_freq) * sysclk_freq;
            } else {
                // Code is not designed to work with PLL sources that aren't MSI
                return ERR_NOSUPPORT;
            }
            break;
        case CLK_MSI:
            // new MSI frequency will be HCLK
            new_apb_freq = cfg->MSI_freq;
            break;
        case CLK_HSI16:
            // Should not occur
            break;
        }
        if (new_apb_freq > apb_freq) {
            // HCLK is increasing, we must increase wait state first
            ret = update_flash_ws(new_apb_freq);
            if (ret != SYS_OK) {
                return ret;
            }
            // Now update the MSI clock speed
            ret = msiclk_init(cfg);
            if (ret != SYS_OK) {
                return ret;
            }
        } else {
            // HCLK is decreasing, we must decrease wait state last
            ret = msiclk_init(cfg);
            if (ret != SYS_OK) {
                return ret;
            }
            ret = update_flash_ws(new_apb_freq);
            if (ret != SYS_OK) {
                return ret;
            }
        }
    } else {
        // System clock will not be affected. Set MSI independently
        ret = msiclk_init(cfg);
        if (ret != SYS_OK) {
            return ret;
        }
    }
    /* ------------ Configure the PLL clock ------------------ */
    if (READBITS(RCC->CFGR, RCC_CFGR_SWS_PLL) == RCC_CFGR_SWS_PLL) {
        // If user is attempting to modify PLL we must return error
        if (plln != cfg->PLLN_mul || pllr != cfg->PLLR_div || !cfg->PLL_en) {
            // PLL clock cannot be modified, is it is in use by system
            return ERR_BADPARAM;
        }
    } else {
        // Start the PLL clock
        ret = pllclk_init(cfg);
        if (ret != SYS_OK) {
            return ret;
        }
    }
    /* -------- HSI16 Configuration ----------------- */
    if (cfg->HSI16_freq == HSI16_freq_16MHz) {
        // Enable the HSI16 oscillator
        SETBITS(RCC->CR, RCC_CR_HSION);
        while (READBITS(RCC->CR, RCC_CR_HSIRDY) == 0)
            ; // Wait for clock to stabilize
    } else {
        // Disable HSI16
        CLEARBITS(RCC->CR, RCC_CR_HSION);
    }
    // Record new hsi clock
    hsi16_freq = cfg->HSI16_freq;
    /* -------- LSI Configuration ----------------- */
    if (cfg->LSI_freq == LSI_freq_32MHz) {
        // Enable the LSI oscillator
        SETBITS(RCC->CSR, RCC_CSR_LSION);
        while (READBITS(RCC->CSR, RCC_CSR_LSIRDY) == 0)
            ; // Wait for clock to stabilize
    } else {
        // Disable LSI
        CLEARBITS(RCC->CSR, RCC_CSR_LSION);
    }
    // Record new lsi clock
    lsi32_freq = cfg->LSI_freq;
    /* -------- System Clock Configuration ----------- */
    // Calculate new system clock frequency
    switch (cfg->sysclk_src) {
    case CLK_MSI:
        new_sysclock_freq = msi_freq;
        SW = RCC_CFGR_SW_MSI;
        break;
    case CLK_PLL:
        new_sysclock_freq = pll_freq;
        SW = RCC_CFGR_SW_PLL;
        break;
    case CLK_HSI16:
        new_sysclock_freq = hsi16_freq;
        SW = RCC_CFGR_SW_HSI;
        break;
    default:
        return ERR_BADPARAM;
    }
    if (new_sysclock_freq > 80000000UL || new_sysclock_freq == 0) {
        return ERR_BADPARAM;
    }
    // We are changing system clock frequency, so we must change flash ws
    if (new_sysclock_freq > sysclk_freq) {
        // Update flash ws first
        ret = update_flash_ws(new_sysclock_freq);
        if (ret != SYS_OK) {
            return ret;
        }
        // Now set system clock
        MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, SW);
        // Make sure change propagated
        if (verify_reg(RCC->CFGR, RCC_CFGR_SWS, SW << 2) != SYS_OK) {
            return ERR_DEVICE;
        }
    } else {
        // Now set system clock
        MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, SW);
        // Make sure change propagated
        if (verify_reg(RCC->CFGR, RCC_CFGR_SWS, SW << 2) != SYS_OK) {
            return ERR_DEVICE;
        }
        // Update flash ws last
        ret = update_flash_ws(new_sysclock_freq);
        if (ret != SYS_OK) {
            return ret;
        }
    }
    // Record new system clock
    sysclk_freq = new_sysclock_freq;
    system_clk_src = cfg->sysclk_src;
    // Record HCLK frequency
    apb_freq = sysclk_freq;
    /* ------------------ APB1 and APB2 divisors ---------------- */
    switch (cfg->APB1_scale) {
    case APB_scale_div1:
        apb_scale = RCC_CFGR_PPRE1_DIV1;
        apb1_freq = apb_freq;
        break;
    case APB_scale_div2:
        apb_scale = RCC_CFGR_PPRE1_DIV2;
        apb1_freq = apb_freq >> 1;
        break;
    case APB_scale_div4:
        apb_scale = RCC_CFGR_PPRE1_DIV4;
        apb1_freq = apb_freq >> 2;
        break;
    case APB_scale_div8:
        apb_scale = RCC_CFGR_PPRE1_DIV8;
        apb1_freq = apb_freq >> 3;
        break;
    case APB_scale_div16:
        apb_scale = RCC_CFGR_PPRE1_DIV16;
        apb1_freq = apb_freq >> 4;
        break;
    }
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, apb_scale);
    // APB2
    switch (cfg->APB2_scale) {
    case APB_scale_div1:
        apb_scale = RCC_CFGR_PPRE2_DIV1;
        apb2_freq = apb_freq;
        break;
    case APB_scale_div2:
        apb_scale = RCC_CFGR_PPRE2_DIV2;
        apb2_freq = apb_freq >> 1;
        break;
    case APB_scale_div4:
        apb_scale = RCC_CFGR_PPRE2_DIV4;
        apb2_freq = apb_freq >> 2;
        break;
    case APB_scale_div8:
        apb_scale = RCC_CFGR_PPRE2_DIV8;
        apb2_freq = apb_freq >> 3;
        break;
    case APB_scale_div16:
        apb_scale = RCC_CFGR_PPRE1_DIV16;
        apb2_freq = apb_freq >> 4;
        break;
    }
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, apb_scale);
    return SYS_OK;
}

/**
 * Resets all system clocks to known good values.
 * This function should be called before main.
 * After calling this function, the device will be using the MSI clock at
 * 4MHZ as the system clock
 */
void reset_clocks() {
    // RCC register reset values are taken from p.243 of the reference manual
    RCC->CR = 0x63U;
    RCC->CFGR = 0x00U;
    RCC->PLLCFGR = 0x1000U;
    RCC->PLLSAI1CFGR = 0x00U;
    RCC->CIER = 0x00U;
    // Additionally, reset the flash access control register
    FLASH->ACR = 0x600U;
}

/*
 * Returns the system clock, in Hz
 */
uint64_t sysclock_freq() { return sysclk_freq; }

/**
 * Returns the msi clock, in Hz
 * A value of 0 indicates the clock is inactive
 */
uint64_t msiclock_freq() { return msi_freq; }

/**
 * Returns the PLL frequency, in Hz
 * A value of 0 indicates the clock is inactive
 */
uint64_t pllclock_freq() { return pll_freq; }

/**
 * Returns the PCLK1 (APB1) frequency
 */
uint64_t pclk1_freq() { return apb1_freq; }

/**
 * Returns the PCLK2 (APB2) frequency
 */
uint64_t pclk2_freq() { return apb2_freq; }

/**
 * Returns the LSI frequency
 * A value of zero indicates the oscillator is disabled
 */
uint64_t lsi_freq() { return lsi32_freq; }

/**
 * Returns the HSI16 frequency
 * A value of zero indicates the oscillator is disabled
 */
uint64_t hsi_freq() { return hsi16_freq; }

/**
 * Returns the HCLK (APB) frequency
 */
uint64_t hclk_freq() { return apb_freq; }

/**
 * Delays the system by a given number of milliseconds.
 * This function simply spins the processor during this delay
 * If possible, use task_delay() instead of this function
 * @param delay: length to delay in ms
 */
void blocking_delay_ms(uint32_t delay) {
    /**
     * Approximate the delay by simply subtractring from a counter.
     * All instructions here should take 1 clock cycle
     * one iteration (from the loop label back up to it) takes:
     * 1 cycle for sub
     * 1 cycle for mov
     * innerloop takes 2 * 4096 = 8192 cycles
     * 1 cycle for cmp
     * 1 cycle for bne
     * total of 8196 cycles
     * divide by 8,196,000, because we want a target in milliseconds
     */
    uint32_t target = (sysclk_freq / 8196000UL) * delay;
    // use assembly here to ensure known number of instructions
    asm volatile("loop:\n"
                 "sub %0, #1\n"
                 "mov r1, #4095\n"
                 "innerloop:\n"
                 "subs r1 , #1\n"  // Runs 4096 times
                 "bne innerloop\n" // Runs 4096 times
                 "cmp %0, #0\n"
                 "bge loop\n"
                 :
                 : "r"(target)
                 : "r1");
}

/**
 * Starts the PLL clock, at the selected frequency
 * @param cfg: clock configuration structure
 */
static syserr_t pllclk_init(clock_cfg_t *cfg) {
    uint32_t PLLR;
    if (!cfg->PLL_en) {
        // Disable the PLL
        CLEARBITS(RCC->CR, RCC_CR_PLLON);
        CLEARBITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLREN);
        pll_freq = PLL_freq_disabled;
        return SYS_OK;
    }
    // First disable the PLL to configure it
    CLEARBITS(RCC->CR, RCC_CR_PLLON);
    // Wait for the PLL to shutdown
    while (READBITS(RCC->CR, RCC_CR_PLLRDY))
        ;
    /**
     * Now that PLL is off, configure it. we must
     * chain the PLLSAI1 clock to the VCO input frequency, multiply the
     * VCO output frequency, then divide the VCO output frequency to
     * generate PLLCLK
     */
    /* Now we can set the PLLSAI1 clock to use the MSI clock */
    MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, RCC_PLLCFGR_PLLSRC_MSI);
    // Set the VCO input frequency division to 1.
    CLEARBITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLM);
    // Set the VCO frequency multiplier
    if (cfg->PLLN_mul > 7 && cfg->PLLN_mul < 87) {
        MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLN,
                   cfg->PLLN_mul << RCC_PLLCFGR_PLLN_Pos);
    } else {
        // PLLN is out of range
        return ERR_BADPARAM;
    }
    // Set PLLR to divide the VCO output frequency
    switch (cfg->PLLR_div) {
    case PLLR_2:
        PLLR = 0;
        break;
    case PLLR_4:
        PLLR = RCC_PLLCFGR_PLLR_0;
        break;
    case PLLR_6:
        PLLR = RCC_PLLCFGR_PLLR_1;
        break;
    case PLLR_8:
        PLLR = RCC_PLLCFGR_PLLR_1 | RCC_PLLCFGR_PLLR_0;
        break;
    }
    MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLR, PLLR);
    /* Reenable the PLL */
    SETBITS(RCC->CR, RCC_CR_PLLON);
    // Wait for the PLL to stabilize
    while (READBITS(RCC->CR, RCC_CR_PLLRDY) == 0)
        ;
    // Enable the PLL output
    SETBITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLREN);
    // Update the PLL frequency
    pll_freq = (cfg->PLLN_mul / cfg->PLLR_div) * msi_freq;
    // Update saved plln and pllr
    pllr = cfg->PLLR_div;
    plln = cfg->PLLN_mul;
    return SYS_OK;
}

/**
 * Starts the MSI clock, at the selected frequency
 * @param cfg: clock configuration structure
 */
static syserr_t msiclk_init(clock_cfg_t *cfg) {
    uint32_t range;
    // Update the MSI clock frequency
    msi_freq = cfg->MSI_freq;
    // Select the MSI frequency range
    switch (cfg->MSI_freq) {
    case MSI_freq_disabled:
        // Disable the MSI clock and return
        CLEARBITS(RCC->CR, RCC_CR_MSION);
        return SYS_OK;
        break;
    case MSI_freq_100kHz:
        range = RCC_CR_MSIRANGE_0;
        break;
    case MSI_freq_200kHz:
        range = RCC_CR_MSIRANGE_1;
        break;
    case MSI_freq_400kHz:
        range = RCC_CR_MSIRANGE_2;
        break;
    case MSI_freq_800kHz:
        range = RCC_CR_MSIRANGE_3;
        break;
    case MSI_freq_1MHz:
        range = RCC_CR_MSIRANGE_4;
        break;
    case MSI_freq_2MHz:
        range = RCC_CR_MSIRANGE_5;
        break;
    case MSI_freq_4MHz:
        range = RCC_CR_MSIRANGE_6;
        break;
    case MSI_freq_8MHz:
        range = RCC_CR_MSIRANGE_7;
        break;
    case MSI_freq_16MHz:
        range = RCC_CR_MSIRANGE_8;
        break;
    case MSI_freq_24MHz:
        range = RCC_CR_MSIRANGE_9;
        break;
    case MSI_freq_32MHz:
        range = RCC_CR_MSIRANGE_10;
        break;
    case MSI_freq_48MHz:
        range = RCC_CR_MSIRANGE_11;
        break;
    }
    // Ensure the clock is enabled
    SETBITS(RCC->CR, RCC_CR_MSION);
    // Wait for the MSI clock to be ready by polling MSIRDY bit
    while (READBITS(RCC->CR, RCC_CR_MSIRDY) == 0)
        ;
    /**
     * Now, switch the selection register for MSI range to MSIRANGE
     * The selection register at startup only allows up to 8MHz
     */
    SETBITS(RCC->CR, RCC_CR_MSIRGSEL);
    MODIFY_REG(RCC->CR, RCC_CR_MSIRANGE_Msk, range);
    return SYS_OK;
}

/**
 * Updates flash wait state to the correct value for the new provided
 * frequency
 * @param new_freq: new HCLK frequency in Hz
 */
static syserr_t update_flash_ws(uint64_t new_freq) {
    uint32_t vcore_range;
    uint32_t latency;
    /**
     * When the system clock frequency changes, we must update the
     * access control wait state for the flash to match.
     */
    // Find the VCORE range
    if (READBITS(RCC->APB1ENR1, RCC_APB1ENR1_PWREN) == 0) {
        // Power up the PWR peripheral to read VCORE bit
        SETBITS(RCC->APB1ENR1, RCC_APB1ENR1_PWREN);
        vcore_range = READBITS(PWR->CR1, PWR_CR1_VOS_Msk);
        // Turn PWN peripheral off again
        CLEARBITS(RCC->APB1ENR1, RCC_APB1ENR1_PWREN);
    } else {
        // PWR peripheral is on, just read bits
        vcore_range = READBITS(PWR->CR1, PWR_CR1_VOS_Msk);
    }
    /**
     * The relation between VCORE ranges and wait states is given on p.79 of
     * technical ref manual
     */
    if (vcore_range == PWR_CR1_VOS_0) {
        // We are in VCORE range 1
        if (new_freq > 64000000UL) {
            // >64MHz, WS 4
            latency = FLASH_ACR_LATENCY_4WS;
        } else if (new_freq > 48000000UL) {
            // >48MHz, WS 3
            latency = FLASH_ACR_LATENCY_3WS;
        } else if (new_freq > 32000000UL) {
            // >32MHz, WS 2
            latency = FLASH_ACR_LATENCY_2WS;
        } else if (new_freq > 16000000UL) {
            // >16MHz, WS1
            latency = FLASH_ACR_LATENCY_1WS;
        } else {
            // WS0
            latency = FLASH_ACR_LATENCY_0WS;
        }
    } else {
        // We are in VCORE range 2
        if (new_freq > 18000000UL) {
            // >18MHz, WS 3
            latency = FLASH_ACR_LATENCY_3WS;
        } else if (new_freq > 12000000UL) {
            // >12MHz, WS 2
            latency = FLASH_ACR_LATENCY_2WS;
        } else if (new_freq > 6000000UL) {
            // >6MHz, WS1
            latency = FLASH_ACR_LATENCY_1WS;
        } else {
            // WS0
            latency = FLASH_ACR_LATENCY_0WS;
        }
    }
    volatile FLASH_TypeDef *flash = FLASH;
    (void)flash;
    /* Set the new latency in the flash ACR register */
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY_Msk, latency);
    // Verify that the new latency was set
    if (verify_reg(FLASH->ACR, FLASH_ACR_LATENCY_Msk, latency) != SYS_OK) {
        return ERR_DEVICE;
    } else {
        return SYS_OK;
    }
}

/**
 * Verifies a register's contents, with a timeout
 * @param reg: Register value to check
 * @param msk: Mask to apply to register
 * @param expect: expected value for register
 */
static inline syserr_t verify_reg(uint32_t reg, uint32_t msk, uint32_t expect) {
    int timeout = REG_VERIFY_TIMEOUT;
    while (timeout--) {
        if (READBITS(reg, msk) == expect) {
            return SYS_OK;
        }
    }
    // Timeout expired
    return ERR_DEVICE;
}