/*

Copyright (c) 2016, Stefan Cloudt

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
 * @file sortedarray.c
 * 
 * @brief File containing the implementation of sortedarray.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sortedarray.h"

#ifdef ALLOC_TESTING
#include "alloc-testing.h"
#endif

/**
 * Definition of a @ref SortedArray
 */
struct _SortedArray {
	/**
	 * This field contains the actual array. The array always has a length
	 * of value of field length.
	 */
	/*@null@*/ /*@owned@*/ SortedArrayValue *data;

	/**
	 * The length of the sorted array.
	 */
	unsigned int length;

	/**
	 * Field for internal usage only indicating how much memory already has
	 * been allocated for *data.
	 */
	unsigned int _alloced;

	/**
	 * The callback used to determine if two values equal.
	 */
	/*@notnull@*/ SortedArrayEqualFunc equ_func;

	/**
	 * The callback use to determine the order of two values.
	 */
	/*@notnull@*/ SortedArrayCompareFunc cmp_func;
};

/* Function for finding first index of range which equals data. An equal value
   must be present. */
static unsigned int sortedarray_first_index(
		/*@notnull@*/ /*@temp@*/ SortedArray *sortedarray,
        /*@null@*/ /*@shared@*/ SortedArrayValue data, unsigned int left,
                                   unsigned int right)
{
	unsigned int index = left;

	while (left < right) {
		int order;

		index = (left + right) / 2;

		/* Data is only null when length == 0, because left >= 0 and right < 
		 * length by def, we have that if data == NULL then this code is not
		 * executed.*/
		/*@-nullderef@*/
		order = sortedarray->cmp_func(data, sortedarray->data[index]);
		/*@=nullderef@*/

		if (order > 0) {
			left = index + 1;
		} else {
			right = index;
		}
	}

	return index;
}

/* Function for finding last index of range which equals data. An equal value
   must be present. */
static unsigned int sortedarray_last_index(
		/*@notnull@*/ /*@temp@*/ SortedArray *sortedarray, 
        /*@null@*/ /*@shared@*/ SortedArrayValue data, unsigned int left, 
                                  unsigned int right)
{
	unsigned int index = right;

	while (left < right) {
		int order;
		index = (left + right) / 2;

		/* Data is only null when length == 0, because left >= 0 and right < 
		 * length by def, we have that if data == NULL then this code is not
		 * executed.*/
		/*@-nullderef@*/
		order = sortedarray->cmp_func(data, sortedarray->data[index]);
		/*@=nullderef@*/

		if (order <= 0) {
			left = index + 1;
		} else {
			right = index;
		}
	}

	return index;
}

/*@null@*/ /*@shared@*/ SortedArrayValue *sortedarray_get(
		/*@null@*/ /*@temp@*/ SortedArray *array, unsigned int i)
{
	if (array == NULL) {
		return NULL;
	}

	/* Do bounds check. */
	if (i < array->length) {
		/* Since i < length we have that length != 0, therefore data != NULL. */
		/*@-nullderef@*/
		return array->data[i];
		/*@=nullderef@*/
	} else {
		return NULL;
	}
}

unsigned int sortedarray_length(/*@null@*/ /*@temp@*/ SortedArray *array)
{
	if (array == NULL) {
		return 0;
	} else {
		return array->length;
	}
}

/*@null@*/ /*@only@*/ SortedArray *sortedarray_new(
		unsigned int length, 
		/*@null@*/ SortedArrayEqualFunc equ_func,
        /*@null@*/ SortedArrayCompareFunc cmp_func)
{
	SortedArrayValue *array;
	SortedArray *sortedarray;

	/* check input requirements */
	if (equ_func == NULL || cmp_func == NULL) {
		return NULL;
	}

	/* If length is 0, set it to a default. */
	if (length == 0) {
		length = 16;
	}

	array = malloc(sizeof(SortedArrayValue) * length);

	/* on failure, return null */
	if (array == NULL) {
		return NULL;
	}

	sortedarray = malloc(sizeof(SortedArray));    

	/* check for failure */
	if (sortedarray == NULL) {
		free(array);
		return NULL;
	}
    
	/* init */
	sortedarray->data = array;
	sortedarray->length = 0;
	sortedarray->_alloced = length;
	sortedarray->equ_func = equ_func;
	sortedarray->cmp_func = cmp_func;

	/* Though sortedarray->data is not defined all elements within the bounds of
	 * [0,length) = [0,0) = emptyset are defined. */
	/*@-compdef@*/
	return sortedarray;
	/*@=compdef@*/
}

void sortedarray_free(/*@null@*/ /*@only@*/ /*@in@*/ SortedArray *sortedarray)
{
	if (sortedarray != NULL) {
		free(sortedarray->data);
		free(sortedarray);
	}
}

void sortedarray_remove(
		/*@null@*/ /*@temp@*/ SortedArray *sortedarray, unsigned int index)
{
	/* same as remove range of length 1 */
	sortedarray_remove_range(sortedarray, index, 1);
}

