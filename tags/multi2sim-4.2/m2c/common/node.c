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


#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/string.h>

#include "ctree.h"
#include "node.h"


/*
 * Class 'LeafNode'
 */

CLASS_IMPLEMENTATION(LeafNode)

void LeafNodeCreate(LeafNode *self, char *name)
{
	/* Parent */
	NodeCreate(asNode(self), name);

	/* Virtual functions */
	asObject(self)->Dump = LeafNodeDump;
}


void LeafNodeDestroy(LeafNode *self)
{
	if (self->basic_block)
		delete(self->basic_block);
}


void LeafNodeDump(Object *self, FILE *f)
{
	/* Parent */
	NodeDump(self, f);

	/* Type */
	fprintf(f, " type=leaf");
}




/*
 * Class 'AbstractNode'
 */

CLASS_IMPLEMENTATION(AbstractNode)


struct str_map_t abstract_node_region_map =
{
	9,
	{
		{ "block", AbstractNodeBlock },
		{ "if_then", AbstractNodeIfThen },
		{ "if_then_else", AbstractNodeIfThenElse },
		{ "while_loop", AbstractNodeWhileLoop },
		{ "loop", AbstractNodeLoop },
		{ "proper_interval", AbstractNodeProperInterval },
		{ "improper_interval", AbstractNodeImproperInterval },
		{ "proper_outer_interval", AbstractNodeProperOuterInterval },
		{ "improper_outer_interval", AbstractNodeImproperOuterInterval }
	}
};


void AbstractNodeCreate(AbstractNode *self, char *name, AbstractNodeRegion region)
{
	/* Parent */
	NodeCreate(asNode(self), name);

	/* Initialize */
	self->region = region;
	self->child_list = linked_list_create();

	/* Virtual functions */
	asObject(self)->Dump = AbstractNodeDump;
	asNode(self)->NodeCompare = AbstractNodeCompare;
}


void AbstractNodeDestroy(AbstractNode *self)
{
	linked_list_free(self->child_list);
}
	
	
void AbstractNodeDump(Object *self, FILE *f)
{
	AbstractNode *node;

	/* Dump parent */
	NodeDump(self, f);

	/* Type */
	node = asAbstractNode(self);
	fprintf(f, " type=abstract");

	/* List of child elements */
	fprintf(f, " children=");
	NodeListDump(node->child_list, f);
}


void AbstractNodeCompare(Node *self, Node *node2)
{
	AbstractNode *abs_node;
	AbstractNode *abs_node2;
	Node *tmp_node;

	int differ;

	/* Call parent compare function */
	NodeCompare(self, node2);

	/* Parent has compared whether the two subclasses are the same, so here
	 * we can assume that 'node2' is also an abstract node. */
	abs_node = asAbstractNode(self);
	abs_node2 = asAbstractNode(node2);

	/* Compare region */
	if (abs_node->region != abs_node2->region)
		fatal("region differs for '%s' and '%s'",
				self->name, node2->name);

	/* Compare children */
	differ = abs_node->child_list->count != abs_node2->child_list->count;
	LINKED_LIST_FOR_EACH(abs_node->child_list)
	{
		tmp_node = linked_list_get(abs_node->child_list);
		tmp_node = CTreeGetNode(node2->ctree, tmp_node->name);
		assert(tmp_node);
		if (!NodeInList(tmp_node, abs_node2->child_list))
			differ = 1;
	}
	if (differ)
		fatal("children differ for '%s' and '%s'",
				self->name, node2->name);
}




/*
 * Class 'Node'
 */

CLASS_IMPLEMENTATION(Node);


struct str_map_t node_kind_map =
{
	2,
	{
		{ "Leaf", node_leaf },
		{ "Abstract", node_abstract }
	}
};


struct str_map_t node_role_map =
{
	7,
	{
		{ "if", node_role_if },
		{ "then", node_role_then },
		{ "else", node_role_else },
		{ "head", node_role_head },
		{ "tail", node_role_tail },
		{ "pre", node_role_pre },
		{ "exit", node_role_exit }
	}
};


