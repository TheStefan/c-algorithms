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

/* Hash table implementation */

#include <stdlib.h>
#include <string.h>

#include "hash-table.h"

/* malloc() / free() testing */

#ifdef ALLOC_TESTING
#include "alloc-testing.h"
#endif

struct _HashTableEntry {
	/*@notnull@*/ /*@only@*/ HashTablePair pair;
	/*@null@*/ /*@only@*/ HashTableEntry *next;
};

struct _HashTable {
	/*@notnull@*/ /*@only@*/ HashTableEntry **table;
	unsigned int table_size;
	/*@notnull@*/ HashTableHashFunc hash_func;
	/*@notnull@*/ HashTableEqualFunc equal_func;
	/*@null@*/ HashTableKeyFreeFunc key_free_func;
	/*@null@*/ HashTableValueFreeFunc value_free_func;
	unsigned int entries;
	unsigned int prime_index;
};

/* This is a set of good hash table prime numbers, from:
 *   http://planetmath.org/encyclopedia/GoodHashTablePrimes.html
 * Each prime is roughly double the previous value, and as far as
 * possible from the nearest powers of two. */

static const unsigned int hash_table_primes[] = {
	193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
	196613, 393241, 786433, 1572869, 3145739, 6291469,
	12582917, 25165843, 50331653, 100663319, 201326611,
	402653189, 805306457, 1610612741,
};

static const unsigned int hash_table_num_primes
	= (unsigned int) (sizeof(hash_table_primes) / sizeof(int));

/* Internal function used to allocate the table on hash table creation
 * and when enlarging the table */

static int hash_table_allocate_table(
		/*@notnull@*/ /*@temp@*/ /*@special@*/ HashTable *hash_table)
	/*@defines hash_table->table, hash_table->table_size@*/
{
	unsigned int new_table_size;

	/* Determine the table size based on the current prime index.
	 * An attempt is made here to ensure sensible behavior if the
	 * maximum prime is exceeded, but in practice other things are
	 * likely to break long before that happens. */

	if (hash_table->prime_index < hash_table_num_primes) {
		new_table_size = hash_table_primes[hash_table->prime_index];
	} else {
		new_table_size = hash_table->entries * 10;
	}

	hash_table->table_size = new_table_size;

	/* Allocate the table and initialise to NULL for all entries */

	hash_table->table = calloc((size_t) hash_table->table_size,
	                           sizeof(HashTableEntry *));

	return (int) (hash_table->table != NULL);
}

/* Free an entry, calling the free functions if there are any registered */

static void hash_table_free_entry(
		/*@notnull@*/ /*@temp@*/ HashTable *hash_table, 
		/*@notnull@*/ /*@only@*/ HashTableEntry *entry)
{
	HashTablePair *pair;

	pair = &(entry->pair);

	/* If there is a function registered for freeing keys, use it to free
	 * the key */


	/* Turned off warning because the memory model is different depending on
	 * whether or not key_free_func is defined.*/
	/*@-dependenttrans-branchstate@*/
	if (hash_table->key_free_func != NULL) {
		hash_table->key_free_func(pair->key);
	}
	/*@=dependenttrans=branchstate@*/

	/* Likewise with the value */

	/*@-dependenttrans-branchstate@*/
	if (hash_table->value_free_func != NULL) {
		hash_table->value_free_func(pair->value);
	}
	/*@=dependenttrans=branchstate@*/

	/* Free the data structure */

	free(entry);
}

/*@null@*/ /*@only@*/ HashTable *hash_table_new(
		/*@notnull@*/ HashTableHashFunc hash_func,
        /*@notnull@*/ HashTableEqualFunc equal_func)
{
	HashTable *hash_table;

	/* Allocate a new hash table structure */

	hash_table = (HashTable *) malloc(sizeof(HashTable));

	if (hash_table == NULL) {
		return NULL;
	}

	hash_table->hash_func = hash_func;
	hash_table->equal_func = equal_func;
	hash_table->key_free_func = NULL;
	hash_table->value_free_func = NULL;
	hash_table->entries = 0;
	hash_table->prime_index = 0;

	/* Allocate the table */

	if (hash_table_allocate_table(hash_table) <= 0) {
		/*@-compdestroy@*/
		free(hash_table);
		/*@=compdestroy@*/

		return NULL;
	}

	return hash_table;
}

void hash_table_free(/*@null@*/ /*@only@*/ /*@in@*/ HashTable *hash_table)
{
	HashTableEntry *rover;
	HashTableEntry *next;
	unsigned int i;

	if (hash_table == NULL) {
		return;
	}

	/* Free all entries in all chains */

	for (i=0; i<hash_table->table_size; ++i) {
		rover = hash_table->table[i];
		while (rover != NULL) {
			next = rover->next;
			hash_table_free_entry(hash_table, rover);
			rover = next;
		}
	}

	/* Free the table */

	free(hash_table->table);

	/* Free the hash table structure */

	free(hash_table);

	/* Rover is NULL because when the loop terminates rover == NULL */
	/*@-mustfreeonly@*/
}
/*@=mustfreeonly@*/

