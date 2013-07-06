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
#include "ctree.h"
#include "function.h"
#include "llvm2si.h"
#include "node.h"


struct str_map_t llvm2si_node_kind_map =
{
	2,
	{
		{ "Leaf", llvm2si_node_leaf },
		{ "Abstract", llvm2si_node_abstract }
	}
};


struct str_map_t llvm2si_node_region_map =
{
	9,
	{
		{ "block", llvm2si_node_block },
		{ "if_then", llvm2si_node_if_then },
		{ "if_then_else", llvm2si_node_if_then_else },
		{ "while_loop", llvm2si_node_while_loop },
		{ "loop", llvm2si_node_loop },
		{ "proper_interval", llvm2si_node_proper_interval },
		{ "improper_interval", llvm2si_node_improper_interval },
		{ "proper_outer_interval", llvm2si_node_proper_outer_interval },
		{ "improper_outer_interval", llvm2si_node_improper_outer_interval }
	}
};


static struct llvm2si_node_t *llvm2si_node_create(
		enum llvm2si_node_kind_t kind)
{
	struct llvm2si_node_t *node;

	/* Initialize */
	node = xcalloc(1, sizeof(struct llvm2si_node_t));
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


struct llvm2si_node_t *llvm2si_node_create_leaf(char *name)
{
	struct llvm2si_node_t *node;

	/* Initialize */
	node = llvm2si_node_create(llvm2si_node_leaf);
	node->name = str_set(node->name, name);

	/* Return */
	return node;
}


struct llvm2si_node_t *llvm2si_node_create_abstract(char *name,
		enum llvm2si_node_region_t region)
{
	struct llvm2si_node_t *node;

	/* Initialize */
	node = llvm2si_node_create(llvm2si_node_abstract);
	node->name = str_set(node->name, name && *name ? name : "<abstract>");
	node->abstract.region = region;
	node->abstract.child_list = linked_list_create();
	
	/* Return */
	return node;
}


void llvm2si_node_free(struct llvm2si_node_t *node)
{
	if (node->kind == llvm2si_node_abstract)
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
int llvm2si_node_in_list(struct llvm2si_node_t *node,
		struct linked_list_t *list)
{
	struct linked_list_iter_t *iter;
	int found;

	iter = linked_list_iter_create(list);
	found = linked_list_iter_find(iter, node);
	linked_list_iter_free(iter);

	return found;
}


void llvm2si_node_list_dump(struct linked_list_t *list, FILE *f)
{
	char *comma;
	struct linked_list_iter_t *iter;
	struct llvm2si_node_t *node;

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


void llvm2si_node_list_dump_buf(struct linked_list_t *list, char *buf, int size)
{
	struct llvm2si_node_t *node;

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


void llvm2si_node_dump(struct llvm2si_node_t *node, FILE *f)
{
	struct llvm2si_node_t *succ_node;
	char *no_name;
	char *comma;

	no_name = "<no-name>";
	fprintf(f, "Node '%s':", *node->name ? node->name : no_name);
	fprintf(f, " type=%s", str_map_value(&llvm2si_node_kind_map,
			node->kind));
	fprintf(f, " pred=");
	llvm2si_node_list_dump(node->pred_list, f);

	/* List of successors */
	fprintf(f, " succ={");
	comma = "";
	LINKED_LIST_FOR_EACH(node->succ_list)
	{
		succ_node = linked_list_get(node->succ_list);
		fprintf(f, "%s", comma);
		if (llvm2si_node_in_list(succ_node,
				node->back_edge_list))
			fprintf(f, "-");
		else if (llvm2si_node_in_list(succ_node,
				node->forward_edge_list))
			fprintf(f, "+");
		else if (llvm2si_node_in_list(succ_node,
				node->tree_edge_list))
			fprintf(f, "|");
		else if (llvm2si_node_in_list(succ_node,
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
	if (node->kind == llvm2si_node_abstract)
	{
		fprintf(f, " children=");
		llvm2si_node_list_dump(node->abstract.child_list, f);
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


void llvm2si_node_try_connect(struct llvm2si_node_t *node,
		struct llvm2si_node_t *node_dest)
{
	/* Nothing if edge already exists */
	if (llvm2si_node_in_list(node_dest, node->succ_list))
		return;

	/* Add edge */
	assert(!llvm2si_node_in_list(node, node_dest->pred_list));
	linked_list_add(node->succ_list, node_dest);
	linked_list_add(node_dest->pred_list, node);
}


void llvm2si_node_connect(struct llvm2si_node_t *node,
		struct llvm2si_node_t *node_dest)
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


void llvm2si_node_try_disconnect(struct llvm2si_node_t *node,
		struct llvm2si_node_t *node_dest)
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


void llvm2si_node_disconnect(struct llvm2si_node_t *node,
		struct llvm2si_node_t *node_dest)
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


void llvm2si_node_compare(struct llvm2si_node_t *node1,
		struct llvm2si_node_t *node2)
{
	struct llvm2si_ctree_t *ctree1;
	struct llvm2si_ctree_t *ctree2;
	struct llvm2si_node_t *tmp_node;

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
		tmp_node = llvm2si_ctree_get_node(ctree2, tmp_node->name);
		assert(tmp_node);
		if (!llvm2si_node_in_list(tmp_node, node2->succ_list))
			differ = 1;
	}
	if (differ)
		fatal("successors differ for '%s' and '%s'",
				node_name1, node_name2);

	/* Abstract node */
	if (node1->kind == llvm2si_node_abstract)
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
			tmp_node = llvm2si_ctree_get_node(ctree2, tmp_node->name);
			assert(tmp_node);
			if (!llvm2si_node_in_list(tmp_node, node2->abstract.child_list))
				differ = 1;
		}
		if (differ)
			fatal("children differ for '%s' and '%s'",
					node_name1, node_name2);
	}
}

