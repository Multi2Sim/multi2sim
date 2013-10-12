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

#include <lib/class/list.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "ctree.h"


/*
 * Variables
 */

char *ctree_config_file_name = "";
char *ctree_debug_file_name = "";
int ctree_debug_category;

/* List of control trees created during the parsing of the configuration file,
 * to keep track of loaded control trees. */
static List *ctree_list;





/*
 * Non-Class Functions
 */

/* Return a created control tree given its name. */
static CTree *llvm2si_ctree_get(char *name)
{
	CTree *ctree;

	/* Search control tree */
	ListForEach(ctree_list, ctree, CTree)
		if (!strcmp(ctree->name, name))
			return ctree;

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
		CTree *ctree;
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
		ctree = new(CTree, ctree_name);
		CTreeReadFromConfig(ctree, ctree_config, ctree_name);
		ListAdd(ctree_list, asObject(ctree));

		/* Close */
		config_free(ctree_config);
	}
	else if (!strcasecmp(command, "SaveCTree"))
	{
		CTree *ctree;
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
		ctree = llvm2si_ctree_get(ctree_name);
		if (!ctree)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name);

		/* Save control tree in INI file */
		ctree_config = config_create(file_name);
		CTreeWriteToConfig(ctree, ctree_config);
		config_save(ctree_config);
		config_free(ctree_config);
	}
	else if (!strcasecmp(command, "RenameCTree"))
	{
		CTree *ctree;

		char *ctree_name;
		char *ctree_name2;

		/* Syntax: RenameCTree <ctree> <name> */
		if (token_list->count != 3)
			fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command);
		ctree_name = list_get(token_list, 1);
		ctree_name2 = list_get(token_list, 2);

		/* Get control tree */
		ctree = llvm2si_ctree_get(ctree_name);
		if (!ctree)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name);

		/* Rename */
		ctree->name = str_set(ctree->name, ctree_name2);
	}
	else if (!strcasecmp(command, "CompareCTree"))
	{
		CTree *ctree1;
		CTree *ctree2;

		char *ctree_name1;
		char *ctree_name2;

		/* Syntax: CompareCTree <ctree1> <ctree2> */
		if (token_list->count != 3)
			fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command);
		ctree_name1 = list_get(token_list, 1);
		ctree_name2 = list_get(token_list, 2);

		/* Get first control tree */
		ctree1 = llvm2si_ctree_get(ctree_name1);
		if (!ctree1)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name1);

		/* Get second control tree */
		ctree2 = llvm2si_ctree_get(ctree_name2);
		if (!ctree2)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name2);

		/* Compare them */
		CTreeCompare(ctree1, ctree2);
	}
	else if (!strcasecmp(command, "StructuralAnalysis"))
	{
		CTree *ctree;
		char *ctree_name;

		/* Syntax: StructuralAnalysis <ctree> */
		if (token_list->count != 2)
			fatal("%s: %s: invalid syntax",
					__FUNCTION__, command);
		ctree_name = list_get(token_list, 1);

		/* Get control tree */
		ctree = llvm2si_ctree_get(ctree_name);
		if (!ctree)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name);

		/* Structural analysis */
		CTreeStructuralAnalysis(ctree);
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




/*
 * Private Functions
 */


/* Recursive DFS traversal for a node. */
static int CTreeDFSNode(CTree *self, Node *node,
		List *postorder_list, int time)
{
	Node *succ_node;

	node->color = 1;  /* Gray */
	node->preorder_id = time++;
	ListForEach(node->succ_list, succ_node, Node)
	{
		if (succ_node->color == 2)  /* Black */
		{
			/* Forward- or cross-edge */
			if (node->preorder_id < succ_node->preorder_id)
				ListAdd(node->forward_edge_list,
					asObject(succ_node));
			else
				ListAdd(node->cross_edge_list,
					asObject(succ_node));
		}
		else if (succ_node->color == 1)  /* Gray */
		{
			/* This is a back-edge */
			ListAdd(node->back_edge_list, asObject(succ_node));
		}
		else  /* White */
		{
			/* This is a tree-edge */
			ListAdd(node->tree_edge_list, asObject(succ_node));
			time = CTreeDFSNode(self, succ_node, postorder_list, time);
		}
	}
	node->color = 2;  /* Black */
	node->postorder_id = time++;
	if (postorder_list)
		ListAdd(postorder_list, asObject(node));
	return time;
}


/* Depth-first search on function. This creates a depth-first spanning tree and
 * classifies graph edges as tree-, forward-, cross-, and back-edges.
 * Also, a post-order traversal of the graph is dumped in 'postorder_list'.
 * We follow the algorithm presented in  http://www.personal.kent.edu/
 *    ~rmuhamma/Algorithms/MyAlgorithms/GraphAlgor/depthSearch.htm
 */
