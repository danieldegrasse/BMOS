/**
 * @file semihost.c
 * Implements support for semihosting functionality
 */

#include <config.h>

#include "semihost.h"

static char semihost_buf[SYSLOG_BUFSIZE];
static char *write_offset = semihost_buf;

/**
 * Writes a single :character to the semihost output.
 * This character will be sent immediately.
 * @param c character to write
 */
void semihost_writechar(char c) {
    char *ptr = &c;
    /**
     * Ensure a point to c is in r1, then call bkpt instruction with
     * semihosting immediate. Set r0 to 0x03 to indicate a WRITEC operation
     */
    asm("mov r0, #0x03\n"
        "mov r1, %0\n"
        "bkpt 0xAB\n"
        :
        : "r"(ptr)
        : "r0", "r1");
}

/**
 * Writes a null-terminated string to the semihost output.
 * This string will be send immediately. User MUST ensure that string
 * is null-terminated
 * @param str: string to write
 */
void semihost_writestr(char *str) {
    /**
     * Ensure str is in r1, then call bkpt instruction with
     * semihosting immediate. Set r0 to 0x04 to indicate a WRITE0 operation
     */
    asm("mov r0, #0x04\n"
        "mov r1, %0\n"
        "bkpt 0xAB\n"
        :
        : "r"(str)
        : "r0", "r1");
}

/**
 * Writes a buffer to the semihost output. This buffer will be sent
 * asynchronously, and does not have to be null terminated.
 * Null characters in this buffer will be skipped.
 * @param buf: buffer to write
 * @param len: length to write
 */
void semihost_writebuf(char *buf, int len) {
    int remaining = len;
    char *buf_max = (SYSLOG_BUFSIZE - 1) + semihost_buf;
    while (remaining--) {
        if (*buf == '\0') {
            // We do not support writing null characters to stream
            continue;
        }
        // Write data while there is data to write
        *write_offset++ = *buf++;
        // If buffer is full, flush it
        if (write_offset == buf_max) {
            semihost_flush();
        }
    }
}

/**
 * Forces the semihost output to flush to the debugger
 */
void semihost_flush() {
    // Null terminate the semihost buffer
    *write_offset = '\0';
    write_offset++;
    // Write the buffer
    semihost_writestr(semihost_buf);
    // Reset buffer write index
    write_offset = semihost_buf;
}