/**
 * @file config.h
 * Configuration parameters for the driver subsystem
 *
 * Defaults can be changed here,
 * or overriden at compile time by passing
 * "CFLAGS=-Dparameter_here=value_here" to the make command
 */

#ifndef CONFIG_H
#define CONFIG_H

/**
 * Configuration constants
 */

/** System exit types */
/** Minimal exit implementation. Spins in a while loop */
#define SYSEXIT_MIN 0
/** Full exit implementation. Logs code to debug output */
#define SYSEXIT_FULL 1

/** System abort types */
/** Minimal abort implementation. Spins in a while loop */
#define SYSABORT_MIN 0
/** Full abort implementation. Logs abort reason to debug output */
#define SYSABORT_FULL 1

/** System heap constants */
/** Empty heap. Disables memory allocation */
#define SYSHEAPSIZE_NONE 0
/** Default heap size. Can be changed */
#define SYSHEAPSIZE_DEFAULT 2048

/** System log types */
/** printf and logging directed to LPUART1, running at 115200 baud and 8n1. */
#define SYSLOG_LPUART1 0
/** printf and logging directed to semihosting system. This requires a debugger
 * with semihosting enabled */
#define SYSLOG_SEMIHOST 1
/** system logging is disabled */
#define SYSLOG_DISABLED 2

/**
 * System log levels
 */
#define SYSLOGLEVEL_DEBUG 0
#define SYSLOGLEVEL_INFO 1
#define SYSLOGLEVEL_WARNING 2
#define SYSLOGLEVEL_ERROR 3

/**
 * System exit type. Set by passing -DSYSEXIT=val
 */
#ifndef SYSEXIT
#define SYSEXIT SYSEXIT_MIN
#endif

/**
 * System abort type. Set by passing -DSYSABORT=val
 */
#ifndef SYSABORT
#define SYSABORT SYSABORT_FULL
#endif

/**
 * System heap size in bytes. Set to 0 to disable memory allocation
 * Set by passing -DSYSHEAPSIZE=val
 */
#ifndef SYSHEAPSIZE
#define SYSHEAPSIZE SYSHEAPSIZE_DEFAULT
#endif

/**
 * System log subsystem. Can use a uart device, or disable system logging.
 * Set by passing -DSYSLOG=val
 */
#ifndef SYSLOG
#define SYSLOG SYSLOG_SEMIHOST
#endif

/**
 * System logging level. Set by passing -DSYSLOGLEVEL=val
 * Any log call with a level below the set level will not be printed.
 */
#ifndef SYSLOGLEVEL
#define SYSLOGLEVEL SYSLOGLEVEL_DEBUG
#endif

/**
 * System log buffer size. Any log string longer than this in bytes will not
 * be rendered properly
 * set with -DSYSLOGBUFSIZE=val
 */
#ifndef SYSLOGBUFSIZE
#define SYSLOGBUFSIZE 256
#endif

#endif