static void CTreeDFS(CTree *self, List *postorder_list)
{
	Node *node;

	/* Function must have an entry */
	assert(self->entry_node);

	/* Clear postorder list */
	if (postorder_list)
		ListClear(postorder_list);

	/* Initialize nodes */
	ListForEach(self->node_list, node, Node)
	{
		node->preorder_id = -1;
		node->postorder_id = -1;
		node->color = 0;  /* White */
		ListClear(node->back_edge_list);
		ListClear(node->cross_edge_list);
		ListClear(node->tree_edge_list);
		ListClear(node->forward_edge_list);
	}

	/* Initiate recursion */
	CTreeDFSNode(self, self->entry_node, postorder_list, 0);
}


/* Recursive helper function for natural loop discovery */
static void CTreeReachUnderNode(CTree *self, Node *header_node,
		Node *node, List *reach_under_list)
{
	Node *pred_node;

	/* Label as visited and add node */
	node->color = 1;
	ListAdd(reach_under_list, asObject(node));

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
	ListForEach(node->pred_list, pred_node, Node)
		if (!pred_node->color)
			CTreeReachUnderNode(self, header_node, pred_node,
					reach_under_list);
}


/* Discover the natural loop (interval) with header 'header_node'. The interval
 * is composed of all those nodes with a path from the header to the tail that
 * doesn't go through the header, where the tail is a node that is connected to
 * the header with a back-edge. */
static void CTreeReachUnder(CTree *self, Node *header_node,
		List *reach_under_list)
{
	Node *node;
	Node *pred_node;

	/* Reset output list */
	ListClear(reach_under_list);

	/* Initialize nodes */
	ListForEach(self->node_list, node, Node)
		node->color = 0;  /* Not visited */

	/* For all back-edges entering 'header_node', follow edges backwards and
	 * keep adding nodes. */
	ListForEach(header_node->pred_list, pred_node, Node)
		if (NodeInList(header_node, pred_node->back_edge_list))
			CTreeReachUnderNode(self, header_node, pred_node,
					reach_under_list);
}


/* Given an abstract node of type 'block' that was just reduced, take its
 * sub-block regions and flatten them to avoid hierarchical blocks.
 */
static void CTreeFlattenBlock(CTree *self, AbstractNode *abs_node)
{
	List *node_list;

	Node *in_node;
	Node *out_node;
	Node *tmp_node;

	FILE *f;

	/* Initialize */
	node_list = new(List);

	/* Get nodes */
	assert(!asNode(abs_node)->parent);
	assert(abs_node->region == AbstractNodeBlock);
	assert(abs_node->child_list->count == 2);
	in_node = asNode(ListGoto(abs_node->child_list, 0));
	out_node = asNode(ListGoto(abs_node->child_list, 1));

	/* Remove existing connection between child nodes */
	NodeDisconnect(in_node, out_node);
	assert(!in_node->pred_list->count);
	assert(!in_node->succ_list->count);
	assert(!out_node->pred_list->count);
	assert(!out_node->succ_list->count);

	/* Add elements of 'in_node' to 'node_list' */
	if (isAbstractNode(in_node) && asAbstractNode(in_node)->region
			== AbstractNodeBlock)
	{
		/* Save child nodes */
		assert(asAbstractNode(in_node)->region == AbstractNodeBlock);
		ListForEach(asAbstractNode(in_node)->child_list, tmp_node, Node)
			ListAdd(node_list, asObject(tmp_node));

		/* Remove from parent node */
		in_node = asNode(ListFind(abs_node->child_list, asObject(in_node)));
		assert(in_node);
		ListRemove(abs_node->child_list);

		/* Remove from control tree */
		in_node = asNode(ListFind(self->node_list, asObject(in_node)));
		assert(in_node);
		ListRemove(self->node_list);

		/* Free node */
		delete(in_node);
	}
	else
	{
		/* Save node */
		ListAdd(node_list, asObject(in_node));

		/* Remove from children */
		in_node = asNode(ListFind(abs_node->child_list, asObject(in_node)));
		assert(in_node);
		ListRemove(abs_node->child_list);
	}

	/* Add elements of 'out_node' to 'node_list' */
	if (isAbstractNode(out_node) && asAbstractNode(out_node)->region
			== AbstractNodeBlock)
	{
		/* Save child nodes */
		ListForEach(asAbstractNode(out_node)->child_list, tmp_node, Node)
			ListAdd(node_list, asObject(tmp_node));

		/* Remove from parent node */
		out_node = asNode(ListFind(abs_node->child_list,
				asObject(out_node)));
		assert(out_node);
		ListRemove(abs_node->child_list);

		/* Remove from control tree */
		out_node = asNode(ListFind(self->node_list,
				asObject(out_node)));
		assert(out_node);
		ListRemove(self->node_list);

		/* Free node */
		delete(out_node);
	}
	else
	{
		/* Save node */
		ListAdd(node_list, asObject(out_node));

		/* Remove from children */
		out_node = asNode(ListFind(abs_node->child_list,
				asObject(out_node)));
		assert(out_node);
		ListRemove(abs_node->child_list);
	}

	/* Adopt orphan nodes */
	assert(!abs_node->child_list->count);
	ListForEach(node_list, tmp_node, Node)
	{
		ListAdd(abs_node->child_list, asObject(tmp_node));
		tmp_node->parent = asNode(abs_node);
	}

	/* Debug */
	f = debug_file(ctree_debug_category);
	if (f)
	{
		fprintf(f, "Flatten block region '%s' -> ", asNode(abs_node)->name);
		NodeListDump(node_list, f);
		fprintf(f, "\n");
	}

	/* Done */
	delete(node_list);
}


