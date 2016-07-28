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

#include "tree.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>

/**
 * The structure of a tree node.
 */
struct _TreeNode {
	/**
	 * The parent of a node.
	 */
	/*@null@*/ /*@dependent@*/ TreeNode* parent;

	/**
	 * The children of the node. This is an array whose size is _alloced. All 
	 * elements in the range [0,outDegree) are defined and not null.
	 */
	/*@null@*/ /*@owned@*/ TreeNode** children;

	/**
	 * The amount of children of this node.
	 */
	unsigned int outDegree;

	/**
	 * The data this node stores.
	 */
	/*@null@*/ /*@shared@*/ TreeNodeValue data;

	/**
	 * The allocated size of children.
	 */
	unsigned int _alloced;

	/**
	 * The current position of this node in its parent's child array.
	 */
	unsigned int i;

	/**
	 * The height of this node.
	 */
	unsigned int h;
};

struct _TreeIterator {
	/*@null@*/ /*@dependent@*/ Tree* t;
	/*@null@*/ /*@dependent@*/ TreeNode* prev;
	/*@null@*/ /*@dependent@*/ TreeNode* current;
	/*@null@*/ /*@dependent@*/ TreeNode* next;
};

/* Updates the height values for the nodes when n is changed. */
static void tree_update_height(/*@null@*/ /*@temp@*/ TreeNode* n)
{
	while (n != NULL) {
		unsigned int h = 0;
		unsigned int i = 0;
		for (i = 0; i < n->outDegree; i++) {
			/*@-nullderef@*/
			h = (h > n->children[i]->h) ? h : n->children[i]->h;
			/*@=nullderef@*/
		}

		n->h = h;
		n = n->parent;
	}
}

/* Grows the internal children array to size s if it is smaller. */
static int tree_children_grow_min_size(
		/*@notnull@*/ /*@temp@*/ TreeNode* n,
		unsigned int s)
{
	TreeNode** arr;

	if (n->_alloced >= s) {
		return 1;
	}

	arr = realloc(n->children, sizeof(TreeNode*) * s);

	if (arr == NULL) {
		/*@-usereleased-compdef@*/
		return 0;
		/*@=usereleased=compdef@*/
	}

	n->children = arr;
	n->_alloced = s;

	/*@-compmempass@*/
	return 1;
	/*@=compmempass@*/
}

/* Removes node c as child from its parent child array. */
static void tree_node_remove_child(/*@notnull@*/ /*@temp@*/ TreeNode* c)
	/*@requires notnull c->parent,c->parent->children@*/
{
	TreeNode* p = c->parent;
	unsigned int i = 0;

	/* Loop to the place where c resides. */
	for (i = 0; i < p->outDegree && p->children[i] != c; i++); 

	p->children[i] = NULL;

	/* Move all others one place back. */
	for (; i < p->outDegree - 1; i++) {
		/*@-nullderef@*/
		p->children[i] = p->children[i+1];
		p->children[i]->i = i;
		/*@=nullderef@*/
	}

	p->outDegree--;
	/*@-nullstate@*/
}
/*@=nullstate@*/

/*@null@*/ /*@only@*/ Tree* tree_alloc(
		/*@null@*/ /*@shared@*/ TreeNodeValue data) 
{
	Tree* t = malloc(sizeof(struct _TreeNode));

	if (t == NULL) {
		return NULL;
	}

	t->parent = NULL;
	t->children = NULL;
	t->outDegree = 0;
	t->data = data;
	t->_alloced = 0;
	t->i = 0;
	t->h = 0;

	return t;
}

void tree_free(/*@null@*/ /*@only@*/ /*@in@*/ Tree* t)
{
	if (t != NULL) {
		free(t->children);
		free(t);
	}
}

