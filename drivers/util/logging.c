
/**
 * @file logging.c
 * Implements system logging facilities
 */
#include <stdarg.h>
#include <stdio.h>

#include <config.h>

#include "logging.h"

/**
 * ASCII color codes
 */
#define BLK "\e[30m"
#define RED "\e[31m"
#define GRN "\e[32m"
#define YEL "\e[33m"
#define BLU "\e[34m"
#define MAG "\e[35m"
#define CYN "\e[36m"
#define WHT "\e[37m"
// Reset Color
#define RST "\e[m"


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
    syslog(SYSLOGLEVEL_WARNING, format, valist);
    // Free valist
    va_end(valist);
}

static void syslog(int log_level, char *format, va_list ap) {
    if (log_level >= SYSLOGLEVEL) {
        // Log message level
        switch (log_level) {
        case SYSLOGLEVEL_DEBUG: // prints with magenta color
            printf(MAG"DEBUG: ");
            break;
        case SYSLOGLEVEL_INFO: // prints with cyan color
            printf(CYN"INFO: ");
            break;
        case SYSLOGLEVEL_WARNING: // prints with yellow color
            printf(YEL"WARNING: ");
            break;
        case SYSLOGLEVEL_ERROR: // prints with red color
            printf(RED"ERROR: ");
            break;
        default: // prints with white color
            printf(WHT"LOG: ");
            break;
        }
        // Log message
        vprintf(format, ap);
        // Print newline and color reset
        printf(RST"\n");
    }
}