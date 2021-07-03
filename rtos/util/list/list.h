/**
 * @file list.h
 * Implements a generic doubly linked list
 * Requires memory allocation support
 */

#ifndef LIST_H
#define LIST_H

typedef void *list_t;

/**
 * List iteration function return codes
 */
typedef enum list_return {
    LST_BRK,    /*!< End iteration */
    LST_CONT,   /*!< Continue iteration */
    LST_RM,     /*!< Remove element and continue iteration */
    LST_RM_BRK, /*!< Remove element and cease iteration */
} list_return_t;

/**
 * Appends element to a list
 * @param list: List to append to (if NULL, new list is created)
 * @param elem: element to append to list
 * @return new list on success, or NULL on error
 */
list_t *list_append(list_t *list, void *elem);

/**
 * Prepends element to a list
 * @param list: List to prepend to (if NULL, new list is created)
 * @param elem: element to prepend to list
 * @return new list on success, or NULL on error
 */
list_t *list_prepend(list_t *list, void *elem);

/**
 * Iterates through linked list. If iterator function returns LST_BRK,
 * iteration will cease at that list element
 * @param list: list to iterate over
 * @param itr: iteration function. Will be called with the element being
 * iterated over, and return value dermines if iteration should continue
 * @return last element touched by iteration
 */
void *list_iterate(list_t *list, list_return_t (*itr)(void *));

/**
 * Filters a linked list. Iterator will be called with element in list, and
 * the return value determines if it should be removed, and if iteration should
 * continue.
 * @param itr: iteration function. Will be called with the element being
 * iterated over, and return value dermines if iteration should continue
 * @return last element touched by iteration
 */
void *list_filter(list_t *list, list_return_t (*itr)(void *));

#endif