/*@null@*/ /*@owned@*/ TreeNode* tree_root(/*@null@*/ /*@owned@*/ Tree* t)
{
	return t;
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_abs_root(
		/*@null@*/ /*@dependent@*/ TreeNode* n)
{
	TreeNode* b = n;

	while (n != NULL) {
		b = n;
		n = n->parent;
	}

	return b;
}

/*@null@*/ /*@dependent@*/ Tree* tree_subtree(
		/*@null@*/ /*@dependent@*/ TreeNode* n)
{
	return n;
}

/*@null@*/ /*@shared@*/ TreeNodeValue tree_data(
		/*@notnull@*/ /*@temp@*/ TreeNode* n)
{
	return n->data;
}

void tree_set_data(
		/*@notnull@*/ /*@temp@*/ TreeNode* n,
		/*@null@*/ /*@shared@*/ TreeNodeValue d)
{
	n->data = d;
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_parent(
		/*@null@*/ /*@temp@*/ Tree* t,
		/*@null@*/ /*@dependent@*/ TreeNode* n)
{
	if (n == NULL) {
		return NULL;
	}

	/* If t == n then n is root of t so no parent in t. */
	if (t == n) {
		return NULL;
	}

	return n->parent;
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_child(
		/*@notnull@*/ /*@temp@*/ TreeNode* n,
		unsigned int i)
{
	if (tree_out_degree(n) > i) {
		/*@-nullderef@*/
		return n->children[i];
		/*@=nullderef@*/
	}
	else {
		return NULL;
	}
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_first_child(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n)
{
	if (tree_out_degree(n) > 0) {
		/*@-nullderef@*/
		return n->children[0];
		/*@=nullderef@*/
	}
	else {
		return NULL;
	}
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_last_child(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n)
{
	unsigned int od = tree_out_degree(n);

	if (od > 0) {
		/* By def of tree node this cannot be NULL. */
		/*@-nullderef@*/
		return n->children[od - 1];
		/*@=nullderef@*/
	}
	else {
		return NULL;
	}
}

unsigned int tree_out_degree(/*@notnull@*/ /*@temp@*/ TreeNode* n)
{
	return n->outDegree;
}

/*@null@*/ /*@dependent@*/ TreeNode** tree_children(
		/*@notnull@*/ /*@temp@*/ TreeNode* n)
{
	if (tree_out_degree(n) == 0) {
		return NULL;
	}
	else {
		return n->children;
	}
}

int tree_is_leaf(/*@notnull@*/ /*@temp@*/ TreeNode* n)
{
	return tree_out_degree(n) == 0;
}

int tree_is_descendant_of(
		/*@notnull@*/ /*@temp@*/ TreeNode* d,
		/*@notnull@*/ /*@temp@*/ TreeNode* n)
{
	while (d != NULL && d != n) {
		d = d->parent;
	}

	return d == n;
}

int tree_is_ancestor_of(
		/*@notnull@*/ /*@temp@*/ TreeNode* a,
		/*@notnull@*/ /*@temp@*/ TreeNode* n)
{
	return tree_is_descendant_of(n, a);
}

unsigned int tree_depth(
		/*@notnull@*/ /*@temp@*/ Tree* t,
		/*@notnull@*/ /*@temp@*/ TreeNode* n)
{
	unsigned int d = 0;

	if (!tree_is_ancestor_of(t, n)) {
		return 0;
	}

	while (n != NULL && n != t) {
		n = n->parent;
		d++;
	}

	return d;
}

unsigned int tree_level(
		/*@notnull@*/ /*@temp@*/ Tree* t,
		/*@notnull@*/ /*@temp@*/ TreeNode* n)
{
	if (!tree_is_ancestor_of(t, n)) {
		return 0;
	}

	return tree_depth(t, n) + 1;
}

unsigned int tree_height(
		/*@null@*/ /*@temp@*/ Tree* t)
{
	if (t == NULL) {
		return 0;
	}

	return t->h;
}

unsigned int tree_height_node(TreeNode* n)
{
	return n->h;
}

/*@notnull@*/ /*@owned@*/ Tree* tree_remove(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n)
{
	if (n->parent == NULL) {
		/*@-dependenttrans@*/
		return n;
		/*@=dependenttrans@*/
	}

	/* Remove n from children of p. */
	tree_node_remove_child(n);

	tree_update_height(n->parent);

	n->parent = NULL;
	n->i = 0;

	/*@-dependenttrans@*/
	return n;
	/*@=dependenttrans@*/
}

void tree_delete(/*@notnull@*/ /*@dependent@*/ TreeNode* n)
{
	TreeNode* r = tree_remove(n);
	tree_free(r);
}

/*@null@*/ /*@owned@*/ Tree* tree_add_subtree(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n,
		/*@null@*/ /*@owned@*/ Tree* t)
{
	if (t == NULL || t->parent != NULL) {
		return NULL;
	}

	(void) tree_remove((TreeNode*) t);

	if (!tree_children_grow_min_size(n, n->outDegree + 1)) {
		return t;
	}

	/*@-nullderef@*/
	n->children[n->outDegree] = t;
	/*@=nullderef@*/

	t->parent = n;
	t->i = n->outDegree;
	n->outDegree++;

	tree_update_height(n);

	/*@-mustfreeonly@*/
	return NULL;
	/*@=mustfreeonly@*/
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_add_child(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n,
		/*@null@*/ /*@shared@*/ TreeNodeValue data)
{
	TreeNode* c;

	if (!tree_children_grow_min_size(n, n->outDegree + 1)) {
		return NULL;
	}
	
	c = tree_alloc(data);

	if (c == NULL) {
		return NULL;
	}

	/*@-nullderef@*/
	n->children[n->outDegree] = c;
	/*@=nullderef@*/

	c->parent = n;
	c->i = n->outDegree;
	n->outDegree++;

	tree_update_height(n);

	/*@-compmempass@*/
	return c;
	/*@=compmempass@*/
}

/*@null@*/ /*@owned@*/ Tree* tree_insert_subtree(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n,
		/*@null@*/ /*@owned@*/ Tree* t,
		unsigned int i)
{
	unsigned int a;

	if (t == NULL || i > n->outDegree || t->parent != NULL) {
		return t;
	}

	if (!tree_children_grow_min_size(n, n->outDegree + 1)) {
		return t;
	}

	/* create space for t */
	for (a = n->outDegree - 1; a >= i; a--) {
		/*@-nullderef@*/
		n->children[a + 1] = n->children[a];
		n->children[a + 1]->i = a + 1;
		/*@=nullderef@*/
	}

	/*@-nullderef@*/
	n->children[i] = t;
	/*@=nullderef@*/
	n->outDegree++;

	t->parent = n;
	t->i = i;

	tree_update_height(n);

	/*@-mustfreeonly@*/
	return NULL;
	/*@=mustfreeonly@*/
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_insert_child(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n,
		/*@null@*/ /*@shared@*/ TreeNodeValue data,
		unsigned int i)
{
	TreeNode* c;

	if (i > n->outDegree) {
		return NULL;
	}

	c = tree_alloc(data);
	c = tree_insert_subtree(n, c, i);

	if (c == NULL) {
		return c;
	}
	else {
		tree_free(c);
		return NULL;
	}
}

/*@null@*/ /*@owned@*/ Tree* tree_set_subtree(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n,
		/*@null@*/ /*@owned@*/ Tree* t,
		unsigned int i)
{
	if (t == NULL || i > n->outDegree || t->parent != NULL) {
		return t;
	}

	if (!tree_children_grow_min_size(n, n->outDegree + 1)) {
		return t;
	}

	if (i == n->outDegree) {
		/*@-nullderef@*/
		n->children[i] = t;
		/*@=nullderef@*/
		n->outDegree++;
	}
	else {
		/*@-nullderef-unqualifiedtrans@*/
		tree_free(n->children[i]);
		n->children[i] = t;
		/*@=nullderef=unqualifiedtrans@*/
	}

	t->parent = n;
	t->i = i;

	tree_update_height(n);

	/*@-mustfreeonly@*/
	return NULL;
	/*@=mustfreeonly@*/
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_set_child(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n,
		/*@null@*/ /*@shared@*/ TreeNodeValue data,
		unsigned int i)
{
	TreeNode* c;

	if (i > n->outDegree) {
		return NULL;
	}

	c = tree_alloc(data);
	c = tree_set_subtree(n, c, i);

	if (c == NULL) {
		return c;
	}
	else {
		tree_free(c);
		return NULL;
	}
}

/*@null@*/ /*@shared@*/ TreeNodeValue tree_iter_data(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i)
{
	if (i->current == NULL) {
		return NULL;
	}

	return i->current->data;
}

int tree_iter_has_next(/*@notnull@*/ /*@temp@*/ TreeIterator* i)
{
	return i->next != NULL;
}

int tree_iter_has_prev(/*@notnull@*/ /*@temp@*/ TreeIterator* i)
{
	return i->prev != NULL;
}

/*@null@*/ /*@only@*/ TreeIterator* tree_iter_alloc()
{
	TreeIterator* i = malloc(sizeof(TreeIterator));

	if (i == NULL) {
		return NULL;
	}

	i->current = NULL;
	i->t = NULL;
	i->next = NULL;
	i->prev = NULL;

	return i;
}

void tree_iter_free(/*@null@*/ /*@only@*/ /*@in@*/ TreeIterator* i)
{
	free(i);
}

void tree_iter_leaves_first(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i,
		/*@null@*/ /*@dependent@*/ Tree* t)
{
	TreeNode* nc = t;
	TreeNode* cc = t;

	i->t = t;

	while (nc != NULL) {
		cc = nc;
		
		if (cc->outDegree > 0) {
			/*@-nullderef@*/
			nc = nc->children[0];
			/*@=nullderef@*/
		}
		else {
			nc = NULL;
		}
	}
	
	i->next = cc;
}

void tree_iter_leaves_last(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i,
		/*@null@*/ /*@dependent@*/ Tree* t)
{
	TreeNode* nc = t;
	TreeNode* cc = t;

	i->t = t;

	while (nc != NULL) {
		cc = nc;
		
		if (cc->outDegree > 0) {
			/*@-nullderef@*/
			nc = nc->children[nc->outDegree - 1];
			/*@=nullderef@*/
		}
		else {
			nc = NULL;
		}
	}
	
	i->prev = cc;
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_iter_leaves_next(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i)
{
	TreeNode* p;

	i->prev = i->current;
	i->current = i->next;

	/* Find layer where there is a right sibling. */
	p = i->next;
	while (p != NULL && p != i->t) {
		if (p->parent != NULL && p->i < p->parent->outDegree - 1) {
			break;	
		}
		
		p = p->parent;
	}

	if (p == NULL || p == i->t) {
		i->next = NULL;
		return i->current;
	}

	/* Now move down the right branch to it's children. */
	/*@-nullderef@*/
	p = p->parent->children[p->i + 1];
	while (p->outDegree > 0) {
		p = p->children[0];
	}
	/*@=nullderef@*/

	i->next = p;
	
	return i->current;
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_iter_leaves_prev(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i)
{
	TreeNode* p;

	i->next = i->current;
	i->current = i->prev;

	/* Find layer where there is a left sibling. */
	p = i->prev;
	while (p != NULL && p != i->t) {
		if (p->i > 0) {
			break;
		}

		p = p->parent;
	}

	if (p == NULL || p == i->t) {
		i->prev = NULL;
		return i->current;
	}

	/* Now we go down to find the previous child */
	/*@-nullderef@*/
	p = p->parent->children[p->i - 1];
	while (p->outDegree > 0) {
		p = p->children[p->outDegree - 1];
	}
	/*@=nullderef@*/

	i->prev = p;

	return i->current;
}

void tree_iter_parents(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i,
		/*@notnull@*/ /*@dependent@*/ Tree* t,
		/*@notnull@*/ /*@dependent@*/ TreeNode* n)
{
	i->t = t;
	i->prev = NULL;
	i->current = NULL;
	i->next = n;
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_iter_parents_next(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i)
{
	i->prev = i->current;
	i->current = i->next;

	if (i->next != NULL && i->next != i->t) {
		i->next = i->next->parent;
	}
	else {
		i->next = NULL;
	}

	return i->current;
}

void tree_preorder_walk(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i,
		/*@null@*/ /*@dependent@*/ Tree* t)
{
	i->t = t;
	i->next = t;
	i->current = NULL;
	i->prev = NULL;
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_preorder_walk_next(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i)
{
	i->prev = i->current;
	i->current = i->next;

	if (i->next == NULL) {
		return i->current;
	}

	if (i->next->outDegree > 0) {
		/*@-nullderef@*/
		i->next = i->next->children[0];
		/*@-nullderef@*/
		return i->current;
	}

	while (i->next != NULL && 
			i->next->parent != NULL && 
			i->next != i->t) {
		if (i->next->i < i->next->parent->outDegree - 1) {
			i->next = i->next->parent->children[i->next->i + 1];
			return i->current;
		}

		i->next = i->next->parent;
	}

	i->next = NULL;
	return i->current;
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_preorder_walk_prev(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i)
{
	i->next = i->current;
	i->current = i->prev;

	if (i->prev == i->t || i->prev == NULL) {
		i->prev = NULL;
	}
	else if (i->prev->i > 0) {
		/*@-nullderef@*/
		i->prev = i->prev->parent->children[i->prev->i - 1];

		while (i->prev->outDegree > 0) {
			i->prev = i->prev->children[i->prev->outDegree - 1];
		}
		/*@=nullderef@*/
	}
	else {
		i->prev = i->prev->parent;
	}

	return i->current;
}

void tree_postorder_walk(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i,
		/*@null@*/ /*@dependent@*/ Tree* t)
{
	i->t = t;
	i->next = t;
	i->current = NULL;
	i->prev = NULL;

	while (i->next != NULL && i->next->outDegree > 0) {
		/*@-nullderef@*/
		i->next = i->next->children[0];
		/*@=nullderef@*/
	}
}

/*@null@*/ /*@dependent@*/ TreeNode* tree_postorder_walk_next(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i)
{
	i->prev = i->current;
	i->current = i->next;

	if (i->next == NULL || 
			i->next->parent == NULL || 
			i->next == i->t) {
		i->next = NULL;
	}
    else if (i->next->i < i->next->parent->outDegree - 1) {
    	/*@-nullderef@*/
		i->next = i->next->parent->children[i->next->i + 1];

		while (i->next->outDegree > 0) {
			i->next = i->next->children[0];
		}
		/*@=nullderef@*/
	}
	else {
		i->next = i->next->parent;
	}

	return i->current;
}


/*@null@*/ /*@dependent@*/ TreeNode* tree_postorder_walk_prev(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i)
{
	i->next = i->current;
	i->current = i->prev;

	if (i->prev == NULL) {
		return i->current;
	}

	if (i->prev->outDegree > 0) {
		/*@-nullderef@*/
		i->prev = i->prev->children[i->prev->outDegree - 1];
		return i->prev;
	}

	while (i->prev->parent != NULL && 
			i->prev != NULL && 
			i->prev != i->t) {
		if (i->prev ->i > 0) {
			i->prev = i->prev->parent->children[i->prev->i - 1];
			return i->prev;
		}
		/*@=nullderef@*/

		i->prev = i->prev->parent;
	}

	i->prev = NULL;
	return i->current;
}
