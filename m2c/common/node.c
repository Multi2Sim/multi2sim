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
#include <lib/util/debug.h>
#include <lib/util/string.h>

#include "ctree.h"
#include "node.h"


/*
 * Class 'LeafNode'
 */

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
	self->child_list = new(List);

	/* Virtual functions */
	asObject(self)->Dump = AbstractNodeDump;
	asNode(self)->NodeCompare = AbstractNodeCompare;
}


void AbstractNodeDestroy(AbstractNode *self)
{
	delete(self->child_list);
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
	ListForEach(abs_node->child_list, tmp_node, Node)
	{
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
	self->pred_list = new(List);
	self->succ_list = new(List);
	self->back_edge_list = new(List);
	self->forward_edge_list = new(List);
	self->cross_edge_list = new(List);
	self->tree_edge_list = new(List);
	self->preorder_id = -1;
	self->postorder_id = -1;

	/* Virtual functions */
	self->NodeCompare = NodeCompare;
}


void NodeDestroy(Node *self)
{
	delete(self->pred_list);
	delete(self->succ_list);
	delete(self->back_edge_list);
	delete(self->forward_edge_list);
	delete(self->tree_edge_list);
	delete(self->cross_edge_list);
	self->name = str_free(self->name);
}


int NodeInList(Node *self, List *list)
{
	ListIterator *iter;
	Object *found;

	iter = new(ListIterator, list);
	found = ListIteratorFind(iter, asObject(self));
	delete(iter);

	return found != NULL;
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
	ListForEach(node->succ_list, succ_node, Node)
	{
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
	ListAdd(self->succ_list, asObject(node_dest));
	ListAdd(node_dest->pred_list, asObject(self));
}


void NodeConnect(Node *self, Node *node_dest)
{
#ifndef NDEBUG

	/* Make sure that connection does not exist */
	ListFind(self->succ_list, asObject(node_dest));
	ListFind(node_dest->pred_list, asObject(self));
	if (!self->succ_list->error || !node_dest->pred_list->error)
		panic("%s: redundant connection between control tree nodes",
				__FUNCTION__);
#endif

	/* Make connection */
	ListAdd(self->succ_list, asObject(node_dest));
	ListAdd(node_dest->pred_list, asObject(self));
}


void NodeTryDisconnect(Node *self, Node *node_dest)
{
	/* Check if connection exists */
	ListFind(self->succ_list, asObject(node_dest));
	ListFind(node_dest->pred_list, asObject(self));

	/* Either both are present, or none */
	assert((self->succ_list->error && node_dest->pred_list->error)
			|| (!self->succ_list->error &&
			!node_dest->pred_list->error));

	/* No connection existed */
	if (self->succ_list->error)
		return;
	
	/* Remove existing connection */
	ListRemove(self->succ_list);
	ListRemove(node_dest->pred_list);
}


void NodeDisconnect(Node *self, Node *node_dest)
{
	/* Make sure that connection exists */
	ListFind(self->succ_list, asObject(node_dest));
	ListFind(node_dest->pred_list, asObject(self));
	if (self->succ_list->error || node_dest->pred_list->error)
		panic("%s: invalid connection between control tree nodes",
				__FUNCTION__);
	
	/* Remove it */
	ListRemove(self->succ_list);
	ListRemove(node_dest->pred_list);
}


void NodeReconnectDest(Node *src_node, Node *dest_node, Node *new_dest_node)
{
	/* Old and new destination must be different */
	if (dest_node == new_dest_node)
		panic("%s: old and new nodes are the same", __FUNCTION__);

	/* Connection must exist */
	ListFind(src_node->succ_list, asObject(dest_node));
	ListFind(dest_node->pred_list, asObject(src_node));
	if (src_node->succ_list->error || dest_node->pred_list->error)
		panic("%s: old edge does not exist", __FUNCTION__);

	/* Remove old edge */
	ListRemove(src_node->succ_list);
	ListRemove(dest_node->pred_list);

	/* If new edge does not already exists, create it here. Notice that the
	 * successor of 'src_node' will be inserted in the exact same position.
	 * This behavior is critical for some uses of this function. */
	if (!NodeInList(src_node, new_dest_node->pred_list))
	{
		ListInsert(src_node->succ_list, asObject(new_dest_node));
		ListAdd(new_dest_node->pred_list, asObject(src_node));
	}
}


void NodeReconnectSource(Node *src_node, Node *dest_node, Node *new_src_node)
{
	/* Old and new sources must be different */
	if (src_node == new_src_node)
		panic("%s: old and new nodes are the same", __FUNCTION__);

	/* Connection must exist */
	ListFind(src_node->succ_list, asObject(dest_node));
	ListFind(dest_node->pred_list, asObject(src_node));
	if (src_node->succ_list->error || dest_node->pred_list->error)
		panic("%s: old edge does not exist", __FUNCTION__);

	/* Remove old edge */
	ListRemove(src_node->succ_list);
	ListRemove(dest_node->pred_list);

	/* If new edge does not already exists, create it here. Notice that the
	 * predecessor of 'dst_node' will be inserted in the exact same position.
	 * This behavior is critical for some uses of this function. */
	if (!NodeInList(dest_node, new_src_node->succ_list))
	{
		ListInsert(dest_node->pred_list, asObject(new_src_node));
		ListAdd(new_src_node->succ_list, asObject(dest_node));
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
	ListFind(parent->child_list, asObject(before));
	assert(!parent->child_list->error);
	ListInsert(parent->child_list, asObject(self));
}


void NodeInsertAfter(Node *self, Node *after)
{
	AbstractNode *parent;
	List *child_list;

	/* Check parent */
	parent = asAbstractNode(after->parent);
	if (!parent)
		panic("%s: node '%s' has no parent",
				__FUNCTION__, after->name);

	/* Insert in common parent */
	self->parent = asNode(parent);
	child_list = parent->child_list;
	assert(!NodeInList(self, child_list));
	ListFind(child_list, asObject(after));
	assert(!child_list->error);
	ListNext(child_list);
	ListInsert(child_list, asObject(self));
}


Node *NodeGetFirstLeaf(Node *self)
{
	Node *child_node;
	Node *node;

	/* Traverse syntax tree down */
	node = self;
	while (isAbstractNode(node))
	{
		ListHead(asAbstractNode(node)->child_list);
		child_node = asNode(ListGet(asAbstractNode(node)->child_list));
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
		ListTail(asAbstractNode(node)->child_list);
		child_node = asNode(ListGet(asAbstractNode(node)->child_list));
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
	ListForEach(self->succ_list, tmp_node, Node)
	{
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

void NodeListDump(List *list, FILE *f)
{
	char *comma;
	ListIterator *iter;
	Node *node;

	comma = "";
	fprintf(f, "{");
	iter = new(ListIterator, list);
	ListIteratorForEach(iter, node, Node)
	{
		fprintf(f, "%s%s", comma, node->name);
		comma = ",";
	}
	delete(iter);
	fprintf(f, "}");
}


void NodeListDumpBuf(List *list, char *buf, int size)
{
	Node *node;

	/* Reset buffer */
	if (size)
		*buf = '\0';

	/* Dump elements */
	ListForEach(list, node, Node)
		str_printf(&buf, &size, "%s ", node->name);
}

