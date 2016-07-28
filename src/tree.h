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
 * @file tree.h
 *
 * @brief A tree data structure with common functions to be used on a tree.
 *
 * A tree is an Abstract Data Type containing nodes in a tree structure. Every
 * tree has one root node which has no incoming edges. All other nodes are 
 * connected by one edge to another node where all nodes except the root have 
 * only 1 incoming edge and zero or more outgoing edges.
 */

#ifndef ALGORITHM_SORTEDARRAY_H
#define ALGORITHM_SORTEDARRAY_H

#ifdef __cplusplus
extern "C" {
#endif
/*@-exportlocal@*/

/**
 * Definition of a TreeNode. A node has a parent node reference which may be 
 * null, indicating it is the root node. A node might have zero or more child
 * nodes.
 */
typedef /*@abstract@*/ struct _TreeNode TreeNode;

/**
 * Definition of a Tree. A tree is actually a tree node where the tree node is
 * regarded the root of the tree.
 */
typedef /*@abstract@*/ TreeNode Tree;

/**
 * A value to store in the tree.
 */
typedef void* TreeNodeValue;

/**
 * Iterator used for iterating the tree's nodes.
 */
typedef /*@abstract@*/ struct _TreeIterator TreeIterator;

/**
 * Allocates an tree.
 *
 * @param data The data to put into the root node.
 * @return A newly allocated Tree or NULL on failure.
 */
/*@null@*/ /*@only@*/ Tree* tree_alloc(
		/*@null@*/ /*@shared@*/ TreeNodeValue data);

/**
 * Frees all nodes of the given tree.
 *
 * Since a subtree is just a tree it can be freed to. However note that the 
 * subtree will also be automaticly removed from the parent tree.
 *
 * @param t The tree to free all nodes of.
 */
void tree_free(/*@null@*/ /*@only@*/ /*@in@*/ Tree* t);

/**
 * Returns the root of t.
 *
 * @param t The tree to return the root of.
 * @return The root of t.
 */
/*@null@*/ /*@owned@*/ TreeNode* tree_root(/*@null@*/ /*@owned@*/ Tree* t);

/**
 * Returns the absolute root of node n. The absolute node is defined as the node
 * which has no parent.
 *
 * @param n The node.
 * @return The absolute root of n.
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_abs_root(
		/*@null@*/ /*@dependent@*/ TreeNode* n);

/**
 * Returns a subtree rooted n.
 *
 * @param n The root of the new subtree.
 * @return The subtree rooted n.
 */
/*@null@*/ /*@dependent@*/ Tree* tree_subtree(
		/*@null@*/ /*@dependent@*/ TreeNode* n);

/**
 * Returns the data stored inside a node.
 *
 * @param n The data to return.
 * @return The data stored in n.
 */
/*@null@*/ /*@shared@*/ TreeNodeValue tree_data(
		/*@notnull@*/ /*@temp@*/ TreeNode* n);

/**
 * Sets the data d of node n.
 *
 * @param n The node to set the data of.
 * @param d The data to set.
 */
void tree_set_data(
		/*@notnull@*/ /*@temp@*/ TreeNode* n, 
		/*@null@*/ /*@shared@*/ TreeNodeValue d);

/**
 * Returns the parent of node n in tree t.
 *
 * If n is the root of t then NULL is returned.
 *
 * @param t The tree in which the parent is.
 * @param n The node to get the parent of.
 * @return The parent node of n in t. Or NULL if n is root of t.
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_parent(
		/*@null@*/ /*@temp@*/ Tree* t, 
		/*@null@*/ /*@dependent@*/ TreeNode* n);

/**
 * Returns the i-th child of n.
 *
 * @param n The node.
 * @param i The offset.
 * @return NULL if the child at i does not exists, or the i-th child.
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_child(
		/*@notnull@*/ /*@temp@*/ TreeNode* n, 
		unsigned int i);

/**
 * Returns the first child of n if any.
 *
 * @param n The node.
 * @return The first child of n if any otherwise NULL.
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_first_child(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n);

/**
 * Returns the last child of n if any.
 *
 * @param n The node.
 * @return The last child of n if any otherwise NULL.
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_last_child(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n);

/**
 * Returns the out degree of node n.
 *
 * @param n The node.
 * @return The out degree.
 */
unsigned int tree_out_degree(/*@notnull@*/ /*@temp@*/ TreeNode* n);

/**
 * Returns an array containing the children of node n. Using the function 
 * @ref tree_out_degree you can get the amount of children a node has.
 *
 * @param n The node.
 * @return A pointer to an array containing the children of node n. If is has no
 * children then NULL is returned.
 */
/*@null@*/ /*@dependent@*/ TreeNode** tree_children(
		/*@notnull@*/ /*@temp@*/ TreeNode* n);

