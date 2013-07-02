/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <assert.h>
#include <stdlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "basic-block.h"
#include "function.h"
#include "struct-analysis.h"


/*
 * Control Tree Node
 * Object 'llvm2si_function_node_t'.
 */

struct str_map_t llvm2si_function_node_kind_map =
{
	2,
	{
		{ "leaf", llvm2si_function_node_leaf },
		{ "abstract", llvm2si_function_node_abstract }
	}
};


static struct llvm2si_function_node_t *llvm2si_function_node_create(
		struct llvm2si_function_t *function,
		enum llvm2si_function_node_kind_t kind)
{
	struct llvm2si_function_node_t *node;

	/* Initialize */
	node = xcalloc(1, sizeof(struct llvm2si_function_node_t));
	node->function = function;
	node->kind = kind;
	node->pred_list = linked_list_create();
	node->succ_list = linked_list_create();

	/* Return */
	return node;
}


struct llvm2si_function_node_t *llvm2si_function_node_create_leaf(
		struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block)
{
	struct llvm2si_function_node_t *node;

	/* Initialize */
	node = llvm2si_function_node_create(function,
			llvm2si_function_node_leaf);
	node->leaf.basic_block = basic_block;
	node->name = str_set(node->name, basic_block->name);

	/* Record node in basic block */
	assert(!basic_block->node);
	basic_block->node = node;

	/* Return */
	return node;
}


struct llvm2si_function_node_t *llvm2si_function_node_create_abstract(
		struct llvm2si_function_t *function,
		char *name)
{
	struct llvm2si_function_node_t *node;

	/* Initialize */
	node = llvm2si_function_node_create(function,
			llvm2si_function_node_abstract);
	node->name = str_set(node->name, name && *name ? name : "<abstract>");
	node->abstract.child_list = linked_list_create();
	
	/* Return */
	return node;
}


void llvm2si_function_node_free(struct llvm2si_function_node_t *node)
{
	if (node->kind == llvm2si_function_node_abstract)
		linked_list_free(node->abstract.child_list);
	linked_list_free(node->pred_list);
	linked_list_free(node->succ_list);
	str_free(node->name);
	free(node);
}


void llvm2si_function_node_list_dump(struct linked_list_t *list, FILE *f)
{
	char *comma;
	struct linked_list_iter_t *iter;
	struct llvm2si_function_node_t *node;

	comma = "";
	fprintf(f, "{");
	iter = linked_list_iter_create(list);
	LINKED_LIST_ITER_FOR_EACH(iter)
	{
		node = linked_list_iter_get(iter);
		fprintf(f, "%s%s", comma, node->name);
		comma = ",";
	}
	linked_list_iter_free(iter);
	fprintf(f, "}");
}


void llvm2si_function_node_list_dump_detail(struct linked_list_t *list, FILE *f)
{
	struct linked_list_iter_t *iter;
	struct llvm2si_function_node_t *node;

	iter = linked_list_iter_create(list);
	LINKED_LIST_ITER_FOR_EACH(iter)
	{
		node = linked_list_iter_get(iter);
		llvm2si_function_node_dump(node, f);
	}
	linked_list_iter_free(iter);
}


void llvm2si_function_node_dump(struct llvm2si_function_node_t *node, FILE *f)
{
	char *no_name;

	no_name = "<no-name>";
	fprintf(f, "Node '%s':", *node->name ? node->name : no_name);
	fprintf(f, " type=%s", str_map_value(&llvm2si_function_node_kind_map,
			node->kind));
	fprintf(f, " pred=");
	llvm2si_function_node_list_dump(node->pred_list, f);
	fprintf(f, " succ=");
	llvm2si_function_node_list_dump(node->succ_list, f);
	fprintf(f, " parent=%s", node->parent ? node->parent->name : "-");

	/* List of child elements */
	if (node->kind == llvm2si_function_node_abstract)
	{
		fprintf(f, " children=");
		llvm2si_function_node_list_dump(node->abstract.child_list, f);
	}

	/* Preorder traversal ID */
	fprintf(f, " preorder_id=%d", node->preorder_id);

	/* End */
	fprintf(f, "\n");
}


/* Return true if 'node' is in the linked list of nodes passed as the second
 * argument. This function does not call 'linked_list_find'. Instead, it
 * traverses the list using a dedicated iterator, so that the current element of
 * the list is not lost. */
