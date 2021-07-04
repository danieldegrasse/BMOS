/**
 * @file task.h
 * Implements task creation, destruction, and scheduling
 */

#ifndef TASK_H
#define TASK_H

#include <sys/err.h>

#define DEFAULT_STACKSIZE 2048
#define DEFAULT_PRIORITY 5

typedef void *task_handle_t;

/**
 * Task configuration structure
 */
typedef struct task_config {
    char *task_stack;   /*!< Optional statically allocated task stack */
    int task_stacksize; /*!< Desired size of task stack. If stack is provided
                           set this to size of task stack*/
    uint32_t task_priority; /*!< Task priority */
} task_config_t;

/**
 * Creates a system task. Requires memory allocation to be enabled to succeed.
 * @param entry: task entry point. Must be a function taking a void* and
 * returning a type of syserr_t
 * @param arg: task argument. May be NULL. Will be passed to the task entry
 * point function
 * @param cfg: task configuration structure. May be NULL
 * @return created task handle on success, or NULL on error
 */
task_handle_t task_create(syserr_t (*entry)(void *), void *arg,
                          task_config_t *cfg);

/**
 * Destroys a task. Will stop task execution immediately.
 * @param task: Task handle to destroy
 */
void task_destroy(task_handle_t task);

/**
 * Default task configuration
 */
#define DEFAULT_TASK_CONFIG                                                    \
    {                                                                          \
        .task_stack = NULL, .task_stacksize = DEFAULT_STACKSIZE,               \
        .task_priority = DEFAULT_PRIORITY                                      \
    }

#endif