void hash_table_register_free_functions(
		/*@notnull@*/ /*@temp@*/ HashTable *hash_table,
        /*@null@*/ HashTableKeyFreeFunc key_free_func,
        /*@null@*/ HashTableValueFreeFunc value_free_func)
{
	hash_table->key_free_func = key_free_func;
	hash_table->value_free_func = value_free_func;
}


static int hash_table_enlarge(/*@notnull@*/ /*@temp@*/ HashTable *hash_table)
{
	HashTableEntry **old_table;
	unsigned int old_table_size;
	unsigned int old_prime_index;
	HashTableEntry *rover;
	HashTablePair *pair;
	HashTableEntry *next;
	unsigned int index;
	unsigned int i;

	/* Store a copy of the old table */

	old_table = hash_table->table;
	old_table_size = hash_table->table_size;
	old_prime_index = hash_table->prime_index;

	/* Allocate a new, larger table */

	++hash_table->prime_index;

	/* Last reference is not lost, it is in old_table. */
	/*@-mustfreeonly@*/
	if (hash_table_allocate_table(hash_table) <= 0) {
		/*@=mustfreeonly@*/

		/* Failed to allocate the new table */

		hash_table->table = old_table;
		hash_table->table_size = old_table_size;
		hash_table->prime_index = old_prime_index;

		/*@-compmempass@*/
		return 0;
		/*@=compmempass@*/
	}

	/* Link all entries from all chains into the new table */

	for (i=0; i<old_table_size; ++i) {
		rover = old_table[i];

		while (rover != NULL) {
			next = rover->next;

			/* Fetch rover HashTablePair */

			pair = &(rover->pair);

			/* Find the index into the new table */

			index = hash_table->hash_func(pair->key) % hash_table->table_size;

			/* Link this entry into the chain */

			/*@-unqualifiedtrans@*/
			rover->next = hash_table->table[index];
			hash_table->table[index] = rover;
			/*@=unqualifiedtrans@*/

			/* Advance to next in the chain */

			rover = next;
		}
	}

	/* Free the old table */

	free(old_table);

	/* hash_table->table is not dangling because hash_table_allocate_table has 
	 * allocated a live pointer. 
	 * hash_table->table is defined by func hash_table_allocate_table. */
	/*@-usereleased-compdef@*/
	return 1;
	/*@=usereleased=compdef@*/
}

int hash_table_insert(
		/*@notnull@*/ /*@temp@*/ HashTable *hash_table, 
		/*@null@*/ /*@dependent@*/ HashTableKey key,
        /*@null@*/ /*@dependent@*/ HashTableValue value)
{
	HashTableEntry *rover;
	HashTablePair *pair;
	HashTableEntry *newentry;
	unsigned int index;

	/* If there are too many items in the table with respect to the table
	 * size, the number of hash collisions increases and performance
	 * decreases. Enlarge the table size to prevent this happening */

	if ((hash_table->entries * 3) / hash_table->table_size > 0) {

		/* Table is more than 1/3 full */

		if (hash_table_enlarge(hash_table) <= 0) {

			/* Failed to enlarge the table */

			return 0;
		}
	}

	/* Generate the hash of the key and hence the index into the table */

	index = hash_table->hash_func(key) % hash_table->table_size;

	/* Traverse the chain at this location and look for an existing
	 * entry with the same key */

	rover = hash_table->table[index];

	while (rover != NULL) {

		/* Fetch rover's HashTablePair entry */

		pair = &(rover->pair);

		if (hash_table->equal_func(pair->key, key) != 0) {

			/* Same key: overwrite this entry with new data */

			/* If there is a value free function, free the old data
			 * before adding in the new data */

			/* Ignore transfer warnings because memory model depends on whehter
			 * or not value_free_func is not NULL. */
			/*@-dependenttrans-branchstate@*/
			if (hash_table->value_free_func != NULL) {
				hash_table->value_free_func(pair->value);
			}
			/*@=dependenttrans=branchstate@*/

			/* Same with the key: use the new key value and free
			 * the old one */

			/*@-dependenttrans-branchstate@*/
			if (hash_table->key_free_func != NULL) {
				hash_table->key_free_func(pair->key);
			}
			/*@=dependenttrans=branchstate@*/

			pair->key = key;
			pair->value = value;

			/* Finished */

			return 1;
		}

		rover = rover->next;
	}

	/* Not in the hash table yet.  Create a new entry */

	newentry = (HashTableEntry *) malloc(sizeof(HashTableEntry));

	if (newentry == NULL) {
		return 0;
	}

	newentry->pair.key = key;
	newentry->pair.value = value;

	/* Link into the list */

	/*@-unqualifiedtrans@*/
	newentry->next = hash_table->table[index];
	hash_table->table[index] = newentry;
	/*@=unqualifiedtrans@*/

	/* Maintain the count of the number of entries */

	++hash_table->entries;

	/* Added successfully */

	return 1;
}

