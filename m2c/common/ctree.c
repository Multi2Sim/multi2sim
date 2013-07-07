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
#include "cnode.h"
#include "ctree.h"


/*
 * Variables
 */

char *ctree_config_file_name = "";
char *ctree_debug_file_name = "";
int ctree_debug_category;

/* List of control trees created during the parsing of the configuration file,
 * to keep track of loaded control trees. */
static struct linked_list_t *ctree_list;





/*
 * Private Functions
 */

/* Return a created control tree given its name. */
static struct ctree_t *llvm2si_get_ctree(char *name)
{
	struct ctree_t *ctree;

	/* Search control tree */
	LINKED_LIST_FOR_EACH(ctree_list)
	{
		ctree = linked_list_get(ctree_list);
		if (!strcmp(ctree->name, name))
			return ctree;
	}

	/* Not found */
	return NULL;
}


/* Process one command read from the control tree configuration file */
static void ctree_process_command(char *string)
{
	struct list_t *token_list;
	char *command;

	/* Get list of tokens */
	token_list = str_token_list_create(string, " ");
	command = list_get(token_list, 0);
	if (!command)
		fatal("%s: empty command", __FUNCTION__);
	
	/* Process command */
	if (!strcasecmp(command, "LoadCTree"))
	{
		struct ctree_t *ctree;
		struct config_t *ctree_config;

		char *file_name;
		char *ctree_name;

		/* Syntax: LoadCTree <file> <name> */
		if (token_list->count != 3)
			fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command);
		file_name = list_get(token_list, 1);
		ctree_name = list_get(token_list, 2);

		/* Open control tree INI file */
		ctree_config = config_create(file_name);
		config_load(ctree_config);
		
		/* Load control tree */
		ctree = ctree_create(ctree_name);
		ctree_read_from_config(ctree, ctree_config, ctree_name);
		linked_list_add(ctree_list, ctree);

		/* Close */
		config_free(ctree_config);
	}
	else if (!strcasecmp(command, "SaveCTree"))
	{
		struct ctree_t *ctree;
		struct config_t *ctree_config;

		char *file_name;
		char *ctree_name;

		/* Syntax: SaveCTree <file> <name> */
		if (token_list->count != 3)
			fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command);
		file_name = list_get(token_list, 1);
		ctree_name = list_get(token_list, 2);

		/* Get control tree */
		ctree = llvm2si_get_ctree(ctree_name);
		if (!ctree)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name);

		/* Save control tree in INI file */
		ctree_config = config_create(file_name);
		ctree_write_to_config(ctree, ctree_config);
		config_save(ctree_config);
		config_free(ctree_config);
	}
	else if (!strcasecmp(command, "RenameCTree"))
	{
		struct ctree_t *ctree;

		char *ctree_name;
		char *ctree_name2;

		/* Syntax: RenameCTree <ctree> <name> */
		if (token_list->count != 3)
			fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command);
		ctree_name = list_get(token_list, 1);
		ctree_name2 = list_get(token_list, 2);

		/* Get control tree */
		ctree = llvm2si_get_ctree(ctree_name);
		if (!ctree)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name);

		/* Rename */
		ctree->name = str_set(ctree->name, ctree_name2);
	}
	else if (!strcasecmp(command, "CompareCTree"))
	{
		struct ctree_t *ctree1;
		struct ctree_t *ctree2;

		char *ctree_name1;
		char *ctree_name2;

		/* Syntax: CompareCTree <ctree1> <ctree2> */
		if (token_list->count != 3)
			fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command);
		ctree_name1 = list_get(token_list, 1);
		ctree_name2 = list_get(token_list, 2);

		/* Get first control tree */
		ctree1 = llvm2si_get_ctree(ctree_name1);
		if (!ctree1)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name1);

		/* Get second control tree */
		ctree2 = llvm2si_get_ctree(ctree_name2);
		if (!ctree2)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name2);

		/* Compare them */
		ctree_compare(ctree1, ctree2);
	}
	else if (!strcasecmp(command, "StructuralAnalysis"))
	{
		struct ctree_t *ctree;
		char *ctree_name;

		/* Syntax: StructuralAnalysis <ctree> */
		if (token_list->count != 2)
			fatal("%s: %s: invalid syntax",
					__FUNCTION__, command);
		ctree_name = list_get(token_list, 1);

		/* Get control tree */
		ctree = llvm2si_get_ctree(ctree_name);
		if (!ctree)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name);

		/* Structural analysis */
		ctree_structural_analysis(ctree);
	}
	else
		fatal("%s: invalid command: %s", __FUNCTION__, command);
	
	/* Free tokens */
	str_token_list_free(token_list);
}


