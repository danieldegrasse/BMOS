/**
 * @file logging.h
 * Implements system logging facilities
 */

#ifndef LOGGING_H
#define LOGGING_H
/**
 * System debugging log. Uses same format as printf
 * @param tag: logging tag
 * @param format: printf style formatting string
 */
void LOG_D(const char *tag, char *format, ...);

/**
 * System info log. Uses same format as printf
 * @param tag: logging tag
 * @param format: printf style formatting string
 */
void LOG_I(const char *tag, char *format, ...);

/**
 * System warning log. Uses same format as printf
 * @param tag: logging tag
 * @param format: printf style formatting string
 */
void LOG_W(const char *tag, char *format, ...);

/**
 * System error log. Uses same format as printf
 * @param tag: logging tag
 * @param format: printf style formatting string
 */
void LOG_E(const char *tag, char *format, ...);

/**
 * Minimal system log. Useful for tasks with small stacks, or other low memory
 * environments. Does not implement printf style formatting.
 * @param log_level: logging level to use
 * @param tag: Tag to log (NULL terminated)
 * @param logstr: NULL terminated log string
 */
void LOG_MIN(int log_level, const char *tag, const char *logstr);

#endif