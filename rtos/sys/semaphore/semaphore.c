/**
 * @file semaphore.c
 * implements binary and counting semaphores
 */
#include <stdlib.h>

#include <sys/err.h>
#include <sys/task/task.h>
#include <util/list/list.h>
#include <util/logging/logging.h>

#include "semaphore.h"

#define SEMAPHORE_UNLOCKED 0x00
#define SEMAPHORE_LOCKED 0xFF

/** Semaphore type */
typedef enum {
    SEMAPHORE_COUNTING,
    SEMAPHORE_BINARY,
} semaphore_type_t;

/** Internal defintion of semaphore structure */
typedef struct semaphore_state {
    char lock;             /*!< Semaphore lock. 0 when open, 0xFF when locked*/
    unsigned int value;    /*!< Semaphore value */
    semaphore_type_t type; /*!< Semaphore type */
    list_t waiting_tasks;  /*!< List of tasks waiting on the semaphore */
} semaphore_state_t;

/** Waiting task structure */
typedef struct waiting_task {
    task_handle_t task;
    list_state_t list_state;
} waiting_task_t;

static const char *TAG = "semaphore.c";

// Static functions
static void get_semaphore_lock(semaphore_state_t *sem);
static void drop_semaphore_lock(semaphore_state_t *sem);

/**
 * creates a new counting semaphore
 * @param start: starting value for counting semaphore
 * @return handle to created semaphore, or null on error
 */
semaphore_t semaphore_create_counting(unsigned int start) {
    semaphore_state_t *sem = malloc(sizeof(semaphore_state_t));
    if (sem == NULL) {
        return NULL;
    }
    // Initialize semaphore
    sem->lock = SEMAPHORE_UNLOCKED;
    sem->type = SEMAPHORE_COUNTING;
    sem->value = start;
    sem->waiting_tasks = NULL;
    return (semaphore_t)sem;
}

/**
 * creates a new binary semaphore. semaphore always starts at 0.
 * @return handle to created semaphore, or null on error
 */
semaphore_t semaphore_create_binary() {
    semaphore_state_t *sem = malloc(sizeof(semaphore_state_t));
    if (sem == NULL) {
        return NULL;
    }
    // Initialize semaphore
    sem->lock = SEMAPHORE_UNLOCKED;
    sem->type = SEMAPHORE_BINARY;
    sem->value = 0;
    sem->waiting_tasks = NULL;
    return (semaphore_t)sem;
}

/**
 * pends on a semaphore (p). if other tasks are pending on semaphore, calling
 * task will be given lowest priority. blocks until semaphore value is nonzero
 * and all tasks that pended before caller have been unblocked.
 * @param sem: semaphore to pend on
 */
void semaphore_pend(semaphore_t sem) {
    semaphore_state_t *semaphore = (semaphore_state_t *)sem;
    waiting_task_t *queue_entry;
    while (1) {
        // Get the semaphore lock
        get_semaphore_lock(semaphore);
        // Check semaphore value
        if (semaphore->value > 0) {
            semaphore->value--;
            // Release semaphore lock and return
            drop_semaphore_lock(semaphore);
            return;
        } else {
            // Place this task into semaphore's queue
            queue_entry = malloc(sizeof(waiting_task_t));
            if (queue_entry == NULL) {
                LOG_E(TAG, "Out of memory to allocate queue entry\n");
                exit(ERR_NOMEM);
            }
            queue_entry->task = get_active_task();
            // Add queue entry to semaphore queue
            semaphore->waiting_tasks =
                list_append(semaphore->waiting_tasks, queue_entry,
                            &(queue_entry->list_state));
            // Drop semaphore lock
            drop_semaphore_lock(semaphore);
            block_active_task(BLOCK_SEMAPHORE);
        }
    }
}

/**
 * posts to a semaphore (v), incrementing the value by one. does not block.
 * If the semaphore is a binary one and the value is already one, this call
 * has no effect.
 * @param sem: semaphore to post to
 */
