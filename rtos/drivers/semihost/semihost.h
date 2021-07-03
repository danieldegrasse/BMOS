/**
 * @file semihost.h
 * Implements support for semihosting functionality
 */
#ifndef SEMIHOST_H
#define SEMIHOST_H

/**
 * Writes a single character to the semihost output.
 * This character will be sent immediately.
 * @param c character to write
 */
void semihost_writechar(char c);

/**
 * Writes a null-terminated string to the semihost output.
 * This string will be send immediately. User MUST ensure that string
 * is null-terminated
 * @param str: string to write
 */
void semihost_writestr(char* str);

/**
 * Writes a buffer to the semihost output. This buffer will be sent
 * asynchronously, and does not have to be null terminated.
 * Null characters in this buffer will be skipped.
 * @param buf: buffer to write
 * @param len: length to write
 */
void semihost_writebuf(char *buf, int len);

/**
 * Forces the semihost output to flush to the debugger
 */
void semihost_flush();

#endif