/* Reduce the list of nodes in 'node_list' with a newly created abstract node,
 * returned as the function result.
 * Argument 'name' gives the name of the new abstract node.
 * All incoming edges to any of the nodes in the list will point to 'node'.
 * Likewise, all outgoing edges from any node in the list will come from
 * 'node'.
 */
static AbstractNode *CTreeReduce(CTree *self, List *node_list,
		AbstractNodeRegion region)
{
	AbstractNode *abs_node;
	Node *tmp_node;
	Node *out_node;
	Node *in_node;
	Node *src_node;
	Node *dest_node;

	List *out_edge_src_list;
	List *out_edge_dest_list;
	List *in_edge_src_list;
	List *in_edge_dest_list;

	char name[MAX_STRING_SIZE];

	int cyclic_block;

	FILE *f;

#ifndef NDEBUG

	/* List of nodes must contain at least one node */
	if (!node_list->count)
		panic("%s: node list empty", __FUNCTION__);

	/* All nodes in 'node_list' must be part of the control tree, and none
	 * of them can have a parent yet. */
	ListForEach(node_list, tmp_node, Node)
	{
		if (!NodeInList(tmp_node, self->node_list))
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
		str_map_value(&abstract_node_region_map, region),
		self->name_counter[region]);
	self->name_counter[region]++;

	/* Create new abstract node */
	abs_node = new(AbstractNode, name, region);
	CTreeAddNode(self, asNode(abs_node));

	/* Debug */
	f = debug_file(ctree_debug_category);
	if (f)
	{
		fprintf(f, "\nReducing %s region: ",
			str_map_value(&abstract_node_region_map, region));
		NodeListDump(node_list, f);
		fprintf(f, " -> '%s'\n", asNode(abs_node)->name);
	}

	/* Special case of block regions: record whether there is an edge that
	 * goes from the last node into the first. In this case, this edge
	 * should stay outside of the reduced region. */
	cyclic_block = 0;
	if (region == AbstractNodeBlock)
	{
		in_node = asNode(ListGoto(node_list, 0));
		out_node = asNode(ListGoto(node_list, node_list->count - 1));
		assert(in_node && out_node);
		if (NodeInList(in_node, out_node->succ_list))
		{
			cyclic_block = 1;
			NodeDisconnect(out_node, in_node);
		}
	}

	/* Create a list of incoming edges from the control tree into the
	 * region given in 'node_list', and a list of outgoing edges from the
	 * region in 'node_list' into the rest of the control tree. */
	in_edge_src_list = new(List);
	in_edge_dest_list = new(List);
	out_edge_src_list = new(List);
	out_edge_dest_list = new(List);
	ListForEach(node_list, tmp_node, Node)
	{
		/* Traverse incoming edges, and store those
		 * that come from outside of 'node_list'. */
		ListForEach(tmp_node->pred_list, in_node, Node)
		{
			if (!NodeInList(in_node, node_list))
			{
				ListAdd(in_edge_src_list, asObject(in_node));
				ListAdd(in_edge_dest_list, asObject(tmp_node));
			}
		}

		/* Traverse outgoing edges, and store those
		 * that go outside of 'node_list'. */
		ListForEach(tmp_node->succ_list, out_node, Node)
		{
			if (!NodeInList(out_node, node_list))
			{
				ListAdd(out_edge_src_list, asObject(tmp_node));
				ListAdd(out_edge_dest_list, asObject(out_node));
			}
		}
	}

	/* Reconnect incoming edges to the new abstract node */
	while (in_edge_src_list->count || in_edge_dest_list->count)
	{
		ListHead(in_edge_src_list);
		ListHead(in_edge_dest_list);
		src_node = asNode(ListRemove(in_edge_src_list));
		dest_node = asNode(ListRemove(in_edge_dest_list));
		assert(src_node);
		assert(dest_node);
		NodeReconnectDest(src_node, dest_node, asNode(abs_node));
	}

	/* Reconnect outgoing edges from the new abstract node */
	while (out_edge_src_list->count || out_edge_dest_list->count)
	{
		ListHead(out_edge_src_list);
		ListHead(out_edge_dest_list);
		src_node = asNode(ListRemove(out_edge_src_list));
		dest_node = asNode(ListRemove(out_edge_dest_list));
		assert(src_node);
		assert(dest_node);
		NodeReconnectSource(src_node, dest_node, asNode(abs_node));
	}

	/* Add all nodes as child nodes of the new abstract node */
	assert(!abs_node->child_list->count);
	ListForEach(node_list, tmp_node, Node)
	{
		assert(!tmp_node->parent);
		tmp_node->parent = asNode(abs_node);
		ListAdd(abs_node->child_list, asObject(tmp_node));
	}

	/* Special case for block regions: if a cyclic block was detected, now
	 * the cycle must be inserted as a self-loop in the abstract node. */
	if (cyclic_block && !NodeInList(asNode(abs_node),
			asNode(abs_node)->succ_list))
		NodeConnect(asNode(abs_node), asNode(abs_node));

	/* If entry node is part of the nodes that were replaced, set it to the
	 * new abstract node. */
	if (NodeInList(self->entry_node, node_list))
		self->entry_node = asNode(abs_node);

	/* Free structures */
	delete(in_edge_src_list);
	delete(in_edge_dest_list);
	delete(out_edge_src_list);
	delete(out_edge_dest_list);

	/* Special case for block regions: in order to avoid nested blocks,
	 * block regions are flattened when we detect that one block contains
	 * another. */
	if (region == AbstractNodeBlock)
	{
		assert(node_list->count == 2);
		in_node = asNode(ListGoto(node_list, 0));
		out_node = asNode(ListGoto(node_list, 1));
		assert(in_node && out_node);

		if ((isAbstractNode(in_node) &&
				asAbstractNode(in_node)->region == AbstractNodeBlock) ||
				(isAbstractNode(out_node) &&
				asAbstractNode(out_node)->region == AbstractNodeBlock))
			CTreeFlattenBlock(self, abs_node);
	}

	/* Special case for while loops: a pre-header and exit blocks are added
	 * into the region. */
	if (region == AbstractNodeWhileLoop)
	{
		Node *head_node;
		Node *tail_node;
		LeafNode *pre_node;
		LeafNode *exit_node;

		char pre_name[MAX_STRING_SIZE];
		char exit_name[MAX_STRING_SIZE];

		/* Get original nodes */
		assert(node_list->count == 2);
		head_node = asNode(ListGoto(node_list, 0));
		tail_node = asNode(ListGoto(node_list, 1));
		assert(isLeafNode(head_node));
		assert(head_node->role == node_role_head);
		assert(tail_node->role == node_role_tail);

		/* Create pre-header and exit nodes */
		snprintf(pre_name, sizeof pre_name, "%s_pre", asNode(abs_node)->name);
		snprintf(exit_name, sizeof exit_name, "%s_exit", asNode(abs_node)->name);
		pre_node = new(LeafNode, pre_name);
		exit_node = new(LeafNode, exit_name);

		/* Insert pre-header node into control tree */
		CTreeAddNode(self, asNode(pre_node));
		NodeInsertBefore(asNode(pre_node), head_node);
		NodeConnect(asNode(pre_node), head_node);
		asNode(pre_node)->role = node_role_pre;

		/* Insert exit node into control tree */
		CTreeAddNode(self, asNode(exit_node));
		NodeInsertAfter(asNode(exit_node), tail_node);
		NodeConnect(head_node, asNode(exit_node));
		asNode(exit_node)->role = node_role_exit;
	}

	/* Return created abstract node */
	return abs_node;
}


