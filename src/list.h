/*

Copyright (c) 2005-2008, Simon Howard

Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted, provided
that the above copyright notice and this permission notice appear
in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */

/**
 * @file list.h
 *
 * @brief Doubly-linked list.
 *
 * A doubly-linked list stores a collection of values.  Each entry in
 * the list (represented by a pointer a @ref ListEntry structure)
 * contains a link to the next entry and the previous entry.
 * It is therefore possible to iterate over entries in the list in either
 * direction.
 *
 * To create an empty list, create a new variable which is a pointer to
 * a @ref ListEntry structure, and initialise it to NULL.
 * To destroy an entire list, use @ref list_free.
 *
 * To add a value to a list, use @ref list_append or @ref list_prepend.
 *
 * To remove a value from a list, use @ref list_remove_entry or
 * @ref list_remove_data.
 *
 * To iterate over entries in a list, use @ref list_iterate to initialise
 * a @ref ListIterator structure, with @ref list_iter_next and
 * @ref list_iter_has_more to retrieve each value in turn.
 * @ref list_iter_remove can be used to remove the current entry.
 *
 * To access an entry in the list by index, use @ref list_nth_entry or
 * @ref list_nth_data.
 *
 * To modify data in the list use @ref list_set_data.
 *
 * To sort a list, use @ref list_sort.
 *
 */

#ifndef ALGORITHM_LIST_H
#define ALGORITHM_LIST_H

