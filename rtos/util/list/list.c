/**
 * @file list.h
 * Implements a generic doubly linked list
 * Requires memory allocation support
 */

#include <stdbool.h>
#include <stdlib.h>
#include <util/logging/logging.h>

#include "list.h"

// Internal functions
static list_t list_add(list_t list, void *elem, list_state_t *state,
                       bool prepend);

/**
 * Appends element to a list
 * @param list: List to append to (if NULL, new list is created)
 * @param elem: element to append to list
 * @param state: list element state. Should be associated with elem.
 * @return new list on success, or NULL on error
 */
list_t list_append(list_t list, void *elem, list_state_t *state) {
    return list_add(list, elem, state, false);
}

/**
 * Prepends element to a list
 * @param list: List to prepend to (if NULL, new list is created)
 * @param elem: element to append to list
 * @param state: list element state. Should be associated with elem.
 * @return new list on success, or NULL on error
 */
list_t list_prepend(list_t list, void *elem, list_state_t *state) {
    return list_add(list, elem, state, true);
}
/**
 * Iterates through linked list. If iterator function returns LST_BRK,
 * iteration will cease at that list element
 * @param list: list to iterate over
 * @param itr: iteration function. Will be called with the element being
 * iterated over, and return value dermines if iteration should continue
 * LST_CONT: iteration continues
 * LST_BRK: iteration ends on this element
 * LST_REM: unused
 * @return last list entry touched by iteration
 */
void *list_iterate(list_t list, list_return_t (*itr)(void *)) {
    list_state_t *head, *current;
    list_return_t ret;
    // Check parameters
    if (list == NULL || itr == NULL) {
        return NULL;
    }
    /**
     * Iterate through the linked list until return value is LST_BRK or
     * we touch the entire list
     */
    head = current = (list_state_t *)list;
    do {
        // Call itr with the data stored by list entry
        ret = itr(current->_container);
        current = current->_next;
    } while (ret == LST_CONT && current != head);
    /**
     * Return data in entry before current (last one itr was called with data
     * from)
     */
    return current->_prev->_container;
}

/**
 * Filters a linked list, using "itr" to determine if elements should be removed
 * @param list: list to filter
 * @param itr: iterator function. Return value of LST_CONT continues, LST_BRK
 * stops iteration, and LST_REM removes the current element from the list and
 * continues iteration.
 * @param destructor: Function called with each element that the iterator
 * returns LST_REM for. Allows caller to free list elements.
 * @return new list after modification (or NULL on error/empty list)
 */
list_t list_filter(list_t list, list_return_t (*itr)(void *),
                   void (*destructor)(void *)) {
    list_state_t *head, *current, *remove;
    list_return_t ret;
    // Check parameters
    if (list == NULL || itr == NULL || destructor == NULL) {
        return NULL;
    }
    // Iterate through the list, until we cycle back around to the head.
    head = current = (list_state_t *)list;
    do {
        // Call iterator
        ret = itr(current->_container);
        if (ret == LST_REM) {
            remove = current;
            current = current->_next;
            list = list_remove(list, remove);
            destructor(remove->_container);
            if (remove == head) {
                /**
                 * Removed head. Call this function with new list.
                 * Parameters check will handle base case where list is NULL
                 */
                return list_filter(list, itr, destructor);
            }
        } else {
            // Assign current to next entry
            current = current->_next;
        }
    } while (head != current && ret != LST_BRK);
    return list;
}

/**
 * Remove the provided list_state_t from the list
 * @param list: list to remove from
 * @param target: list element state to remove
 * @return new list on success, or NULL on error
 */
list_t list_remove(list_t list, list_state_t *target) {
    list_state_t *head;
    // Check parameters
    if (list == NULL || target == NULL) {
        return NULL;
    }
    head = (list_state_t *)list;
    if (target->_next == target) {
        // List has one entry. Return null for empty list.
        head = NULL;
    }
    if (target == head) {
        // Move head to next entry
        head = head->_next;
    }
    // Remove target
    target->_prev->_next = target->_next;
    target->_next->_prev = target->_prev;
    // Clear rem's references
    target->_next = target->_prev = NULL;
    // Return new head
    return head;
}

/**
 * Gets the head of a list without removing it
 * @param list: list to get head of
 * @return pointer to head element of list, or NULL for empty list
 */
void *list_get_head(list_t list) {
    // Simply return data in list head
    return list == NULL ? list : ((list_state_t *)list)->_container;
}

/**
 * Gets the tail of a list without removing it
 * @param list: list to get tail of
 * @return pointer to tail element of list, or NULL for empty list
 */
void *list_get_tail(list_t list) {
    // Get the tail from the prev ref of the head, return its data
    return list == NULL ? list : ((list_state_t *)list)->_prev->_container;
}

/**
 * Adds to a list. Since list_prepend and list_append only differ in which entry
 * of the circular linked list they designate as "head", this function prevents
 * code duplication
 * @param list: list to modify
 * @param elem: element to add
 * @param state: element state structure
 * @return new head of list
 */
static list_t list_add(list_t list, void *elem, list_state_t *state,
                       bool prepend) {
    list_state_t *head, *tail;
    // Check parameters
    if (state == NULL || elem == NULL) {
        return NULL;
    }
    // Associate the element with the container field of state
    state->_container = elem;
    if (list == NULL) {
        // Single entry circular linked list
        state->_prev = state->_next = state;
        // Return the state data as the list reference
        return state;
    } else {
        // Place state data in list.
        head = (list_state_t *)list;
        tail = head->_prev;
        head->_prev = state;
        state->_next = head;
        state->_prev = tail;
        tail->_next = state;
        if (prepend) {
            // The new head should be state
            return state;
        } else {
            // keep current head, making state_data the tail
            return head;
        }
    }
}