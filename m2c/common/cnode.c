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
#include "cnode.h"
#include "ctree.h"


struct str_map_t cnode_kind_map =
{
	2,
	{
		{ "Leaf", cnode_leaf },
		{ "Abstract", cnode_abstract }
	}
};


struct str_map_t cnode_region_map =
{
	9,
	{
		{ "block", cnode_region_block },
		{ "if_then", cnode_region_if_then },
		{ "if_then_else", cnode_region_if_then_else },
		{ "while_loop", cnode_region_while_loop },
		{ "loop", cnode_region_loop },
		{ "proper_interval", cnode_region_proper_interval },
		{ "improper_interval", cnode_region_improper_interval },
		{ "proper_outer_interval", cnode_region_proper_outer_interval },
		{ "improper_outer_interval", cnode_region_improper_outer_interval }
	}
};


struct str_map_t cnode_role_map =
{
	7,
	{
		{ "if", cnode_role_if },
		{ "then", cnode_role_then },
		{ "else", cnode_role_else },
		{ "head", cnode_role_head },
		{ "tail", cnode_role_tail },
		{ "pre", cnode_role_pre },
		{ "exit", cnode_role_exit }
	}
};


static struct cnode_t *cnode_create(
		enum cnode_kind_t kind)
{
	struct cnode_t *node;

	/* Initialize */
	node = xcalloc(1, sizeof(struct cnode_t));
	node->kind = kind;
	node->pred_list = linked_list_create();
	node->succ_list = linked_list_create();
	node->back_edge_list = linked_list_create();
	node->forward_edge_list = linked_list_create();
	node->cross_edge_list = linked_list_create();
	node->tree_edge_list = linked_list_create();
	node->preorder_id = -1;
	node->postorder_id = -1;

	/* Return */
	return node;
}


struct cnode_t *cnode_create_leaf(char *name,
		struct basic_block_t *basic_block)
{
	struct cnode_t *node;

	/* Initialize */
	node = cnode_create(cnode_leaf);
	node->name = str_set(node->name, name);
	node->leaf.basic_block = basic_block;

	/* Return */
	return node;
}


struct cnode_t *cnode_create_abstract(char *name,
		enum cnode_region_t region)
{
	struct cnode_t *node;

	/* Initialize */
	node = cnode_create(cnode_abstract);
	node->name = str_set(node->name, name && *name ? name : "<abstract>");
	node->abstract.region = region;
	node->abstract.child_list = linked_list_create();
	
	/* Return */
	return node;
}


void cnode_free(struct cnode_t *node)
{
	switch (node->kind)
	{
	case cnode_abstract:

		linked_list_free(node->abstract.child_list);
		break;

	case cnode_leaf:

		/* Virtual call to basic block destructor */
		if (node->leaf.basic_block)
			node->leaf.basic_block->destroy(node->leaf.basic_block);
		break;

	default:
		abort();
	}

	linked_list_free(node->pred_list);
	linked_list_free(node->succ_list);
	linked_list_free(node->back_edge_list);
	linked_list_free(node->forward_edge_list);
	linked_list_free(node->tree_edge_list);
	linked_list_free(node->cross_edge_list);
	str_free(node->name);
	free(node);
}


struct basic_block_t *cnode_get_basic_block(struct cnode_t *node)
{
	/* Check that basic block if a leaf */
	if (node->kind != cnode_leaf)
		panic("%s: node '%s' is not a leaf",
				__FUNCTION__, node->name);

	/* Return associated basic block */
	return node->leaf.basic_block;
}


int cnode_in_list(struct cnode_t *node,
		struct linked_list_t *list)
{
	struct linked_list_iter_t *iter;
	int found;

	iter = linked_list_iter_create(list);
	found = linked_list_iter_find(iter, node);
	linked_list_iter_free(iter);

	return found;
}