void NodeCreate(Node *self, char *name)
{
	/* Initialize */
	self->name = str_set(self->name, name);
	self->pred_list = linked_list_create();
	self->succ_list = linked_list_create();
	self->back_edge_list = linked_list_create();
	self->forward_edge_list = linked_list_create();
	self->cross_edge_list = linked_list_create();
	self->tree_edge_list = linked_list_create();
	self->preorder_id = -1;
	self->postorder_id = -1;

	/* Virtual functions */
	self->NodeCompare = NodeCompare;
}


void NodeDestroy(Node *self)
{
	linked_list_free(self->pred_list);
	linked_list_free(self->succ_list);
	linked_list_free(self->back_edge_list);
	linked_list_free(self->forward_edge_list);
	linked_list_free(self->tree_edge_list);
	linked_list_free(self->cross_edge_list);
	self->name = str_free(self->name);
}


int NodeInList(Node *self, struct linked_list_t *list)
{
	struct linked_list_iter_t *iter;
	int found;

	iter = linked_list_iter_create(list);
	found = linked_list_iter_find(iter, self);
	linked_list_iter_free(iter);

	return found;
}


void NodeDump(Object *self, FILE *f)
{
	Node *node;
	Node *succ_node;
	char *no_name;
	char *comma;

	node = asNode(self);
	no_name = "<no-name>";
	fprintf(f, "Node '%s':", *node->name ? node->name : no_name);
	fprintf(f, " pred=");
	NodeListDump(node->pred_list, f);

	/* List of successors */
	fprintf(f, " succ={");
	comma = "";
	LINKED_LIST_FOR_EACH(node->succ_list)
	{
		succ_node = linked_list_get(node->succ_list);
		fprintf(f, "%s", comma);
		if (NodeInList(succ_node,
				node->back_edge_list))
			fprintf(f, "-");
		else if (NodeInList(succ_node,
				node->forward_edge_list))
			fprintf(f, "+");
		else if (NodeInList(succ_node,
				node->tree_edge_list))
			fprintf(f, "|");
		else if (NodeInList(succ_node,
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
				&node_role_map, node->role));

	/* Loop head nodes exit if false/true */
	if (node->role == node_role_head)
	{
		if (node->exit_if_false)
			fprintf(f, " exit_if_false");
		if (node->exit_if_true)
			fprintf(f, "exit_if_true");
	}

	/* Traversal IDs */
	if (node->preorder_id != -1)
		fprintf(f, " pre=%d", node->preorder_id);
	if (node->postorder_id != -1)
		fprintf(f, " post=%d", node->postorder_id);
}


void NodeTryConnect(Node *self, Node *node_dest)
{
	/* Nothing if edge already exists */
	if (NodeInList(node_dest, self->succ_list))
		return;

	/* Add edge */
	assert(!NodeInList(self, node_dest->pred_list));
	linked_list_add(self->succ_list, node_dest);
	linked_list_add(node_dest->pred_list, self);
}


void NodeConnect(Node *self, Node *node_dest)
{
#ifndef NDEBUG

	/* Make sure that connection does not exist */
	linked_list_find(self->succ_list, node_dest);
	linked_list_find(node_dest->pred_list, self);
	if (!self->succ_list->error_code ||
			!node_dest->pred_list->error_code)
		panic("%s: redundant connection between control tree nodes",
				__FUNCTION__);
#endif

	/* Make connection */
	linked_list_add(self->succ_list, node_dest);
	linked_list_add(node_dest->pred_list, self);
}


void NodeTryDisconnect(Node *self, Node *node_dest)
{
	/* Check if connection exists */
	linked_list_find(self->succ_list, node_dest);
	linked_list_find(node_dest->pred_list, self);

	/* Either both are present, or none */
	assert((self->succ_list->error_code && node_dest->pred_list->error_code)
			|| (!self->succ_list->error_code &&
			!node_dest->pred_list->error_code));

	/* No connection existed */
	if (self->succ_list->error_code)
		return;
	
	/* Remove existing connection */
	linked_list_remove(self->succ_list);
	linked_list_remove(node_dest->pred_list);
}


void NodeDisconnect(Node *self, Node *node_dest)
{
	/* Make sure that connection exists */
	linked_list_find(self->succ_list, node_dest);
	linked_list_find(node_dest->pred_list, self);
	if (self->succ_list->error_code ||
			node_dest->pred_list->error_code)
		panic("%s: invalid connection between control tree nodes",
				__FUNCTION__);
	
	/* Remove it */
	linked_list_remove(self->succ_list);
	linked_list_remove(node_dest->pred_list);
}


void NodeReconnectDest(Node *src_node, Node *dest_node, Node *new_dest_node)
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
	if (!NodeInList(src_node, new_dest_node->pred_list))
	{
		linked_list_insert(src_node->succ_list, new_dest_node);
		linked_list_add(new_dest_node->pred_list, src_node);
	}
}