/* Identify a region, and return it in 'node_list'. The list
 * 'node_list' must be empty when the function is called. If a valid block
 * region is identified, the function returns true. Otherwise, it returns
 * false and 'node_list' remains empty.
 * List 'node_list' is an output list. */
static AbstractNodeRegion CTreeRegion(CTree *self, Node *node,
		List *node_list)
{

	/* Reset output region */
	ListClear(node_list);


	/*
	 * Acyclic regions
	 */

	/*** 1. Block region ***/

	/* Find two consecutive nodes A and B, where A is the only predecessor
	 * of B and B is the only successor of A. */
	if (node->succ_list->count == 1)
	{
		Node *succ_node;

		succ_node = asNode(ListGoto(node->succ_list, 0));
		assert(succ_node);

		if (node != succ_node &&
				succ_node != self->entry_node &&
				succ_node->pred_list->count == 1)
		{
			ListAdd(node_list, asObject(node));
			ListAdd(node_list, asObject(succ_node));
			return AbstractNodeBlock;
		}
	}


	/*** 2. If-Then ***/

	if (node->succ_list->count == 2)
	{
		Node *then_node;
		Node *endif_node;
		Node *tmp_node;

		/* Assume one order for 'then' and 'endif' blocks */
		then_node = asNode(ListGoto(node->succ_list, 0));
		endif_node = asNode(ListGoto(node->succ_list, 1));
		assert(then_node && endif_node);

		/* Reverse them if necessary */
		if (NodeInList(then_node, endif_node->succ_list))
		{
			tmp_node = then_node;
			then_node = endif_node;
			endif_node = tmp_node;
		}

		/* Check conditions.
		 * We don't allow 'endif_node' to be the same as 'node'. If they
		 * are, we rather reduce such a scheme as a Loop + WhileLoop + Loop.
		 */
		if (then_node->pred_list->count == 1 &&
				then_node->succ_list->count == 1 &&
				NodeInList(endif_node, then_node->succ_list) &&
				then_node != self->entry_node &&
				node != then_node &&
				node != endif_node)
		{
			/* Create node list - order important! */
			ListAdd(node_list, asObject(node));
			ListAdd(node_list, asObject(then_node));

			/* Set node roles */
			node->role = node_role_if;
			then_node->role = node_role_then;

			/* Return region */
			return AbstractNodeIfThen;
		}
	}


	/*** 3. If-Then-Else ***/

	if (node->succ_list->count == 2)
	{
		Node *then_node;
		Node *else_node;
		Node *then_succ_node;
		Node *else_succ_node;

		then_node = asNode(ListGoto(node->succ_list, 0));
		else_node = asNode(ListGoto(node->succ_list, 1));
		assert(then_node && else_node);

		then_succ_node = asNode(ListGoto(then_node->succ_list, 0));
		else_succ_node = asNode(ListGoto(else_node->succ_list, 0));

		/* As opposed to the 'If-Then' region, we allow here the
		 * 'endif_node' to be the same as 'node'. */
		if (then_node->pred_list->count == 1 &&
			else_node->pred_list->count == 1 &&
			then_node != self->entry_node &&
			else_node != self->entry_node &&
			then_node->succ_list->count == 1 &&
			else_node->succ_list->count == 1 &&
			then_succ_node == else_succ_node &&
			then_succ_node != self->entry_node &&
			else_succ_node != self->entry_node)
		{
			/* Create list of nodes - notice order! */
			ListAdd(node_list, asObject(node));
			ListAdd(node_list, asObject(then_node));
			ListAdd(node_list, asObject(else_node));

			/* Assign roles */
			node->role = node_role_if;
			then_node->role = node_role_then;
			else_node->role = node_role_else;

			/* Return region */
			return AbstractNodeIfThenElse;
		}
	}

	/*** 4. Loop ***/
	if (NodeInList(node, node->succ_list))
	{
		ListAdd(node_list, asObject(node));
		return AbstractNodeLoop;
	}


	
	/*
	 * Cyclic regions
	 */

	/* Obtain the interval in 'node_list' */
	CTreeReachUnder(self, node, node_list);
	if (!node_list->count)
		return AbstractNodeRegionInvalid;
	

	/*** 1. While-loop ***/
	if (node_list->count == 2 && node->succ_list->count == 2)
	{
		Node *head_node;
		Node *tail_node;
		Node *exit_node;

		/* Obtain head and tail nodes */
		head_node = node;
		tail_node = asNode(ListGoto(node_list, 0));
		if (tail_node == head_node)
			tail_node = asNode(ListGoto(node_list, 1));
		assert(tail_node != head_node);

		/* Obtain loop exit node */
		exit_node = asNode(ListGoto(node->succ_list, 0));
		if (exit_node == tail_node)
			exit_node = asNode(ListGoto(node->succ_list, 1));
		assert(exit_node != tail_node);

		/* Check condition for while loop */
		if (tail_node->succ_list->count == 1 &&
				NodeInList(head_node, tail_node->succ_list) &&
				tail_node->pred_list->count == 1 &&
				NodeInList(head_node, tail_node->pred_list) &&
				tail_node != self->entry_node &&
				exit_node != head_node)
		{
			/* Create node list. The order is important, so we make
			 * sure that head node is shown first */
			ListClear(node_list);
			ListAdd(node_list, asObject(head_node));
			ListAdd(node_list, asObject(tail_node));

			/* Set node roles */
			head_node->role = node_role_head;
			tail_node->role = node_role_tail;

			/* Determine here whether the loop exists when the condition
			 * in its head node is evaluated to true or false - we need
			 * this info later!
			 *
			 * This is inferred from the order in which the head's
			 * outgoing edges show up in its successor list. The edge
			 * occurring first points to basic block 'if_true' of the
			 * LLVM 'br' instruction, while the second edge points to
			 * basic block 'if_false'.
			 *
			 * Thus, if edge head=>tail is the first, the loop exists
			 * if the head condition is false. If edge head=>tail is
			 * the second, it exists if the condition is true.
			 */
			ListFind(head_node->succ_list, asObject(tail_node));
			assert(!head_node->succ_list->error);
			head_node->exit_if_false = head_node->succ_list
					->current_index == 0;
			head_node->exit_if_true = head_node->succ_list
					->current_index == 1;

			/* Return region */
			return AbstractNodeWhileLoop;
		}
	}

	
	/* Nothing identified */
	ListClear(node_list);
	return AbstractNodeRegionInvalid;
}


