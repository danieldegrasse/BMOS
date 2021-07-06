/**
 * @file task.h
 * Implements task creation, destruction, and scheduling
 */

#ifndef TASK_H
#define TASK_H

#include <sys/err.h>

#define DEFAULT_STACKSIZE 2048
#define DEFAULT_PRIORITY 5
#define RTOS_PRIORITY_COUNT 7 // 7 independent priority levels
#define IDLE_TASK_PRIORITY 0
#define IDLE_TASK_STACK_SIZE 512
#define SYSTICK_FREQ 1000 // Every 1ms (1000Hz)

typedef void *task_handle_t;

/**
 * Task configuration structure
 */
typedef struct task_config {
    char *task_stack;   /*!< Optional statically allocated task stack */
    int task_stacksize; /*!< Desired size of task stack. If stack is provided
                           set this to size of task stack*/
    uint32_t task_priority; /*!< Task priority */
    char *task_name;        /*< Optional task name */
} task_config_t;

/**
 * Creates a system task. Requires memory allocation to be enabled to succeed.
 * Task will be scheduled, but will not start immediately.
 * @param entry: task entry point. Must be a function taking a void* and
 * returning void
 * @param arg: task argument. May be NULL. Will be passed to the task entry
 * point function
 * @param cfg: task configuration structure. May be NULL
 * @return created task handle on success, or NULL on error
 */
task_handle_t task_create(void (*entry)(void *), void *arg, task_config_t *cfg);

/**
 * Yields task execution. This function will stop execution of the current
 * task, and yield execution to the highest priority task able to run
 */
void task_yield();

/**
 * Destroys a task. Will stop task execution immediately.
 * @param task: Task handle to destroy
 */
void task_destroy(task_handle_t task);

/**
 * Starts the real time operating system. This function will not return.
 *
 * Once the RTOS starts, scheduled tasks will start executing based on priority.
 * If no tasks are scheduled, this function will essentially freeze the system.
 */
void rtos_start();

/**
 * Default task configuration
 */
#define DEFAULT_TASK_CONFIG                                                    \
    {                                                                          \
        .task_stack = NULL, .task_stacksize = DEFAULT_STACKSIZE,               \
        .task_priority = DEFAULT_PRIORITY, .task_name = ""                     \
    }

/** ------------------------ End user functions ---------------------------- */

/**
 * System context switch handler. Stores core registers for current
 * execution context, then selects the highest priority ready task to run.
 *
 * This function SHOULD NOT BE CALLED BY THE USER. It is indended to run in
 * Handler mode, as the PendSV isr
 */
void PendSVHandler();

/**
 * System task creation and rtos startup handler
 *
 * When the new_task->stack_ptr is set to a value not equal to 0x0, saves
 * current processor state into the new_task structure, so that new_task can be
 * resumed from the current point in execution. Otherwise, this step is not
 * performed.
 *
 * Also always switches the processor to use the process stack. Does not modify
 * the program counter or stack pointer.
 *
 * This function SHOULD NOT BE CALLED BY THE USER. It is indended to run in
 * Handler mode, as the SVCall isr
 */
void SVCallHandler();

/**
 * System tick handler. Handles periodic RTOS tasks, such as checking to see
 * if blocked tasks are now unblocked, and preempting tasks if enabled.
 *
 * This function SHOULD NOT BE CALLED BY THE USER. It is indended to run in
 * Handler mode, as the PendSV isr
 */
void SysTickHandler();

#endif
