/**
 * @file task_test.c
 * Test RTOS task creation, switching, and destruction
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <drivers/clock/clock.h>
#include <sys/task/task.h>
#include <util/logging/logging.h>

static void rtos_task1(void *unused);
static void rtos_task2(void *arg);
static void rtos_task3(void *unused);
static void rtos_task4(void *unused);
static char t3stack[2048];
static char t5stack[128];
static task_handle_t task3;

/**
 * Initializes system
 */
static void system_init() {
    clock_cfg_t clk_cfg = CLOCK_DEFAULT_CONFIG;
    clock_init(&clk_cfg);
}

/**
 * Task 1 entry point. Creates tasks and exits.
 * @param unused: Unused arg
 */
static void rtos_task1(void *unused) {
    task_config_t t3cfg = DEFAULT_TASK_CONFIG;
    task_config_t t4cfg = DEFAULT_TASK_CONFIG;
    const char *TAG = "Rtos_Task1";
    LOG_D(TAG, "Task 1: Create task 3 and 4. Starting");
    // Create task 3
    t3cfg.task_stacksize = 2048;
    t3cfg.task_stack = t3stack;
    t3cfg.task_name = "Task3";
    LOG_D(TAG, "Task 1 creating task 3");
    task3 = task_create(rtos_task3, NULL, &t3cfg);
    if (task3 == NULL) {
        LOG_E(TAG, "Could not create task 3");
    }
    // Delay, then create task 4. This task will kill task 3
    task_delay(1000);
    LOG_D(TAG, "Task 1 running. Will create task 4");
    t4cfg.task_name = "Task4";
    t4cfg.task_priority = DEFAULT_PRIORITY + 1;
    if (task_create(rtos_task4, task3, &t4cfg) == NULL) {
        LOG_E(TAG, "Could not create task 4");
    }
    return;
}

/**
 * Task 2 entry point. Will run, and periodically yield execution.
 * The task will exit of its own accord
 * @param arg: String passed at task creation
 */
static void rtos_task2(void *arg) {
    const char *TAG = "Rtos_Task2";
    int i = 20;
    LOG_D(TAG, "Task 2 starting. Argument %s", (char *)arg);
    LOG_D(TAG, "Task 2 will yield, and will exit independently");
    while (i) {
        LOG_D(TAG, "Task 2 running");
        i--;
        task_delay(500);
        if (i % 5 == 0) {
            // Should yield a total of 4 times
            LOG_D(TAG, "Task 2 yielding");
            task_yield();
        }
    }
    // Task runtime has expired. Exit.
    return;
}

/**
 * Task 3 entry point. Will attempt to monopolize CPU time. Task 4 should
 * preempt and destroy it if preemption is enabled
 * @param unused: unused
 */
static void rtos_task3(void *unused) {
    const char *TAG = "Rtos_Task3";
    LOG_D(TAG, "Task 3: Holding CPU time");
    while (1) {
        // Delay_ms does NOT yield
        blocking_delay_ms(500);
        LOG_D(TAG, "Task 3 running");
    }
}

/**
 * Task 4 entry point. Will delay, then destroy task 3
 * @param arg: Task 3 handle
 */
static void rtos_task4(void *arg) {
    const char *TAG = "Rtos_Task4";
    LOG_D(TAG, "Task 4 starting. Dropping into delay, then killing task 3");
    task_delay(2000);
    LOG_D(TAG, "Task 4 destroying task 3");
    task_destroy((task_handle_t)arg);
    LOG_D(TAG, "Task 4 exiting");
}

/**
 * Task 5 entry point. Trys to overflow its stack, than yields
 */
static void rtos_task5(void *arg) {
    const char *TAG = "Rtos_Task5";
    LOG_I(TAG, "Overflowing the stack");
    asm volatile("mov r0, #64\n"   // 64 bytes in stack
                 "loop_%=:\n"      // loop entry
                 "push {r0-r12}\n" // push 8 registers to stack
                 "subs r0, #8\n"
                 "it ne\n"
                 "bne loop_%=\n" // Break to loop
                 ::);
    // 64 bytes have been pushed to stack. It should be overflowed. Yield
    /**
     * Hope the stack padding saves us because there's no
     * space for this function call
     */
    task_yield();
    LOG_E(TAG, "Rtos task 5 did not exit after stack overflow");
}

/**
 * Testing entry point. Tests task creation, switching, and destruction
 */
int main() {
    task_handle_t task1;
    task_config_t task1cfg = DEFAULT_TASK_CONFIG;
    task_handle_t task2;
    task_config_t task2cfg = DEFAULT_TASK_CONFIG;
    task_config_t task5cfg = DEFAULT_TASK_CONFIG;
    char *arg = "Hello";

    system_init();

    /**
     * Task 1 has a high priority and spawns task 3 and 4
     */
    task1cfg.task_name = "Task1";
    task1cfg.task_priority = DEFAULT_PRIORITY + 1;
    task1 = task_create(rtos_task1, NULL, &task1cfg);
    if (!task1) {
        LOG_E(__FILE__, "Failed to create task 1");
        return ERR_FAIL;
    }
    /**
     * Task 2 has a default priority and will exit of its own accord
     * It also will periodically yield to allow another task to run.
     */
    task2cfg.task_name = "Task2";
    task2cfg.task_priority = DEFAULT_PRIORITY;
    task2 = task_create(rtos_task2, arg, &task2cfg);
    if (!task2) {
        LOG_E(__FILE__, "Failed to create task 2");
        return ERR_FAIL;
    }
    /**
     * Task 5 has a low priority and will run when task 2 yields It purposely
     * overflows its stack, then yields to verify stack checking functionality
     */
    task5cfg.task_name = "Task5";
    task5cfg.task_priority = IDLE_TASK_PRIORITY + 1;
    task5cfg.task_stack = t5stack;
    task5cfg.task_stacksize = 64; // Note we don't use entire stack
    if (task_create(rtos_task5, NULL, &task5cfg) == NULL) {
        LOG_E(__FILE__, "Failed to create task 5");
        return ERR_FAIL;
    }
    LOG_D(__FILE__, "Starting RTOS");
    rtos_start();
    return SYS_OK;
}