/**
 * @file swo.c
 * Implements support for single wire output protocol
 */
#include <clock/clock.h>
#include <device/device.h>
#include <gpio/gpio.h>
#include <stdbool.h>
#include <stdint.h>
#include <util/util.h>

#include "swo.h"

#define TPI_SPPR_TXMODE_PARALLEL 0UL
#define TPI_SPPR_TXMODE_MANCHESTER 1UL
#define TPI_SPPR_TXMODE_NRZ 2UL


// ITM lock register. Used for unlocking write access to ITM registers
#define ITM_LOCK_REG (*((volatile uint32_t *)0xE0000FB0))
#define ITM_ACCESS_MAGIC 0xC5ACCE55
#define TCR_TRACEBUS_ID 0x1 << ITM_TCR_TraceBusID_Pos

/**
 * Initializes SWO output with the defined frequency,
 * using NRZ/UART encoding. See p.1570 of technical ref manual
 * @param freq: frequency for SWO output
 * @return SYS_OK on success, or error value on failure
 */
syserr_t SWO_init(int freq) {
    GPIO_config_t swo_pinconf = GPIO_DEFAULT_CONFIG;
    // int prescaler;
    // Enable Trace I/O in asynchronous mode
    // Debug off in sleep modes, TRACE_MODE=0, TRACE_IOEN=1
    DBGMCU->CR = DBGMCU_CR_TRACE_IOEN;
    // Write the magic value to ITM lock registers to allow configuration access
    ITM_LOCK_REG = ITM_ACCESS_MAGIC;
    //  // Enable instrumentation trace macrocell (ITM)
    //  SETBITS(CoreDebug->DEMCR, CoreDebug_DEMCR_TRCENA_Msk);
    //  // Set SWO to use NRZ encoding
    //  MODIFY_REG(TPI->SPPR, TPI_SPPR_TXMODE_Msk, TPI_SPPR_TXMODE_NRZ);
    //  // Calculate SWO prescaler. SWO sources from HCLK.
    //  // SWO output clock = HCLK / (SWOSCALAR + 1)
    //  prescaler = (hclk_freq() - freq) / freq;
    //  if (prescaler > TPI_ACPR_PRESCALER_Msk || prescaler < 0) {
    //      return ERR_NOSUPPORT; // Cannot scale swo frequency to this range
    //  }
    //  // Set prescaler
    //  TPI->ACPR = TPI_ACPR_PRESCALER_Msk & prescaler;
    /**
     * Enable SWO asynchronous clocking
     * We also write 0x01 to the TraceBusID bits to give our stream an ID
     */
    SETBITS(ITM->TCR, TCR_TRACEBUS_ID | ITM_TCR_TSENA_Msk |
                          ITM_TCR_SYNCENA_Msk | ITM_TCR_ITMENA_Msk);
    // Set privileged access for ITM ports 0-7
    SETBITS(ITM->TPR, 0x01);
    // Enable stimulus port 0 for SWO output
    SETBITS(ITM->TER, 0x01);
    /**
     * Initialize pin PB3 for use with SWO
     */
    swo_pinconf.alternate_func = GPIO_af0;
    swo_pinconf.mode = GPIO_mode_afunc;
    swo_pinconf.mode = GPIO_speed_vhigh;
    GPIO_config(GPIO_PORT_B, GPIO_PIN_3, &swo_pinconf);
    return SYS_OK;
}

/**
 * Writes a single character to the SWO output.
 * This character will be sent immediately.
 * @param c character to write
 * @return SYS_OK on success, or SYS_ERR on error
 */
syserr_t SWO_writechar(char c) {
    if ((READBITS(ITM->TCR, ITM_TCR_ITMENA_Msk) != 0UL) && /* ITM enabled */
        (READBITS(ITM->TER, 1UL) != 0UL)) /* ITM Port #0 enabled */
    {
        // Wait for SWO port to be available
        while (ITM->PORT[0].u32 == 0) {
            // Spin
        }
        ITM->PORT[0U].u8 = c;
    }
    return SYS_OK;
}

/**
 * Writes a buffer to the SWO output.
 * @param buf: buffer to write
 * @param len: length to write
 * @return SYS_OK on success, or SYS_ERR on error
 */
syserr_t SWO_writebuf(char *buf, int len) {
    syserr_t ret;
    while (len--) {
        ret = SWO_writechar(*buf);
        if (ret != SYS_OK) {
            return ret;
        }
        // Advance buffer
        buf++;
    }
    return SYS_OK;
}

/**
 * Shuts down SWO, resetting debugging registers
 */
void SWO_close() {
    // Disable stimulus port 0, and ITM
    CLEARBITS(ITM->TER, 0x01);
    CLEARBITS(ITM->TCR, ITM_TCR_SWOENA_Msk | ITM_TCR_ITMENA_Msk);
    // Disable ITM entirely
    CLEARBITS(CoreDebug->DEMCR, CoreDebug_DEMCR_TRCENA_Msk);
}
