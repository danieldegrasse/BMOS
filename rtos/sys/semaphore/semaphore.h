/**
 * @file semaphore.h
 * implements binary and counting semaphores
 */
#ifndef semaphore_h
#define semaphore_h
#include <sys/err.h>

// typedef to obscure internal definition of semaphore
typedef void *semaphore_t;

/**
 * creates a new counting semaphore
 * @param start: starting value for counting semaphore
 * @return handle to created semaphore, or null on error
 */
semaphore_t semaphore_create_counting(unsigned int start);

/**
 * creates a new binary semaphore. semaphore always starts at 0.
 * @return handle to created semaphore, or null on error
 */
semaphore_t semaphore_create_binary();

/**
 * posts to a semaphore (v), incrementing the value by one. does not block.
 * If the semaphore is a binary one and the value is already one, this call
 * has no effect.
 * @param sem: semaphore to post to
 */
void semaphore_pend(semaphore_t sem);

/**
 * posts to a semaphore (v), incrementing the value by one. does not block
 * @param sem: semaphore to post to
 */
void semaphore_post(semaphore_t sem);

/**
 * destroys a semaphore. will fail if any tasks are pending on semaphore
 * @param sem: semaphore to destroy.
 * @return sys_ok on success, or err_badparam when tasks are pending
 */
syserr_t semaphore_destroy(semaphore_t sem);

#endif