int llvm2si_function_node_in_list(struct llvm2si_function_node_t *node,
		struct linked_list_t *list)
{
	struct linked_list_iter_t *iter;
	int found;

	iter = linked_list_iter_create(list);
	found = linked_list_iter_find(iter, node);
	linked_list_iter_free(iter);

	return found;
}


/* Try to create an edge between 'node' and 'node_dest'. If the edge already
 * exist, the function will ignore the call silently. */
void llvm2si_function_try_connect(struct llvm2si_function_node_t *node,
		struct llvm2si_function_node_t *node_dest)
{
	/* Nothing if edge already exists */
	if (llvm2si_function_node_in_list(node_dest, node->succ_list))
		return;

	/* Add edge */
	assert(!llvm2si_function_node_in_list(node, node_dest->pred_list));
	linked_list_add(node->succ_list, node_dest);
	linked_list_add(node_dest->pred_list, node);
}


/* Create an edge between 'node' and 'node_dest'. There should be no existing
 * edge for this source and destination when calling this function. */
void llvm2si_function_node_connect(struct llvm2si_function_node_t *node,
		struct llvm2si_function_node_t *node_dest)
{
#ifndef NDEBUG

	/* Make sure that connection does not exist */
	linked_list_find(node->succ_list, node_dest);
	linked_list_find(node_dest->pred_list, node);
	if (!node->succ_list->error_code ||
			!node_dest->pred_list->error_code)
		panic("%s: redundant connection between control tree nodes",
				__FUNCTION__);
#endif

	/* Make connection */
	linked_list_add(node->succ_list, node_dest);
	linked_list_add(node_dest->pred_list, node);
}


/* Try to remove an edge between 'node' and 'node_dest'. If the edge does not
 * exist, the function exists silently. */
void llvm2si_function_node_try_disconnect(struct llvm2si_function_node_t *node,
		struct llvm2si_function_node_t *node_dest)
{
	/* Check if connection exists */
	linked_list_find(node->succ_list, node_dest);
	linked_list_find(node_dest->pred_list, node);

	/* Either both are present, or none */
	assert((node->succ_list->error_code && node_dest->pred_list->error_code)
			|| (!node->succ_list->error_code &&
			!node_dest->pred_list->error_code));

	/* No connection existed */
	if (node->succ_list->error_code)
		return;
	
	/* Remove existing connection */
	linked_list_remove(node->succ_list);
	linked_list_remove(node_dest->pred_list);
}


/* Disconnect 'node' and 'node_dest'. An edge must exist between both. */
void llvm2si_function_node_disconnect(struct llvm2si_function_node_t *node,
		struct llvm2si_function_node_t *node_dest)
{
	/* Make sure that connection exists */
	linked_list_find(node->succ_list, node_dest);
	linked_list_find(node_dest->pred_list, node);
	if (node->succ_list->error_code ||
			node_dest->pred_list->error_code)
		panic("%s: invalid connection between control tree nodes",
				__FUNCTION__);
	
	/* Remove it */
	linked_list_remove(node->succ_list);
	linked_list_remove(node_dest->pred_list);
}





/*
 * Functions for 'llvm2si_function_t'
 */

/* Perform a preorder traversal of the function control tree starting at
 * node given in argument 'node'. */
static void llvm2si_function_preorder_traversal_node(
		struct llvm2si_function_t *function,
		struct llvm2si_function_node_t *node)
{
	struct llvm2si_function_node_t *node_succ;

	/* Assign new value to node, starting at 0. */
	assert(node->preorder_id == -1);
	node->preorder_id = function->preorder_counter++;

	/* Traverse recursively all successors. */
	LINKED_LIST_FOR_EACH(node->succ_list)
	{
		node_succ = linked_list_get(node->succ_list);
		if (node_succ->preorder_id == -1)
			llvm2si_function_preorder_traversal_node(
					function, node_succ);
	}
}


/* Perform a pre-order traversal of the control flow graph starting at the entry
 * node. The resulting control tree is placed in 'node_list', including only
 * those nodes that are reachable from the entry. All 'preorder_id' values are
 * set in increasing order for reachable nodes, starting at 0. Unreachable nodes
 * set their 'preorder_id' to -1. */
void llvm2si_function_preorder_traversal(struct llvm2si_function_t *function,
		struct linked_list_t *node_list)
{
	struct llvm2si_function_node_t *node;
	struct llvm2si_function_node_t **node_array;
	int i;

