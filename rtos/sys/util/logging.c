
/**
 * @file logging.c
 * Implements system logging facilities
 */
#include <stdarg.h>
#include <stdio.h>

#include <config.h>

#include "logging.h"


static void syslog(int log_level, char *format, va_list ap);

/**
 * System debugging log. Uses same format as printf
 * @param format: printf style formatting string
 */
void LOG_D(char *format, ...) {
    va_list valist;
    // Start valist with format as last argument
    va_start(valist, format);
    syslog(SYSLOGLEVEL_DEBUG, format, valist);
    // Free valist
    va_end(valist);
}

/**
 * System info log. Uses same format as printf
 * @param format: printf style formatting string
 */
void LOG_I(char *format, ...) {
    va_list valist;
    // Start valist with format as last argument
    va_start(valist, format);
    syslog(SYSLOGLEVEL_INFO, format, valist);
    // Free valist
    va_end(valist);
}

/**
 * System warning log. Uses same format as printf
 * @param format: printf style formatting string
 */
void LOG_W(char *format, ...) {
    va_list valist;
    // Start valist with format as last argument
    va_start(valist, format);
    syslog(SYSLOGLEVEL_WARNING, format, valist);
    // Free valist
    va_end(valist);
}

/**
 * System error log. Uses same format as printf
 * @param format: printf style formatting string
 */
void LOG_E(char *format, ...) {
    va_list valist;
    // Start valist with format as last argument
    va_start(valist, format);
    syslog(SYSLOGLEVEL_ERROR, format, valist);
    // Free valist
    va_end(valist);
}

static void syslog(int log_level, char *format, va_list ap) {
    if (log_level >= SYSLOGLEVEL) {
        // Log message level
        switch (log_level) {
        case SYSLOGLEVEL_DEBUG:
            printf("DEBUG: ");
            break;
        case SYSLOGLEVEL_INFO: 
            printf("INFO: ");
            break;
        case SYSLOGLEVEL_WARNING: 
            printf("WARNING: ");
            break;
        case SYSLOGLEVEL_ERROR: 
            printf("ERROR: ");
            break;
        default: 
            printf("LOG: ");
            break;
        }
        // Log message
        vprintf(format, ap);
        // Print newline 
        printf("\n");
    }
}