#ifndef ERR_H
#define ERR_H

/**
 * Error definitions
 */
typedef enum syserr {
    SYS_OK,        /*!< Success return code */
    ERR_FAIL,      /*!< Generic error */
    ERR_BADPARAM,  /*!< Invalid parameters */
    ERR_NOMEM,     /*!< Out of memory */
    ERR_INUSE,     /*!< Device in use */
    ERR_NOSUPPORT, /*!< Feature support is not implemented */
    ERR_DEVICE,    /*!< Device Hardware error */
    ERR_TIMEOUT,   /*!< Device or system timeout */
    ERR_NOTINIT,   /*!< Device not initialized */
    ERR_SCHEDULER, /*!< RTOS scheduler error */
} syserr_t;

#endif