static void ctree_read_config(void)
{
	struct config_t *config;

	char var[MAX_STRING_SIZE];
	char *section;
	char *value;

	int index;

	/* No file specified */
	if (!*ctree_config_file_name)
		return;

	/* Load configuration */
	config = config_create(ctree_config_file_name);
	config_load(config);

	/* Process commands */
	section = "Commands";
	for (index = 0;; index++)
	{
		/* Read next command */
		snprintf(var, sizeof var, "Command[%d]", index);
		value = config_read_string(config, section, var, NULL);
		if (!value)
			break;

		/* Process command */
		ctree_process_command(value);
	}
	
	/* Close configuration file */
	config_check(config);
	config_free(config);
}


/* Recursive DFS traversal for a node. */
static int ctree_dfs_node(struct cnode_t *node,
		struct linked_list_t *postorder_list, int time)
{
	struct cnode_t *succ_node;

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
			time = ctree_dfs_node(succ_node,
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
static void ctree_dfs(struct ctree_t *ctree,
		struct linked_list_t *postorder_list)
{
	struct cnode_t *node;

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
	ctree_dfs_node(ctree->node_entry, postorder_list, 0);
}


/* Recursive helper function for natural loop discovery */
static void ctree_reach_under_node(
		struct cnode_t *header_node,
		struct cnode_t *node,
		struct linked_list_t *reach_under_list)
{
	struct cnode_t *pred_node;

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
			ctree_reach_under_node(
				header_node, pred_node, reach_under_list);
	}
}


/* Discover the natural loop (interval) with header 'header_node'. The interval
 * is composed of all those nodes with a path from the header to the tail that
 * doesn't go through the header, where the tail is a node that is connected to
 * the header with a back-edge. */
static void ctree_reach_under(struct ctree_t *ctree,
		struct cnode_t *header_node,
		struct linked_list_t *reach_under_list)
{
	struct cnode_t *node;
	struct cnode_t *pred_node;

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
		if (cnode_in_list(header_node,
				pred_node->back_edge_list))
			ctree_reach_under_node(header_node,
					pred_node, reach_under_list);
	}
}


/* Given an abstract node of type 'block' that was just reduced, take its
 * sub-block regions and flatten them to avoid hierarchical blocks.
 */
static void ctree_flatten_block(struct ctree_t *ctree,
		struct cnode_t *abs_node)
{
	struct linked_list_t *node_list;

	struct cnode_t *in_node;
	struct cnode_t *out_node;
	struct cnode_t *tmp_node;

	FILE *f;

	/* Initialize */
	node_list = linked_list_create();

	/* Get nodes */
	assert(!abs_node->parent);
	assert(abs_node->kind == cnode_abstract);
	assert(abs_node->abstract.region == cnode_block);
	assert(abs_node->abstract.child_list->count == 2);
	in_node = linked_list_goto(abs_node->abstract.child_list, 0);
	out_node = linked_list_goto(abs_node->abstract.child_list, 1);

	/* Remove existing connection between child nodes */
	cnode_disconnect(in_node, out_node);
	assert(!in_node->pred_list->count);
	assert(!in_node->succ_list->count);
	assert(!out_node->pred_list->count);
	assert(!out_node->succ_list->count);