	/* Reset preorder traversal ID counter */
	assert(function->node_entry);
	function->preorder_counter = 0;

	/* Set all node preorder IDs to -1 */
	LINKED_LIST_FOR_EACH(function->node_list)
	{
		node = linked_list_get(function->node_list);
		node->preorder_id = -1;
	}

	/* Start with entry node, and continue recursively. */
	llvm2si_function_preorder_traversal_node(function,
			function->node_entry);

	/* Nothing else to do if no 'node_list' given */
	if (!node_list)
		return;

	/* Create array of nodes */
	node_array = xcalloc(function->node_list->count, sizeof(void *));
	LINKED_LIST_FOR_EACH(function->node_list)
	{
		node = linked_list_get(function->node_list);
		if (node->preorder_id != -1)
		{
			assert(IN_RANGE(node->preorder_id, 0,
					function->node_list->count - 1));
			node_array[node->preorder_id] = node;
		}
	}
	
	/* Build linked list of nodes in tree */
	linked_list_clear(node_list);
	for (i = 0; i < function->node_list->count && node_array[i]; i++)
		linked_list_add(node_list, node_array[i]);

	/* Free array of nodes */
	free(node_array);
}


/* Reduce the list of nodes in 'node_list' with a newly created abstract node,
 * returned as the function result.
 * All incoming edges to any of the nodes in the list will point to 'node'.
 * Likewise, all outgoing edges from any node in the list will come from
 * 'node'.
 */
struct llvm2si_function_node_t *llvm2si_function_reduce(
		struct llvm2si_function_t *function,
		struct linked_list_t *node_list)
{
	struct llvm2si_function_node_t *abs_node;
	struct llvm2si_function_node_t *tmp_node;
	struct llvm2si_function_node_t *out_node;
	struct llvm2si_function_node_t *in_node;

	struct linked_list_t *out_node_list;
	struct linked_list_t *in_node_list;

#ifndef NDEBUG

	/* List of nodes must contain at least one node */
	if (!node_list->count)
		panic("%s: node list empty", __FUNCTION__);

	/* All nodes in 'node_list' must be part of the control tree, and none
	 * of them can have a parent yet. */
	LINKED_LIST_FOR_EACH(node_list)
	{
		tmp_node = linked_list_get(node_list);
		if (!llvm2si_function_node_in_list(tmp_node,
				function->node_list))
			panic("%s: node not in control tree",
					__FUNCTION__);
		if (tmp_node->parent)
			panic("%s: node has a parent already",
					__FUNCTION__);
	}
#endif

	/* Create new abstract node */
	abs_node = llvm2si_function_node_create_abstract(function, NULL);
	linked_list_add(function->node_list, abs_node);

	/* Create a list of incoming edges from the control tree into the
	 * region given in 'node_list', and a list of outgoing edges from the
	 * region in 'node_list' into the rest of the control tree. */
	out_node_list = linked_list_create();
	in_node_list = linked_list_create();
	LINKED_LIST_FOR_EACH(node_list)
	{
		/* Get node in region 'node_list' */
		tmp_node = linked_list_get(node_list);

		/* Traverse incoming edges, and store in 'in_node_list' those
		 * that come from outside of 'node_list'. */
		LINKED_LIST_FOR_EACH(tmp_node->pred_list)
		{
			in_node = linked_list_get(tmp_node->pred_list);
			if (!llvm2si_function_node_in_list(in_node, node_list) &&
					!llvm2si_function_node_in_list(in_node,
					in_node_list))
				linked_list_add(in_node_list, in_node);
		}

		/* Traverse outgoing edges, and store in 'out_node_list' those
		 * that go outside of 'node_list'. */
		LINKED_LIST_FOR_EACH(tmp_node->succ_list)
		{
			out_node = linked_list_get(tmp_node->succ_list);
			if (!llvm2si_function_node_in_list(out_node, node_list) &&
					!llvm2si_function_node_in_list(out_node,
					out_node_list))
				linked_list_add(out_node_list, out_node);
		}
	}

	/* Remove all incoming/outgoing edges from/to the region outside of
	 * 'node_list'. */
	LINKED_LIST_FOR_EACH(node_list)
	{
		tmp_node = linked_list_get(node_list);
		LINKED_LIST_FOR_EACH(in_node_list)
		{
			in_node = linked_list_get(in_node_list);
			llvm2si_function_node_try_disconnect(in_node, tmp_node);
		}
		LINKED_LIST_FOR_EACH(out_node_list)
		{
			out_node = linked_list_get(out_node_list);
			llvm2si_function_node_try_disconnect(tmp_node, out_node);
		}
	}

