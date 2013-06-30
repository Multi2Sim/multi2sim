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
		struct linked_list_t *elem_list)
{
	struct llvm2si_function_node_t *node;
	struct llvm2si_function_node_t *elem;

	/* Initialize */
	node = llvm2si_function_node_create(function,
			llvm2si_function_node_abstract);
	node->name = str_set(node->name, "<abstract>");

	/* Initialize list of elements */
	node->abstract.elem_list = linked_list_create();
	LINKED_LIST_FOR_EACH(elem_list)
	{
		elem = linked_list_get(elem_list);
		linked_list_add(node->abstract.elem_list, elem);
	}
	
	/* Return */
	return node;
}


void llvm2si_function_node_free(struct llvm2si_function_node_t *node)
{
	if (node->kind == llvm2si_function_node_abstract)
		linked_list_free(node->abstract.elem_list);
	linked_list_free(node->pred_list);
	linked_list_free(node->succ_list);
	str_free(node->name);
	free(node);
}


void llvm2si_function_node_list_dump(struct linked_list_t *list, FILE *f)
{
	char *comma;
	struct llvm2si_function_node_t *node;

	comma = "";
	fprintf(f, "{");
	LINKED_LIST_FOR_EACH(list)
	{
		node = linked_list_get(list);
		fprintf(f, "%s%s", comma, node->name);
		comma = ",";
	}
	fprintf(f, "}");
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

	/* List of elements */
	if (node->kind == llvm2si_function_node_abstract)
	{
		fprintf(f, " elem=");
		llvm2si_function_node_list_dump(node->abstract.elem_list, f);
	}

	/* Preorder traversal ID */
	fprintf(f, " preorder_id=%d", node->preorder_id);

	/* End */
	fprintf(f, "\n");
}


/* Return true if 'node' is in the linked list of nodes passed as the second
 * argument. */
int llvm2si_function_node_in_list(struct llvm2si_function_node_t *node,
		struct linked_list_t *list)
{
	linked_list_find(list, node);
	return !list->error_code;
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


void llvm2si_function_preorder_traversal(struct llvm2si_function_t *function)
{
	struct llvm2si_function_node_t *node;

	/* Reset preorder traversal ID counter */
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
}


/* Replace the list of nodes in the function control tree with 'node'. All
 * incoming edges to any of the nodes in the list will point to 'node'.
 * Likewise, all outcoming edges from any node in the list will come from
 * 'node'.
 * The node passed in 'node' must have been created with a call to
 * 'llvm2si_function_node_create_abstract', but must not be inserted in the
 * function's node list. */
void llvm2si_function_replace_node(struct llvm2si_function_t *function,
		struct llvm2si_function_node_t *node,
		struct linked_list_t *node_list)
{
	struct llvm2si_function_node_t *tmp_node;

#ifndef NDEBUG

	/* All nodes in 'node_list' must be part of the control tree. */
	LINKED_LIST_FOR_EACH(node_list)
	{
		tmp_node = linked_list_get(node_list);
		if (!llvm2si_function_node_in_list(tmp_node,
				function->node_list))
			panic("%s: node not in control tree",
					__FUNCTION__);
	}
#endif

	/* Find outcoming edges */
	LINKED_LIST_FOR_EACH(node_list)
	{
		tmp_node = linked_list_get(node_list);
		/////// Continue here
	}
}


void llvm2si_function_struct_analysis(struct llvm2si_function_t *function)
{
	struct llvm2si_basic_block_t *basic_block;
	struct llvm2si_basic_block_t *basic_block_succ;
	struct llvm2si_function_node_t *node;
	struct llvm2si_function_node_t *node_succ;

	/* Start by creating a graph of nodes identical to the CFG of
	 * basic blocks. */
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

	/* Dump it */
	//llvm2si_function_preorder_traversal(function);
	//llvm2si_function_dump_control_tree(function, stdout);
	//printf("///////\n");
	//llvm2si_function_dump_cfg(function, stdout);
}


void llvm2si_function_dump_control_tree(struct llvm2si_function_t *function,
		FILE *f)
{
	struct llvm2si_function_node_t *node;
	struct linked_list_iter_t *iter;

	iter = linked_list_iter_create(function->node_list);
	LINKED_LIST_ITER_FOR_EACH(iter)
	{
		node = linked_list_iter_get(iter);
		llvm2si_function_node_dump(node, f);
		linked_list_add(function->node_list, NULL);
	}
	linked_list_iter_free(iter);
}

