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

/** System heap constants */
/** Empty heap. Disables memory allocation */
#define SYSHEAPSIZE_NONE 0
/** Default heap size. Can be changed */
#define SYSHEAPSIZE_DEFAULT 8192

/** System log types */
/** printf and logging directed to LPUART1, running at 115200 baud and 8n1. */
#define SYSLOG_LPUART1 0
/** printf and logging directed to semihosting system. This requires a debugger
 * with semihosting enabled */
#define SYSLOG_SEMIHOST 1
/** System logging is done via SWO pin. This requires a debugger that supports
 * SWO */
#define SYSLOG_SWO 2
/** system logging is disabled */
#define SYSLOG_DISABLED 3

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
#define SYSLOG SYSLOG_SWO
#endif

/**
 * System logging level. Set by passing -DSYSLOGLEVEL=val
 * Any log call with a level below the set level will not be printed.
 */
#ifndef SYSLOGLEVEL
#define SYSLOGLEVEL SYSLOGLEVEL_DEBUG
#endif

/**
 * System log buffer size. The system will log to the buffer, and periodically
 * flush it to the output. If output flushing is desired, call
 * fsync(STDOUT_FILENO). This is only used for semihosting.
 * Set by passing -DSYSLOGBUFSIZE=val
 */
#ifndef SYSLOGBUFSIZE
#define SYSLOGBUFSIZE 512
#endif


#endif