	/* Add all incoming/outgoing edges as predecessors/successors of the new
	 * abstract node. */
	LINKED_LIST_FOR_EACH(in_node_list)
	{
		in_node = linked_list_get(in_node_list);
		llvm2si_function_node_connect(in_node, abs_node);
	}
	LINKED_LIST_FOR_EACH(out_node_list)
	{
		out_node = linked_list_get(out_node_list);
		llvm2si_function_node_connect(abs_node, out_node);
	}

	/* Add all nodes as child nodes of the new abstract node */
	assert(!abs_node->abstract.child_list->count);
	LINKED_LIST_FOR_EACH(node_list)
	{
		tmp_node = linked_list_get(node_list);
		assert(!tmp_node->parent);
		tmp_node->parent = abs_node;
		linked_list_add(abs_node->abstract.child_list, tmp_node);
	}

	/* If entry node is part of the nodes that were replaced, set it to the
	 * new abstract node. */
	if (llvm2si_function_node_in_list(function->node_entry, node_list))
		function->node_entry = abs_node;

	/* Free list of outgoing and incoming edges */
	linked_list_free(out_node_list);
	linked_list_free(in_node_list);

	/* Return created abstract node */
	return abs_node;
}


/* Replace the content of the function with the example on page 201 of
 * Muchnick's book. This function is used for debugging purposes. */
void llvm2si_function_example(struct llvm2si_function_t *function)
{
	linked_list_clear(function->node_list);

	struct llvm2si_basic_block_t *bb1 = llvm2si_basic_block_create_with_name("n1");
	struct llvm2si_function_node_t *n1 = llvm2si_function_node_create_leaf(function, bb1);
	linked_list_add(function->node_list, n1);
	struct llvm2si_basic_block_t *bb2 = llvm2si_basic_block_create_with_name("n2");
	struct llvm2si_function_node_t *n2 = llvm2si_function_node_create_leaf(function, bb2);
	linked_list_add(function->node_list, n2);
	struct llvm2si_basic_block_t *bb3 = llvm2si_basic_block_create_with_name("n3");
	struct llvm2si_function_node_t *n3 = llvm2si_function_node_create_leaf(function, bb3);
	linked_list_add(function->node_list, n3);
	struct llvm2si_basic_block_t *bb4 = llvm2si_basic_block_create_with_name("n4");
	struct llvm2si_function_node_t *n4 = llvm2si_function_node_create_leaf(function, bb4);
	linked_list_add(function->node_list, n4);
	struct llvm2si_basic_block_t *bb5 = llvm2si_basic_block_create_with_name("n5");
	struct llvm2si_function_node_t *n5 = llvm2si_function_node_create_leaf(function, bb5);
	linked_list_add(function->node_list, n5);
	struct llvm2si_basic_block_t *bb6 = llvm2si_basic_block_create_with_name("n6");
	struct llvm2si_function_node_t *n6 = llvm2si_function_node_create_leaf(function, bb6);
	linked_list_add(function->node_list, n6);
	struct llvm2si_basic_block_t *bb7 = llvm2si_basic_block_create_with_name("n7");
	struct llvm2si_function_node_t *n7 = llvm2si_function_node_create_leaf(function, bb7);
	linked_list_add(function->node_list, n7);

	llvm2si_function_node_connect(n1, n2);
	llvm2si_function_node_connect(n2, n3);
	llvm2si_function_node_connect(n2, n4);
	llvm2si_function_node_connect(n4, n2);
	llvm2si_function_node_connect(n3, n5);
	llvm2si_function_node_connect(n4, n5);
	llvm2si_function_node_connect(n5, n3);
	llvm2si_function_node_connect(n5, n6);
	//llvm2si_function_node_connect(n6, n5);
	llvm2si_function_node_connect(n6, n7);

	function->node_entry = n1;
}


/* This function has been written for debugging purposes of function
 * 'llvm2si_function_reduce'. It takes the example on page 201 of Muchnick's
 * book and reproduces the control tree reduction step by step. */