	/* Add elements of 'in_node' to 'node_list' */
	if (in_node->kind == cnode_leaf)
	{
		/* Save node */
		linked_list_add(node_list, in_node);

		/* Remove from children */
		in_node = linked_list_find(abs_node->abstract.child_list, in_node);
		assert(in_node);
		linked_list_remove(abs_node->abstract.child_list);
	}
	else
	{
		/* Save child nodes */
		assert(in_node->abstract.region == cnode_block);
		LINKED_LIST_FOR_EACH(in_node->abstract.child_list)
			linked_list_add(node_list,
				linked_list_get(in_node->abstract.child_list));

		/* Remove from parent node */
		in_node = linked_list_find(abs_node->abstract.child_list, in_node);
		assert(in_node);
		linked_list_remove(abs_node->abstract.child_list);

		/* Remove from control tree */
		in_node = linked_list_find(ctree->node_list, in_node);
		assert(in_node);
		linked_list_remove(ctree->node_list);

		/* Free node */
		cnode_free(in_node);
	}

	/* Add elements of 'out_node' to 'node_list' */
	if (out_node->kind == cnode_leaf)
	{
		/* Save node */
		linked_list_add(node_list, out_node);

		/* Remove from children */
		out_node = linked_list_find(abs_node->abstract.child_list, out_node);
		assert(out_node);
		linked_list_remove(abs_node->abstract.child_list);
	}
	else
	{
		/* Save child nodes */
		assert(out_node->abstract.region == cnode_block);
		LINKED_LIST_FOR_EACH(out_node->abstract.child_list)
			linked_list_add(node_list,
				linked_list_get(out_node->abstract.child_list));

		/* Remove from parent node */
		out_node = linked_list_find(abs_node->abstract.child_list, out_node);
		assert(out_node);
		linked_list_remove(abs_node->abstract.child_list);

		/* Remove from control tree */
		out_node = linked_list_find(ctree->node_list, out_node);
		assert(out_node);
		linked_list_remove(ctree->node_list);

		/* Free node */
		cnode_free(out_node);
	}

	/* Adopt orphan nodes */
	assert(!abs_node->abstract.child_list->count);
	LINKED_LIST_FOR_EACH(node_list)
	{
		tmp_node = linked_list_get(node_list);
		linked_list_add(abs_node->abstract.child_list, tmp_node);
		tmp_node->parent = abs_node;
	}

	/* Debug */
	f = debug_file(ctree_debug_category);
	if (f)
	{
		fprintf(f, "Flatten block region '%s' -> ", abs_node->name);
		cnode_list_dump(node_list, f);
		fprintf(f, "\n");
	}

	/* Done */
	linked_list_free(node_list);
}


/* Reduce the list of nodes in 'node_list' with a newly created abstract node,
 * returned as the function result.
 * Argument 'name' gives the name of the new abstract node.
 * All incoming edges to any of the nodes in the list will point to 'node'.
 * Likewise, all outgoing edges from any node in the list will come from
 * 'node'.
 */
static struct cnode_t *ctree_reduce(
		struct ctree_t *ctree,
		struct linked_list_t *node_list,
		enum cnode_region_t region)
{
	struct cnode_t *abs_node;
	struct cnode_t *tmp_node;
	struct cnode_t *out_node;
	struct cnode_t *in_node;

	struct linked_list_t *out_node_list;
	struct linked_list_t *in_node_list;

	char name[MAX_STRING_SIZE];

	int cyclic_block;

	FILE *f;

#ifndef NDEBUG

	/* List of nodes must contain at least one node */
	if (!node_list->count)
		panic("%s: node list empty", __FUNCTION__);

	/* All nodes in 'node_list' must be part of the control tree, and none
	 * of them can have a parent yet. */
	LINKED_LIST_FOR_EACH(node_list)
	{
		tmp_node = linked_list_get(node_list);
		if (!cnode_in_list(tmp_node,
				ctree->node_list))
			panic("%s: node not in control tree",
					__FUNCTION__);
		if (tmp_node->parent)
			panic("%s: node has a parent already",
					__FUNCTION__);
	}
#endif

	/* Figure out a name for the new abstract node */
	assert(region);
	snprintf(name, sizeof name, "__%s_%d",
		str_map_value(&cnode_region_map, region),
		ctree->name_counter[region]);
	ctree->name_counter[region]++;

	/* Create new abstract node */
	abs_node = cnode_create_abstract(name, region);
	ctree_add_node(ctree, abs_node);