#ifdef __cplusplus
extern "C" {
#endif
/*@-exportlocal@*/

/**
 * Represents an entry in a doubly-linked list.  The empty list is
 * represented by a NULL pointer. To initialise a new doubly linked
 * list, simply create a variable of this type
 * containing a pointer to NULL.
 */

typedef struct _ListEntry ListEntry;

/**
 * Structure used to iterate over a list.
 */

typedef struct _ListIterator ListIterator;

/**
 * A value stored in a list.
 */

typedef void *ListValue;

/**
 * Definition of a @ref ListIterator.
 */

struct _ListIterator {
	/*@notnull@*/ /*@dependent@*/ ListEntry **prev_next;
	/*@null@*/ /*@dependent@*/ ListEntry *current;
};

/**
 * A null @ref ListValue.
 */

#define LIST_NULL ((void *) 0)

/**
 * Callback function used to compare values in a list when sorting.
 *
 * @param value1      The first value to compare.
 * @param value2      The second value to compare.
 * @return            A negative value if value1 should be sorted before
 *                    value2, a positive value if value1 should be sorted
 *                    after value2, zero if value1 and value2 are equal.
 */

typedef int (*ListCompareFunc)(ListValue value1, ListValue value2);

/**
 * Callback function used to determine of two values in a list are
 * equal.
 *
 * @param value1      The first value to compare.
 * @param value2      The second value to compare.
 * @return            A non-zero value if value1 and value2 are equal, zero
 *                    if they are not equal.
 */

typedef int (*ListEqualFunc)(ListValue value1, ListValue value2);

/**
 * Free an entire list.
 *
 * @param list         The list to free.
 */

void list_free(/*@null@*/ /*@only@*/ ListEntry *list);

/**
 * Prepend a value to the start of a list.
 *
 * @param list         Pointer to the list to prepend to.
 * @param data         The value to prepend.
 * @return             The new entry in the list, or NULL if it was not
 *                     possible to allocate the memory for the new entry.
 */

/*@null@*/ /*@only@*/ ListEntry *list_prepend(
		/*@null@*/ /*@temp@*/ /*@special@*/ ListEntry **list, 
		/*@null@*/ /*@shared@*/ ListValue data)
	/*@uses *list@*/
	/*@requires owned *list@*/;

/**
 * Append a value to the end of a list.
 *
 * @param list         Pointer to the list to append to.
 * @param data         The value to append.
 * @return             The new entry in the list, or NULL if it was not
 *                     possible to allocate the memory for the new entry.
 */

/*@null@*/ /*@dependent@*/ ListEntry *list_append(
		/*@null@*/ /*@temp@*/ /*@special@*/ ListEntry **list, 
		/*@null@*/ /*@shared@*/ ListValue data)
	/*@uses *list@*/
	/*@requires owned *list@*/;

/**
 * Retrieve the previous entry in a list.
 *
 * @param listentry    Pointer to the list entry.
 * @return             The previous entry in the list, or NULL if this
 *                     was the first entry in the list.
 */

/*@null@*/ /*@dependent@*/ ListEntry *list_prev(
		/*@null@*/ /*@temp@*/ ListEntry *listentry);

/**
 * Retrieve the next entry in a list.
 *
 * @param listentry    Pointer to the list entry.
 * @return             The next entry in the list, or NULL if this was the
 *                     last entry in the list.
 */

/*@null@*/ /*@dependent@*/ ListEntry *list_next(
		/*@null@*/ /*@temp@*/ ListEntry *listentry);

/**
 * Retrieve the value at a list entry.
 *
 * @param listentry    Pointer to the list entry.
 * @return             The value stored at the list entry.
 */

/*@null@*/ /*@shared@*/ ListValue list_data(
		/*@null@*/ /*@temp@*/ ListEntry *listentry);

/**
 * Set the value at a list entry. The value provided will be written to the 
 * given listentry. If listentry is NULL nothing is done.
 *
 * @param listentry 	Pointer to the list entry.
 * @param value			The value to set.
 */
void list_set_data(
		/*@null@*/ /*@temp@*/ ListEntry *listentry, 
		/*@null@*/ /*@shared@*/ ListValue value);

/**
 * Retrieve the entry at a specified index in a list.
 *
 * @param list       The list.
 * @param n          The index into the list .
 * @return           The entry at the specified index, or NULL if out of range.
 */

/*@null@*/ /*@dependent@*/ ListEntry *list_nth_entry(
		/*@null@*/ /*@temp@*/ ListEntry *list, unsigned int n);

/**
 * Retrieve the value at a specified index in the list.
 *
 * @param list       The list.
 * @param n          The index into the list.
 * @return           The value at the specified index, or @ref LIST_NULL if
 *                   unsuccessful.
 */

/*@null@*/ /*@shared@*/ ListValue list_nth_data(
		/*@null@*/ /*@temp@*/ ListEntry *list, unsigned int n);

/**
 * Find the length of a list.
 *
 * @param list       The list.
 * @return           The number of entries in the list.
 */

unsigned int list_length(/*@null@*/ /*@temp@*/ ListEntry *list);

/**
 * Create a C array containing the contents of a list.
 *
 * @param list       The list.
 * @return           A newly-allocated C array containing all values in the
 *                   list, or NULL if it was not possible to allocate the
 *                   memory.  The length of the array is equal to the length
 *                   of the list (see @ref list_length).
 */

/*@null@*/ /*@only@*/ /*@out@*/ ListValue *list_to_array(
		/*@null@*/ /*@temp@*/ ListEntry *list);

/**
 * Remove an entry from a list.
 *
 * @param list       Pointer to the list.
 * @param entry      The list entry to remove .
 * @return           If the entry is not found in the list, returns zero,
 *                   else returns non-zero.
 */

int list_remove_entry(
		/*@null@*/ /*@temp@*/ /*@special@*/ ListEntry **list, 
		/*@null@*/ /*@dependent@*/ ListEntry *entry)
	/*@uses *list@*/
	/*@requires owned *list@*/;

/**
 * Remove all occurrences of a particular value from a list.
 *
 * @param list       Pointer to the list.
 * @param callback   Function to invoke to compare values in the list
 *                   with the value to be removed.
 * @param data       The value to remove from the list.
 * @return           The number of entries removed from the list.
 */

unsigned int list_remove_data(
		/*@null@*/ /*@temp@*/ /*@special@*/ ListEntry **list, 
		/*@notnull@*/ ListEqualFunc callback,
        /*@null@*/ /*@temp@*/ ListValue data)
    /*@uses *list@*/;

/**
 * Sort a list.
 *
 * @param list          Pointer to the list to sort.
 * @param compare_func  Function used to compare values in the list.
 */

void list_sort(
		/*@null@*/ /*@temp@*/ /*@special@*/ ListEntry **list, 
		/*@notnull@*/ ListCompareFunc compare_func)
	/*@uses *list@*/;

/**
 * Find the entry for a particular value in a list.
 *
 * @param list           The list to search.
 * @param callback       Function to invoke to compare values in the list
 *                       with the value to be searched for.
 * @param data           The value to search for.
 * @return               The list entry of the item being searched for, or
 *                       NULL if not found.
 */

/*@null@*/ /*@dependent@*/ ListEntry *list_find_data(
		/*@null@*/ /*@temp@*/ ListEntry *list,
        /*@notnull@*/ ListEqualFunc callback,
        /*@null@*/ /*@shared@*/ /*@in@*/ ListValue data);

/**
 * Initialise a @ref ListIterator structure to iterate over a list.
 *
 * @param list           A pointer to the list to iterate over.
 * @param iter           A pointer to an iterator structure to initialise.
 */

void list_iterate(
		/*@notnull@*/ /*@dependent@*/ ListEntry **list, 
		/*@notnull@*/ /*@temp@*/ ListIterator *iter);

/**
 * Determine if there are more values in the list to iterate over.
 *
 * @param iterator       The list iterator.
 * @return               Zero if there are no more values in the list to
 *                       iterate over, non-zero if there are more values to
 *                       read.
 */

int list_iter_has_more(/*@notnull@*/ /*@temp@*/ ListIterator *iterator);

/**
 * Using a list iterator, retrieve the next value from the list.
 *
 * @param iterator       The list iterator.
 * @return               The next value from the list, or @ref LIST_NULL if
 *                       there are no more values in the list.
 */

/*@null@*/ /*@shared@*/ ListValue list_iter_next(
		/*@notnull@*/ /*@temp@*/ ListIterator *iterator);

/**
 * Delete the current entry in the list (the value last returned from
 * list_iter_next)
 *
 * @param iterator       The list iterator.
 */

void list_iter_remove(/*@notnull@*/ /*@temp@*/ ListIterator *iterator);

/*@=exportlocal@*/
#ifdef __cplusplus
}
#endif

#endif /* #ifndef ALGORITHM_LIST_H */