void llvm2si_function_reduce_example(struct llvm2si_function_t *function)
{
	struct linked_list_t *node_list = linked_list_create();

	/* Create example function */
	llvm2si_function_example(function);

	/* Get nodes */
	linked_list_goto(function->node_list, 0);
	struct llvm2si_function_node_t *n1 = linked_list_get(function->node_list);
	linked_list_goto(function->node_list, 1);
	struct llvm2si_function_node_t *n2 = linked_list_get(function->node_list);
	linked_list_goto(function->node_list, 2);
	struct llvm2si_function_node_t *n3 = linked_list_get(function->node_list);
	linked_list_goto(function->node_list, 3);
	struct llvm2si_function_node_t *n4 = linked_list_get(function->node_list);
	linked_list_goto(function->node_list, 4);
	struct llvm2si_function_node_t *n5 = linked_list_get(function->node_list);
	linked_list_goto(function->node_list, 5);
	struct llvm2si_function_node_t *n6 = linked_list_get(function->node_list);
	linked_list_goto(function->node_list, 6);
	struct llvm2si_function_node_t *n7 = linked_list_get(function->node_list);

	/* Stage 1 */
	printf("\n\n\n====== STAGE 1 ======\n\n\n");
	linked_list_clear(node_list);
	linked_list_add(node_list, n2);
	linked_list_add(node_list, n4);
	struct llvm2si_function_node_t *n2a = llvm2si_function_reduce(function, node_list);
	llvm2si_function_dump_control_tree(function, stdout);

	/* Stage 2 */
	printf("\n\n\n====== STAGE 2 ======\n\n\n");
	linked_list_clear(node_list);
	linked_list_add(node_list, n5);
	linked_list_add(node_list, n6);
	struct llvm2si_function_node_t *n5a = llvm2si_function_reduce(function, node_list);
	llvm2si_function_dump_control_tree(function, stdout);

	/* Stage 3 */
	printf("\n\n\n====== STAGE 3 ======\n\n\n");
	linked_list_clear(node_list);
	linked_list_add(node_list, n3);
	linked_list_add(node_list, n5a);
	struct llvm2si_function_node_t *n3a = llvm2si_function_reduce(function, node_list);
	llvm2si_function_dump_control_tree(function, stdout);

	/* Stage 4 */
	printf("\n\n\n====== STAGE 4 ======\n\n\n");
	linked_list_clear(node_list);
	linked_list_add(node_list, n1);
	linked_list_add(node_list, n2a);
	linked_list_add(node_list, n3a);
	linked_list_add(node_list, n7);
	llvm2si_function_reduce(function, node_list);
	llvm2si_function_dump_control_tree(function, stdout);

	exit(1);
}


static int llvm2si_function_natural_loop_node(
		struct llvm2si_function_node_t *root_node,
		struct llvm2si_function_node_t *current_node,
		struct linked_list_t *node_list)
{
	struct llvm2si_function_node_t *succ_node;
	int back_path;

	/* If node was already traverse, ignore */
	if (current_node->color)
		return 0;

	/* Label as traversed */
	current_node->color = 1;
	back_path = 0;
	
	/* Check successors */
	LINKED_LIST_FOR_EACH(current_node->succ_list)
	{
		/* Get successor */
		succ_node = linked_list_get(current_node->succ_list);

		/* Check if this is a back-edge to root node. If so, the node
		 * should be added to the output list. */
		if (succ_node == root_node && current_node->preorder_id >
				succ_node->preorder_id)
			back_path = 1;

		/* Check recursively for back-edges to root node. If so, node is
		 * also added to the output list. This check needs to be done
		 * even if 'back_path' is already set, since it could find a
		 * larger set of nodes. */
		if (llvm2si_function_natural_loop_node(root_node, succ_node,
				node_list))
			back_path = 1;
	}

	/* Add node to list if back path found */
	if (back_path)
		linked_list_add(node_list, current_node);

	/* Return true is back path was found */
	return back_path;
}


/* Obtain the natural loop with a header given in 'node'. The control tree must
 * be available in 'function' after having performed a pre-order traversal on
 * the control flow graph. This means that each node must have a valid value in
 * its 'node->preorder_id' counter.
 * The nodes forming the natural loop will be placed in 'node_list', which must
 * be an empty list when the instruction is called. If 'node' does not actually
 * head a loop, 'node_list' will be returned empty.
 * The function returns true if 'node' is the header of a loop, and false
 * otherwise. */
