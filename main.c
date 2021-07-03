/** @file log_test.c
 * Tests system logging and printf implementations
 * This test, when successful, should log to the defined system console,
 * which will be one of the following depending on the chosen logger:
 * SYSLOG_LPUART1: LPUART1, hooked up to the USB-serial converter on dev board
 * SYSLOG_SEMIHOST: system debugger console (semihosting must be enabled)
 * SYSLOG_SWO: system swo output (swo must be enabled)
 * SYSLOG_DISABLED: logging should not occur
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <config.h>
#include <drivers/clock/clock.h>
#include <util/logging/logging.h>

/**
 * Initializes system
 */
static void system_init() {
    clock_cfg_t clk_cfg = CLOCK_DEFAULT_CONFIG;
    clock_init(&clk_cfg);
}

/**
 * Base RTOS testing point
 */
int main() {
    system_init();
    printf("Hello world\n");
    return SYS_OK;
}