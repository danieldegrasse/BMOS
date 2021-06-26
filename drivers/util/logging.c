
#include "logging.h"

/**
 * System logging levels
 */
typedef enum {
    LOGLEVEL_DEBUG,
    LOGLEVEL_INFO,
    LOGLEVEL_WARN,
    LOGLEVEL_ERR,
} loglevel_t;

/**
 * System debugging log. Uses same format as printf
 * @param format: printf style formatting string
 */
void LOG_D(char *format, ...) {}

/**
 * System info log. Uses same format as printf
 * @param format: printf style formatting string
 */
void LOG_I(char *format, ...) {}

/**
 * System warning log. Uses same format as printf
 * @param format: printf style formatting string
 */
void LOG_W(char *format, ...) {}

/**
 * System error log. Uses same format as printf
 * @param format: printf style formatting string
 */
void LOG_E(char *format, ...) {}