/*@null@*/ /*@dependent@*/ HashTableValue hash_table_lookup(
		/*@notnull@*/ /*@temp@*/ HashTable *hash_table, 
		/*@null@*/ /*@temp@*/ HashTableKey key)
{
	HashTableEntry *rover;
	HashTablePair *pair;
	unsigned int index;

	/* Generate the hash of the key and hence the index into the table */

	index = hash_table->hash_func(key) % hash_table->table_size;

	/* Walk the chain at this index until the corresponding entry is
	 * found */

	rover = hash_table->table[index];

	while (rover != NULL) {
		pair = &(rover->pair);

		if (hash_table->equal_func(key, pair->key) != 0) {

			/* Found the entry.  Return the data. */

			return pair->value;
		}

		rover = rover->next;
	}

	/* Not found */

	return HASH_TABLE_NULL;
}

int hash_table_remove(
		/*@notnull@*/ /*@temp@*/ HashTable *hash_table, 
		/*@null@*/ /*@temp@*/ HashTableKey key)
{
	HashTableEntry **rover;
	HashTableEntry *entry;
	HashTablePair *pair;
	unsigned int index;
	int result;

	/* Generate the hash of the key and hence the index into the table */

	index = hash_table->hash_func(key) % hash_table->table_size;

	/* Rover points at the pointer which points at the current entry
	 * in the chain being inspected.  ie. the entry in the table, or
	 * the "next" pointer of the previous entry in the chain.  This
	 * allows us to unlink the entry when we find it. */

	result = 0;
	rover = &hash_table->table[index];

	while (*rover != NULL) {

		pair = &((*rover)->pair);

		if (hash_table->equal_func(key, pair->key) != 0) {

			/* This is the entry to remove */

			entry = *rover;

			/* Unlink from the list */

			*rover = entry->next;

			/* Destroy the entry structure */

			/*@-nullstate@*/
			hash_table_free_entry(hash_table, entry);
			/*@=nullstate@*/

			/* Track count of entries */

			--hash_table->entries;

			result = 1;

			break;
		}

		/* Advance to the next entry */

		rover = &((*rover)->next);
	}

	return result;
}

unsigned int hash_table_num_entries(
		/*@notnull@*/ /*@temp@*/ HashTable *hash_table)
{
	return hash_table->entries;
}

void hash_table_iterate(
		/*@notnull@*/ /*@dependent@*/ HashTable *hash_table, 
		/*@notnull@*/ /*@temp@*/ HashTableIterator *iterator)
{
	unsigned int chain;

	iterator->hash_table = hash_table;

	/* Default value of next if no entries are found. */

	iterator->next_entry = NULL;

	/* Find the first entry */

	for (chain=0; chain<hash_table->table_size; ++chain) {

		if (hash_table->table[chain] != NULL) {
			iterator->next_entry = hash_table->table[chain];
			iterator->next_chain = chain;
			break;
		}
	}
}

int hash_table_iter_has_more(
		/*@notnull@*/ /*@temp@*/ HashTableIterator *iterator)
{
	return (int) (iterator->next_entry != NULL);
}

/*@null@*/ HashTablePair hash_table_iter_next(
		/*@notnull@*/ /*@temp@*/ HashTableIterator *iterator)
{
	HashTableEntry *current_entry;
	HashTable *hash_table;
	HashTablePair pair = {NULL, NULL};
	unsigned int chain;

	hash_table = iterator->hash_table;

	if (iterator->next_entry == NULL) {
		return pair;
	}

	/* See the note in the doc block in the header file. */
	/*@-onlytrans@*/

	/* Result is immediately available */

	current_entry = iterator->next_entry;
	pair = current_entry->pair;

	/* Find the next entry */

	if (current_entry->next != NULL) {

		/* Next entry in current chain */

		iterator->next_entry = current_entry->next;

	} else {

		/* None left in this chain, so advance to the next chain */

		chain = iterator->next_chain + 1;

		/* Default value if no next chain found */

		iterator->next_entry = NULL;

		while (chain < hash_table->table_size) {

			/* Is there anything in this chain? */

			if (hash_table->table[chain] != NULL) {
				iterator->next_entry = hash_table->table[chain];
				break;
			}

			/* Try the next chain */

			++chain;
		}

		iterator->next_chain = chain;
	}

	return pair;
	/*@=onlytrans@*/
}

