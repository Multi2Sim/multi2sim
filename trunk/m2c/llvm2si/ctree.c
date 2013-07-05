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
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "basic-block.h"
#include "ctree.h"
#include "llvm2si.h"
#include "node.h"


/*
 * Private Functions
 */

/* Recursive DFS traversal for a node. */
static int llvm2si_ctree_dfs_node(struct llvm2si_node_t *node,
		struct linked_list_t *postorder_list, int time)
{
	struct llvm2si_node_t *succ_node;

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
			time = llvm2si_ctree_dfs_node(succ_node,
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
static void llvm2si_ctree_dfs(struct llvm2si_ctree_t *ctree,
		struct linked_list_t *postorder_list)
{
	struct llvm2si_node_t *node;

	/* Function must have an entry */
	assert(ctree->node_entry);

	/* Clear postorder list */
	if (postorder_list)
		linked_list_clear(postorder_list);

	/* Initialize nodes */
	LINKED_LIST_FOR_EACH(ctree->node_list)
	{
		node = linked_list_get(ctree->node_list);
		node->preorder_id = -1;
		node->postorder_id = -1;
		node->color = 0;  /* White */
		linked_list_clear(node->back_edge_list);
		linked_list_clear(node->cross_edge_list);
		linked_list_clear(node->tree_edge_list);
		linked_list_clear(node->forward_edge_list);
	}

	/* Initiate recursion */
	llvm2si_ctree_dfs_node(ctree->node_entry, postorder_list, 0);
}


/* Recursive helper function for natural loop discovery */
static void llvm2si_ctree_reach_under_node(
		struct llvm2si_node_t *header_node,
		struct llvm2si_node_t *node,
		struct linked_list_t *reach_under_list)
{
	struct llvm2si_node_t *pred_node;

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
			llvm2si_ctree_reach_under_node(
				header_node, pred_node, reach_under_list);
	}
}


/* Discover the natural loop (interval) with header 'header_node'. The interval
 * is composed of all those nodes with a path from the header to the tail that
 * doesn't go through the header, where the tail is a node that is connected to
 * the header with a back-edge. */
static void llvm2si_ctree_reach_under(struct llvm2si_ctree_t *ctree,
		struct llvm2si_node_t *header_node,
		struct linked_list_t *reach_under_list)
{
	struct llvm2si_node_t *node;
	struct llvm2si_node_t *pred_node;

	/* Reset output list */
	linked_list_clear(reach_under_list);

	/* Initialize nodes */
	LINKED_LIST_FOR_EACH(ctree->node_list)
	{
		node = linked_list_get(ctree->node_list);
		node->color = 0;  /* Not visited */
	}

