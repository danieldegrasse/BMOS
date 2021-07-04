
/**
 * @file logging.c
 * Implements system logging facilities
 */
#include <stdarg.h>
#include <stdio.h>

#include <config.h>

#include "logging.h"

static void syslog(int log_level, char *tag, char *format, va_list ap);

/**
 * System debugging log. Uses same format as printf
 * @param tag: logging tag
 * @param format: printf style formatting string
 */
void LOG_D(char *tag, char *format, ...) {
    va_list valist;
    // Start valist with format as last argument
    va_start(valist, format);
    syslog(SYSLOGLEVEL_DEBUG, tag, format, valist);
    // Free valist
    va_end(valist);
}

/**
 * System info log. Uses same format as printf
 * @param tag: logging tag
 * @param format: printf style formatting string
 */
void LOG_I(char *tag, char *format, ...) {
    va_list valist;
    // Start valist with format as last argument
    va_start(valist, format);
    syslog(SYSLOGLEVEL_INFO, tag, format, valist);
    // Free valist
    va_end(valist);
}

/**
 * System warning log. Uses same format as printf
 * @param tag: logging tag
 * @param format: printf style formatting string
 */
void LOG_W(char *tag, char *format, ...) {
    va_list valist;
    // Start valist with format as last argument
    va_start(valist, format);
    syslog(SYSLOGLEVEL_WARNING, tag, format, valist);
    // Free valist
    va_end(valist);
}

/**
 * System error log. Uses same format as printf
 * @param tag: logging tag
 * @param format: printf style formatting string
 */
void LOG_E(char *tag, char *format, ...) {
    va_list valist;
    // Start valist with format as last argument
    va_start(valist, format);
    syslog(SYSLOGLEVEL_ERROR, tag, format, valist);
    // Free valist
    va_end(valist);
}

static void syslog(int log_level, char *tag, char *format, va_list ap) {
    if (log_level >= SYSLOGLEVEL) {
        // Log message level
        switch (log_level) {
        case SYSLOGLEVEL_DEBUG:
            printf("%s [DEBUG]: ", tag);
            break;
        case SYSLOGLEVEL_INFO:
            printf("%s [INFO]: ", tag);
            break;
        case SYSLOGLEVEL_WARNING:
            printf("%s [WARNING]: ", tag);
            break;
        case SYSLOGLEVEL_ERROR:
            printf("%s [ERROR]: ", tag);
            break;
        default:
            printf("%s [LOG]: ", tag);
            break;
        }
        // Log message
        vprintf(format, ap);
        // Print newline
        printf("\n");
    }
}