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

#include <drivers/clock/clock.h>
#include <util/list/list.h>
#include <util/logging/logging.h>

/**
 * @file list_test.c
 * This file verifies the implementation of lists within the RTOS
 * It does so by creating a list, appending elements, and then verifying
 * their order as well as removing them
 */

static char *TAG = "list_test";

/**
 * Initializes system
 */
static void system_init() {
    clock_cfg_t clk_cfg = CLOCK_DEFAULT_CONFIG;
    clock_init(&clk_cfg);
}

/**
 * Prints entries of list
 */
static list_return_t print_iterator(void *elem) {
    if (elem == NULL) {
        // Crash. Should not occur.
        LOG_E(TAG, "print interator failed: null value");
        exit(ERR_FAIL);
    }
    char *ch = (char *)elem;
    printf("%c", *ch);
    return LST_CONT; // Iterate further in list
}

/**
 * Finds the first 'D' character in a list
 */
static list_return_t find_first_D(void *elem) {
    if (elem == NULL) {
        // Crash. Should not occur.
        LOG_E(TAG, "print interator failed: null value");
        exit(ERR_FAIL);
    }
    char *ch = (char *)elem;
    if (*ch == 'D') {
        return LST_BRK;
    } else {
        return LST_CONT;
    }
}

/**
 * Base RTOS testing point
 */
int main() {
    system_init();
    int i;
    void *ret;
    char data[] = "Test Data elements";
    // Make list
    list_t list = NULL;
    for (i = 0; i < sizeof(data) - 1; i++) {
        // Append to list
        list = list_append(list, &data[i]);
        if (list == NULL) {
            LOG_E(TAG, "List return value was null");
            exit(ERR_FAIL);
        }
    }
    // Print list entries
    printf("Test 1: Valid list creation\n"
           "Expected printout: %s\n"
           "Actual printout: ",
           data);
    ret = list_iterate(list, print_iterator);
    printf("\n");
    if ((char *)ret != &data[i - 1]) {
        // returned value should have been last list entry
        LOG_E(TAG, "Iterator has bad return value. Expected %p, got %p",
              &data[i], ret);
        exit(ERR_FAIL);
    }
    // Test list prepending
    list = list_prepend(list, &data[sizeof(data) - 1]);
    if (list == NULL) {
        LOG_E(TAG, "List return value was null");
        exit(ERR_FAIL);
    }
    // Print list entries
    printf("Test 2: Valid list prepend\n"
           "Expected printout: %c%s\n"
           "Actual printout: ",
           data[sizeof(data) - 1], data);
    ret = list_iterate(list, print_iterator);
    printf("\n");
    if ((char *)ret != &data[i - 1]) {
        // returned value should have been last list entry
        LOG_E(TAG, "Iterator has bad return value. Expected %p, got %p",
              &data[i - 1], ret);
        exit(ERR_FAIL);
    }
    // Verify that list iteration can find a value
    printf("Test 3: valid list iteration\n");
    ret = list_iterate(list, find_first_D);
    if ((char *)ret != &data[5]) {
        LOG_E(TAG, "Test 3 failed");
        exit(ERR_FAIL);
    } else {
        printf("Test 3 Passed\n");
    }
    // Verify that list filtering functions
    printf("Test 4: list removal\n");
    ret = list_remove(list, find_first_D);
    if ((char *)ret != &data[5]) {
        LOG_E(TAG, "Test 4 failed");
        exit(ERR_FAIL);
    } else {
        printf("Test 4 Passed\n");
    }
    printf("List contents: ");
    // Verify that list does not have first D
    ret = list_iterate(list, print_iterator);
    if ((char *)ret != &data[i - 1]) {
        // returned value should have been last list entry
        LOG_E(TAG, "Iterator has bad return value. Expected %p, got %p",
              &data[i - 1], ret);
        exit(ERR_FAIL);
    }
    printf("\n");
    // Verify that list can handle another append
    printf("Test 5: List append after remove\n");
    list = list_append(list, &data[5]);
    if (list == NULL) {
        LOG_E(TAG, "List return value was null");
        exit(ERR_FAIL);
    } else {
        printf("Test 5 passed\nList contents: ");
        ret = list_iterate(list, print_iterator);
        if ((char *)ret != &data[5]) {
            // returned value should have been last list entry
            LOG_E(TAG, "Iterator has bad return value. Expected %p, got %p",
                  &data[i - 1], ret);
            exit(ERR_FAIL);
        }
        printf("\n");
    }
    printf("If expected outputs matched actual, all tests passed\n");
    return SYS_OK;
}