static void CTreeTraverseNode(CTree *self, Node *node,
		List *preorder_list, List *postorder_list)
{
	Node *child_node;

	/* Pre-order visit */
	if (preorder_list)
		ListAdd(preorder_list, asObject(node));

	/* Visit children */
	if (isAbstractNode(node))
		ListForEach(asAbstractNode(node)->child_list, child_node, Node)
			CTreeTraverseNode(self, child_node, preorder_list, postorder_list);

	/* Post-order visit */
	if (postorder_list)
		ListAdd(postorder_list, asObject(node));
}




/*
 * Public Functions
 */

void CTreeCreate(CTree *self, char *name)
{
	/* No anonymous */
	if (!name || !*name)
		fatal("%s: no name given", __FUNCTION__);

	/* Initialize */
	self->name = str_set(self->name, name);
	self->node_list = new(List);
	self->node_table = hash_table_create(0, 1);
}


void CTreeDestroy(CTree *self)
{
	CTreeClear(self);
	delete(self->node_list);
	hash_table_free(self->node_table);
	str_free(self->name);
}


void CTreeDump(Object *self, FILE *f)
{
	ListIterator *iter;

	CTree *ctree;
	Node *node;

	/* Legend */
	ctree = asCTree(self);
	fprintf(f, "\nControl tree (edges: +forward, -back, *cross, "
			"|tree, =>entry)\n");
	
	/* Dump all nodes */
	iter = new(ListIterator, ctree->node_list);
	ListIteratorForEach(iter, node, Node)
	{
		if (node == ctree->entry_node)
			fprintf(f, "=>");

		/* Call the virtual function */
		asObject(node)->Dump(asObject(node), f);
		fprintf(f, "\n");
	}
	delete(iter);
	fprintf(f, "\n");
}


