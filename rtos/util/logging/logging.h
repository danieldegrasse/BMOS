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

#endif