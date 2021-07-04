/**
 * @file list.h
 * Implements a generic doubly linked list
 * Requires memory allocation support
 */

#include <stdlib.h>
#include <util/logging/logging.h>

#include "list.h"

/**
 * Internal list structure
 */
typedef struct list_data {
    void *data;             /*!< list element */
    struct list_data *prev; /*!< previous element */
    struct list_data *next; /*!< next element */
} list_data_t;

/**
 * Appends element to a list
 * @param list: List to append to (if NULL, new list is created)
 * @param elem: element to append to list
 * @return new list on success, or NULL on error
 */
list_t list_append(list_t list, void *elem) {
    list_data_t *tail, *entry, *list_ptr, *head;
    list_ptr = (list_data_t *)list;
    // Check parameters
    if (elem == NULL) {
        return NULL;
    }
    // Make new list element
    entry = malloc(sizeof(list_t));
    if (entry == NULL) {
        LOG_E("%s::%i out of memory", __FILE__, __LINE__);
        return NULL;
    }
    entry->data = elem;
    if (list == NULL) {
        // Creating new list
        entry->next = entry;
        entry->prev = entry;
        return entry;
    } else {
        // Get tail of list
        tail = list_ptr->prev;
        head = list_ptr;
        // Add to tail
        tail->next = entry;
        entry->prev = tail;
        entry->next = head;
        head->prev = entry;
        return list;
    }
}

/**
 * Prepends element to a list
 * @param list: List to prepend to (if NULL, new list is created)
 * @param elem: element to prepend to list
 * @return new list on success, or NULL on error
 */
list_t list_prepend(list_t list, void *elem) {
    list_data_t *entry, *list_ptr, *head, *tail;
    list_ptr = (list_data_t *)list;
    // Check paramters
    if (elem == NULL) {
        return NULL;
    }
    // Allocate new list element
    entry = malloc(sizeof(list_t));
    if (entry == NULL) {
        LOG_E("%s::%i out of memory", __FILE__, __LINE__);
        return NULL;
    }
    entry->data = elem;
    if (list == NULL) {
        // Making a new list
        return entry;
    } else {
        // Get head and tail
        head = list_ptr;
        tail = list_ptr->prev;
        // Add to head of list
        tail->next = entry;
        entry->prev = tail;
        entry->next = head;
        head->prev = entry;
        return entry;
    }
}

/**
 * Iterates through linked list. If iterator function returns LST_BRK,
 * iteration will cease at that list element
 * @param list: list to iterate over
 * @param itr: iteration function. Will be called with the element being
 * iterated over, and return value dermines if iteration should continue
 * @return contents of last list entry touched by iteration
 */
void *list_iterate(list_t list, list_return_t (*itr)(void *)) {
    list_data_t *head, *current;
    list_return_t ret;
    head = (list_data_t *)list;
    current = head;
    do {
        // Call iterator
        ret = itr(current->data);
        current = current->next;
    } while (ret == LST_CONT && current != head);
    // Return data in element before current, as this is the last one touched
    return current->prev->data;
}

/**
 * Iterates over a list and removes the first element the iterator function
 * returns LST_BRK for 
 * @param itr: iteration function. Will be called with the element being
 * iterated over, and return value dermines if iteration should continue
 * @return last element touched by iteration
 */
void *list_remove(list_t list, list_return_t (*itr)(void *)) {
    list_data_t *head, *current;
    list_return_t ret;
    void *elem;
    head = (list_data_t *)list;
    current = head;
    do {
        ret = itr(current->data);
        if (ret == LST_BRK) {
            // Remove this element from the list
            elem = current->data;
            current->prev->next = current->next;
            current->next->prev = current->prev;
            free(current);
            return elem; 
        }
        current = current->next;
    } while (current != head);
    return current->prev->data;
}
