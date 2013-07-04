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
#include "llvm2si.h"
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
	node->back_edge_list = linked_list_create();
	node->forward_edge_list = linked_list_create();
	node->cross_edge_list = linked_list_create();
	node->tree_edge_list = linked_list_create();

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
		enum llvm2si_function_node_region_t region,
		char *name)
{
	struct llvm2si_function_node_t *node;

	/* Initialize */
	node = llvm2si_function_node_create(function,
			llvm2si_function_node_abstract);
	node->name = str_set(node->name, name && *name ? name : "<abstract>");
	node->abstract.region = region;
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
	linked_list_free(node->back_edge_list);
	linked_list_free(node->forward_edge_list);
	linked_list_free(node->tree_edge_list);
	linked_list_free(node->cross_edge_list);
	str_free(node->name);
	free(node);
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


void llvm2si_function_node_dump(struct llvm2si_function_node_t *node, FILE *f)
{
	struct llvm2si_function_node_t *succ_node;
	char *no_name;
	char *comma;

	no_name = "<no-name>";
	fprintf(f, "Node '%s':", *node->name ? node->name : no_name);
	fprintf(f, " type=%s", str_map_value(&llvm2si_function_node_kind_map,
			node->kind));
	fprintf(f, " pred=");
	llvm2si_function_node_list_dump(node->pred_list, f);

	/* List of successors */
	fprintf(f, " succ={");
	comma = "";
	LINKED_LIST_FOR_EACH(node->succ_list)
	{
		succ_node = linked_list_get(node->succ_list);
		fprintf(f, "%s", comma);
		if (llvm2si_function_node_in_list(succ_node,
				node->back_edge_list))
			fprintf(f, "-");
		else if (llvm2si_function_node_in_list(succ_node,
				node->forward_edge_list))
			fprintf(f, "+");
		else if (llvm2si_function_node_in_list(succ_node,
				node->tree_edge_list))
			fprintf(f, "|");
		else if (llvm2si_function_node_in_list(succ_node,
				node->cross_edge_list))
			fprintf(f, "*");
		fprintf(f, "%s", succ_node->name);
		comma = ",";
	}

	/* Parent */
	fprintf(f, "} structof=");
	if (node->parent)
		fprintf(f, "'%s'", node->parent->name);
	else
		fprintf(f, "-");

	/* List of child elements */
	if (node->kind == llvm2si_function_node_abstract)
	{
		fprintf(f, " children=");
		llvm2si_function_node_list_dump(node->abstract.child_list, f);
	}

	/* Traversal IDs */
	fprintf(f, " pre=");
	if (node->preorder_id == -1)
		fprintf(f, "-");
	else
		fprintf(f, "%d", node->preorder_id);
	fprintf(f, " post=");
	if (node->postorder_id == -1)
		fprintf(f, "-");
	else
		fprintf(f, "%d", node->postorder_id);

	/* End */
	fprintf(f, "\n");
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
 * Function Object
 */


struct str_map_t llvm2si_function_node_region_map =
{
	9,
	{
		{ "block", llvm2si_function_node_block },
		{ "if_then", llvm2si_function_node_if_then },
		{ "if_then_else", llvm2si_function_node_if_then_else },
		{ "while_loop", llvm2si_function_node_while_loop },
		{ "loop", llvm2si_function_node_loop },
		{ "proper_interval", llvm2si_function_node_proper_interval },
		{ "improper_interval", llvm2si_function_node_improper_interval },
		{ "proper_outer_interval", llvm2si_function_node_proper_outer_interval },
		{ "improper_outer_interval", llvm2si_function_node_improper_outer_interval }
	}
};


/* Recursive DFS traversal for a node. */
static int llvm2si_function_dfs_node(struct llvm2si_function_node_t *node,
		struct linked_list_t *postorder_list, int time)
{
	struct llvm2si_function_node_t *succ_node;

	node->color = 1;  /* Gray */
	node->preorder_id = time++;
	LINKED_LIST_FOR_EACH(node->succ_list)
	{
		succ_node = linked_list_get(node->succ_list);
		if (succ_node->color == 2)  /* Black */
		{
			/* Forward- or cross-edge */
			if (node->preorder_id < succ_node->preorder_id)
				linked_list_add(node->forward_edge_list,
					succ_node);
			else
				linked_list_add(node->cross_edge_list,
					succ_node);
		}
		else if (succ_node->color == 1)  /* Gray */
		{
			/* This is a back-edge */
			linked_list_add(node->back_edge_list, succ_node);
		}
		else  /* White */
		{
			/* This is a tree-edge */
			linked_list_add(node->tree_edge_list, succ_node);
			time = llvm2si_function_dfs_node(succ_node,
					postorder_list, time);
		}
	}
	node->color = 2;  /* Black */
	node->postorder_id = time++;
	if (postorder_list)
		linked_list_add(postorder_list, node);
	return time;
}


/* Depth-first search on function. This creates a depth-first spanning tree and
 * classifies graph edges as tree-, forward-, cross-, and back-edges.
 * Also, a post-order traversal of the graph is dumped in 'postorder_list'.
 * We follow the algorithm presented in  http://www.personal.kent.edu/
 *    ~rmuhamma/Algorithms/MyAlgorithms/GraphAlgor/depthSearch.htm
 */
void llvm2si_function_dfs(struct llvm2si_function_t *function,
		struct linked_list_t *postorder_list)
{
	struct llvm2si_function_node_t *node;

	/* Function must have an entry */
	assert(function->node_entry);

	/* Clear postorder list */
	if (postorder_list)
		linked_list_clear(postorder_list);

	/* Initialize nodes */
	LINKED_LIST_FOR_EACH(function->node_list)
	{
		node = linked_list_get(function->node_list);
		node->preorder_id = -1;
		node->postorder_id = -1;
		node->color = 0;  /* White */
		linked_list_clear(node->back_edge_list);
		linked_list_clear(node->cross_edge_list);
		linked_list_clear(node->tree_edge_list);
		linked_list_clear(node->forward_edge_list);
	}

	/* Initiate recursion */
	llvm2si_function_dfs_node(function->node_entry, postorder_list, 0);
}


/* Recursive helper function for natural loop discovery */
static void llvm2si_function_reach_under_node(
		struct llvm2si_function_node_t *header_node,
		struct llvm2si_function_node_t *node,
		struct linked_list_t *reach_under_list)
{
	struct llvm2si_function_node_t *pred_node;

	/* Label as visited and add node */
	node->color = 1;
	linked_list_add(reach_under_list, node);

	/* Header reached */
	if (node == header_node)
		return;

	/* Node with lower pre-order ID than the head reached. That means that
	 * this is either a cross edge to another branch of the tree, or a
	 * back-edge to a region on top of the tree. This indicates the
	 * occurrence of an improper region. */
	if (node->preorder_id < header_node->preorder_id)
		return;

	/* Add predecessors recursively */
	LINKED_LIST_FOR_EACH(node->pred_list)
	{
		pred_node = linked_list_get(node->pred_list);
		if (!pred_node->color)
			llvm2si_function_reach_under_node(
				header_node, pred_node, reach_under_list);
	}
}


/* Discover the natural loop (interval) with header 'header_node'. The interval
 * is composed of all those nodes with a path from the header to the tail that
 * doesn't go through the header, where the tail is a node that is connected to
 * the header with a back-edge. */
void llvm2si_function_reach_under(struct llvm2si_function_t *function,
		struct llvm2si_function_node_t *header_node,
		struct linked_list_t *reach_under_list)
{
	struct llvm2si_function_node_t *node;
	struct llvm2si_function_node_t *pred_node;

	/* Reset output list */
	linked_list_clear(reach_under_list);

	/* Initialize nodes */
	LINKED_LIST_FOR_EACH(function->node_list)
	{
		node = linked_list_get(function->node_list);
		node->color = 0;  /* Not visited */
	}

	/* For all back-edges entering 'header_node', follow edges backwards and
	 * keep adding nodes. */
	LINKED_LIST_FOR_EACH(header_node->pred_list)
	{
		pred_node = linked_list_get(header_node->pred_list);
		if (llvm2si_function_node_in_list(header_node,
				pred_node->back_edge_list))
			llvm2si_function_reach_under_node(header_node,
					pred_node, reach_under_list);
	}
}


/* Reduce the list of nodes in 'node_list' with a newly created abstract node,
 * returned as the function result.
 * Argument 'name' gives the name of the new abstract node.
 * All incoming edges to any of the nodes in the list will point to 'node'.
 * Likewise, all outgoing edges from any node in the list will come from
 * 'node'.
 */
struct llvm2si_function_node_t *llvm2si_function_reduce(
		struct llvm2si_function_t *function,
		struct linked_list_t *node_list,
		enum llvm2si_function_node_region_t region)
{
	struct llvm2si_function_node_t *abs_node;
	struct llvm2si_function_node_t *tmp_node;
	struct llvm2si_function_node_t *out_node;
	struct llvm2si_function_node_t *in_node;

	struct linked_list_t *out_node_list;
	struct linked_list_t *in_node_list;

	char name[MAX_STRING_SIZE];

	int region_count;
	int cyclic_block;

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

	/* Find the number of existing regions of the same type with in order
	 * to find a unique name for the new abstract node. */
	region_count = 0;
	LINKED_LIST_FOR_EACH(function->node_list)
	{
		tmp_node = linked_list_get(function->node_list);
		if (tmp_node->kind == llvm2si_function_node_abstract &&
				tmp_node->abstract.region == region)
			region_count++;
	}

	/* Figure out a name for the new abstract node */
	snprintf(name, sizeof name, "__%s_%d", str_map_value(
			&llvm2si_function_node_region_map, region),
			region_count);

	/* Create new abstract node */
	abs_node = llvm2si_function_node_create_abstract(function,
			region, name);
	linked_list_add(function->node_list, abs_node);

	/* Special case of block regions: record whether there is an edge that
	 * goes from the last node into the first. In this case, this edge
	 * should stay outside of the reduced region. */
	cyclic_block = 0;
	if (region == llvm2si_function_node_block)
	{
		in_node = linked_list_goto(node_list, 0);
		out_node = linked_list_goto(node_list, node_list->count - 1);
		assert(in_node && out_node);
		if (llvm2si_function_node_in_list(in_node, out_node->succ_list))
		{
			cyclic_block = 1;
			llvm2si_function_node_disconnect(out_node, in_node);
		}
	}

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

	/* Special case for block regions: if a cyclic block was detected, now
	 * the cycle must be inserted as a self-loop in the abstract node. */
	if (cyclic_block && !llvm2si_function_node_in_list(abs_node, abs_node->succ_list))
		llvm2si_function_node_connect(abs_node, abs_node);

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


#define NEW_NODE(name) \
	struct llvm2si_basic_block_t *bb_##name = llvm2si_basic_block_create_with_name(#name); \
	struct llvm2si_function_node_t *name = llvm2si_function_node_create_leaf(function, bb_##name); \
	linked_list_add(function->node_list, name);
#define NEW_EDGE(u, v) \
	llvm2si_function_node_connect(u, v)

/* Replace the content of the function with the example on page 201 of
 * Muchnick's book. This function is used for debugging purposes. */
void llvm2si_function_example(struct llvm2si_function_t *function)
{
	linked_list_clear(function->node_list);

	NEW_NODE(n1);
	NEW_NODE(n2);
	NEW_NODE(n3);
	NEW_NODE(n4);
	NEW_NODE(n5);
	NEW_NODE(n6);
	NEW_NODE(n7);

	NEW_EDGE(n1, n2);
	NEW_EDGE(n2, n3);
	NEW_EDGE(n2, n4);
	NEW_EDGE(n4, n2);
	NEW_EDGE(n3, n5);
	NEW_EDGE(n4, n5);
	NEW_EDGE(n5, n3);
	NEW_EDGE(n5, n6);
	NEW_EDGE(n6, n5);
	NEW_EDGE(n6, n7);

	function->node_entry = n1;
}

void llvm2si_function_example2(struct llvm2si_function_t *function)
{
	linked_list_clear(function->node_list);

	NEW_NODE(n1);
	NEW_NODE(n2);
	NEW_NODE(n3);
	NEW_NODE(n4);
	NEW_NODE(n5);
	NEW_NODE(n6);
	NEW_NODE(n7);
	NEW_NODE(n8);

	NEW_EDGE(n1, n2);
	NEW_EDGE(n1, n8);
	NEW_EDGE(n2, n3);
	NEW_EDGE(n2, n4);
	NEW_EDGE(n3, n7);
	NEW_EDGE(n7, n8);
	NEW_EDGE(n4, n5);
	NEW_EDGE(n5, n6);
	NEW_EDGE(n6, n7);
	NEW_EDGE(n6, n4);

	function->node_entry = n1;
}


void llvm2si_function_example3(struct llvm2si_function_t *function)
{
	linked_list_clear(function->node_list);

	NEW_NODE(A);
	NEW_NODE(B);
	NEW_NODE(C);
	NEW_NODE(D);
	NEW_NODE(E);
	NEW_NODE(F);
	NEW_NODE(G);
	NEW_NODE(H);
	NEW_NODE(I);
	NEW_NODE(J);
	NEW_NODE(K);
	NEW_NODE(L);

	NEW_EDGE(A, B);
	NEW_EDGE(A, K);
	NEW_EDGE(B, C);
	NEW_EDGE(B, I);
	NEW_EDGE(C, D);
	NEW_EDGE(D, H);
	NEW_EDGE(D, E);
	NEW_EDGE(E, F);
	NEW_EDGE(E, G);
	NEW_EDGE(F, D);
	NEW_EDGE(G, D);
	NEW_EDGE(H, I);
	NEW_EDGE(I, J);
	NEW_EDGE(J, A);
	NEW_EDGE(J, I);
	NEW_EDGE(K, L);

	function->node_entry = A;
}


void llvm2si_function_example4(struct llvm2si_function_t *function)
{
	linked_list_clear(function->node_list);

	NEW_NODE(A);
	NEW_NODE(B);
	NEW_NODE(C);
	NEW_NODE(D);

	NEW_EDGE(A, B);
	NEW_EDGE(B, C);
	NEW_EDGE(C, D);
	NEW_EDGE(D, A);

	function->node_entry = A;
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


/* Identify a region, and return it in 'node_list'. The list
 * 'node_list' must be empty when the function is called. If a valid block
 * region is identified, the function returns true. Otherwise, it returns
 * false and 'node_list' remains empty.
 * List 'node_list' is an output list. */
enum llvm2si_function_node_region_t llvm2si_function_region(
		struct llvm2si_function_t *function,
		struct llvm2si_function_node_t *node,
		struct linked_list_t *node_list)
{

	/* Reset output region */
	linked_list_clear(node_list);


	/*
	 * Acyclic regions
	 */

	/*** 1. Block region ***/

	/* Find two consecutive nodes A and B, where A is the only predecessor
	 * of B and B is the only successor of A. */
	if (node->succ_list->count == 1)
	{
		struct llvm2si_function_node_t *succ_node;

		succ_node = linked_list_goto(node->succ_list, 0);
		assert(succ_node);

		if (succ_node != function->node_entry &&
				succ_node->pred_list->count == 1)
		{
			linked_list_add(node_list, node);
			linked_list_add(node_list, succ_node);
			return llvm2si_function_node_block;
		}
	}


	/*** 2. If-Then ***/

	if (node->succ_list->count == 2)
	{
		struct llvm2si_function_node_t *then_node;
		struct llvm2si_function_node_t *endif_node;
		struct llvm2si_function_node_t *tmp_node;

		/* Assume one order for 'then' and 'endif' blocks */
		then_node = linked_list_goto(node->succ_list, 0);
		endif_node = linked_list_goto(node->succ_list, 1);
		assert(then_node && endif_node);

		/* Reverse them if necessary */
		if (llvm2si_function_node_in_list(then_node, endif_node->succ_list))
		{
			tmp_node = then_node;
			then_node = endif_node;
			endif_node = tmp_node;
		}

		/* Check conditions */
		if (then_node->pred_list->count == 1 &&
				llvm2si_function_node_in_list(node, then_node->pred_list) &&
				then_node->succ_list->count == 1 &&
				llvm2si_function_node_in_list(endif_node, then_node->succ_list))
		{
			linked_list_add(node_list, node);
			linked_list_add(node_list, then_node);
			linked_list_add(node_list, endif_node);
			return llvm2si_function_node_if_then;
		}
	}


	/*** 3. If-Then-Else ***/

	if (node->succ_list->count == 2)
	{
		struct llvm2si_function_node_t *then_node;
		struct llvm2si_function_node_t *else_node;
		struct llvm2si_function_node_t *then_succ_node;
		struct llvm2si_function_node_t *else_succ_node;

		then_node = linked_list_goto(node->succ_list, 0);
		else_node = linked_list_goto(node->succ_list, 1);
		assert(then_node && else_node);

		then_succ_node = linked_list_goto(then_node->succ_list, 0);
		else_succ_node = linked_list_goto(else_node->succ_list, 0);

		if (then_node->pred_list->count == 1 &&
			else_node->pred_list->count == 1 &&
			then_node != function->node_entry &&
			else_node != function->node_entry &&
			then_node->succ_list->count == 1 &&
			else_node->succ_list->count == 1 &&
			then_succ_node == else_succ_node &&
			then_succ_node != function->node_entry &&
			else_succ_node != function->node_entry)
		{
			linked_list_add(node_list, node);
			linked_list_add(node_list, then_node);
			linked_list_add(node_list, else_node);
			return llvm2si_function_node_if_then_else;
		}
	}

	
	/*
	 * Cyclic regions
	 */

	/* Obtain the interval in 'node_list' */
	llvm2si_function_reach_under(function, node, node_list);
	if (!node_list->count)
		return llvm2si_function_node_region_invalid;
	

	/*** 1. While-loop ***/
	if (node_list->count == 2)
	{
		struct llvm2si_function_node_t *head_node;
		struct llvm2si_function_node_t *tail_node;

		/* Obtain head and tail nodes */
		head_node = node;
		tail_node = linked_list_goto(node_list, 0);
		if (tail_node == head_node)
			linked_list_goto(node_list, 1);
		assert(tail_node != head_node);

		/* Check condition for while loop */
		if (head_node->succ_list->count == 2 &&
				llvm2si_function_node_in_list(tail_node, head_node->succ_list) &&
				tail_node->succ_list->count == 1 &&
				llvm2si_function_node_in_list(head_node, tail_node->succ_list) &&
				tail_node->pred_list->count == 1 &&
				llvm2si_function_node_in_list(head_node, tail_node->pred_list))
			return llvm2si_function_node_while_loop;
	}

	/*** 2. Loop ***/
	if (node_list->count == 1 && llvm2si_function_node_in_list(node,
			node->succ_list))
		return llvm2si_function_node_loop;


	/* Nothing identified */
	linked_list_clear(node_list);
	return llvm2si_function_node_region_invalid;
}


void llvm2si_function_struct_analysis(struct llvm2si_function_t *function)
{
	enum llvm2si_function_node_region_t region;

	struct llvm2si_function_node_t *node;
	struct llvm2si_function_node_t *abs_node;

	struct linked_list_t *postorder_list;
	struct linked_list_t *region_list;

	/* Create the control flow graph */
	llvm2si_function_node_list_init(function);
	//llvm2si_function_example3(function);  /////

	/* Initialize */
	region_list = linked_list_create();

	/* Obtain the DFS spanning tree first, and a post-order traversal of
	 * the CFG in 'postorder_list'. This list will be used for progressive
	 * reduction steps. */
	postorder_list = linked_list_create();
	llvm2si_function_dfs(function, postorder_list);

	/* Sharir's algorithm */
	while (postorder_list->count)
	{
		/* Extract next node in post-order */
		linked_list_head(postorder_list);
		node = linked_list_remove(postorder_list);
		llvm2si_debug("Processing node '%s'\n", node->name);
		assert(node);

		/* Identify a region starting at 'node'. If a valid region is
		 * found, reduce it into a new abstract node and reconstruct
		 * DFS spanning tree. */
		region = llvm2si_function_region(function, node, region_list);
		if (region)
		{
			/* Reduce and reconstruct DFS */
			abs_node = llvm2si_function_reduce(function,
					region_list, region);
			llvm2si_function_dfs(function, NULL);

			/* Insert new abstract node in post-order list, to make
			 * it be the next one to be processed. */
			linked_list_head(postorder_list);
			linked_list_insert(postorder_list, abs_node);

			/* Debug */
			if (debug_status(llvm2si_debug_category))
			{
				FILE *f = debug_file(llvm2si_debug_category);
				fprintf(f, "\nRegion %s identified: ",
					str_map_value(
					&llvm2si_function_node_region_map,
					region));
				llvm2si_function_node_list_dump(region_list, f);
				fprintf(f, "\n");
				llvm2si_function_dump_control_tree(function, f);
			}
		}
	}

	/* Free data structures */
	linked_list_free(postorder_list);
	linked_list_free(region_list);
}


void llvm2si_function_dump_control_tree(struct llvm2si_function_t *function,
		FILE *f)
{
	struct linked_list_iter_t *iter;
	struct llvm2si_function_node_t *node;

	/* Legend */
	fprintf(f, "\nControl tree (edges: +forward, -back, *cross, "
			"|tree, =>entry)\n");
	
	/* Dump all nodes */
	iter = linked_list_iter_create(function->node_list);
	LINKED_LIST_ITER_FOR_EACH(iter)
	{
		node = linked_list_iter_get(iter);
		if (node == function->node_entry)
			fprintf(f, "=>");
		llvm2si_function_node_dump(node, f);
	}
	linked_list_iter_free(iter);
	fprintf(f, "\n");
}

