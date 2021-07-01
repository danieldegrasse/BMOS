/**
 * @file config.h
 * Configuration parameters for the driver subsystem
 *
 * Defaults can be changed here,
 * or overriden at compile time by passing
 * "-Dparameter_here=value_here" to the compiler
 */

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
#define SYSABORT_MIN 1

/** System heap constants */
/** Empty heap. Disables memory allocation */
#define SYSHEAPSIZE_NONE 0
/** Default heap size. Can be changed */
#define SYSHEAPSIZE_DEFAULT 2048


/** System log types */
/** printf and logging directed to LPUART1, running at 115200 baud and 8n1. */
#define SYSLOG_LPUART1 0
/** system logging is disabled */
#define SYSLOG_DISABLED 1

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


#ifndef SYSLOG
#define SYSLOG SYSLOG_LPUART1
#endif
