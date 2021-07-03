/**
 * @file task.h
 * Implements task creation, destruction, and scheduling
 */

#ifndef TASK_H
#define TASK_H

#include <sys/err.h>

typedef void* task_handle_t;

/**
 * Creates a system task. Requires memory allocation to be enabled to succeed.
 * @param entry: task entry point. Must be a function taking a void* and
 * returning a type of syserr_t
 * @param arg: task argument. May be NULL. Will be passed to the task entry
 * point function
 * @return created task handle on success, or NULL on error
 */
task_handle_t task_create(syserr_t (*entry)(void*), void* arg);

/**
 * Destroys a task. Will stop task execution immediately.
 * @param task: Task handle to destroy
 */
void task_destroy(task_handle_t task);

#endif
