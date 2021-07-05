/**
 * @file task.h
 * Implements task creation, destruction, and scheduling
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <drivers/clock/clock.h>
#include <drivers/device/device.h>
#include <sys/err.h>
#include <sys/isr/isr.h>
#include <util/bitmask.h>
#include <util/list/list.h>
#include <util/logging/logging.h>

#include "task.h"

#define reg_t volatile uint32_t
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
    char *stack_ptr;           /*!< Task stack pointer. MUST be first entry*/
    char *stack_start;         /*!< Task stack start */
    char *stack_end;           /*!< End of task stack */
    void (*entry)(void *);     /*!< task entry point */
    void *arg;                 /*!< Task argument */
    task_state_t state;        /*!< state of task */
    char *name;                /*!< Task name */
    bool stack_allocated;      /*!< Was the stack allocated? */
    block_reason_t blockcause; /*!< cause for task block */
    uint32_t priority;         /*!< Task priority */
    list_state_t list_state;   /*!< Task list state */
} task_status_t;

// Task control block lists
static task_status_t *active_task = NULL;
static task_status_t *new_task = NULL; // Used by SVCall
static list_t ready_tasks[RTOS_PRIORITY_COUNT] = {NULL};
static list_t blocked_tasks = NULL;

// Logging tag
const char *TAG = __FILE__;

// Static functions
static inline void set_pendsv();
static inline void trigger_svcall();
static void idle_entry(void *arg);

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
        task->name = "";
        task->stack_end = malloc(DEFAULT_STACKSIZE);
        task->stack_allocated = true;
        if (task->stack_end == NULL) {
            free(task);
            return NULL;
        }
        task->stack_start = task->stack_end + (DEFAULT_STACKSIZE - 1);
    } else {
        // Check priority
        if (cfg->task_priority > RTOS_PRIORITY_COUNT) {
            return NULL;
        }
        // Check if a stack was provided
        if (cfg->task_stack) {
            task->stack_end = cfg->task_stack;
            task->stack_allocated = false;
        } else {
            task->stack_end = malloc(cfg->task_stacksize);
            task->stack_allocated = true;
            if (task->stack_end == NULL) {
                free(task);
                return NULL;
            }
        }
        if (cfg->task_name) {
            task->name = cfg->task_name;
        } else {
            // Default value
            task->name = "";
        }
        // Calculate start of stack
        task->stack_start = task->stack_end + (cfg->task_stacksize - 1);
        task->priority = cfg->task_priority;
    }
    // Update task state and place in ready queue
    task->blockcause = BLOCK_NONE;
    task->state = TASK_READY;
    task->entry = entry;
    task->arg = arg;
    // Set new_task to this task, and trigger sv call
    new_task = task;
    trigger_svcall(); // Execution will resume here on context switch
    if (active_task == task) {
        // A context switch selected this task. Run entry function
        task->entry(task->arg);
    }
    // SVCall exception handled. Return to calling task.
    return (task_handle_t)task;
};

/**
 * Starts the real time operating system. This function will not return.
 *
 * Once the RTOS starts, scheduled tasks will start executing based on priority.
 * If no tasks are scheduled, this function will essentially freeze the system.
 */
void rtos_start() {
    task_status_t *task;
    uint32_t reload_val;
    /**
     * TODO:
     * 1. create a task control block for idle process (set a low priority,
     *    use the initial stack as the process stack)
     * 2. set this TCB as the new_task
     * 3. trigger an SVCall to populate the stack with saved registers
     * 4. set the idle task TCB as the active task
     * 5. enable the systick interrupt
     * 6. run the idle task
     */
    /* Create a task control block for idle process */
    task = malloc(sizeof(task_status_t));
    if (task == NULL) {
        LOG_E(TAG, "Could not allocate idle task control block");
        exit(ERR_NOMEM);
    }
    // Populate remaining idle task configuration
    task->priority = IDLE_TASK_PRIORITY;
    task->name = "Idle Task";
    task->stack_allocated = false;
    // Base the task start on the current stack pointer
    asm volatile(
        "mrs %[stack_start], msp\n" // save stack pointer into task->stack_start
        :
        : [stack_start] "r"(task->stack_start));
    // Calculate end of the stack
    task->stack_end = task->stack_start - IDLE_TASK_STACK_SIZE + 1;
    task->entry = idle_entry;
    task->arg = NULL;
    task->blockcause = BLOCK_NONE;
    task->state = TASK_ACTIVE;
    /* Set this TCB as the new task */
    new_task = task;
    /* Trigger an SVCall to populate the stack with saved registers */
    trigger_svcall();
    /**
     * Now we are running in process mode. Note that the svcall will not
     * change our stack pointer's location, just the register in use. This
     * means we can simply ignore the top of stack set by the svcall
     */
    // Set this task as the active one
    active_task = task;
    /**
     * The stm32l433 defaults to sourcing the systick clock as HCLK divided
     * by 8. We must set the reload value (24 bits) to achieve the desired
     * systick rate of 5ms
     */
    reload_val = (hclk_freq() >> 3) / SYSTICK_FREQ;
    if (reload_val > SysTick_LOAD_RELOAD_Msk) {
        LOG_E(TAG, "Oversized systick reload value");
        exit(ERR_BADPARAM);
    }
    // Set the reload value (interrupt fires when counting from 1 to 0)
    SysTick->LOAD = reload_val - 1;
    // Enable the systick interrupt
    SETBITS(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk);
    // Start the idle task
    task->entry(task->arg);
    // If the idle loop returns, log an error
    LOG_E(TAG, "Idle loop returned!");
    exit(ERR_FAIL);
}

/**
 * Yields task execution. This function will stop execution of the current
 * task, and yield execution to the highest priority task able to run
 */
