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

static void rtos_task1(void *unused) {
    const char *TAG = "Rtos_Task1";
    int i = 10;
    LOG_D(TAG, "Task 1 starting");
    while (1) {
        delay_ms(500);
        LOG_D(TAG, "Task 1 running");
        i--;
        if (i == 0) {
            LOG_D(TAG, "Task 1 yielding");
            i = 10;
            task_yield();
        }
    }
}

/**
 * Base RTOS testing point
 */
int main() {
    task_handle_t task1;
    task_config_t task1cfg = DEFAULT_TASK_CONFIG;

    system_init();

    task1cfg.task_name = "Task1";
    task1 = task_create(rtos_task1, NULL, &task1cfg);
    if (!task1) {
        LOG_E(__FILE__, "Failed to create task 1\n");
        return ERR_FAIL;
    }
    LOG_D(__FILE__, "Starting RTOS");
    rtos_start();
    return SYS_OK;
}