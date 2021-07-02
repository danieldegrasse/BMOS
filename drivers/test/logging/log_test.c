/** @file log_test.c
 * Tests system logging and printf implementations
 * This test, when successful, should log to the defined system console,
 * which will be one of the following depending on the chosen logger:
 * SYSLOG_LPUART1: LPUART1, hooked up to the USB-serial converter on dev board
 * SYSLOG_SEMIHOST: system debugger console (semihosting must be enabled)
 * SYSLOG_SWO: system swo output (swo must be enabled)
 * SYSLOG_DISABLED: logging should not occur
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <config.h>
#include <sys/clock.h>
#include <util/logging.h>

/**
 * Initializes system
 */
static void system_init() {
    clock_cfg_t clk_cfg = CLOCK_DEFAULT_CONFIG;
    clock_init(&clk_cfg);
}

/**
 * Logs a test string until the system device is forced to flush naturally
 */
static syserr_t test1_naturallog() {
    int iterations, ret, log_strlen;
    char log_str[] = "abcdefghijklmnopqrstuvwxyz\n";
    log_strlen = sizeof(log_str) - 1;
    if (SYSLOGBUFSIZE) {
        iterations = (SYSLOGBUFSIZE / (sizeof(log_str) - 1)) + 1;
    } else {
        // If no log buffer exists, this test can pass without logging data
        iterations = 0;
    }
    while (iterations--) {
        // Log output string
        ret = printf(log_str);
        if (ret != log_strlen) {
            return ERR_FAIL;
        }
    }
    ret = printf("---- TEST 1 Passed! ----- \n");
    if (ret != 27) {
        // Test 1 didn't actually pass....
        while (1)
            ; // Spin so debugger catches here
    }
    return SYS_OK;
}

/**
 * Logs a string, and then forces it to flush
 */
static syserr_t test2_forcedflush() {
    int ret;
    ret = printf(
        "This test string should print several seconds before the next one\n");
    if (ret != 66) {
        return ERR_FAIL;
    }
    if (fsync(STDOUT_FILENO) != 0) {
        return ERR_FAIL;
    }
    delay_ms(2000);
    ret = printf("This is the second string\n");
    if (ret != 26) {
        return ERR_FAIL;
    }
    ret = printf("---- TEST 2 Passed! ----- \n");
    if (ret != 27) {
        // Test 2 didn't actually pass....
        while (1)
            ; // Spin so debugger catches here
    }
    return SYS_OK;
}

/**
 * Tests system log level to be sure that only those logs of a given priority
 * print
 */
static syserr_t test3_loglevel() {
    printf("This test logs output with various debugging levels\n");
    printf("Your current logging level is %d\n", SYSLOGLEVEL);
    LOG_E("This message should be visible if %i>=%i", SYSLOGLEVEL_ERROR,
          SYSLOGLEVEL);
    LOG_W("This message should be visible if %i>=%i", SYSLOGLEVEL_WARNING,
          SYSLOGLEVEL);
    LOG_I("This message should be visible if %i>=%i", SYSLOGLEVEL_INFO,
          SYSLOGLEVEL);
    LOG_D("This message should be visible if %i>=%i", SYSLOGLEVEL_DEBUG,
          SYSLOGLEVEL);
    printf("---- Test 3 Complete -----\n");
    fsync(STDIN_FILENO);
    return SYS_OK;
}

/**
 * This test logs a variety of strings to the output, deliberately testing
 * flushing the output early as well as logging strings that will require it
 * to be flushed.
 */
int main() {
    system_init();
    /**
     * Log a series of strings, to force the output to flush naturally
     */
    printf("This is the system logging test\n");
    printf("You should be seeing these strings logged to your selected logging "
           "device\n");
    printf("If any string appears truncated, the test likely failed\n");
    printf("----- TEST 1: Natural Flush --------\n");
    if (test1_naturallog() != SYS_OK) {
        LOG_E("Natural log flushing test failed!\n");
        exit(ERR_FAIL);
    }
    // Test fsync
    if (fsync(STDOUT_FILENO) != 0) {
        LOG_E("fsync() does not work");
        exit(ERR_FAIL);
    }
    printf("----- TEST 2: Forced flush -------\n");
    if (test2_forcedflush() != SYS_OK) {
        LOG_E("Forced flush test failed");
        exit(ERR_FAIL);
    }
    // Test log levels
    printf("----- TEST 3: Log Levels -------\n");
    if (test3_loglevel() != SYS_OK) {
        LOG_E("Log level tests failed");
        exit(ERR_FAIL);
    }
    printf("All tests completed\n");
    return SYS_OK;
}