void task_yield() {
    // Mark task as ready, not active
    active_task->state = TASK_READY;
    // Trigger a system context switch switch by setting pendsv bit
    set_pendsv();
}

/**
 * Destroys a task. Will stop task execution immediately.
 * @param task: Task handle to destroy
 */
void task_destroy(task_handle_t task) {}

/**
 * System task creation handler. Saves current processor state into the
 * new_task structure, so that new_task can be resumed from the current point
 * in execution. Also switches the processor to use the process stack. Does
 * not modify the program counter or stack pointer.
 *
 * This function SHOULD NOT BE CALLED BY THE USER. It is indended to run in
 * Handler mode, as the SVCall isr
 */
void SVCallHandler() {
    /**
     * Save the context of the new task
     */
    asm volatile(
        "bkpt\n" // For testing code
        "tst lr, #0x04\n"
        "ite eq\n"
        "mrseq r0, msp\n"        // Load main stack pointer to r0
        "mrsne r0, psp\n"        // Load process stack pointer to r0
        "mov r1, r0\n"           // Save current stack pointer
        "ldr r3, %[new_task] \n" // Load memory location of new task
        "ldr r2, [r3]\n"         // Load new task struct, to access top of stack

        "stmfd r0!, {r4-r11, lr}\n" // Save calle-saved registers
        "str r0, [r2]\n"            // Store the new top of the stack

        "msr psp, r1\n"   // Load r1 as the process stack pointer
        "orr lr, #0x04\n" // Logical OR sets bit 4 in LR so that processor
                          // returns to process stack

        "bx lr\n" // Exception return. Core will intercept load of
                  // 0xFXXXXXXX to PC and return from exception
        :
        : [new_task] "m"(new_task));
}

/**
 * System context switch handler. Stores core registers for current
 * execution context, then selects the highest priority ready task to run.
 *
 * This function SHOULD NOT BE CALLED BY THE USER. It is indended to run in
 * Handler mode, as the PendSV isr
 */
void PendSVHandler() {
    /**
     * Save the context of the currently running task.
     * Assumes task is running with process stack
     */
    asm volatile(
        "bkpt\n"                    // For testing code
        "mrs r0, psp\n"             // Load process stack pointer to r0
        "ldr r3, %[active_task] \n" // Load memory location of active task
        "ldr r2, [r3]\n" // Load active task struct, to access top of stack

        "stmfd r0!, {r4-r11, lr}\n" // Save calle-saved registers
        "str r0, [r2]\n"            // Store the new top of the stack

        "cpsid i\n"               // Disable interrupts (set PRIMASK to 1)
        "stmfd sp!, {r0-r3}\n"    // Save caller saved regs to main stack
        "bl select_active_task\n" // Call Sfunction to select new active
                                  // task
        "ldmfd sp!, {r0-r3}\n"    // Restore registers after function call
        "cpsie i\n"               // Reenable interrupt

        "ldr r2, [r3]\n" // Reload data stored in active_task
        "ldr r0, [r2]\n" // Load the address of the top of task stack

        "ldmfd r0!, {r4-r11, lr}\n" // Restore calle-saved registers for
                                    // task
        "msr psp, r0\n"             // Load r0 as the stack pointer

        "bx lr\n" // Exception return. Core will intercept load of
                  // 0xFXXXXXXX to PC and return from exception
        :
        : [active_task] "m"(active_task));
}

/**
 * System tick handler. Handles periodic RTOS tasks, such as checking to see
 * if blocked tasks are now unblocked, and preempting tasks if enabled.
 *
 * This function SHOULD NOT BE CALLED BY THE USER. It is indended to run in
 * Handler mode, as the PendSV isr
 */
void SysTickHandler() {
    /**
     * TODO: check if preemption should occur (if enabled), and what tasks
     * are runnable
     */
    asm volatile("bkpt"); // Temporary, for debugging
}

/**
 * This function should ONLY be called by internal routines.
 * Selects new active task from all the tasks in ready lists. Selects the
 * highest priority task available to run.
 * Does not update active task state, but will refer to task state when
 * placing it into blocked/ready list. Does update active task state.
 */
void select_active_task() {
    int i;
    task_status_t *new_active;
    /**
     * Examine task lists to find the highest priority list with tasks ready
     * to run
     */
    for (i = RTOS_PRIORITY_COUNT - 1; i > 0; i--) {
        // If task list has tasks, break
        if (ready_tasks[i] != NULL)
            break;
    }
    // Select the head of this ready task list
    new_active = list_get_head(ready_tasks[i]);
    ready_tasks[i] = list_remove(ready_tasks[i], &(new_active->list_state));
    /**
     * Based on the block state of the active task, store it in a blocked
     * list or the ready list
     */
    if (active_task->state == TASK_BLOCKED) {
        blocked_tasks =
            list_append(blocked_tasks, active_task, &(active_task->list_state));
    } else {
        // Append active task to appropriate ready list
        ready_tasks[i] = list_append(ready_tasks[i], active_task,
                                     &(active_task->list_state));
    }
    // Change the active task
    active_task = new_active;
    active_task->state = TASK_ACTIVE;
}

/**
 * Idle loop. This task runs when no other tasks can.
 * @param arg: unused.
 */
static void idle_entry(void *arg) {
    while (1) {
        LOG_E(TAG, "Idle loop");
        delay_ms(200);
    }
}

/**
 * Triggers a context switch via setting pendsv (will trigger pendsv
 * interrupt)
 */
static inline void set_pendsv() { SETBITS(SCB->ICSR, SCB_ICSR_PENDSVSET_Msk); }

/**
 * Triggers a task setup (essentially populating a task control block) via
 * an SVCall exception
 */
static inline void trigger_svcall() { asm volatile("svc 0"); }