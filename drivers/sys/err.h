#ifndef ERR_H
#define ERR_H

/**
 * Error definitions
 */
typedef enum syserr {
    SYS_OK, /**! Success return code */
    ERR_FAIL, /**! Generic error */
    ERR_BADPARAM, /**! Invalid parameters */
} syserr_t;


#endif