/**
 * Returns whether or not node n is a leaf.
 *
 * @param n The node.
 * @return >0 if n is a leaf, <=0 if not.
 */
int tree_is_leaf(/*@notnull@*/ /*@temp@*/ TreeNode* n);

/**
 * Returns whether or not node d is a descendent of node n.
 *
 * @param d The possible descendent node.
 * @param n The node.
 * @return >0 if d is a descendent of n, <=0 if not.
 */
int tree_is_descendant_of(
		/*@notnull@*/ /*@temp@*/ TreeNode* d, 
		/*@notnull@*/ /*@temp@*/ TreeNode* n);

/**
 * Returns whether or not node a is an ancestor of node n.
 *
 * @param a The possible ancestor node.
 * @param n The node.
 * @return >0 if a is an ancestor of n, <=0 if not.
 */
int tree_is_ancestor_of(
		/*@notnull@*/ /*@temp@*/ TreeNode* a, 
		/*@notnull@*/ /*@temp@*/ TreeNode* n);

/**
 * Returns the depth of node n in t. The depth of a tree is the number of edges 
 * from this node to the root node.
 *
 * Note that if n is not inside t then the function will return 0.
 *
 * @param t The tree.
 * @param n The node.
 * @return The depth of n in t.
 */
unsigned int tree_depth(
		/*@notnull@*/ /*@temp@*/ Tree* t, 
		/*@notnull@*/ /*@temp@*/ TreeNode* n);

/**
 * Returns the level of node n in t. The level of a tree is defined as 1 + the
 * depth.
 *
 * Note that if n is not inside t then the function will return 0.
 *
 * @param t The tree.
 * @param n The node.
 * @return The level of n in t.
 */
unsigned int tree_level(
		/*@notnull@*/ /*@temp@*/ Tree* t, 
		/*@notnull@*/ /*@temp@*/ TreeNode* n);

/**
 * Returns the height of tree t. The height of a tree is the largest amount of 
 * edges between the root of t and a leaf.
 *
 * @param t The tree.
 * @return The height of t.
 */
unsigned int tree_height(
		/*@null@*/ /*@temp@*/ Tree* t);

/**
 * Returns the height of node n. The height of a node is the longest path 
 * between the node and a leaf.
 *
 * @param n The node.
 * @return The height of n.
 */
unsigned int tree_height_node(TreeNode* n);

/**
 * Removes node n and all its child nodes from its tree and thereby creating a 
 * new tree rooted n which is returned. 
 *
 * @param n The node to remove from its parent tree
 * @return The subtree created rooted n.
 */
/*@notnull@*/ /*@owned@*/ Tree* tree_remove(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n);

/**
 * Deletes node n and all its child nodes. This means that the node is removed 
 * from its parent tree and n is freed from memory.
 *
 * @param n The node.
 * @return >0 on success, <=0 on failure.
 */
void tree_delete(/*@notnull@*/ /*@dependent@*/ TreeNode* n);

/**
 * Adds a tree t as a subtree of node n. To make sure that every node is only 
 * part of one tree the node is first removed before it is added to its new 
 * location.
 *
 * @param n The node.
 * @param t The subtree.
 * @return Null on success, or the node which was to be added to the tree. 
 */
/*@null@*/ /*@owned@*/ Tree* tree_add_subtree(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n, 
		/*@null@*/ /*@owned@*/ Tree* t);

/**
 * Adds a child storing data as child of node n.
 *
 * @param n The node.
 * @param data The data.
 * @return The newly added child or NULL on failure. 
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_add_child(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n, 
		/*@null@*/ /*@shared@*/ TreeNodeValue data);

/**
 * Inserts a tree t as subtree of node n at offset i of n's children. When t is 
 * NULL nothing is inserted. The range of i must be 0 <= i <= l, where l is the 
 * out degree of n.
 *
 * @param n The node.
 * @param t The tree.
 * @param i The offset.
 * @return NULL on success, or tree t on failure. 
 */
/*@null@*/ /*@owned@*/ Tree* tree_insert_subtree(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n, 
		/*@null@*/ /*@owned@*/ Tree* t, 
		unsigned int i);

/**
 * Inserts a child storing value data as child of node n at offset i of n's 
 * children. The range of i must be 0 <= i <= l, where l is the 
 * out degree of n.
 *
 * @param n The node.
 * @param data The data.
 * @param i The offset.
 * @return The newly inserted child or NULL on failure. 
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_insert_child(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n, 
		/*@null@*/ /*@shared@*/ TreeNodeValue data, 
		unsigned int i);

/**
 * Sets the child at position i of node n's children to tree t. Let l be the 
 * amount of children then i should be in the range 0 <= i <= l.
 *
 * @param n The node.
 * @param t The tree.
 * @param i The offset.
 * @return Null on success, or t on failure.
 */