void CTreeAddNode(CTree *self, Node *node)
{
	/* Insert node in list */
	assert(!NodeInList(node, self->node_list));
	ListAdd(self->node_list, asObject(node));

	/* Insert in hash table */
	if (!hash_table_insert(self->node_table, node->name, node))
		fatal("%s: duplicate node name ('%s')",
				__FUNCTION__, node->name);

	/* Record tree in node */
	assert(!node->ctree);
	node->ctree = self;
}


void CTreeClear(CTree *self)
{
	ListDeleteObjects(self->node_list);
	hash_table_clear(self->node_table);
	self->entry_node = NULL;
}


void CTreeStructuralAnalysis(CTree *self)
{
	AbstractNodeRegion region;

	Node *node;
	AbstractNode *abs_node;

	List *postorder_list;
	List *region_list;

	FILE *f;

	/* Debug */
	ctree_debug("Starting structural analysis on tree '%s'\n\n",
			self->name);

	/* Initialize */
	region_list = new(List);

	/* Obtain the DFS spanning tree first, and a post-order traversal of
	 * the CFG in 'postorder_list'. This list will be used for progressive
	 * reduction steps. */
	postorder_list = new(List);
	CTreeDFS(self, postorder_list);

	/* Sharir's algorithm */
	while (postorder_list->count)
	{
		/* Extract next node in post-order */
		ListHead(postorder_list);
		node = asNode(ListRemove(postorder_list));
		ctree_debug("Processing node '%s'\n", node->name);
		assert(node);

		/* Identify a region starting at 'node'. If a valid region is
		 * found, reduce it into a new abstract node and reconstruct
		 * DFS spanning tree. */
		region = CTreeRegion(self, node, region_list);
		if (region)
		{
			/* Reduce and reconstruct DFS */
			abs_node = CTreeReduce(self, region_list, region);
			CTreeDFS(self, NULL);

			/* Insert new abstract node in post-order list, to make
			 * it be the next one to be processed. */
			ListHead(postorder_list);
			ListInsert(postorder_list, asObject(abs_node));

			/* Debug */
			f = debug_file(ctree_debug_category);
			if (f)
				CTreeDump(asObject(self), f);
		}
	}

	/* Free data structures */
	delete(postorder_list);
	delete(region_list);

	/* Remember that we have run a structural analysis */
	self->structural_analysis_done = 1;

	/* Debug */
	ctree_debug("Done.\n\n");
}


void CTreeTraverse(CTree *self, List *preorder_list,
		List *postorder_list)
{
	FILE *f;

	/* A structural analysis must have been run first */
	if (!self->structural_analysis_done)
		fatal("%s: %s: tree traversal requires structural analysis",
				__FUNCTION__, self->name);

	/* Clear lists */
	if (preorder_list)
		ListClear(preorder_list);
	if (postorder_list)
		ListClear(postorder_list);

	/* Traverse tree recursively */
	CTreeTraverseNode(self, self->entry_node, preorder_list,
			postorder_list);

	/* Debug */
	f = debug_file(ctree_debug_category);
	if (f)
	{
		if (preorder_list)
		{
			fprintf(f, "Traversal of tree '%s' in pre-order:\n",
					self->name);
			NodeListDump(preorder_list, f);
			fprintf(f, "\n\n");
		}
		if (postorder_list)
		{
			fprintf(f, "Traversal of tree '%s' in post-order:\n",
					self->name);
			NodeListDump(postorder_list, f);
			fprintf(f, "\n\n");
		}
	}
}


#ifdef HAVE_LLVM

