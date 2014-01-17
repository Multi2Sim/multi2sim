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

#include <lib/cpp/Misc.h>

#include "BasicBlock.h"
#include "Node.h"
#include "Tree.h"


using namespace misc;


namespace Common
{


/*
 * Class 'LeafNode'
 */

LeafNode::LeafNode(const std::string &name)
		: Node(name, NodeKindLeaf)
{
	basic_block = nullptr;
	llvm_basic_block = nullptr;
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
	{ "improper_outer_interval", AbstractNodeImproperOuterInterval }
};


AbstractNode::AbstractNode(const std::string &name, AbstractNodeRegion region)
		: Node(name, NodeKindAbstract)
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
	DumpList(os, child_list);
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
				getName().c_str(),
				node->getName().c_str());

	/* Compare children */
	differ = child_list.size() != abs_node->child_list.size();
	for (auto &child : child_list)
	{
		Node *tmp_node = abs_node->getTree()->GetNode(child->getName());
		assert(tmp_node);
		if (!tmp_node->InList(abs_node->child_list))
			differ = true;
	}
	if (differ)
		fatal("children differ for '%s' and '%s'",
				getName().c_str(),
				node->getName().c_str());
}




/*
 * Class 'Node'
 */

StringMap node_kind_map =
{
	{ "Leaf", NodeKindLeaf },
	{ "Abstract", NodeKindAbstract }
};


StringMap node_role_map =
{
	{ "if", NodeRoleIf },
	{ "then", NodeRoleThen },
	{ "else", NodeRoleElse },
	{ "head", NodeRoleHead },
	{ "tail", NodeRoleTail },
	{ "pre", NodeRolePre },
	{ "exit", NodeRoleExit }
};


Node::Node(const std::string &name, NodeKind kind)
{
	/* Initialize */
	this->name = name;
	this->kind = kind;
	tree = nullptr;
	parent = nullptr;
	role = NodeRoleInvalid;
	preorder_id = -1;
	postorder_id = -1;
}


void Node::Dump(std::ostream &os)
{
	std::string no_name = "<no-name>";
	os << "Node '" << (name.empty() ? "<anonymous>" : name) << "':";
	os << " pred=";
	DumpList(os, pred_list);

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
		os << succ_node->getName();
		comma = ",";
	}

	/* Parent */
	os << "} structof=";
	if (parent)
		os << "'" << parent->getName() << "'";
	else
		os << '-';

	/* Role */
	if (role)
		os << " role=" << node_role_map.MapValue(role);

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


bool Node::InList(std::list<Node *> &list)
{
	return std::find(list.begin(), list.end(), this) != list.end();
}


bool Node::InList(std::list<std::unique_ptr<Node>> &list)
{
	for (auto &node : list)
		if (node.get() == this)
			return true;
	return false;
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
	assert(!InList(node_dest->pred_list));
	succ_list.push_back(node_dest);
	node_dest->pred_list.push_back(this);
}


void Node::TryDisconnect(Node *node_dest)
{
	/* Check if connection exists */
	auto it1 = std::find(succ_list.begin(), succ_list.end(), node_dest);
	auto it2 = std::find(node_dest->pred_list.begin(),
			node_dest->pred_list.end(), this);

	/* Either both are present, or none */
	assert((it1 == succ_list.end() && it2 == node_dest->pred_list.end())
			|| (it1 != succ_list.end() &&
			it2 != node_dest->pred_list.end()));

	/* No connection existed */
	if (it1 == succ_list.end())
		return;
	
	/* Remove existing connection */
	succ_list.erase(it1);
	node_dest->pred_list.erase(it2);
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
	node_dest->pred_list.erase(it2);
}


void Node::ReconnectDest(Node *dest_node, Node *new_dest_node)
{
	/* Old and new destination must be different */
	if (dest_node == new_dest_node)
		panic("%s: old and new nodes are the same", __FUNCTION__);

	/* Connection must exist */
	auto it1 = std::find(succ_list.begin(), succ_list.end(), dest_node);
	auto it2 = std::find(dest_node->pred_list.begin(),
			dest_node->pred_list.end(), this);
	if (it1 == succ_list.end() || it2 == dest_node->pred_list.end())
		panic("%s: old edge does not exist", __FUNCTION__);

	/* Remove old edge. Make iterators point to the new element at the
	 * position where the node was removed. */
	it1 = succ_list.erase(it1);
	it2 = dest_node->pred_list.erase(it2);

	/* If new edge does not already exists, create it here. Notice that the
	 * successor of 'src_node' will be inserted in the exact same position.
	 * This behavior is critical for some uses of this function. */
	if (!InList(new_dest_node->pred_list))
	{
		succ_list.insert(it1, new_dest_node);
		new_dest_node->pred_list.push_back(this);
	}
}


