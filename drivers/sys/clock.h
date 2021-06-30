/**
 * @file clock.h
 * Implements system clock support, including setting the system clock
 * source and delaying by a defined number of milliseconds
 */
#ifndef CLOCK_H
#define CLOCK_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/err.h>

/**
 * HSI Clock Frequency options
 */
typedef enum {
    HSI16_freq_disabled = 0UL,     /*!< Clock is disabled */
    HSI16_freq_16MHz = 16000000UL, /*!< Clock is at 16 MHz */
} HSI16_Freq_t;

typedef enum {
    MSI_freq_disabled = 0UL,
    MSI_freq_100kHz = 100000UL,
    MSI_freq_200kHz = 200000UL,
    MSI_freq_400kHz = 400000UL,
    MSI_freq_800kHz = 800000UL,
    MSI_freq_1MHz = 1000000UL,
    MSI_freq_2MHz = 2000000UL,
    MSI_freq_4MHz = 4000000UL,
    MSI_freq_8MHz = 8000000UL,
    MSI_freq_16MHz = 16000000UL,
    MSI_freq_24MHz = 24000000UL,
    MSI_freq_32MHz = 32000000UL,
    MSI_freq_48MHz = 48000000UL,
} MSI_Freq_t;

typedef enum {
    LSI_freq_32MHz,
    LSI_freq_disabled,
} LSI_Freq_t;

#define PLL_freq_disabled 0UL

/**
 * PLL frequency scaling options.
 * PLL frequency will be MSI frequency * (PLLN/PLLR)
 */
typedef enum {
    PLLR_2 = 2,
    PLLR_4 = 4,
    PLLR_6 = 6,
    PLLR_8 = 8,
} PLLR_div_t;

/**
 * System clock source options
 * Note this code does not support HSE Clocks.
 */
typedef enum {
    CLK_MSI,
    CLK_PLL,
    CLK_HSI16,
} sysclock_src_t;

typedef enum {
    APB_scale_div1 = 0,  /*!< APB is system clock */
    APB_scale_div2 = 1,  /*!< APB is system clock divided by 2 */
    APB_scale_div4 = 2,  /*!< APB is system clock divided by 4 */
    APB_scale_div8 = 3,  /*!< APB is system clock divided by 8 */
    APB_scale_div16 = 4, /*!< APB is system clock divided by 16 */
} APB_scale_t;

/**
 * Clock configuration structure
 */
typedef struct clock_cfg {
    HSI16_Freq_t HSI16_freq; /*!< Frequency of HSI oscillator (only 16 MHz)*/
    MSI_Freq_t MSI_freq;     /*!< Frequency of MSI oscillator (0.1-48 MHz) */
    LSI_Freq_t LSI_freq;     /*!< Frequency of LSI oscillator (only 32 MHz) */
    bool PLL_en;             /*!< Is PLL output enabled */
    /*! Division factor of PLL [f_PLL=f_MSI * (PLLN_mul/PLLR_div) */
    PLLR_div_t PLLR_div;
    /*! Multiplication factor of PLL [f_PLL=f_MSI * (PLLN_mul/PLLR_div) */
    uint16_t PLLN_mul;
    APB_scale_t APB1_scale;    /*!< APB1 prescaler (divides system clock) */
    APB_scale_t APB2_scale;    /*!< APB2 prescaler (divides system clock) */
    sysclock_src_t sysclk_src; /*!< System clock source */
} clock_cfg_t;

/**
 * Default clock config. No division on APB clocks,
 * PLL is system clock at 80 MHz
 */
#define CLOCK_DEFAULT_CONFIG                                                   \
    {                                                                          \
        .HSI16_freq = HSI16_freq_disabled, .MSI_freq = MSI_freq_4MHz,          \
        .LSI_freq = LSI_freq_disabled, .PLL_en = true, .PLLR_div = PLLR_2,     \
        .PLLN_mul = 40, .APB1_scale = APB_scale_div1,                          \
        .APB2_scale = APB_scale_div1, .sysclk_src = CLK_PLL                    \
    }

/**
 * Initializes device clocks. This function should be called at boot
 * @return SYS_OK on successful configuration, or ERR_BADPARAM if a clock
 * parameter is invalid
 */
syserr_t clock_init(clock_cfg_t *cfg);

/**
 * Returns the system clock, in Hz
 */
uint64_t sysclock_freq();

/**
 * Returns the msi clock, in Hz
 * A value of 0 indicates the clock is inactive
 */
uint64_t msiclock_freq();

/**
 * Returns the PLL frequency, in Hz
 * A value of 0 indicates the clock in inactive
 */
uint64_t pllclock_freq();

/**
 * Returns the PCLK1 (APB1) frequency
 */
uint64_t pclk1_freq();

/**
 * Returns the PCLK2 (APB2) frequency
 */
uint64_t pclk2_freq();

/**
 * Returns the LSI frequency
 * A value of zero indicates the oscillator is disabled
 */
uint64_t lsi_freq();

/**
 * Returns the HSI16 frequency
 * A value of zero indicates the oscillator is disabled
 */
uint64_t hsi_freq();

/**
 * Delays the system by a given number of milliseconds.
 * This function simply spins the processor during this delay
 * @param delay: length to delay in ms
 */
void delay_ms(uint32_t delay);

/**
 * Resets all system clocks to known good values.
 * This function should be called before main.
 * After calling this function, the device will be using the MSI clock at
 * 4MHZ as the system clock
 */
void reset_clocks();

#endif