void cnode_list_dump(struct linked_list_t *list, FILE *f)
{
	char *comma;
	struct linked_list_iter_t *iter;
	struct cnode_t *node;

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


void cnode_list_dump_buf(struct linked_list_t *list, char *buf, int size)
{
	struct cnode_t *node;

	/* Reset buffer */
	if (size)
		*buf = '\0';

	/* Dump elements */
	LINKED_LIST_FOR_EACH(list)
	{
		node = linked_list_get(list);
		str_printf(&buf, &size, "%s ", node->name);
	}
}


void cnode_dump(struct cnode_t *node, FILE *f)
{
	struct cnode_t *succ_node;
	char *no_name;
	char *comma;

	no_name = "<no-name>";
	fprintf(f, "Node '%s':", *node->name ? node->name : no_name);
	fprintf(f, " type=%s", str_map_value(&cnode_kind_map,
			node->kind));
	fprintf(f, " pred=");
	cnode_list_dump(node->pred_list, f);

	/* List of successors */
	fprintf(f, " succ={");
	comma = "";
	LINKED_LIST_FOR_EACH(node->succ_list)
	{
		succ_node = linked_list_get(node->succ_list);
		fprintf(f, "%s", comma);
		if (cnode_in_list(succ_node,
				node->back_edge_list))
			fprintf(f, "-");
		else if (cnode_in_list(succ_node,
				node->forward_edge_list))
			fprintf(f, "+");
		else if (cnode_in_list(succ_node,
				node->tree_edge_list))
			fprintf(f, "|");
		else if (cnode_in_list(succ_node,
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

	/* Role */
	if (node->role)
		fprintf(f, " role=%s", str_map_value(
				&cnode_role_map, node->role));

	/* Loop head nodes exit if false/true */
	if (node->role == cnode_role_head)
	{
		if (node->exit_if_false)
			fprintf(f, " exit_if_false");
		if (node->exit_if_true)
			fprintf(f, "exit_if_true");
	}

	/* List of child elements */
	if (node->kind == cnode_abstract)
	{
		fprintf(f, " children=");
		cnode_list_dump(node->abstract.child_list, f);
	}

	/* Traversal IDs */
	if (node->preorder_id != -1)
		fprintf(f, " pre=%d", node->preorder_id);
	if (node->postorder_id != -1)
		fprintf(f, " post=%d", node->postorder_id);

	/* End */
	fprintf(f, "\n");
}


void cnode_try_connect(struct cnode_t *node,
		struct cnode_t *node_dest)
{
	/* Nothing if edge already exists */
	if (cnode_in_list(node_dest, node->succ_list))
		return;

	/* Add edge */
	assert(!cnode_in_list(node, node_dest->pred_list));
	linked_list_add(node->succ_list, node_dest);
	linked_list_add(node_dest->pred_list, node);
}


void cnode_connect(struct cnode_t *node,
		struct cnode_t *node_dest)
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


void cnode_try_disconnect(struct cnode_t *node,
		struct cnode_t *node_dest)
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


void cnode_disconnect(struct cnode_t *node,
		struct cnode_t *node_dest)
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


void cnode_reconnect_dest(struct cnode_t *src_node,
		struct cnode_t *dest_node,
		struct cnode_t *new_dest_node)
{
	/* Old and new destination must be different */
	if (dest_node == new_dest_node)
		panic("%s: old and new nodes are the same", __FUNCTION__);

	/* Connection must exist */
	linked_list_find(src_node->succ_list, dest_node);
	linked_list_find(dest_node->pred_list, src_node);
	if (src_node->succ_list->error_code ||
			dest_node->pred_list->error_code)
		panic("%s: old edge does not exist", __FUNCTION__);

	/* Remove old edge */
	linked_list_remove(src_node->succ_list);
	linked_list_remove(dest_node->pred_list);

	/* If new edge does not already exists, create it here. Notice that the
	 * successor of 'src_node' will be inserted in the exact same position.
	 * This behavior is critical for some uses of this function. */
	if (!cnode_in_list(src_node, new_dest_node->pred_list))
	{
		linked_list_insert(src_node->succ_list, new_dest_node);
		linked_list_add(new_dest_node->pred_list, src_node);
	}
}


void cnode_reconnect_source(struct cnode_t *src_node,
		struct cnode_t *dest_node,
		struct cnode_t *new_src_node)
{
	/* Old and new sources must be different */
	if (src_node == new_src_node)
		panic("%s: old and new nodes are the same", __FUNCTION__);

	/* Connection must exist */
	linked_list_find(src_node->succ_list, dest_node);
	linked_list_find(dest_node->pred_list, src_node);
	if (src_node->succ_list->error_code ||
			dest_node->pred_list->error_code)
		panic("%s: old edge does not exist", __FUNCTION__);

	/* Remove old edge */
	linked_list_remove(src_node->succ_list);
	linked_list_remove(dest_node->pred_list);

	/* If new edge does not already exists, create it here. Notice that the
	 * predecessor of 'dst_node' will be inserted in the exact same position.
	 * This behavior is critical for some uses of this function. */
	if (!cnode_in_list(dest_node, new_src_node->succ_list))
	{
		linked_list_insert(dest_node->pred_list, new_src_node);
		linked_list_add(new_src_node->succ_list, dest_node);
	}
}


void cnode_insert_before(struct cnode_t *node, struct cnode_t *before)
{
	struct cnode_t *parent;

	/* Check parent */
	parent = before->parent;
	if (!parent)
		panic("%s: node '%s' has no parent",
				__FUNCTION__, before->name);

	/* Insert in common parent */
	node->parent = parent;
	assert(parent->kind == cnode_abstract);
	assert(!cnode_in_list(node, parent->abstract.child_list));
	linked_list_find(parent->abstract.child_list, before);
	assert(!parent->abstract.child_list->error_code);
	linked_list_insert(parent->abstract.child_list, node);
}


void cnode_insert_after(struct cnode_t *node, struct cnode_t *after)
{
	struct cnode_t *parent;
	struct linked_list_t *child_list;

	/* Check parent */
	parent = after->parent;
	if (!parent)
		panic("%s: node '%s' has no parent",
				__FUNCTION__, after->name);

	/* Insert in common parent */
	node->parent = parent;
	assert(parent->kind == cnode_abstract);
	child_list = parent->abstract.child_list;
	assert(!cnode_in_list(node, child_list));
	linked_list_find(child_list, after);
	assert(!child_list->error_code);
	linked_list_next(child_list);
	linked_list_insert(child_list, node);
}


struct cnode_t *cnode_get_first_leaf(struct cnode_t *node)
{
	struct cnode_t *child_node;

	/* Traverse syntax tree down */
	while (node->kind == cnode_abstract)
	{
		linked_list_head(node->abstract.child_list);
		child_node = linked_list_get(node->abstract.child_list);
		assert(child_node);
		node = child_node;
	}

	/* Return leaf */
	assert(node->kind == cnode_leaf);
	return node;
}


struct cnode_t *cnode_get_last_leaf(struct cnode_t *node)
{
	struct cnode_t *child_node;

	/* Traverse syntax tree down */
	while (node->kind == cnode_abstract)
	{
		linked_list_tail(node->abstract.child_list);
		child_node = linked_list_get(node->abstract.child_list);
		assert(child_node);
		node = child_node;
	}

	/* Return leaf */
	assert(node->kind == cnode_leaf);
	return node;
}


void cnode_compare(struct cnode_t *node1,
		struct cnode_t *node2)
{
	struct ctree_t *ctree1;
	struct ctree_t *ctree2;
	struct cnode_t *tmp_node;

	char node_name1[MAX_STRING_SIZE];
	char node_name2[MAX_STRING_SIZE];

	int differ;

	/* Store names */
	ctree1 = node1->ctree;
	ctree2 = node2->ctree;
	snprintf(node_name1, sizeof node_name1, "%s.%s", ctree1->name, node1->name);
	snprintf(node_name2, sizeof node_name2, "%s.%s", ctree2->name, node2->name);

	/* Compare kind */
	if (node1->kind != node2->kind)
		fatal("node kind differs for '%s' and '%s'",
				node_name1, node_name2);

	/* Compare successors */
	differ = node1->succ_list->count != node2->succ_list->count;
	LINKED_LIST_FOR_EACH(node1->succ_list)
	{
		tmp_node = linked_list_get(node1->succ_list);
		tmp_node = ctree_get_node(ctree2, tmp_node->name);
		assert(tmp_node);
		if (!cnode_in_list(tmp_node, node2->succ_list))
			differ = 1;
	}
	if (differ)
		fatal("successors differ for '%s' and '%s'",
				node_name1, node_name2);

	/* Abstract node */
	if (node1->kind == cnode_abstract)
	{
		/* Compare region */
		if (node1->abstract.region != node2->abstract.region)
			fatal("region differs for '%s' and '%s'",
					node_name1, node_name2);

		/* Compare children */
		differ = node1->abstract.child_list->count !=
				node2->abstract.child_list->count;
		LINKED_LIST_FOR_EACH(node1->abstract.child_list)
		{
			tmp_node = linked_list_get(node1->abstract.child_list);
			tmp_node = ctree_get_node(ctree2, tmp_node->name);
			assert(tmp_node);
			if (!cnode_in_list(tmp_node, node2->abstract.child_list))
				differ = 1;
		}
		if (differ)
			fatal("children differ for '%s' and '%s'",
					node_name1, node_name2);
	}
}

