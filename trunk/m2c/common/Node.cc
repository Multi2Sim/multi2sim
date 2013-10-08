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

#include <iostream>

#include "BasicBlock.h"
#include "Node.h"
#include "Tree.h"


using namespace Misc;


namespace Common
{


/*
 * Class 'LeafNode'
 */

LeafNode::LeafNode(const std::string &name)
		: Node(name)
{
}


LeafNode::~LeafNode()
{
	if (basic_block)
		delete basic_block;
}


void LeafNode::Dump(std::ostream &os)
{
	Node::Dump(os);
	os << " type=leaf";
}




/*
 * Class 'AbstractNode'
 */

StringMap abstract_node_region_map =
{
	{ "block", AbstractNodeBlock },
	{ "if_then", AbstractNodeIfThen },
	{ "if_then_else", AbstractNodeIfThenElse },
	{ "while_loop", AbstractNodeWhileLoop },
	{ "loop", AbstractNodeLoop },
	{ "proper_interval", AbstractNodeProperInterval },
	{ "improper_interval", AbstractNodeImproperInterval },
	{ "proper_outer_interval", AbstractNodeProperOuterInterval },
	{ "improper_outer_interval", AbstractNodeImproperOuterInterval },
	{ 0, 0 }
};


AbstractNode::AbstractNode(const std::string &name, AbstractNodeRegion region)
		: Node(name)
{
	/* Initialize */
	this->region = region;
}


void AbstractNode::Dump(std::ostream &os)
{
	/* Dump */
	Node::Dump(os);
	os << " type=abstract";

	/* List of child elements */
	os << " children=";
	DumpNodeList(os, child_list);
}


void AbstractNode::Compare(Node *node)
{
	bool differ;

	/* Call parent compare function */
	Node::Compare(node);

	/* Parent has compared whether the two subclasses are the same, so here
	 * we can assume that 'node' is also an abstract node. */
	AbstractNode *abs_node = dynamic_cast<AbstractNode *>(node);
	assert(abs_node);

	/* Compare region */
	if (region != abs_node->region)
		fatal("region differs for '%s' and '%s'",
				GetName().c_str(),
				node->GetName().c_str());

	/* Compare children */
	differ = child_list.size() != abs_node->child_list.size();
	for (auto &child : child_list)
	{
		Node *tmp_node = abs_node->GetTree()->GetNode(child->GetName());
		assert(tmp_node);
		if (!tmp_node->InList(abs_node->child_list))
			differ = true;
	}
	if (differ)
		fatal("children differ for '%s' and '%s'",
				GetName().c_str(),
				node->GetName().c_str());
}




/*
 * Class 'Node'
 */

StringMap node_kind_map =
{
	{ "Leaf", NodeKindLeaf },
	{ "Abstract", NodeKindAbstract },
	{ 0, 0 }
};


StringMap node_role_map =
{
	{ "if", NodeRoleIf },
	{ "then", NodeRoleThen },
	{ "else", NodeRoleElse },
	{ "head", NodeRoleHead },
	{ "tail", NodeRoleTail },
	{ "pre", NodeRolePre },
	{ "exit", NodeRoleExit },
	{ 0, 0 }
};


Node::Node(const std::string &name)
{
	/* Initialize */
	this->name = name;
	preorder_id = -1;
	postorder_id = -1;
}


void Node::Dump(std::ostream &os)
{
	std::string no_name = "<no-name>";
	os << "Node '" << (name.empty() ? "<anonymous>" : name) << "':";
	os << " pred=";
	DumpNodeList(os, pred_list);

	/* List of successors */
	os << " succ={";
	std::string comma = "";
	for (auto &succ_node : succ_list)
	{
		os << comma;
		if (succ_node->InList(back_edge_list))
			os << '-';
		else if (succ_node->InList(forward_edge_list))
			os << '+';
		else if (succ_node->InList(tree_edge_list))
			os << '|';
		else if (succ_node->InList(cross_edge_list))
			os << '*';
		os << succ_node->GetName();
		comma = ",";
	}

	/* Parent */
	os << "} structof=";
	if (parent)
		os << "'" << parent->GetName() << "'";
	else
		os << '-';

	/* Role */
	if (role)
		os << " role=" << StringMapValue(node_role_map, role);

	/* Loop head nodes exit if false/true */
	if (role == NodeRoleHead)
	{
		if (exit_if_false)
			os << " exit_if_false";
		if (exit_if_true)
			os << "exit_if_true";
	}

	/* Traversal IDs */
	if (preorder_id != -1)
		os << " pre=" << preorder_id;
	if (postorder_id != -1)
		os << " post=" << postorder_id;
}


void Node::TryConnect(Node *node_dest)
{
	/* Nothing if edge already exists */
	if (node_dest->InList(succ_list))
		return;

	/* Add edge */
	assert(!InList(node_dest->pred_list));
	succ_list.push_back(node_dest);
	node_dest->pred_list.push_back(this);
}


void Node::Connect(Node *node_dest)
{
	assert(!node_dest->InList(succ_list));
	assert(!InList, node_dest->pred_list);
	succ_list.push_back(node_dest);
	node_dest->pred_list.push_back(this);
}


void Node::TryDisconnect(Node *node_dest)
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


void Node::Disconnect(Node *node_dest)
{
	auto it1 = std::find(succ_list.begin(), succ_list.end(), node_dest);
	auto it2 = std::find(node_dest->pred_list.begin(),
			node_dest->pred_list.end(), this);
	if (it1 == succ_list.end() || it2 == node_dest->pred_list.end())
		panic("%s: invalid connection between control tree nodes",
				__FUNCTION__);
	
	/* Remove it */
	succ_list.erase(it1);
	node_dest->pred_list.eras(it2);
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


}  /* namespace Common */

