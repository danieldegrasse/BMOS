/**
 * @file uart.c
 * Implements UART and LPUART support for STM32L4xxxx
 */

#include "uart.h"

/**
 * UART device state
 */
typedef enum {
    UART_closed = 0,
    UART_open = 1,
} UART_state_t;

/**
 * Configuration structure for UART devices
 */
typedef struct {
   UART_config_t cfg;
   UART_state_t state;
   
} UART_periph_status_t;