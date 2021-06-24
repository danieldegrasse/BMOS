/**
 * @file uart.h
 * Implements UART and LPUART support for STM32L4xxxx
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>

#include <sys/err.h>

/**
 * UART word lengths
 */
typedef enum {
    UART_word_7n1, /*!< 1 start bit, 7 data bits, n stop bits */
    UART_word_8n1, /*!< 1 start bit, 8 data bits, n stop bits */
    UART_word_9n1, /*!< 1 start bit, 9 data bits, n stop bits */
} UART_wordlen_t;

/**
 * UART parity control
 */
typedef enum {
    UART_parity_disabled, /*!< No parity */
    UART_parity_even,     /*!< Even parity bit */
    UART_parity_odd       /*!< Odd parity bit */
} UART_parity_t;

/**
 * UART TX/RX inversion
 */
typedef enum {
    UART_pin_normal,  /*!< UART pins have normal wiring */
    UART_pin_swapped, /*!< UART pins have TX and RX swapped */
} UART_pinswap_t;

/**
 * UART Bit Order
 */
typedef enum {
    UART_lsb_first, /*!< UART least significant bit first */
    UART_msb_first, /*!< UART most significant bit first */
} UART_bitorder_t;

/**
 * UART flow control
 */
typedef enum {
    UART_no_flow,      /*!< No flow control, RTS and CTS not used */
    UART_flow_control, /*!< Flow control is enabled on RTS and CTS pins */
} UART_flow_control_t;

/**
 * UART baud rate selection
 */
typedef enum {
    UART_baud_1200 = 0,
    UART_baud_2400 = 1,
    UART_baud_4800 = 2,
    UART_baud_9600 = 3,
    UART_baud_19200 = 4,
    UART_baud_38400 = 5,
    UART_baud_57600 = 6,
    UART_baud_115200 = 7,
} UART_baud_rate_t;

/**
 * UART peripheral list. See datasheet for pin connections.
 */
typedef enum {
    LPUART1 = 0,
    USART1 = 1,
    USART2 = 2,
    USART3 = 3,
} UART_periph_t;

#define NUM_UARTS 4

/* UART read/write timeout (ms) */
typedef int UART_timeout_t;
#define UART_TIMEOUT_NONE 0 // No timeout
#define UART_TIMEOUT_INF -1 // Infinite timeout

/**
 * UART configuration structure
 */
typedef struct UART_config {
    UART_wordlen_t UART_wordlen;          /*!< UART word length */
    UART_parity_t UART_parity;            /*!< UART parity selection */
    UART_pinswap_t UART_pin_swap;         /*!< UART swap pin rx and tx */
    UART_bitorder_t UART_bit_order;       /*!< UART bit MSB or LSB first */
    UART_flow_control_t UART_flowcontrol; /*!< UART flow control setting */
    UART_baud_rate_t UART_baud_rate;      /*!< UART baud rate */
    UART_timeout_t timeout;               /*!< UART read/write timeout */
} UART_config_t;

typedef void *UART_handle_t;

/**
 * Opens a UART or LPUART device for read/write access
 * @param periph: Identifier of UART to open
 * @param config: UART configuration structure
 * @param err: Set on function error
 * @return NULL on error, or a UART handle to the open peripheral
 */
UART_handle_t UART_open(UART_periph_t periph, UART_config_t *config,
                        syserr_t *err);

/**
 * Reads data from a UART or LPUART device
 * @param handle: UART handle to access
 * @param buf: Buffer to read data into
 * @param len: buffer length
 * @param err: Set on error
 * @return number of bytes read, or -1 on error
 */
int UART_read(UART_handle_t handle, uint8_t *buf, uint32_t len, syserr_t *err);

/**
 * Writes data to a UART or LPUART device
 * @param handle: UART handle to access
 * @param buf: buffer to write data from
 * @param len: buffer length
 * @param err: set on error
 * @return number of bytes written, or -1 on error
 */
int UART_write(UART_handle_t handle, uint8_t *buf, uint32_t len, syserr_t *err);


#endif