	/* For all back-edges entering 'header_node', follow edges backwards and
	 * keep adding nodes. */
	LINKED_LIST_FOR_EACH(header_node->pred_list)
	{
		pred_node = linked_list_get(header_node->pred_list);
		if (llvm2si_node_in_list(header_node,
				pred_node->back_edge_list))
			llvm2si_ctree_reach_under_node(header_node,
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
static struct llvm2si_node_t *llvm2si_ctree_reduce(
		struct llvm2si_ctree_t *ctree,
		struct linked_list_t *node_list,
		enum llvm2si_node_region_t region)
{
	struct llvm2si_node_t *abs_node;
	struct llvm2si_node_t *tmp_node;
	struct llvm2si_node_t *out_node;
	struct llvm2si_node_t *in_node;

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
		if (!llvm2si_node_in_list(tmp_node,
				ctree->node_list))
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
	LINKED_LIST_FOR_EACH(ctree->node_list)
	{
		tmp_node = linked_list_get(ctree->node_list);
		if (tmp_node->kind == llvm2si_node_abstract &&
				tmp_node->abstract.region == region)
			region_count++;
	}

	/* Figure out a name for the new abstract node */
	snprintf(name, sizeof name, "__%s_%d", str_map_value(
			&llvm2si_node_region_map, region),
			region_count);

	/* Create new abstract node */
	abs_node = llvm2si_node_create_abstract(name, region);
	llvm2si_ctree_add_node(ctree, abs_node);

	/* Special case of block regions: record whether there is an edge that
	 * goes from the last node into the first. In this case, this edge
	 * should stay outside of the reduced region. */
	cyclic_block = 0;
	if (region == llvm2si_node_block)
	{
		in_node = linked_list_goto(node_list, 0);
		out_node = linked_list_goto(node_list, node_list->count - 1);
		assert(in_node && out_node);
		if (llvm2si_node_in_list(in_node, out_node->succ_list))
		{
			cyclic_block = 1;
			llvm2si_node_disconnect(out_node, in_node);
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
			if (!llvm2si_node_in_list(in_node, node_list) &&
					!llvm2si_node_in_list(in_node,
					in_node_list))
				linked_list_add(in_node_list, in_node);
		}

		/* Traverse outgoing edges, and store in 'out_node_list' those
		 * that go outside of 'node_list'. */
		LINKED_LIST_FOR_EACH(tmp_node->succ_list)
		{
			out_node = linked_list_get(tmp_node->succ_list);
			if (!llvm2si_node_in_list(out_node, node_list) &&
					!llvm2si_node_in_list(out_node,
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
			llvm2si_node_try_disconnect(in_node, tmp_node);
		}
		LINKED_LIST_FOR_EACH(out_node_list)
		{
			out_node = linked_list_get(out_node_list);
			llvm2si_node_try_disconnect(tmp_node, out_node);
		}
	}

	/* Add all incoming/outgoing edges as predecessors/successors of the new
	 * abstract node. */
	LINKED_LIST_FOR_EACH(in_node_list)
	{
		in_node = linked_list_get(in_node_list);
		llvm2si_node_connect(in_node, abs_node);
	}
	LINKED_LIST_FOR_EACH(out_node_list)
	{
		out_node = linked_list_get(out_node_list);
		llvm2si_node_connect(abs_node, out_node);
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
	if (cyclic_block && !llvm2si_node_in_list(abs_node, abs_node->succ_list))
		llvm2si_node_connect(abs_node, abs_node);

	/* If entry node is part of the nodes that were replaced, set it to the
	 * new abstract node. */
	if (llvm2si_node_in_list(ctree->node_entry, node_list))
		ctree->node_entry = abs_node;

	/* Free list of outgoing and incoming edges */
	linked_list_free(out_node_list);
	linked_list_free(in_node_list);

	/* Return created abstract node */
	return abs_node;
}


#define NEW_NODE(name) \
	struct llvm2si_node_t *name = llvm2si_node_create_leaf(#name); \
	llvm2si_ctree_add_node(ctree, name);
#define NEW_EDGE(u, v) \
	llvm2si_node_connect(u, v)

/* Replace the content of the function with the example on page 201 of
 * Muchnick's book. This function is used for debugging purposes. */
void llvm2si_ctree_example(struct llvm2si_ctree_t *ctree)
{
	linked_list_clear(ctree->node_list);

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

	ctree->node_entry = n1;
}

void llvm2si_ctree_example2(struct llvm2si_ctree_t *ctree)
{
	linked_list_clear(ctree->node_list);

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

	ctree->node_entry = n1;
}


void llvm2si_ctree_example3(struct llvm2si_ctree_t *ctree)
{
	linked_list_clear(ctree->node_list);

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

	ctree->node_entry = A;
}


void llvm2si_ctree_example4(struct llvm2si_ctree_t *ctree)
{
	linked_list_clear(ctree->node_list);

	NEW_NODE(A);
	NEW_NODE(B);
	NEW_NODE(C);
	NEW_NODE(D);

	NEW_EDGE(A, B);
	NEW_EDGE(B, C);
	NEW_EDGE(C, D);
	NEW_EDGE(D, A);

	ctree->node_entry = A;
}


/* Identify a region, and return it in 'node_list'. The list
 * 'node_list' must be empty when the function is called. If a valid block
 * region is identified, the function returns true. Otherwise, it returns
 * false and 'node_list' remains empty.
 * List 'node_list' is an output list. */
static enum llvm2si_node_region_t llvm2si_ctree_region(
		struct llvm2si_ctree_t *ctree,
		struct llvm2si_node_t *node,
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
		struct llvm2si_node_t *succ_node;

		succ_node = linked_list_goto(node->succ_list, 0);
		assert(succ_node);

		if (succ_node != ctree->node_entry &&
				succ_node->pred_list->count == 1)
		{
			linked_list_add(node_list, node);
			linked_list_add(node_list, succ_node);
			return llvm2si_node_block;
		}
	}


	/*** 2. If-Then ***/

	if (node->succ_list->count == 2)
	{
		struct llvm2si_node_t *then_node;
		struct llvm2si_node_t *endif_node;
		struct llvm2si_node_t *tmp_node;

		/* Assume one order for 'then' and 'endif' blocks */
		then_node = linked_list_goto(node->succ_list, 0);
		endif_node = linked_list_goto(node->succ_list, 1);
		assert(then_node && endif_node);

		/* Reverse them if necessary */
		if (llvm2si_node_in_list(then_node, endif_node->succ_list))
		{
			tmp_node = then_node;
			then_node = endif_node;
			endif_node = tmp_node;
		}

		/* Check conditions */
		if (then_node->pred_list->count == 1 &&
				llvm2si_node_in_list(node, then_node->pred_list) &&
				then_node->succ_list->count == 1 &&
				llvm2si_node_in_list(endif_node, then_node->succ_list))
		{
			linked_list_add(node_list, node);
			linked_list_add(node_list, then_node);
			linked_list_add(node_list, endif_node);
			return llvm2si_node_if_then;
		}
	}


	/*** 3. If-Then-Else ***/

	if (node->succ_list->count == 2)
	{
		struct llvm2si_node_t *then_node;
		struct llvm2si_node_t *else_node;
		struct llvm2si_node_t *then_succ_node;
		struct llvm2si_node_t *else_succ_node;

		then_node = linked_list_goto(node->succ_list, 0);
		else_node = linked_list_goto(node->succ_list, 1);
		assert(then_node && else_node);

		then_succ_node = linked_list_goto(then_node->succ_list, 0);
		else_succ_node = linked_list_goto(else_node->succ_list, 0);

		if (then_node->pred_list->count == 1 &&
			else_node->pred_list->count == 1 &&
			then_node != ctree->node_entry &&
			else_node != ctree->node_entry &&
			then_node->succ_list->count == 1 &&
			else_node->succ_list->count == 1 &&
			then_succ_node == else_succ_node &&
			then_succ_node != ctree->node_entry &&
			else_succ_node != ctree->node_entry)
		{
			linked_list_add(node_list, node);
			linked_list_add(node_list, then_node);
			linked_list_add(node_list, else_node);
			return llvm2si_node_if_then_else;
		}
	}

	
	/*
	 * Cyclic regions
	 */

	/* Obtain the interval in 'node_list' */
	llvm2si_ctree_reach_under(ctree, node, node_list);
	if (!node_list->count)
		return llvm2si_node_region_invalid;
	

	/*** 1. While-loop ***/
	if (node_list->count == 2)
	{
		struct llvm2si_node_t *head_node;
		struct llvm2si_node_t *tail_node;

		/* Obtain head and tail nodes */
		head_node = node;
		tail_node = linked_list_goto(node_list, 0);
		if (tail_node == head_node)
			linked_list_goto(node_list, 1);
		assert(tail_node != head_node);

		/* Check condition for while loop */
		if (head_node->succ_list->count == 2 &&
				llvm2si_node_in_list(tail_node, head_node->succ_list) &&
				tail_node->succ_list->count == 1 &&
				llvm2si_node_in_list(head_node, tail_node->succ_list) &&
				tail_node->pred_list->count == 1 &&
				llvm2si_node_in_list(head_node, tail_node->pred_list))
			return llvm2si_node_while_loop;
	}

	/*** 2. Loop ***/
	if (node_list->count == 1 && llvm2si_node_in_list(node,
			node->succ_list))
		return llvm2si_node_loop;


	/* Nothing identified */
	linked_list_clear(node_list);
	return llvm2si_node_region_invalid;
}




/*
 * Public Functions
 */

struct llvm2si_ctree_t *llvm2si_ctree_create(void)
{
	struct llvm2si_ctree_t *ctree;

	/* Initialize */
	ctree = xcalloc(1, sizeof(struct llvm2si_ctree_t));
	ctree->node_list = linked_list_create();

	/* Return */
	return ctree;
}


void llvm2si_ctree_free(struct llvm2si_ctree_t *ctree)
{
	llvm2si_ctree_clear(ctree);
	linked_list_free(ctree->node_list);
	free(ctree);
}


void llvm2si_ctree_add_node(struct llvm2si_ctree_t *ctree,
		struct llvm2si_node_t *node)
{
	assert(!llvm2si_node_in_list(node, ctree->node_list));
	linked_list_add(ctree->node_list, node);
	node->ctree = ctree;
}


void llvm2si_ctree_clear(struct llvm2si_ctree_t *ctree)
{
	LINKED_LIST_FOR_EACH(ctree->node_list)
		llvm2si_node_free(linked_list_get(ctree->node_list));
	linked_list_clear(ctree->node_list);
	ctree->node_entry = NULL;
}


void llvm2si_ctree_structural_analysis(struct llvm2si_ctree_t *ctree)
{
	enum llvm2si_node_region_t region;

	struct llvm2si_node_t *node;
	struct llvm2si_node_t *abs_node;

	struct linked_list_t *postorder_list;
	struct linked_list_t *region_list;

	/* Dump into INI file */
	{ /////
/*		struct config_t *config;

		config = config_create("ctree.ini");
		config_load(config);
		llvm2si_ctree_read_from_config(ctree, config);
		config_free(config);*/
	}

	/* Initialize */
	region_list = linked_list_create();

	/* Obtain the DFS spanning tree first, and a post-order traversal of
	 * the CFG in 'postorder_list'. This list will be used for progressive
	 * reduction steps. */
	postorder_list = linked_list_create();
	llvm2si_ctree_dfs(ctree, postorder_list);

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
		region = llvm2si_ctree_region(ctree, node, region_list);
		if (region)
		{
			/* Reduce and reconstruct DFS */
			abs_node = llvm2si_ctree_reduce(ctree,
					region_list, region);
			llvm2si_ctree_dfs(ctree, NULL);

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
					&llvm2si_node_region_map,
					region));
				llvm2si_node_list_dump(region_list, f);
				fprintf(f, "\n");
				llvm2si_ctree_dump(ctree, f);
			}
		}
	}

	/* Free data structures */
	linked_list_free(postorder_list);
	linked_list_free(region_list);
}


void llvm2si_ctree_dump(struct llvm2si_ctree_t *ctree, FILE *f)
{
	struct linked_list_iter_t *iter;
	struct llvm2si_node_t *node;

	/* Legend */
	fprintf(f, "\nControl tree (edges: +forward, -back, *cross, "
			"|tree, =>entry)\n");
	
	/* Dump all nodes */
	iter = linked_list_iter_create(ctree->node_list);
	LINKED_LIST_ITER_FOR_EACH(iter)
	{
		node = linked_list_iter_get(iter);
		if (node == ctree->node_entry)
			fprintf(f, "=>");
		llvm2si_node_dump(node, f);
	}
	linked_list_iter_free(iter);
	fprintf(f, "\n");
}


void llvm2si_ctree_write_to_config(struct llvm2si_ctree_t *ctree,
		struct config_t *config)
{
	struct llvm2si_node_t *node;

	char section[MAX_STRING_SIZE];
	char buf[MAX_STRING_SIZE];

	/* Write information about the node */
	LINKED_LIST_FOR_EACH(ctree->node_list)
	{
		/* Get node */
		node = linked_list_get(ctree->node_list);
		snprintf(section, sizeof section, "Node %s", node->name);
		if (config_section_exists(config, section))
			fatal("%s: duplicate node name ('%s')", __FUNCTION__,
					node->name);

		/* Dump node properties */
		config_write_string(config, section, "Kind", str_map_value(
				&llvm2si_node_kind_map, node->kind));
		if (ctree->node_entry == node)
			config_write_bool(config, section, "Entry", 1);

		/* Successors */
		llvm2si_node_list_dump_buf(node->succ_list, buf, sizeof buf);
		config_write_string(config, section, "Succ", buf);

		/* Predecessors */
		llvm2si_node_list_dump_buf(node->pred_list, buf, sizeof buf);
		config_write_string(config, section, "Pred", buf);

		/* Parent */
		if (node->parent)
			config_write_string(config, section, "StructOf",
					node->parent->name);

		/* Abstract node */
		if (node->kind == llvm2si_node_abstract)
		{
			/* Children */
			llvm2si_node_list_dump_buf(node->abstract.child_list,
					buf, sizeof buf);
			config_write_string(config, section, "Child", buf);

			/* Region */
			config_write_string(config, section, "Region",
					str_map_value(&llvm2si_node_region_map,
					node->abstract.region));
		}

	}


}


void llvm2si_ctree_read_from_config(struct llvm2si_ctree_t *ctree,
		struct config_t *config)
{
	struct list_t *token_list;

	char *section;
	char *file_name;
	char *node_name;
	char *node_kind_str;
	
	enum llvm2si_node_kind_t node_kind;

	/* Clear existing tree */
	llvm2si_ctree_clear(ctree);

	/* Read sections */
	file_name = config_get_file_name(config);
	CONFIG_SECTION_FOR_EACH(config, section)
	{
		/* Section name must be "Node <name>" */
		token_list = str_token_list_create(section, " ");
		if (token_list->count != 2 || strcasecmp(list_get(token_list,
				0), "Node"))
			fatal("%s: %s: invalid section name",
					file_name, section);
		node_name = list_get(token_list, 1);

		/* Get node type */
		node_kind_str = config_read_string(config, section, "Kind", "Leaf");
		node_kind = str_map_string(&llvm2si_node_kind_map,
				node_kind_str);
		if (!node_kind)
			fatal("%s: %s: invalid value for 'Kind'",
					file_name, section);

		printf("%s\n", node_name);

		/* Free section name */
		str_token_list_free(token_list);
	}

	/* There must be an entry */
	if (!ctree->node_entry)
		fatal("%s: no entry node found", file_name);
}