/*@null@*/ /*@owned@*/ Tree* tree_set_subtree(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n, 
		/*@null@*/ /*@owned@*/ Tree* t, 
		unsigned int i);

/**
 * Sets a child storing value data as child stored at position i of node n's 
 * children. Let l be the amount of children then i should be in the range of 
 * 0 <= i <= l.
 *
 * @param n The node.
 * @param data The data.
 * @param i The offset.
 * @return The newly set child or NULL on failure. 
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_set_child(
		/*@notnull@*/ /*@dependent@*/ TreeNode* n, 
		/*@null@*/ /*@shared@*/ TreeNodeValue data, 
		unsigned int i);

/**
 * Returns the data of the current node of iterator i.
 *
 * @param i The iterator.
 * @return The data of the current node of i.
 */
/*@null@*/ /*@shared@*/ TreeNodeValue tree_iter_data(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i);

/**
 * Returns whether or not iteration is not finished. 
 *
 * @param i The iterator.
 * @return >0 If iteration is not done, <=0 if iteration is done.
 */
int tree_iter_has_next(/*@notnull@*/ /*@temp@*/ TreeIterator* i);

/**
 * Returns whether or not iterations is not finished.
 *
 * @param i The iterator.
 * @return >0 If iteration is not done, <=0 if iteration is done.
 */
int tree_iter_has_prev(/*@notnull@*/ /*@temp@*/ TreeIterator* i);

/**
 * Allocates an tree iterator.
 *
 * @return The iterator or NULL on failure.
 */
/*@null@*/ /*@only@*/ TreeIterator* tree_iter_alloc();

/**
 * Frees the iterator from memory.
 *
 * @param i The iterator to free.
 */
void tree_iter_free(/*@null@*/ /*@only@*/ /*@in@*/ TreeIterator* i);

/**
 * Initializes an iterator to iterate over the leaves of tree t. Starting with 
 * the first leaf.
 *
 * @param i The iterator.
 * @param t The tree.
 */
void tree_iter_leaves_first(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i,
		/*@null@*/ /*@dependent@*/ Tree* t);

/**
 * Initializes an iterator to iterate over the leaves of tree t. Starting with 
 * the last leaf.
 *
 * @param i The iterator.
 * @param t The tree.
 */
void tree_iter_leaves_last(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i,
		/*@null@*/ /*@dependent@*/ Tree* t);

/**
 * Iterates to the next leaf of iterator i.
 *
 * @param i The iterator.
 * @return The current leaf or NULL if none.
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_iter_leaves_next(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i);

/**
 * Iterates to the previous leaf of iterator i.
 *
 * @param i The iterator.
 * @return The current leaf or NULL if none.
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_iter_leaves_prev(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i);

/**
 * Iterates over all parents of node n in tree t starting from the node n and 
 * ending the root of tree t.
 *
 * @param i The iterator.
 * @param t The tree.
 * @param n The node.
 */
void tree_iter_parents(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i,
		/*@notnull@*/ /*@dependent@*/ Tree* t, 
		/*@notnull@*/ /*@dependent@*/ TreeNode* n);

/**
 * Iterate to the next parent of the current node of iterator i.
 *
 * @param i The iterator.
 * @return The current parent, or NULL if none.
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_iter_parents_next(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i);

/**
 * Initializes an iterator to do a preorder walk over tree t.
 *
 * @param i The iterator.
 * @param t The tree.
 */
void tree_preorder_walk(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i,
		/*@null@*/ /*@dependent@*/ Tree* t);

/**
 * Iterates to the next node in the preorder walk of iterator i.
 *
 * @param i The iterator.
 * @return The current node or NULL if none.
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_preorder_walk_next(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i);

/**
 * Iterates to the previous node in the preorder walk of iterator i.
 *
 * @param i The iterator.
 * @return The current node or NULL if none.
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_preorder_walk_prev(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i);

/**
 * Initializes an iterator to do a postorder walk over tree t.
 *
 * @param i The iterator.
 * @param t The tree.
 */
void tree_postorder_walk(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i,
		/*@null@*/ /*@dependent@*/ Tree* t);

/**
 * Iterates to the next node in the postorder walk of iterator i.
 *
 * @param i The iterator.
 * @return The current node or NULL if none.
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_postorder_walk_next(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i);

/**
 * Iterates to the previous node in the postorder walk of iterator i.
 *
 * @param i The iterator.
 * @return The current node or NULL if none.
 */
/*@null@*/ /*@dependent@*/ TreeNode* tree_postorder_walk_prev(
		/*@notnull@*/ /*@temp@*/ TreeIterator* i);

/*@=exportlocal@*/
#ifdef __cplusplus
}
#endif
#endif