void semaphore_post(semaphore_t sem) {
    semaphore_state_t *semaphore = (semaphore_state_t *)sem;
    waiting_task_t *runnable_queue_entry;
    task_handle_t runnable_task;
    // Get the semaphore lock
    get_semaphore_lock(semaphore);
    if (semaphore->type == SEMAPHORE_BINARY && semaphore->value == 1) {
        // Drop lock and return
        drop_semaphore_lock(semaphore);
        return;
    }
    // Increment the semaphore value
    semaphore->value++;
    // If tasks are waiting, unblock one
    if (semaphore->waiting_tasks != NULL) {
        runnable_queue_entry = list_get_head(semaphore->waiting_tasks);
        semaphore->waiting_tasks = list_remove(
            semaphore->waiting_tasks, &(runnable_queue_entry->list_state));
        runnable_task = runnable_queue_entry->task;
        // Release memory for queue entry
        free(runnable_queue_entry);
        // Drop the semaphore lock
        drop_semaphore_lock(semaphore);
        // Mark the selected task as runnable
        unblock_task(runnable_task, BLOCK_SEMAPHORE);
        return;
    }
    // Drop the semaphore lock
    drop_semaphore_lock(semaphore);
}

/**
 * destroys a semaphore. will fail if any tasks are pending on semaphore
 * @param sem: semaphore to destroy.
 * @return sys_ok on success, or err_badparam when tasks are pending
 */
syserr_t semaphore_destroy(semaphore_t sem) {
    semaphore_state_t *semaphore = (semaphore_state_t *)sem;
    // Get the semaphore lock
    get_semaphore_lock(semaphore);
    if (semaphore->waiting_tasks != NULL) {
        LOG_D(TAG, "Cannot destroy semaphore, tasks are pending");
        // Drop semaphore
        drop_semaphore_lock(semaphore);
        return ERR_BADPARAM;
    } else {
        // Free semaphore resources
        free(semaphore);
        return SYS_OK; // No need to drop lock, we just freed it
    }
}

/**
 * Gets semaphore lock. Returns when lock is acquired
 * @param sem: Semaphore state to get lock for.
 */
static void get_semaphore_lock(semaphore_state_t *sem) {
    /**
     * Load semaphore lock using LDREXB. Check if lock is 0x00, and if so
     * acquire it. If not, drop memory access with a strexb instruction, and
     * retry
     */
    asm volatile(
        "try_lock_%=:\n"         // Entry point for reading the lock value
        "ldrexb r0, [%[lock]]\n" // Get lock
        "cmp r0, %[UNLOCKED]\n"  // Check if lock is open
        "it eq\n"
        "beq take_lock_%=\n"         // Lock is open, take it
        "strexb r1, r0, [%[lock]]\n" // Lock is closed. Release memory access.
        "cmp r1, #0x0\n"             // Check to make sure strexb updated memory
        "it ne\n"
        "bne try_lock_%=\n"          // If strexb failed, try to get lock again
        "take_lock_%=:\n"            // Section for taking the lock
        "mov r0, %[LOCKED]\n"        // Set r0 to the locked value
        "strexb r1, r0, [%[lock]]\n" // Try to store new locked value
        "cmp r1, #0x0\n"             // Check to ensure strexb succeeded
        "it ne\n"
        "bne try_lock_%=\n" // strexb failed. Try to get lock again.
        :
        : [ lock ] "p"(&(sem->lock)), [ LOCKED ] "i"(SEMAPHORE_LOCKED),
          [ UNLOCKED ] "i"(SEMAPHORE_UNLOCKED));
}

/**
 * Drops semaphore lock. MUST not be called without a matching call to
 * get_semaphore_lock before. Returns when semaphore lock has been dropped.
 * @param sem: Semaphore state to drop lock for.
 */
static void drop_semaphore_lock(semaphore_state_t *sem) {
    /**
     * Load semaphore lock using LDREXB. Check if lock is 0xFF, and if so drop
     * it. If not, spin the processor so user knowns there was an error
     */
    asm volatile("ldrexb r0, [%[lock]]\n"
                 "spin_%=:\n"            // Offset to spin processor from
                 "cmp r0, %[UNLOCKED]\n" // Check if lock is unlocked
                 "it eq\n"
                 "beq spin_%=\n" // Spin here, lock is unlocked
                 "mov r0, %[UNLOCKED]\n"
                 "try_drop_%=:\n"
                 "strexb r1, r0, [%[lock]]\n" // Set lock as unlocked
                 "cmp r1, #0x0\n"             // Check if strexb succeeded
                 "it ne\n"
                 "bne try_drop_%=\n" // strexb failed, retry lock drop
                 :
                 : [ lock ] "p"(&(sem->lock)), [ LOCKED ] "i"(SEMAPHORE_LOCKED),
                   [ UNLOCKED ] "i"(SEMAPHORE_UNLOCKED));
}
