/**
 * @file ringbuf.c
 * Implements a simple ring buffer with no dynamic allocation
 */

#include "ringbuf.h"

/**
 * Initializes a ring buffer configuration.
 * This function is required to set the buffer to back the ringbuffer
 * @param buf: Ringbuffer configuration to init
 * @param store: Ringbuffer storage to utilize (may be alloced or static)
 * @param storelen: length of storage buffer
 */
syserr_t buf_init(RingBuf_t *buf, uint8_t *store, uint32_t storelen) {
    buf->buff = store;
    buf->len = storelen;
    buf->read_offset = store;
    buf->write_offset = store;
    buf->buf_end = (uint8_t *)(store + storelen);
    return SYS_OK;
}

/**
 * "Peek" at first character in buffer, without removing it
 * @param buf: Ringbuffer configuration
 * @param data: pointer filled with first character in buffer
 * @return SYS_OK, ERR_NOMEM if buffer is empty
 */
syserr_t buf_peek(RingBuf_t *buf, char *data) {
    // Check if a character is present
    if (buf->size == 0) {
        return ERR_NOMEM;
    }
    // If character is present, copy it to output
    *data = *((char*)buf->read_offset);
    return SYS_OK;
}

/**
 * Read and remove one character from the buffer
 * @param buf: Ringbuffer configuration
 * @param data: pointer filled with first character in buffer
 * @return SYS_OK, ERR_NOMEM if buffer is empty
 */
syserr_t buf_read(RingBuf_t *buf, char *data) {
    if (buf_readblock(buf, (uint8_t*)data, 1) == 0) {
        // No data in buffer to return;
        return ERR_NOMEM;
    }
    // Read succeeded, return success
    return SYS_OK;
}

/**
 * Write one character into the buffer
 * @param buf: Ringbuffer configuration
 * @param data: character to place into buffer
 * @return SYS_OK, ERR_NOMEM if buffer is full
 */
syserr_t buf_write(RingBuf_t *buf, char data) {
    if (buf_writeblock(buf, (uint8_t*)&data, 1) == 0) {
        // No space in buffer to write to
        return ERR_NOMEM;
    }
    // Write succeeded, return success
    return SYS_OK;
}

/**
 * Read (and take) a block of characters from the buffer
 * @param buf: Ringbuffer configuration
 * @param data: pointer to output buffer
 * @param rlen: length of data buffer
 * @return number of characters read into buffer
 */
uint32_t buf_readblock(RingBuf_t *buf, uint8_t *data, uint32_t rlen) {
    uint32_t rdoffset = 0;
    // While output buffer has space and ring buffer has data, copy it
    while (buf->size > 0 && rdoffset < rlen) {
        data[rdoffset] = *(buf->read_offset);
        // Move read offset forwards
        buf->read_offset++;
        // Check if we need to wrap read offset around
        if (buf->read_offset == buf->buf_end) {
            buf->read_offset = buf->buff;
        }
        buf->size--;
        rdoffset++;
    }
    return rdoffset;
}

/**
 * Write a block of characters into the buffer
 * @param buf: Ringbuffer configuration
 * @param data: pointer to input buffer
 * @param wlen: length of data buffer
 * @return number of characters written into buffer
 */
uint32_t buf_writeblock(RingBuf_t *buf, uint8_t *data, uint32_t wlen) {
    uint32_t wroffset = 0;
    // While output buffer has space and ring buffer has data, copy it
    while (buf->size < buf->len && wroffset < wlen) {
        *(buf->write_offset) = data[wroffset];
        // Move write offset forwards
        buf->write_offset++;
        // Check if we need to wrap write offset around
        if (buf->write_offset == buf->buf_end) {
            buf->write_offset = buf->buff;
        }
        buf->size++;
        wroffset++;
    }
    return wroffset;
}

/**
 * Get the length of data present in the buffer
 * @param buf: Ringbuffer configuration
 * @return number of bytes in buffer
 */
uint32_t buf_getsize(RingBuf_t *buf) { return buf->size; }