void sortedarray_remove_range(
		/*@null@*/ /*@temp@*/ SortedArray *sortedarray, 
		unsigned int index,
        unsigned int length)
{
	if (sortedarray == NULL) {
		return;
	}

	/* removal does not violate sorted property */

	/* check if valid range */
	if (index > sortedarray->length || index + length > sortedarray->length) {
		return;
	}

	/* move entries back */
	/* sortedarray->data cannot be null since then length should be 0, if length
	 * = 0 then this code is not reached.*/
	/*@-nullderef@*/
	memmove(&sortedarray->data[index],
	        &sortedarray->data[index + length],
	        (sortedarray->length - (index + length)) 
	              * sizeof(SortedArrayValue));
	/*@=nullderef@*/

	sortedarray->length -= length;
}

int sortedarray_insert(
		/*@null@*/ /*@temp@*/ SortedArray *sortedarray, 
		/*@null@*/ /*@shared@*/ SortedArrayValue data)
{
	unsigned int left, right, index;

	if (sortedarray == NULL) {
		return -1;
	}

	/* do a binary search like loop to find right position */
	left  = 0;
	right = sortedarray->length;
	index = 0;

	/* When length is 1 set right to 0 so that the loop is not entered */	
	right = (right > 1) ? right : 0;

	while (left != right) {
		int order;

		index = (left + right) / 2;

		/* sortedarray->data cannot be null because then length would be 0, for 
		 * which this code would never be executed. */
		/*@-nullderef@*/
		order = sortedarray->cmp_func(data, sortedarray->data[index]);
		/*@=nullderef@*/

		if (order < 0) {
			/* value should be left of index */
			right = index;
		} else if (order > 0) {
			/* value should be right of index */
			left = index + 1;
		} else {
			/* value should be at index */
			break;
		}
	}

	/* look whether the item should be put before or after the index */
	/* Because left to right evaluation and length being > 0 data can't be null
	 * when accessed. */
	/*@-nullderef@*/
	if (sortedarray->length > 0 && sortedarray->cmp_func(data, 
	                       sortedarray->data[index]) > 0) {
		index++;
	}
	/*@=nullderef@*/

	/* insert element at index */
	if (sortedarray->length + 1 > sortedarray->_alloced) {
		/* enlarge the array */
		unsigned int newsize;
		SortedArrayValue *dataarray;

		newsize = sortedarray->_alloced * 2;
		/* sortedarray->data is unless what splint thinks never released by 
		 * realloc since realloc only releases when passed size equals 0 which
		 * cannot be the case. */
		/*@-usereleased-branchstate@*/
		dataarray = realloc(sortedarray->data, 
				sizeof(SortedArrayValue) * newsize);

		if (dataarray == NULL) {
			/* sortedarray->data is ensured to be defined in range [0,length)*/
			/*@-compdef@*/
			return 0;
			/*@=compdef@*/
		} else {
			sortedarray->data = dataarray;
			sortedarray->_alloced = newsize;
		}
	}
	/*@=branchstate@*/

	/* move all other elements */
	/* sortedarray->data can't be null because then length == 0 and this code 
	 * would not be executed.*/
	/*@-nullderef@*/
	memmove(&sortedarray->data[index + 1],
	        &sortedarray->data[index],
	        (sortedarray->length - index) * sizeof(SortedArrayValue));

	/* insert entry */
	sortedarray->data[index] = data;
	++(sortedarray->length);

	/* sortedarray->data is ensured to be defined in range [0,length)*/
	/*@-compdef@*/
	return 1;
	/*@=nullderef=usereleased=compdef@*/
}

int sortedarray_index_of(
		/*@null@*/ /*@temp@*/ SortedArray *sortedarray, 
		/*@null@*/ /*@shared@*/ SortedArrayValue data)
{
	unsigned int left, right, index;
	if (sortedarray == NULL) {
		return -1;
	}
	
	/* do a binary search */
	left = 0;
	right = sortedarray->length;
	index = 0;

	/* safe subtract 1 of right without going negative */
	right = (right > 1) ? right : 0;

	while (left != right) {
		int order;

		index = (left + right) / 2;

		/* sortedarray->data can't be null, length == 0, so code would not be 
		 * reachable. */
		/*@-nullderef@*/
		order = sortedarray->cmp_func(data, sortedarray->data[index]);
		/*@=nullderef@*/

		if (order < 0) {
			/* value should be left */
			right = index;
		} else if (order > 0) {
			/* value should be right */
			left = index + 1;
		} else {
			/* no binary search can be done anymore, 
			   search linear now */
			left = sortedarray_first_index(sortedarray, data, left,
			                               index);
			right = sortedarray_last_index(sortedarray, data, 
			                               index, right);

			for (index = left; index <= right; index++) {
				/* sortedarray->data cant be null, same reason as above.*/
				/*@-nullderef@*/
				if (sortedarray->equ_func(data, 
				                sortedarray->data[index]) > 0) {
					return (int) index;
				}
				/*@=nullderef@*/
			}

			/* nothing is found */
			return -1;
		}
	}

	return -1;
}

void sortedarray_clear(/*@null@*/ /*@temp@*/ SortedArray *sortedarray)
{
	if (sortedarray != NULL) {
		/* set length to 0 */
		sortedarray->length = 0;
	}
}
