/**
 * @file task.h
 * Implements task creation, destruction, and scheduling
 */

#include <stdint.h>
#include <stdlib.h>
#include <sys/err.h>
#include <util/list/list.h>

#include "task.h"

#define reg_t volatile uint32_t

/**
 * Register state structure. Used to store register state on task switch
 */
typedef struct regstate {
    reg_t pc;        /*!< program counter */
    reg_t sp;        /*!< stack pointer */
    reg_t lr;        /*!< link register */
    reg_t regs[15U]; /*!< general registers */
} regstate_t;

/**
 * Task state enum
 */
typedef enum task_state {
    TASK_EXITED,  /*!< Task exited */
    TASK_BLOCKED, /*!< Task blocked and cannot run */
    TASK_READY,   /*!< Task is ready but not running */
    TASK_ACTIVE,  /*!< Task is running */
} task_state_t;

/**
 * Task block reason
 */
typedef enum block_reason {
    BLOCK_NONE,      /*!< Task is not blocked */
    BLOCK_SEMAPHORE, /*!< Task is blocked due to sempahore pend */
    BLOCK_TIMER,     /*!< Task is blocked due to timer */
} block_reason_t;

/**
 * Task control block. Keeps task status and recordkeeping information.
 */
typedef struct task_status {
    regstate_t registers;      /*!< task register state */
    void (*entry)(void *);     /*!< task entry point */
    void *arg;                 /*!< Task argument */
    task_state_t state;        /*!< state of task */
    block_reason_t blockcause; /*!< cause for task block */
    char *stack_start;         /*!< Start of task stack */
    char *stack_end;           /*!< End of task stack */
    syserr_t task_ret;         /*!< Task return value */
    uint32_t priority;         /*!< Task priority */
    list_state_t list_state; /*!< Task list state */
} task_status_t;

// Task control block lists
task_status_t *active_task = NULL;
list_t ready_tasks = NULL;
list_t blocked_tasks = NULL;

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
task_handle_t task_create(void (*entry)(void *), void *arg,
                          task_config_t *cfg) {
    task_status_t *task;
    // Check parameters
    if (entry == NULL) {
        return NULL;
    }
    task = malloc(sizeof(task_status_t));
    if (task == NULL) {
        return NULL;
    }
    // Allocate task block
    if (cfg == NULL) {
        // Set default task parameters
        task->priority = DEFAULT_PRIORITY;
        task->stack_start = malloc(DEFAULT_STACKSIZE);
        if (task->stack_start == NULL) {
            free(task);
            return NULL;
        }
        task->stack_end = task->stack_end + DEFAULT_STACKSIZE;
    } else {
        // Check if a stack was provided
        if (cfg->task_stack) {
            task->stack_start = cfg->task_stack;
        } else {
            task->stack_start = malloc(cfg->task_stacksize);
            if (task->stack_start == NULL) {
                free(task);
                return NULL;
            }
        }
        // Calculate end of stack
        task->stack_end = task->stack_start + cfg->task_stacksize;
        task->priority = cfg->task_priority;
    }
    // Update task state and place in ready queue
    task->blockcause = BLOCK_NONE;
    task->state = TASK_READY;
    task->entry = entry;
    task->arg = arg;
    // Mark task as ready to run
    ready_tasks = list_append(ready_tasks, task, &(task->list_state));
    return (task_handle_t)task;
};

/**
 * Destroys a task. Will stop task execution immediately.
 * @param task: Task handle to destroy
 */
void task_destroy(task_handle_t task) {}