int llvm2si_function_natural_loop(struct llvm2si_function_t *function,
		struct llvm2si_function_node_t *node,
		struct linked_list_t *node_list)
{
	struct llvm2si_function_node_t *tmp_node;

	/* Preliminary checks */
	assert(!node_list->count);
	assert(node->preorder_id != -1);
	assert(!node->parent);

	/* Reset nodes' color */
	LINKED_LIST_FOR_EACH(function->node_list)
	{
		tmp_node = linked_list_get(function->node_list);
		tmp_node->color = 0;
	}

	/* Make recursive calls */
	return llvm2si_function_natural_loop_node(node, node, node_list);
}


void llvm2si_function_natural_loop_example(struct llvm2si_function_t *function)
{
	/* Create example function */
	llvm2si_function_example(function);

	/* Get nodes */
	linked_list_goto(function->node_list, 0);
	struct llvm2si_function_node_t *n1 = linked_list_get(function->node_list);
	linked_list_goto(function->node_list, 1);
	struct llvm2si_function_node_t *n2 = linked_list_get(function->node_list);
	linked_list_goto(function->node_list, 2);
	struct llvm2si_function_node_t *n3 = linked_list_get(function->node_list);
	linked_list_goto(function->node_list, 3);
	struct llvm2si_function_node_t *n4 = linked_list_get(function->node_list);
	linked_list_goto(function->node_list, 4);
	struct llvm2si_function_node_t *n5 = linked_list_get(function->node_list);
	linked_list_goto(function->node_list, 5);
	struct llvm2si_function_node_t *n6 = linked_list_get(function->node_list);
	linked_list_goto(function->node_list, 6);
	struct llvm2si_function_node_t *n7 = linked_list_get(function->node_list);

	/* Pre-order traversal */
	llvm2si_function_preorder_traversal(function, NULL);

	struct linked_list_t *node_list = linked_list_create();

	printf("\n\n\n=== Loop from node 'n1' ===\n\n");
	linked_list_clear(node_list);
	llvm2si_function_natural_loop(function, n1, node_list);
	llvm2si_function_node_list_dump(node_list, stdout);

	printf("\n\n\n=== Loop from node 'n2' ===\n\n");
	linked_list_clear(node_list);
	llvm2si_function_natural_loop(function, n2, node_list);
	llvm2si_function_node_list_dump(node_list, stdout);

	printf("\n\n\n=== Loop from node 'n3' ===\n\n");
	linked_list_clear(node_list);
	llvm2si_function_natural_loop(function, n3, node_list);
	llvm2si_function_node_list_dump(node_list, stdout);

	printf("\n\n\n=== Loop from node 'n4' ===\n\n");
	linked_list_clear(node_list);
	llvm2si_function_natural_loop(function, n4, node_list);
	llvm2si_function_node_list_dump(node_list, stdout);

	printf("\n\n\n=== Loop from node 'n5' ===\n\n");
	linked_list_clear(node_list);
	llvm2si_function_natural_loop(function, n5, node_list);
	llvm2si_function_node_list_dump(node_list, stdout);

	printf("\n\n\n=== Loop from node 'n6' ===\n\n");
	linked_list_clear(node_list);
	llvm2si_function_natural_loop(function, n6, node_list);
	llvm2si_function_node_list_dump(node_list, stdout);

	printf("\n\n\n=== Loop from node 'n7' ===\n\n");
	linked_list_clear(node_list);
	llvm2si_function_natural_loop(function, n7, node_list);
	llvm2si_function_node_list_dump(node_list, stdout);

	printf("\n\n");
	exit(0);
}


/* Create a list of nodes and their edges identical to the CFG given by the
 * function's basic blocks. */
void llvm2si_function_node_list_init(struct llvm2si_function_t *function)
{
	struct llvm2si_basic_block_t *basic_block;
	struct llvm2si_basic_block_t *basic_block_succ;
	struct llvm2si_function_node_t *node;
	struct llvm2si_function_node_t *node_succ;

	/* Create the nodes */
	assert(function->basic_block_entry);
	LINKED_LIST_FOR_EACH(function->basic_block_list)
	{
		basic_block = linked_list_get(function->basic_block_list);
		node = llvm2si_function_node_create_leaf(function, basic_block);
		linked_list_add(function->node_list, node);

		/* Set head node */
		if (basic_block == function->basic_block_entry)
		{
			assert(!function->node_entry);
			function->node_entry = node;
		}
	}

	/* An entry node must have been created */
	assert(function->node_entry);

	/* Add edges to the graph */
	LINKED_LIST_FOR_EACH(function->basic_block_list)
	{
		basic_block = linked_list_get(function->basic_block_list);
		node = basic_block->node;
		LINKED_LIST_FOR_EACH(basic_block->succ_list)
		{
			basic_block_succ = linked_list_get(basic_block->succ_list);
			node_succ = basic_block_succ->node;
			assert(node_succ);
			llvm2si_function_node_connect(node, node_succ);
		}
	}
}