void NodeReconnectSource(Node *src_node, Node *dest_node, Node *new_src_node)
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
	if (!NodeInList(dest_node, new_src_node->succ_list))
	{
		linked_list_insert(dest_node->pred_list, new_src_node);
		linked_list_add(new_src_node->succ_list, dest_node);
	}
}


void NodeInsertBefore(Node *self, Node *before)
{
	AbstractNode *parent;

	/* Check parent */
	parent = asAbstractNode(before->parent);
	if (!parent)
		panic("%s: node '%s' has no parent",
				__FUNCTION__, before->name);

	/* Insert in common parent */
	self->parent = asNode(parent);
	assert(!NodeInList(self, parent->child_list));
	linked_list_find(parent->child_list, before);
	assert(!parent->child_list->error_code);
	linked_list_insert(parent->child_list, self);
}


void NodeInsertAfter(Node *self, Node *after)
{
	AbstractNode *parent;
	struct linked_list_t *child_list;

	/* Check parent */
	parent = asAbstractNode(after->parent);
	if (!parent)
		panic("%s: node '%s' has no parent",
				__FUNCTION__, after->name);

	/* Insert in common parent */
	self->parent = asNode(parent);
	child_list = parent->child_list;
	assert(!NodeInList(self, child_list));
	linked_list_find(child_list, after);
	assert(!child_list->error_code);
	linked_list_next(child_list);
	linked_list_insert(child_list, self);
}


Node *NodeGetFirstLeaf(Node *self)
{
	Node *child_node;
	Node *node;

	/* Traverse syntax tree down */
	node = self;
	while (isAbstractNode(node))
	{
		linked_list_head(asAbstractNode(node)->child_list);
		child_node = linked_list_get(asAbstractNode(node)->child_list);
		assert(child_node);
		node = child_node;
	}

	/* Return leaf */
	assert(isLeafNode(node));
	return node;
}


Node *NodeGetLastLeaf(Node *self)
{
	Node *child_node;
	Node *node;

	/* Traverse syntax tree down */
	node = self;
	while (isAbstractNode(node))
	{
		linked_list_tail(asAbstractNode(node)->child_list);
		child_node = linked_list_get(asAbstractNode(node)->child_list);
		assert(child_node);
		node = child_node;
	}

	/* Return leaf */
	assert(isLeafNode(node));
	return node;
}


void NodeCompare(Node *self, Node *node2)
{
	CTree *ctree1;
	CTree *ctree2;
	Node *tmp_node;

	char node_name1[MAX_STRING_SIZE];
	char node_name2[MAX_STRING_SIZE];

	int differ;

	/* Store names */
	ctree1 = self->ctree;
	ctree2 = node2->ctree;
	snprintf(node_name1, sizeof node_name1, "%s.%s", ctree1->name, self->name);
	snprintf(node_name2, sizeof node_name2, "%s.%s", ctree2->name, node2->name);

	/* Compare kind */
	if (class_of(self) != class_of(node2))
		fatal("node kind differs for '%s' and '%s'",
				node_name1, node_name2);

	/* Compare successors */
	differ = self->succ_list->count != node2->succ_list->count;
	LINKED_LIST_FOR_EACH(self->succ_list)
	{
		tmp_node = linked_list_get(self->succ_list);
		tmp_node = CTreeGetNode(ctree2, tmp_node->name);
		assert(tmp_node);
		if (!NodeInList(tmp_node, node2->succ_list))
			differ = 1;
	}
	if (differ)
		fatal("successors differ for '%s' and '%s'",
				node_name1, node_name2);
}



/*
 * Non-Class Functions
 */

void NodeListDump(struct linked_list_t *list, FILE *f)
{
	char *comma;
	struct linked_list_iter_t *iter;
	Node *node;

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


void NodeListDumpBuf(struct linked_list_t *list, char *buf, int size)
{
	Node *node;

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