static LeafNode *CTreeAddLlvmCFGNode(CTree *self, LLVMBasicBlockRef llbb)
{
	LeafNode *node;
	LeafNode *succ_node;
	LeafNode *true_node;
	LeafNode *false_node;

	LLVMValueRef llinst;
	LLVMValueRef llbb_value;
	LLVMValueRef succ_llbb_value;
	LLVMValueRef true_llbb_value;
	LLVMValueRef false_llbb_value;

	LLVMBasicBlockRef succ_llbb;
	LLVMBasicBlockRef true_llbb;
	LLVMBasicBlockRef false_llbb;

	LLVMOpcode llopcode;

	int num_operands;

	char *name;


	/* Get basic block name */
	llbb_value = LLVMBasicBlockAsValue(llbb);
	name = (char *) LLVMGetValueName(llbb_value);
	if (!name || !*name)
		fatal("%s: anonymous LLVM basic blocks not allowed",
			__FUNCTION__);

	/* If node already exists, just return it */
	node = hash_table_get(self->node_table, name);
	if (node)
		return node;

	/* Create node */
	node = new(LeafNode, name);
	CTreeAddNode(self, asNode(node));
	node->llbb = llbb;

	/* Get basic block terminator */
	llinst = LLVMGetBasicBlockTerminator(llbb);
	llopcode = LLVMGetInstructionOpcode(llinst);
	num_operands = LLVMGetNumOperands(llinst);

	/* Unconditional branch: br label <dest> */
	if (llopcode == LLVMBr && num_operands == 1)
	{
		succ_llbb_value = LLVMGetOperand(llinst, 0);
		succ_llbb = LLVMValueAsBasicBlock(succ_llbb_value);
		succ_node = CTreeAddLlvmCFGNode(self, succ_llbb);
		NodeConnect(asNode(node), asNode(succ_node));
		return node;
	}

	/* Conditional branch: br i1 <cond>, label <iftrue>, label <iffalse>
	 * For some reason, LLVM stores the 'then' block as the last operand of
	 * the instruction (see Instructions.cpp, constructor
	 * BranchInst::BranchInst */
	if (llopcode == LLVMBr && num_operands == 3)
	{
		true_llbb_value = LLVMGetOperand(llinst, 2);
		true_llbb = LLVMValueAsBasicBlock(true_llbb_value);
		true_node = CTreeAddLlvmCFGNode(self, true_llbb);
		NodeConnect(asNode(node), asNode(true_node));

		false_llbb_value = LLVMGetOperand(llinst, 1);
		false_llbb = LLVMValueAsBasicBlock(false_llbb_value);
		false_node = CTreeAddLlvmCFGNode(self, false_llbb);
		NodeConnect(asNode(node), asNode(false_node));

		return node;
	}

	/* Function exit: ret */
	if (llopcode == LLVMRet)
		return node;

	/* Invalid terminator */
	fatal("%s: block terminator not supported (%d)",
		__FUNCTION__, llopcode);
	return NULL;
}


LeafNode *CTreeAddLlvmCFG(CTree *self, LLVMValueRef llfunction)
{
	LLVMBasicBlockRef llbb;

	/* Obtain entry basic block */
	llbb = LLVMGetEntryBasicBlock(llfunction);
	assert(llbb);

	/* Insert basic block recursively */
	return CTreeAddLlvmCFGNode(self, llbb);
}

#endif  /* HAVE_LLVM */


Node *CTreeGetNode(CTree *self, char *name)
{
	return hash_table_get(self->node_table, name);
}


void CTreeGetNodeList(CTree *self, List *node_list,
		char *node_list_str)
{
	struct list_t *token_list;
	Node *node;

	char *name;
	int index;

	/* Clear list */
	ListClear(node_list);

	/* Extract nodes */
	token_list = str_token_list_create(node_list_str, ", ");
	LIST_FOR_EACH(token_list, index)
	{
		name = list_get(token_list, index);
		node = CTreeGetNode(self, name);
		if (!node)
			fatal("%s: invalid node name", name);
		ListAdd(node_list, asObject(node));
	}
	str_token_list_free(token_list);
}


void CTreeWriteToConfig(CTree *self, struct config_t *config)
{
	Node *node;

	char section[MAX_STRING_SIZE];
	char buf[MAX_STRING_SIZE];

	/* Control tree must have entry node */
	if (!self->entry_node)
		fatal("%s: control tree without entry node", __FUNCTION__);

	/* Dump control tree section */
	snprintf(section, sizeof section, "CTree.%s", self->name);
	config_write_string(config, section, "Entry", self->entry_node->name);

	/* Write information about the node */
	ListForEach(self->node_list, node, Node)
	{
		snprintf(section, sizeof section, "CTree.%s.Node.%s",
				self->name, node->name);
		if (config_section_exists(config, section))
			fatal("%s: duplicate node name ('%s')", __FUNCTION__,
					node->name);

		/* Dump node properties */
		if (isAbstractNode(node))
			config_write_string(config, section, "Kind", "Abstract");
		else if (isLeafNode(node))
			config_write_string(config, section, "Kind", "Leaf");
		else
			fatal("%s: unknown type of node '%s'", __FUNCTION__,
					node->name);

		/* Successors */
		NodeListDumpBuf(node->succ_list, buf, sizeof buf);
		config_write_string(config, section, "Succ", buf);

		/* Abstract node */
		if (isAbstractNode(node))
		{
			/* Children */
			NodeListDumpBuf(asAbstractNode(node)->child_list,
					buf, sizeof buf);
			config_write_string(config, section, "Child", buf);

			/* Region */
			config_write_string(config, section, "Region",
					str_map_value(&abstract_node_region_map,
					asAbstractNode(node)->region));
		}

	}


}