enum llvm2si_function_region_t
{
	llvm2si_function_region_invalid,
	llvm2si_function_region_block,
	llvm2si_function_region_if_then,
	llvm2si_function_region_if_then_else,
	llvm2si_function_region_self_loop,
	llvm2si_function_region_while_loop,
	llvm2si_function_region_natural_loop,
	llvm2si_function_region_improper
};


struct str_map_t llvm2si_function_region_map =
{
	7,
	{
		{ "block", llvm2si_function_region_block },
		{ "if-then", llvm2si_function_region_if_then },
		{ "if-then-else", llvm2si_function_region_if_then_else },
		{ "self-loop", llvm2si_function_region_self_loop },
		{ "while-loop", llvm2si_function_region_while_loop },
		{ "natural-loop", llvm2si_function_region_natural_loop },
		{ "improper", llvm2si_function_region_improper }
	}
};


/* Identify an block region, and return it in 'node_list'. The list
 * 'node_list' must be empty when the function is called. If a valid block
 * region is identified, the function returns true. Otherwise, it returns
 * false and 'node_list' remains empty. */
int llvm2si_function_get_region_block(struct llvm2si_function_t *function,
		struct llvm2si_function_node_t *node,
		struct linked_list_t *node_list)
{
	int has_one_pred;
	int has_one_succ;

	/* List of nodes must be empty */
	assert(!node_list->count);

	/* Try to find a block of consecutive nodes */
	has_one_pred = 1;
	has_one_succ = node->succ_list->count == 1;
	while (has_one_pred && has_one_succ)
	{
		linked_list_add(node_list, node);
		linked_list_head(node->succ_list);
		node = linked_list_get(node->succ_list);
		assert(node);
		has_one_pred = node->pred_list->count == 1;
		has_one_succ = node->succ_list->count == 1;
	}
	if (has_one_pred)
		linked_list_add(node_list, node);
	
	/* Region identified */
	if (node_list->count > 1)
		return 1;

	/* Nothing identified */
	linked_list_clear(node_list);
	return 0;
}


void llvm2si_function_struct_analysis(struct llvm2si_function_t *function)
{
	struct llvm2si_function_node_t *node;

	struct linked_list_t *preorder_list;
	struct linked_list_t *node_list;

	int reduction;

	/* Create the control flow graph */
	llvm2si_function_node_list_init(function);

	/* Main algorithm, based on the pseudo-code given in "Advanced Compiler
	 * Design and Implementation" by Steven S. Muchnick, page 206. */
	llvm2si_function_example(function); /////////
	preorder_list = linked_list_create();
	node_list = linked_list_create();
	do
	{
		reduction = 0;
		linked_list_clear(preorder_list);
		llvm2si_function_preorder_traversal(function, preorder_list);
		while (preorder_list->count > 1 && !reduction)
		{
			/* Get the node with lowest preorder ID */
			linked_list_head(preorder_list);
			node = linked_list_get(preorder_list);

			/* Identify acyclic region */
			linked_list_clear(node_list);
			if (llvm2si_function_get_region_block(function,
					node, node_list))
			{
				//printf("region = block\n");
				//llvm2si_function_node_list_dump(node_list, stdout);
				//printf("\n\n");

				llvm2si_function_reduce(function, node_list);
				reduction = 1;

				//llvm2si_function_dump_control_tree(function, stdout);

				continue;
			}

			/* No region identified, remove one node */
			linked_list_head(preorder_list);
			linked_list_remove(preorder_list);
		}
	} while (reduction);

	/* Free data structures */
	linked_list_free(preorder_list);
	linked_list_free(node_list);
}


void llvm2si_function_dump_control_tree(struct llvm2si_function_t *function,
		FILE *f)
{
	llvm2si_function_node_list_dump_detail(function->node_list, f);
}

