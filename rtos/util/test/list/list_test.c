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

struct list_entry {
    char *data;
    list_state_t state;
};

static char *TAG = "list_test";
static char data[] = "Test Data elements";
static struct list_entry elements[sizeof(data) + 2];

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
    char *ch = ((struct list_entry *)elem)->data;
    printf("%c", *ch);
    return LST_CONT; // Iterate further in list
}

/**
 * Removes all list elements that are a T or t
 */
static list_return_t remove_t(void *elem) {
    if (elem == NULL) {
        // Crash. Should not occur.
        LOG_E(TAG, "print interator failed: null value");
        exit(ERR_FAIL);
    }
    char *ch = ((struct list_entry *)elem)->data;
    if (*ch == 'T' || *ch == 't') {
        return LST_REM;
    } else {
        return LST_CONT;
    }
}

/**
 * Finds the first 'D' character in a list
 */
static list_return_t find_first_D(void *elem) {
    if (elem == NULL) {
        // Crash. Should not occur.
        LOG_E(TAG, "remove interator failed: null value");
        exit(ERR_FAIL);
    }
    char *ch = ((struct list_entry *)elem)->data;
    if (*ch == 'D') {
        return LST_BRK;
    } else {
        return LST_CONT;
    }
}

/**
 * Dummy function to free resources of elements added to list
 */
void destructor(void *elem) {
    if (elem == NULL) {
        // Crash. Should not occur.
        LOG_E(TAG, "destructor interator failed: null value");
        exit(ERR_FAIL);
    }
    char *ch = ((struct list_entry *)elem)->data;
    if (*ch != 'T' && *ch != 't') {
        LOG_E(TAG, "destructor was asked to free the wrong entry");
        exit(ERR_FAIL);
    }
}

/**
 * List test function
 */
int main() {
    system_init();
    struct list_entry *ret;
    int i;
    // Make list
    list_t list = NULL;
    for (i = 0; i < sizeof(data) - 1; i++) {
        // Populate list entry
        elements[i].data = data + i;
        // Append to list
        list = list_append(list, (elements + i), &elements[i].state);
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
    if ((struct list_entry *)ret != &elements[i - 1]) {
        // returned value should have been last list entry
        LOG_E(TAG, "Iterator has bad return value. Expected %p, got %p",
              &elements[i - 1], ret);
        exit(ERR_FAIL);
    }
    // Test list prepending
    elements[sizeof(data)].data = &data[0];
    list = list_prepend(list, &elements[sizeof(data)],
                        &elements[sizeof(data)].state);
    if (list == NULL) {
        LOG_E(TAG, "List return value was null");
        exit(ERR_FAIL);
    }
    // Print list entries
    printf("Test 2: Valid list prepend\n"
           "Expected printout: %c%s\n"
           "Actual printout: ",
           data[0], data);
    ret = list_iterate(list, print_iterator);
    printf("\n");
    if (ret != &elements[i - 1]) {
        // returned value should have been last list entry
        LOG_E(TAG, "Iterator has bad return value. Expected %p, got %p",
              &elements[i - 1], ret);
        exit(ERR_FAIL);
    }
    // Verify that list iteration can find a value
    printf("Test 3: valid list iteration\n");
    ret = list_iterate(list, find_first_D);
    if (ret != &elements[5]) {
        LOG_E(TAG, "Test 3 failed");
        exit(ERR_FAIL);
    } else {
        printf("Test 3 Passed\n");
    }
    // Verify that list filtering functions
    printf("Test 4: list removal\n");
    list = list_remove(list, &(ret->state));
    if (list == NULL) {
        LOG_E(TAG, "Test 4 failed");
        exit(ERR_FAIL);
    } else {
        printf("Test 4 Passed\n");
    }
    printf("List contents: ");
    // Verify that list does not have first D
    ret = list_iterate(list, print_iterator);
    if ((struct list_entry *)ret != &elements[i - 1]) {
        // returned value should have been last list entry
        LOG_E(TAG, "Iterator has bad return value. Expected %p, got %p",
              &elements[i - 1], ret);
        exit(ERR_FAIL);
    }
    printf("\n");
    // Verify that list can handle another append
    printf("Test 5: List append after remove\n");
    list = list_append(list, &elements[5], &elements[5].state);
    if (list == NULL) {
        LOG_E(TAG, "List return value was null");
        exit(ERR_FAIL);
    } else {
        printf("Test 5 passed\nList contents: ");
        ret = list_iterate(list, print_iterator);
        if ((struct list_entry *)ret != &elements[5]) {
            // returned value should have been last list entry
            LOG_E(TAG, "Iterator has bad return value. Expected %p, got %p",
                  &elements[5], ret);
            exit(ERR_FAIL);
        }
        printf("\n");
    }
    printf("Test 6: Removing Ts. If the list printed has any 'T' or 't's in\n"
           "it, this test failed\nList Contents:\n");
    list = list_filter(list, remove_t, destructor);
    if (list == NULL) {
        LOG_E(TAG, "Test 6 failed\n");
        exit(ERR_FAIL);
    }
    ret = list_iterate(list, print_iterator);
    if (ret != &elements[5]) {
        LOG_E(TAG, "Test 6 failed");
        exit(ERR_FAIL);
    }
    printf("\n");
    printf(
        "Test 7: Removing all elements\n"
        "This test should print out the list contents as they are removed\n");
    i = 0;
    while (list != NULL) {
        ret = list_get_head(list);
        printf("%c", *((char *)ret->data));
        list = list_remove(list, &(ret->state));
        i++;
    }
    printf("\n");
    if (i == 14) {
        printf("Test 7 passed\n");
    } else {
        printf("Test 7 failed\n");
    }
    printf("If expected outputs matched actual, all tests passed\n");
    return SYS_OK;
}