	/* Debug */
	f = debug_file(ctree_debug_category);
	if (f)
	{
		fprintf(f, "\nReducing %s region: ",
			str_map_value(&cnode_region_map, region));
		cnode_list_dump(node_list, f);
		fprintf(f, " -> '%s'\n", abs_node->name);
	}

	/* Special case of block regions: record whether there is an edge that
	 * goes from the last node into the first. In this case, this edge
	 * should stay outside of the reduced region. */
	cyclic_block = 0;
	if (region == cnode_block)
	{
		in_node = linked_list_goto(node_list, 0);
		out_node = linked_list_goto(node_list, node_list->count - 1);
		assert(in_node && out_node);
		if (cnode_in_list(in_node, out_node->succ_list))
		{
			cyclic_block = 1;
			cnode_disconnect(out_node, in_node);
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
			if (!cnode_in_list(in_node, node_list) &&
					!cnode_in_list(in_node,
					in_node_list))
				linked_list_add(in_node_list, in_node);
		}

		/* Traverse outgoing edges, and store in 'out_node_list' those
		 * that go outside of 'node_list'. */
		LINKED_LIST_FOR_EACH(tmp_node->succ_list)
		{
			out_node = linked_list_get(tmp_node->succ_list);
			if (!cnode_in_list(out_node, node_list) &&
					!cnode_in_list(out_node,
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
			cnode_try_disconnect(in_node, tmp_node);
		}
		LINKED_LIST_FOR_EACH(out_node_list)
		{
			out_node = linked_list_get(out_node_list);
			cnode_try_disconnect(tmp_node, out_node);
		}
	}

	/* Add all incoming/outgoing edges as predecessors/successors of the new
	 * abstract node. */
	LINKED_LIST_FOR_EACH(in_node_list)
	{
		in_node = linked_list_get(in_node_list);
		cnode_connect(in_node, abs_node);
	}
	LINKED_LIST_FOR_EACH(out_node_list)
	{
		out_node = linked_list_get(out_node_list);
		cnode_connect(abs_node, out_node);
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
	if (cyclic_block && !cnode_in_list(abs_node, abs_node->succ_list))
		cnode_connect(abs_node, abs_node);

	/* If entry node is part of the nodes that were replaced, set it to the
	 * new abstract node. */
	if (cnode_in_list(ctree->node_entry, node_list))
		ctree->node_entry = abs_node;

	/* Free list of outgoing and incoming edges */
	linked_list_free(out_node_list);
	linked_list_free(in_node_list);

	/* Special case for nested block regions: when a block has been created
	 * from block+leaf, leaf+block, or block+block, then the new
	 * hierarchical structure should be flattened into one single block.
	 */
	if (region == cnode_block)
	{
		int in_node_is_block;
		int in_node_is_leaf;
		int out_node_is_block;
		int out_node_is_leaf;

		assert(node_list->count == 2);
		in_node = linked_list_goto(node_list, 0);
		out_node = linked_list_goto(node_list, 1);
		assert(in_node && out_node);

		in_node_is_block = in_node->kind == cnode_abstract &&
				in_node->abstract.region == cnode_block;
		out_node_is_block = out_node->kind == cnode_abstract &&
				out_node->abstract.region == cnode_block;
		in_node_is_leaf = in_node->kind == cnode_leaf;
		out_node_is_leaf = out_node->kind == cnode_leaf;
		if ((in_node_is_block && out_node_is_block) ||
				(in_node_is_block && out_node_is_leaf) ||
				(in_node_is_leaf && out_node_is_block))
			ctree_flatten_block(ctree, abs_node);
	}

	/* Return created abstract node */
	return abs_node;
}


#define NEW_NODE(name) \
	struct cnode_t *name = cnode_create_leaf(#name, NULL); \
	ctree_add_node(ctree, name);
#define NEW_EDGE(u, v) \
	cnode_connect(u, v)

/* Replace the content of the function with the example on page 201 of
 * Muchnick's book. This function is used for debugging purposes. */
void ctree_example(struct ctree_t *ctree)
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

void ctree_example2(struct ctree_t *ctree)
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


void ctree_example3(struct ctree_t *ctree)
{
	linked_list_clear(ctree->node_list);
	ctree->name = str_set(ctree->name, "Sharir");

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


void ctree_example4(struct ctree_t *ctree)
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
static enum cnode_region_t ctree_region(
		struct ctree_t *ctree,
		struct cnode_t *node,
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
		struct cnode_t *succ_node;

		succ_node = linked_list_goto(node->succ_list, 0);
		assert(succ_node);

		if (succ_node != ctree->node_entry &&
				succ_node->pred_list->count == 1)
		{
			linked_list_add(node_list, node);
			linked_list_add(node_list, succ_node);
			return cnode_block;
		}
	}


	/*** 2. If-Then ***/

	if (node->succ_list->count == 2)
	{
		struct cnode_t *then_node;
		struct cnode_t *endif_node;
		struct cnode_t *tmp_node;

		/* Assume one order for 'then' and 'endif' blocks */
		then_node = linked_list_goto(node->succ_list, 0);
		endif_node = linked_list_goto(node->succ_list, 1);
		assert(then_node && endif_node);

		/* Reverse them if necessary */
		if (cnode_in_list(then_node, endif_node->succ_list))
		{
			tmp_node = then_node;
			then_node = endif_node;
			endif_node = tmp_node;
		}

		/* Check conditions */
		if (then_node->pred_list->count == 1 &&
				cnode_in_list(node, then_node->pred_list) &&
				then_node->succ_list->count == 1 &&
				cnode_in_list(endif_node, then_node->succ_list))
		{
			linked_list_add(node_list, node);
			linked_list_add(node_list, then_node);
			linked_list_add(node_list, endif_node);
			return cnode_if_then;
		}
	}


	/*** 3. If-Then-Else ***/

	if (node->succ_list->count == 2)
	{
		struct cnode_t *then_node;
		struct cnode_t *else_node;
		struct cnode_t *then_succ_node;
		struct cnode_t *else_succ_node;

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
			return cnode_if_then_else;
		}
	}

	
	/*
	 * Cyclic regions
	 */

	/* Obtain the interval in 'node_list' */
	ctree_reach_under(ctree, node, node_list);
	if (!node_list->count)
		return cnode_region_invalid;
	

	/*** 1. While-loop ***/
	if (node_list->count == 2)
	{
		struct cnode_t *head_node;
		struct cnode_t *tail_node;

		/* Obtain head and tail nodes */
		head_node = node;
		tail_node = linked_list_goto(node_list, 0);
		if (tail_node == head_node)
			linked_list_goto(node_list, 1);
		assert(tail_node != head_node);

		/* Check condition for while loop */
		if (head_node->succ_list->count == 2 &&
				cnode_in_list(tail_node, head_node->succ_list) &&
				tail_node->succ_list->count == 1 &&
				cnode_in_list(head_node, tail_node->succ_list) &&
				tail_node->pred_list->count == 1 &&
				cnode_in_list(head_node, tail_node->pred_list))
			return cnode_while_loop;
	}

	/*** 2. Loop ***/
	if (node_list->count == 1 && cnode_in_list(node,
			node->succ_list))
		return cnode_loop;


	/* Nothing identified */
	linked_list_clear(node_list);
	return cnode_region_invalid;
}




/*
 * Public Functions
 */


void ctree_init(void)
{
	ctree_debug_category = debug_new_category(ctree_debug_file_name);
	ctree_list = linked_list_create();
	ctree_read_config();
}


void ctree_done(void)
{
	/* Free list of control trees */
	LINKED_LIST_FOR_EACH(ctree_list)
		ctree_free(linked_list_get(ctree_list));
	linked_list_free(ctree_list);
}


struct ctree_t *ctree_create(char *name)
{
	struct ctree_t *ctree;

	/* No anonymous */
	if (!name || !*name)
		fatal("%s: anonymous control tree not valid",
				__FUNCTION__);

	/* Initialize */
	ctree = xcalloc(1, sizeof(struct ctree_t));
	ctree->name = str_set(ctree->name, name);
	ctree->node_list = linked_list_create();

	/* Return */
	return ctree;
}


void ctree_free(struct ctree_t *ctree)
{
	ctree_clear(ctree);
	linked_list_free(ctree->node_list);
	ctree->name = str_free(ctree->name);
	free(ctree);
}


void ctree_add_node(struct ctree_t *ctree,
		struct cnode_t *node)
{
	assert(!cnode_in_list(node, ctree->node_list));
	linked_list_add(ctree->node_list, node);
	node->ctree = ctree;
}


void ctree_clear(struct ctree_t *ctree)
{
	LINKED_LIST_FOR_EACH(ctree->node_list)
		cnode_free(linked_list_get(ctree->node_list));
	linked_list_clear(ctree->node_list);
	ctree->node_entry = NULL;
}


void ctree_structural_analysis(struct ctree_t *ctree)
{
	enum cnode_region_t region;

	struct cnode_t *node;
	struct cnode_t *abs_node;

	struct linked_list_t *postorder_list;
	struct linked_list_t *region_list;

	FILE *f;

	/* Initialize */
	region_list = linked_list_create();

	/* Obtain the DFS spanning tree first, and a post-order traversal of
	 * the CFG in 'postorder_list'. This list will be used for progressive
	 * reduction steps. */
	postorder_list = linked_list_create();
	ctree_dfs(ctree, postorder_list);

	/* Sharir's algorithm */
	while (postorder_list->count)
	{
		/* Extract next node in post-order */
		linked_list_head(postorder_list);
		node = linked_list_remove(postorder_list);
		ctree_debug("Processing node '%s'\n", node->name);
		assert(node);

		/* Identify a region starting at 'node'. If a valid region is
		 * found, reduce it into a new abstract node and reconstruct
		 * DFS spanning tree. */
		region = ctree_region(ctree, node, region_list);
		if (region)
		{
			/* Reduce and reconstruct DFS */
			abs_node = ctree_reduce(ctree, region_list, region);
			ctree_dfs(ctree, NULL);

			/* Insert new abstract node in post-order list, to make
			 * it be the next one to be processed. */
			linked_list_head(postorder_list);
			linked_list_insert(postorder_list, abs_node);

			/* Debug */
			f = debug_file(ctree_debug_category);
			if (f)
				ctree_dump(ctree, f);
		}
	}

	/* Free data structures */
	linked_list_free(postorder_list);
	linked_list_free(region_list);

	/* Debug */
	ctree_debug("Done.\n");
}


void ctree_dump(struct ctree_t *ctree, FILE *f)
{
	struct linked_list_iter_t *iter;
	struct cnode_t *node;

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
		cnode_dump(node, f);
	}
	linked_list_iter_free(iter);
	fprintf(f, "\n");
}


struct cnode_t *ctree_get_node(struct ctree_t *ctree,
		char *name)
{
	struct cnode_t *node;

	/* Search node */
	LINKED_LIST_FOR_EACH(ctree->node_list)
	{
		node = linked_list_get(ctree->node_list);
		if (!strcmp(node->name, name))
			return node;
	}

	/* Not find */
	return NULL;
}


void ctree_get_node_list(struct ctree_t *ctree,
		struct linked_list_t *node_list, char *node_list_str)
{
	struct list_t *token_list;
	struct cnode_t *node;

	char *name;
	int index;

	/* Clear list */
	linked_list_clear(node_list);

	/* Extract nodes */
	token_list = str_token_list_create(node_list_str, ", ");
	LIST_FOR_EACH(token_list, index)
	{
		name = list_get(token_list, index);
		node = ctree_get_node(ctree, name);
		if (!node)
			fatal("%s: invalid node name", name);
		linked_list_add(node_list, node);
	}
	str_token_list_free(token_list);
}


void ctree_write_to_config(struct ctree_t *ctree,
		struct config_t *config)
{
	struct cnode_t *node;

	char section[MAX_STRING_SIZE];
	char buf[MAX_STRING_SIZE];

	/* Control tree must have entry node */
	if (!ctree->node_entry)
		fatal("%s: control tree without entry node", __FUNCTION__);

	/* Dump control tree section */
	snprintf(section, sizeof section, "CTree.%s", ctree->name);
	config_write_string(config, section, "Entry", ctree->node_entry->name);

	/* Write information about the node */
	LINKED_LIST_FOR_EACH(ctree->node_list)
	{
		/* Get node */
		node = linked_list_get(ctree->node_list);
		snprintf(section, sizeof section, "CTree.%s.Node.%s",
				ctree->name, node->name);
		if (config_section_exists(config, section))
			fatal("%s: duplicate node name ('%s')", __FUNCTION__,
					node->name);

		/* Dump node properties */
		config_write_string(config, section, "Kind", str_map_value(
				&cnode_kind_map, node->kind));

		/* Successors */
		cnode_list_dump_buf(node->succ_list, buf, sizeof buf);
		config_write_string(config, section, "Succ", buf);

		/* Abstract node */
		if (node->kind == cnode_abstract)
		{
			/* Children */
			cnode_list_dump_buf(node->abstract.child_list,
					buf, sizeof buf);
			config_write_string(config, section, "Child", buf);

			/* Region */
			config_write_string(config, section, "Region",
					str_map_value(&cnode_region_map,
					node->abstract.region));
		}

	}


}


void ctree_read_from_config(struct ctree_t *ctree,
		struct config_t *config, char *name)
{
	struct list_t *token_list;
	struct linked_list_iter_t *iter;
	struct cnode_t *node;

	char section_str[MAX_STRING_SIZE];
	char *section;
	char *file_name;
	char *node_name;
	char *kind_str;
	char *region_str;
	
	enum cnode_kind_t kind;
	enum cnode_region_t region;

	/* Clear existing tree */
	ctree_clear(ctree);

	/* Set tree name */
	ctree->name = str_set(ctree->name, name);

	/* Check that it exists in configuration file */
	snprintf(section_str, sizeof section_str, "CTree.%s", name);
	if (!config_section_exists(config, section_str))
		fatal("%s: %s: tree not found", __FUNCTION__, name);

	/* Read nodes */
	file_name = config_get_file_name(config);
	CONFIG_SECTION_FOR_EACH(config, section)
	{
		/* Section name must be "CTree.<tree>.Node.<node>" */
		token_list = str_token_list_create(section, ".");
		if (token_list->count != 4 ||
				strcasecmp(list_get(token_list, 0), "CTree") ||
				strcasecmp(list_get(token_list, 1), name) ||
				strcasecmp(list_get(token_list, 2), "Node"))
		{
			str_token_list_free(token_list);
			continue;
		}
		
		/* Get node properties */
		node_name = list_get(token_list, 3);
		kind_str = config_read_string(config, section, "Kind", "Leaf");
		kind = str_map_string_case(&cnode_kind_map, kind_str);
		if (!kind)
			fatal("%s: %s: invalid value for 'Kind'",
					file_name, section);

		/* Create node */
		if (kind == cnode_leaf)
		{
			node = cnode_create_leaf(node_name, NULL);
		}
		else
		{
			/* Read region */
			region_str = config_read_string(config, section,
					"Region", "");
			region = str_map_string_case(&cnode_region_map,
					region_str);
			if (!region)
				fatal("%s: %s: invalid or missing 'Region'",
						file_name, node_name);

			/* Create node */
			node = cnode_create_abstract(node_name, region);
		}

		/* Add node */
		ctree_add_node(ctree, node);

		/* Free section name */
		str_token_list_free(token_list);
	}

	/* Read node properties */
	iter = linked_list_iter_create(ctree->node_list);
	LINKED_LIST_ITER_FOR_EACH(iter)
	{
		char *node_list_str;

		struct linked_list_t *node_list;
		struct cnode_t *tmp_node;

		/* Get section name */
		node = linked_list_iter_get(iter);
		snprintf(section_str, sizeof section_str, "CTree.%s.Node.%s",
				ctree->name, node->name);
		section = section_str;

		/* Successors */
		node_list_str = config_read_string(config, section, "Succ", "");
		node_list = linked_list_create();
		ctree_get_node_list(ctree, node_list, node_list_str);
		LINKED_LIST_FOR_EACH(node_list)
		{
			tmp_node = linked_list_get(node_list);
			if (cnode_in_list(tmp_node, node->succ_list))
				fatal("%s.%s: duplicate successor", ctree->name,
						node->name);
			cnode_connect(node, tmp_node);
		}
		linked_list_free(node_list);

		/* Abstract node */
		if (node->kind == cnode_abstract)
		{
			/* Children */
			node_list_str = config_read_string(config, section, "Child", "");
			node_list = linked_list_create();
			ctree_get_node_list(ctree, node_list, node_list_str);
			LINKED_LIST_FOR_EACH(node_list)
			{
				tmp_node = linked_list_get(node_list);
				tmp_node->parent = node;
				if (cnode_in_list(tmp_node, node->abstract.child_list))
					fatal("%s.%s: duplicate child", ctree->name,
							node->name);
				linked_list_add(node->abstract.child_list, tmp_node);
			}
			linked_list_free(node_list);
		}
	}
	linked_list_iter_free(iter);

	/* Read entry node name */
	snprintf(section_str, sizeof section_str, "CTree.%s", name);
	node_name = config_read_string(config, section_str, "Entry", NULL);
	if (!node_name)
		fatal("%s: %s: no entry node", __FUNCTION__, name);
	ctree->node_entry = ctree_get_node(ctree, node_name);
	if (!ctree->node_entry)
		fatal("%s: %s: invalid node name", __FUNCTION__, node_name);

	/* Check configuration file syntax */
	config_check(config);
}


void ctree_compare(struct ctree_t *ctree1,
		struct ctree_t *ctree2)
{
	struct cnode_t *node;
	struct cnode_t *node2;

	/* Compare entry nodes */
	assert(ctree1->node_entry);
	assert(ctree2->node_entry);
	if (strcmp(ctree1->node_entry->name, ctree2->node_entry->name))
		fatal("'%s' vs '%s': entry nodes differ", ctree1->name,
				ctree2->name);
	
	/* Check that all nodes in tree 1 are in tree 2 */
	LINKED_LIST_FOR_EACH(ctree1->node_list)
	{
		node = linked_list_get(ctree1->node_list);
		if (!ctree_get_node(ctree2, node->name))
			fatal("node '%s.%s' not present in tree '%s'",
				ctree1->name, node->name, ctree2->name);
	}

	/* Check that all nodes in tree 2 are in tree 1 */
	LINKED_LIST_FOR_EACH(ctree2->node_list)
	{
		node = linked_list_get(ctree2->node_list);
		if (!ctree_get_node(ctree1, node->name))
			fatal("node '%s.%s' not present in tree '%s'",
				ctree2->name, node->name, ctree1->name);
	}

	/* Compare all nodes */
	LINKED_LIST_FOR_EACH(ctree1->node_list)
	{
		node = linked_list_get(ctree1->node_list);
		node2 = ctree_get_node(ctree2, node->name);
		assert(node2);
		cnode_compare(node, node2);
	}
}


void ctree_load_from_cfg(struct ctree_t *ctree,
		struct linked_list_t *basic_block_list,
		struct basic_block_t *basic_block_entry)
{
	struct basic_block_t *basic_block;
	struct basic_block_t *basic_block_succ;

	struct cnode_t *node;
	struct cnode_t *node_succ;

	/* Clear first */
	ctree_clear(ctree);

	/* Create the nodes */
	LINKED_LIST_FOR_EACH(basic_block_list)
	{
		basic_block = BASIC_BLOCK(linked_list_get(basic_block_list));
		node = cnode_create_leaf(basic_block->name, basic_block);
		ctree_add_node(ctree, node);
		basic_block->cnode = node;

		/* Set head node */
		if (basic_block == basic_block_entry)
		{
			assert(!ctree->node_entry);
			ctree->node_entry = node;
		}
	}

	/* An entry node must have been created */
	assert(ctree->node_entry);

	/* Add edges to the graph */
	LINKED_LIST_FOR_EACH(basic_block_list)
	{
		basic_block = BASIC_BLOCK(linked_list_get(basic_block_list));
		node = basic_block->cnode;
		LINKED_LIST_FOR_EACH(basic_block->succ_list)
		{
			basic_block_succ = BASIC_BLOCK(linked_list_get(basic_block->succ_list));
			node_succ = basic_block_succ->cnode;
			assert(node_succ);
			cnode_connect(node, node_succ);
		}
	}
}
