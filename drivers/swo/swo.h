/**
 * @file swo.h
 * Implements support for single wire output protocol
 */
#ifndef SWO_H
#define SWO_H

#include <sys/err.h>

/**
 * Initializes SWO output with the defined frequency
 * @param freq: frequency for SWO output
 * @return SYS_OK on success, or error value on failure
 */
//syserr_t SWO_init(int freq);

/**
 * Writes a single character to the SWO output.
 * This character will be sent immediately.
 * @param c character to write
 * @return SYS_OK on success, or SYS_ERR on error
 */
syserr_t SWO_writechar(char c);

/**
 * Writes a buffer to the SWO output. This buffer will be sent
 * asynchronously, and does not have to be null terminated.
 * Null characters in this buffer will be skipped.
 * @param buf: buffer to write
 * @param len: length to write
 * @return SYS_OK on success, or SYS_ERR on error
 */
syserr_t SWO_writebuf(char *buf, int len);

/**
 * Shuts down SWO, resetting debugging registers
 */
//void SWO_close();

#endif