
/**
 * @file logging.c
 * Implements system logging facilities
 */
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <config.h>

#include "logging.h"

#if SYSLOG == SYSLOG_DISABLED
/** Define all functions as stubs to minimize code size */
void LOG_D(const char *tag, char *format, ...) {
    (void)tag;
    (void)format;
}

void LOG_I(const char *tag, char *format, ...) {
    (void)tag;
    (void)format;
}

void LOG_W(const char *tag, char *format, ...) {
    (void)tag;
    (void)format;
}

void LOG_E(const char *tag, char *format, ...) {
    (void)tag;
    (void)format;
}

/**
 * Minimal system log. Useful for tasks with small stacks, or other low memory
 * environments. Does not implement printf style formatting.
 * @param log_level: logging level to use
 * @param tag: Tag to log (NULL terminated)
 * @param logstr: NULL terminated log string
 */
void LOG_MIN(int log_level, const char *tag, const char *logstr) {
    (void)log_level;
    (void)tag;
    (void)logstr;
}

#else

static void syslog(int log_level, const char *tag, char *format, va_list ap);

/**
 * System debugging log. Uses same format as printf
 * @param tag: logging tag
 * @param format: printf style formatting string
 */
void LOG_D(const char *tag, char *format, ...) {
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
void LOG_I(const char *tag, char *format, ...) {
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
void LOG_W(const char *tag, char *format, ...) {
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
void LOG_E(const char *tag, char *format, ...) {
    va_list valist;
    // Start valist with format as last argument
    va_start(valist, format);
    syslog(SYSLOGLEVEL_ERROR, tag, format, valist);
    // Free valist
    va_end(valist);
}

/**
 * Minimal system log. Useful for tasks with small stacks, or other low memory
 * environments. Does not implement printf style formatting.
 * @param log_level: logging level to use
 * @param tag: Tag to log (NULL terminated)
 * @param logstr: NULL terminated log string
 */
void LOG_MIN(int log_level, const char *tag, const char *logstr) {
    if (log_level >= SYSLOGLEVEL) {
        // Log tag and level
        write(STDOUT_FILENO, tag, strlen(tag));
        switch (log_level) {
        case SYSLOGLEVEL_DEBUG:
            write(STDOUT_FILENO, "[DEBUG]: ", 9);
            break;
        case SYSLOGLEVEL_INFO:
            write(STDOUT_FILENO, "[INFO]: ", 8);
            break;
        case SYSLOGLEVEL_WARNING:
            write(STDOUT_FILENO, "[WARNING]: ", 11);
            break;
        case SYSLOGLEVEL_ERROR:
            write(STDOUT_FILENO, "[ERROR]: ", 9);
            break;
        default:
            write(STDOUT_FILENO, "[LOG]: ", 7);
            break;
        }
        // Log logging string
        write(STDOUT_FILENO, logstr, strlen(logstr));
    }
}

static void syslog(int log_level, const char *tag, char *format, va_list ap) {
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

#endif