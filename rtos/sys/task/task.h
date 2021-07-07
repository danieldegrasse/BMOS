/**
 * @file task.h
 * Implements task creation, destruction, and scheduling
 */

#ifndef TASK_H
#define TASK_H

#include <sys/err.h>
#include <stdint.h>

#define DEFAULT_STACKSIZE 2048
#define DEFAULT_PRIORITY 5
#define RTOS_PRIORITY_COUNT 7 // 7 independent priority levels
#define IDLE_TASK_PRIORITY 0
#define IDLE_TASK_STACK_SIZE 1024
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
    const char *task_name;  /*< Optional task name */
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
 * If no tasks are scheduled, this function will essentially freeze the system
 * in the idle task.
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
 * Task block reason. Not intended for user use, used by system drivers.
 */
typedef enum block_reason {
    BLOCK_NONE,      /*!< Task is not blocked */
    BLOCK_SEMAPHORE, /*!< Task is blocked due to sempahore pend */
} block_reason_t;

/**
 * Gets the active task. Used by system drivers
 * @return handle to active task
 */
task_handle_t get_active_task();

/**
 * Blocks the running task, and switches to a new runnable one. This function
 * does not return. Used by system drivers.
 * @param reason: reason for task block
 */
void block_active_task(block_reason_t reason);

/**
 * Unblocks a task. Caller must give correct reason task was blocked. If
 * reason is incorrect, this call has no effect. Used by system drivers.
 * Task will not run immediately unless it has higher priority than running task
 * and preemption is enabled.
 * @param task: task to unblock
 * @param reason: reason task was blocked.
 */
void unblock_task(task_handle_t task, block_reason_t reason);

/**
 * System context switch handler. Stores core registers for current
 * execution context, then selects the highest priority ready task to run.
 *
 * This function SHOULD NOT BE CALLED BY THE USER. It is indended to run in
 * Handler mode, as the PendSV isr
 */
void PendSVHandler();

/**
 * SVCall handler. Enables the system tick, switches the processor to the
 * process stack, and starts the RTOS scheduler.
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