void Node::ReconnectSource(Node *dest_node, Node *new_src_node)
{
	/* Old and new sources must be different */
	if (this == new_src_node)
		panic("%s: old and new nodes are the same", __FUNCTION__);

	/* Connection must exist */
	auto it1 = std::find(succ_list.begin(), succ_list.end(), dest_node);
	auto it2 = std::find(dest_node->pred_list.begin(),
			dest_node->pred_list.end(), this);
	if (it1 == succ_list.end() || it2 == dest_node->pred_list.end())
		panic("%s: old edge does not exist", __FUNCTION__);

	/* Remove old edge */
	it1 = succ_list.erase(it1);
	it2 = dest_node->pred_list.erase(it2);

	/* If new edge does not already exists, create it here. Notice that the
	 * predecessor of 'dst_node' will be inserted in the exact same position.
	 * This behavior is critical for some uses of this function. */
	if (!dest_node->InList(new_src_node->succ_list))
	{
		dest_node->pred_list.insert(it2, new_src_node);
		new_src_node->succ_list.push_back(dest_node);
	}
}


void Node::InsertBefore(Node *before)
{
	/* Check parent */
	AbstractNode *parent = dynamic_cast<AbstractNode *>(before->parent);
	if (!parent)
		panic("%s: node '%s' has no parent",
				__FUNCTION__, before->name.c_str());

	/* Insert in common parent */
	this->parent = parent;
	std::list<Node *> &child_list = parent->GetChildList();
	assert(!InList(child_list));
	auto it = std::find(child_list.begin(), child_list.end(), before);
	assert(it != child_list.end());
	child_list.insert(it, this);
}


void Node::InsertAfter(Node *after)
{
	/* Check parent */
	AbstractNode *parent = dynamic_cast<AbstractNode *>(after->parent);
	if (!parent)
		panic("%s: node '%s' has no parent",
				__FUNCTION__, after->name.c_str());

	/* Insert in common parent */
	this->parent = parent;
	std::list<Node *> &child_list = parent->GetChildList();
	assert(!InList(child_list));
	auto it = std::find(child_list.begin(), child_list.end(), after);
	assert(it != child_list.end());
	child_list.insert(++it, this);
}


Node *Node::GetFirstLeaf()
{
	/* Traverse syntax tree down */
	Node *node = this;
	while (node->kind == NodeKindAbstract)
	{
		auto child_list = dynamic_cast<AbstractNode *>(node)
				->GetChildList();
		assert(child_list.size());
		node = child_list.front();
	}

	/* Return leaf */
	assert(node->kind == NodeKindLeaf);
	return node;
}


Node *Node::GetLastLeaf()
{
	/* Traverse syntax tree down */
	Node *node = this;
	while (node->kind == NodeKindAbstract)
	{
		auto child_list = dynamic_cast<AbstractNode *>(node)
				->GetChildList();
		assert(child_list.size());
		node = child_list.back();
	}

	/* Return leaf */
	assert(node->kind == NodeKindLeaf);
	return node;
}


void Node::Compare(Node *node2)
{
	/* Store names */
	Tree *tree2 = node2->tree;
	std::string node_name = tree->GetName() + '.' + name;
	std::string node_name2 = tree2->GetName() + '.' + node2->name;

	/* Compare kind */
	if (kind != node2->kind)
		fatal("node kind differs for '%s' and '%s'",
				node_name.c_str(), node_name2.c_str());

	/* Compare successors */
	bool differ = succ_list.size() != node2->succ_list.size();
	for (auto &tmp_node : succ_list)
	{
		Node *tmp_node2 = tree2->GetNode(tmp_node->name);
		assert(tmp_node2);
		if (!tmp_node2->InList(node2->succ_list))
			differ = true;
	}
	if (differ)
		fatal("successors differ for '%s' and '%s'",
				node_name.c_str(), node_name2.c_str());
}


void Node::DumpList(std::ostream &os, std::list<Node *> &list)
{
	std::string comma = "";
	os << '{';
	for (auto &node : list)
	{
		os << comma << node->getName();
		comma = ",";
	}
	os << '}';
}


void Node::DumpListDetail(std::ostream &os, std::list<Node *> &list)
{
	for (auto &node : list)
		os << node->getName() << ' ';
}


bool Node::RemoveFromList(std::list<Node *> &list, Node *node)
{
	/* Find it */
	auto it = std::find(list.begin(), list.end(), node);
	if (it == list.end())
		return false;

	/* Remove it */
	list.erase(it);
	return true;
}
	

bool Node::RemoveFromList(std::list<std::unique_ptr<Node>> &list,
		Node *node)
{
	/* Find it */
	auto it = list.begin();
	while (it != list.end() && it->get() != node)
		++it;
	
	/* Not found */
	if (it == list.end())
		return false;

	/* Found */
	list.erase(it);
	return true;
}


}  /* namespace Common */

