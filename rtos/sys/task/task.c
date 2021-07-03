/**
 * @file task.h
 * Implements task creation, destruction, and scheduling
 */

#include <stdint.h>
#include <stdlib.h>
#include <sys/err.h>

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
    BLOCK_SEMAPHORE, /*!< Task is blocked due to sempahore pend */
    BLOCK_TIMER,     /*!< Task is blocked due to timer */
} block_reason_t;

/**
 * Task control block. Keeps task status and recordkeeping information.
 */
typedef struct task_status {
    regstate_t registers;      /*!< task register state */
    syserr_t (*entry)(void *); /*!< task entry point */
    task_state_t state;        /*!< state of task */
    block_reason_t blockcause; /*!< cause for task block */
} task_status_t;

/**
 * Creates a system task. Requires memory allocation to be enabled to succeed.
 * @param entry: task entry point. Must be a function taking a void* and
 * returning a type of syserr_t
 * @param arg: task argument. May be NULL. Will be passed to the task entry
 * point function
 * @return created task handle on success, or NULL on error
 */
task_handle_t task_create(syserr_t (*entry)(void *), void *arg) { return NULL; }

/**
 * Destroys a task. Will stop task execution immediately.
 * @param task: Task handle to destroy
 */
void task_destroy(task_handle_t task) {}