void CTreeReadFromConfig(CTree *self, struct config_t *config, char *name)
{
	struct list_t *token_list;
	ListIterator *iter;
	Node *node;

	char section_str[MAX_STRING_SIZE];
	char *section;
	char *file_name;
	char *node_name;
	char *kind_str;
	char *region_str;
	
	enum node_kind_t kind;
	AbstractNodeRegion region;

	/* Clear existing tree */
	CTreeClear(self);

	/* Set tree name */
	self->name = str_set(self->name, name);

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
		kind = str_map_string_case(&node_kind_map, kind_str);
		if (!kind)
			fatal("%s: %s: invalid value for 'Kind'",
					file_name, section);

		/* Create node */
		if (kind == node_leaf)
		{
			node = asNode(new(LeafNode, node_name));
		}
		else
		{
			/* Read region */
			region_str = config_read_string(config, section,
					"Region", "");
			region = str_map_string_case(&abstract_node_region_map,
					region_str);
			if (!region)
				fatal("%s: %s: invalid or missing 'Region'",
						file_name, node_name);

			/* Create node */
			node = asNode(new(AbstractNode, node_name, region));
		}

		/* Add node */
		CTreeAddNode(self, node);

		/* Free section name */
		str_token_list_free(token_list);
	}

	/* Read node properties */
	iter = new(ListIterator, self->node_list);
	ListIteratorForEach(iter, node, Node)
	{
		char *node_list_str;

		List *node_list;
		Node *tmp_node;

		/* Get section name */
		snprintf(section_str, sizeof section_str, "CTree.%s.Node.%s",
				self->name, node->name);
		section = section_str;

		/* Successors */
		node_list_str = config_read_string(config, section, "Succ", "");
		node_list = new(List);
		CTreeGetNodeList(self, node_list, node_list_str);
		ListForEach(node_list, tmp_node, Node)
		{
			if (NodeInList(tmp_node, node->succ_list))
				fatal("%s.%s: duplicate successor", self->name,
						node->name);
			NodeConnect(node, tmp_node);
		}
		delete(node_list);

		/* Abstract node */
		if (isAbstractNode(node))
		{
			/* Children */
			node_list_str = config_read_string(config, section, "Child", "");
			node_list = new(List);
			CTreeGetNodeList(self, node_list, node_list_str);
			ListForEach(node_list, tmp_node, Node)
			{
				tmp_node->parent = node;
				if (NodeInList(tmp_node, asAbstractNode(node)->child_list))
					fatal("%s.%s: duplicate child", self->name,
							node->name);
				ListAdd(asAbstractNode(node)->child_list, asObject(tmp_node));
			}
			delete(node_list);
		}
	}
	delete(iter);

	/* Read entry node name */
	snprintf(section_str, sizeof section_str, "CTree.%s", name);
	node_name = config_read_string(config, section_str, "Entry", NULL);
	if (!node_name)
		fatal("%s: %s: no entry node", __FUNCTION__, name);
	self->entry_node = CTreeGetNode(self, node_name);
	if (!self->entry_node)
		fatal("%s: %s: invalid node name", __FUNCTION__, node_name);

	/* Check configuration file syntax */
	config_check(config);
}


void CTreeCompare(CTree *self, CTree *ctree2)
{
	Node *node;
	Node *node2;

	/* Compare entry nodes */
	assert(self->entry_node);
	assert(ctree2->entry_node);
	if (strcmp(self->entry_node->name, ctree2->entry_node->name))
		fatal("'%s' vs '%s': entry nodes differ", self->name,
				ctree2->name);
	
	/* Check that all nodes in tree 1 are in tree 2 */
	ListForEach(self->node_list, node, Node)
		if (!CTreeGetNode(ctree2, node->name))
			fatal("node '%s.%s' not present in tree '%s'",
				self->name, node->name, ctree2->name);

	/* Check that all nodes in tree 2 are in tree 1 */
	ListForEach(ctree2->node_list, node, Node)
		if (!CTreeGetNode(self, node->name))
			fatal("node '%s.%s' not present in tree '%s'",
				ctree2->name, node->name, self->name);

	/* Compare all nodes */
	ListForEach(self->node_list, node, Node)
	{
		node2 = CTreeGetNode(ctree2, node->name);
		assert(node2);
		NodeCompare(node, node2);
	}
}



/*
 * Non-Class Functions
 */

void ctree_init(void)
{
	ctree_debug_category = debug_new_category(ctree_debug_file_name);
	ctree_list = new(List);
	ctree_read_config();
}


void ctree_done(void)
{
	/* Free list of control trees */
	ListDeleteObjects(ctree_list);
	delete(ctree_list);
}

