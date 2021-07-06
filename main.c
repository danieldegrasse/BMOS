#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <drivers/clock/clock.h>
#include <sys/task/task.h>
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
    LOG_D(__FILE__, "Starting RTOS");
    rtos_start();
    return SYS_OK;
}