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