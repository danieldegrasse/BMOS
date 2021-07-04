/**
 * @file list.h
 * Implements a generic doubly linked list
 * This list stores the state of entries within a list_state_t
 * structure. Each element added to the list should have some form of
 * list_state_t structure associated with it.
 *
 * For example, this structure would store well in the list:
 * struct example {
 *      void *data;
 *      list_state_t state;
 * };
 *
 * This call to list_append would create a new list with "ex" as the only
 * member: 
 * list_t alist; 
 * struct example ex; 
 * ex->data = buffer; // buffer definition emitted for clarity 
 * alist = list_append(NULL, &ex, ex.state);
 */

#ifndef LIST_H
#define LIST_H

typedef void *list_t;

/**
 * Internal list state structure. Do NOT manipule these fields.
 * Declared in header file so that compiler knows type size
 */
typedef struct list_state {
    void* _container;
    struct list_state* _next;
    struct list_state* _prev; 
} list_state_t;

/**
 * List iteration function return codes
 */
typedef enum list_return {
    LST_BRK,  /*!< End iteration */
    LST_CONT, /*!< Continue iteration */
} list_return_t;

/**
 * Appends element to a list
 * @param list: List to append to (if NULL, new list is created)
 * @param elem: element to append to list
 * @param state: list element state. Should be associated with elem.
 * @return new list on success, or NULL on error
 */
list_t list_append(list_t list, void *elem, list_state_t* state);

/**
 * Prepends element to a list
 * @param list: List to prepend to (if NULL, new list is created)
 * @param elem: element to append to list
 * @param state: list element state. Should be associated with elem.
 * @return new list on success, or NULL on error
 */
list_t list_prepend(list_t list, void *elem, list_state_t* state);

/**
 * Iterates through linked list. If iterator function returns LST_BRK,
 * iteration will cease at that list element
 * @param list: list to iterate over
 * @param itr: iteration function. Will be called with the element being
 * iterated over, and return value dermines if iteration should continue
 * @return last list entry touched by iteration
 */
void* list_iterate(list_t list, list_return_t (*itr)(void *));

/**
 * Remove the provided list_state_t from the list
 * @param list: list to remove from
 * @param target: list element state to remove
 * @return new list on success, or NULL on error. NULL is also returned if
 * list is empty.
 */
list_t list_remove(list_t list, list_state_t* target);


/**
 * Gets the head of a list without removing it
 * @param list: list to get head of
 * @return pointer to head element of list
 */
void* list_get_head(list_t list);

/**
 * Gets the tail of a list without removing it
 * @param list: list to get tail of
 * @return pointer to tail element of list
 */
void* list_get_tail(list_t list);

#endif