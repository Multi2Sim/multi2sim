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

#ifndef M2C_COMMON_NODE_H
#define M2C_COMMON_NODE_H

#include <iostream>
#include <list>
#include <memory>

#include <lib/cpp/String.h>
#include <llvm/BasicBlock.h>


namespace Common
{

/* Forward declarations */
class BasicBlock;
class Tree;


/* Kind of control tree node. The node can be a leaf representing a basic block
 * of the function, or an abstract node, representing a reduction of the control
 * flow graph. */
extern misc::StringMap node_kind_map;
enum NodeKind
{
	NodeKindInvalid,
	NodeKindLeaf,
	NodeKindAbstract
};

extern misc::StringMap node_role_map;
enum NodeRole
{
	NodeRoleInvalid,

	NodeRoleIf,
	NodeRoleThen,
	NodeRoleElse,
	NodeRoleHead,
	NodeRoleTail,
	NodeRolePre,  /* Loop pre-header */
	NodeRoleExit,  /* Loop exit */

	NodeRoleCount
};


/* Node of the control tree */
class Node
{
	friend class Tree;

	std::string name;
	NodeKind kind;

	/* Control tree that the node belongs to */
	Tree *tree;

	std::list<Node *> succ_list;

	std::list<Node *> forward_edge_list;
	std::list<Node *> back_edge_list;
	std::list<Node *> tree_edge_list;
	std::list<Node *> cross_edge_list;

	/* If the node is part of a higher-level abstract node, this field
	 * points to it. If not, the field is null. */
	Node *parent;

	/* Role that the node plays inside of its parent abstract node.
	 * This field is other than 'NodeRoleInvalid' only when 'parent' is
	 * not null. */
	NodeRole role;

	/* Flags indicating when a node with role 'NodeRoleHead' belonging
	 * to a parent region 'WhileLoop' exists the loop when its condition
	 * is evaluated to true or false. Only one of these two flags can be
	 * set. */
	bool exit_if_true;
	bool exit_if_false;

	/* Identifiers assigned during the depth-first search */
	int preorder_id;
	int postorder_id;

	/* Color used for traversal algorithms */
	int color;

public:

	std::list<Node *> pred_list;

	/* Constructor/destructor */
	Node(const std::string &name, NodeKind kind);
	virtual ~Node() { }

	/* Getters */
	Tree *getTree() { return tree; }
	const std::string &getName() { return name; }
	NodeKind getKind() { return kind; }
	NodeRole getRole() { return role; }
	bool getExitIfTrue() { return exit_if_true; }
	bool getExitIfFalse() { return exit_if_false; }

	/* Dump node */
	virtual void Dump(std::ostream &os);
	friend std::ostream &operator<<(std::ostream &os, Node &node) {
			node.Dump(os); return os; }

	/* Return true if 'node' is in the linked list of nodes passed as the
	 * second argument. */
	bool InList(std::list<Node *> &list);
	bool InList(std::list<std::unique_ptr<Node>> &list);

	/* Try to create an edge between 'this' and 'node'. If the edge already
	 * exist, the function will ignore the call silently. */
	void TryConnect(Node *node);

	/* Create an edge between 'this' and 'node'. There should be no
	 * existing edge for this source and destination when calling this
	 * function. */
	void Connect(Node *node);

	/* Try to remove an edge between 'this' and 'node'. If the edge does
	 * not exist, the function exists silently. */
	void TryDisconnect(Node *node);

	/* Disconnect 'this' and 'node'. An edge must exist between both. */
	void Disconnect(Node *node);

	/* Try to reconnect a source node with a new destination node. This is
	 * equivalent to disconnecting and connecting it, except that the order
	 * of the edge within the successor list of the source node is
	 * guaranteed to stay the same. If an edge already exists between the
	 * source and the new destination, the original edge will just be
	 * completely removed. */
	void ReconnectDest(Node *dest_node, Node *new_dest_node);

	/* Try to replace the source node of an edge. This is equivalent to
	 * disconnecting and connecting it, except that the order of the
	 * predecessor list of the destination node is guaranteed to stay
	 * intact. If an edge already exists between the new source and the
	 * destination, the original edge will just be completely removed. */
	void ReconnectSource(Node *dest_node, Node *new_src_node);

	/* Make 'this' take the same parent as 'before' and place it right
	 * before it in its child list. Node 'before' must have a parent. This
	 * does not insert the node into the control tree structures (an extra
	 * call to 'AddNode' is needed). */
	void InsertBefore(Node *before);

	/* Make 'this' take the same parent as 'after' and place it right after
	 * it in its child list. Node 'after' must have a parent. This does not
	 * insert the node into the control tree structures (an extra call to
	 * 'AddNode' is needed). */
	void InsertAfter(Node *after);

	/* Starting at 'this', traverse the syntax tree (not control tree) in
	 * depth-first and return the first leaf node found (could be 'this'
	 * itself). */
	Node *GetFirstLeaf();

	/* Starting at 'this', traverse the syntax tree (not control tree) in
	 * depth-first and return the last leaf node found (could be 'this'
	 * itself). */
	Node *GetLastLeaf();

	/* Compare two nodes */
	virtual void Compare(Node *node);

	/* Dumping lists of nodes */
	static void DumpList(std::ostream &os, std::list<Node *> &list);
	static void DumpListDetail(std::ostream &os, std::list<Node *> &list);
	
	/* Remove node from a list. If the node is present, return true. Return
	 * false otherwise. The reason to make these statis functions is that
	 * the second version (based on std::unique_ptr) would destruct the
	 * object instance while removing it from the list, which does not seem
	 * something safe to do while a member function is running. */
	static bool RemoveFromList(std::list<Node *> &list, Node *node);
	static bool RemoveFromList(std::list<std::unique_ptr<Node>> &list,
			Node *node);
		
};


class LeafNode : public Node
{
	friend class Tree;

	BasicBlock *basic_block;

	/* When the node is created automatically from an LLVM function's
	 * control flow graph, this fields contains the associated LLVM
	 * basic block. */
	llvm::BasicBlock *llvm_basic_block;

public:

	/* Constructor and destructor */
	LeafNode(const std::string &name);
	~LeafNode();

	/* Getters */
	BasicBlock *GetBasicBlock() { return basic_block; }
	llvm::BasicBlock *GetLlvmBasicBlock() { return llvm_basic_block; }

	/* Setters */
	void SetBasicBlock(BasicBlock *basic_block) { this->basic_block
			= basic_block; }

	/* Dump node */
	void Dump(std::ostream &os);
	friend std::ostream &operator<<(std::ostream &os, LeafNode &node);
};


extern misc::StringMap abstract_node_region_map;
enum AbstractNodeRegion
{
	AbstractNodeRegionInvalid,

	AbstractNodeBlock,
	AbstractNodeIfThen,
	AbstractNodeIfThenElse,
	AbstractNodeWhileLoop,
	AbstractNodeLoop,
	AbstractNodeProperInterval,
	AbstractNodeImproperInterval,
	AbstractNodeProperOuterInterval,
	AbstractNodeImproperOuterInterval,

	AbstractNodeRegionCount
};


class AbstractNode : public Node
{
	friend class Tree;

	/* Type of region */
	AbstractNodeRegion region;

	/* List of function nodes associated with the abstract node */
	std::list<Node *> child_list;
public:
	
	/* Constructor and destructor */
	AbstractNode(const std::string &name, AbstractNodeRegion region);

	/* Getters */
	AbstractNodeRegion GetRegion() { return region; }
	std::list<Node *> &GetChildList() { return child_list; }
	
	/* Dump */
	void Dump(std::ostream &os);
	friend std::ostream &operator<<(std::ostream &os, LeafNode &node);

	/* Compare */
	void Compare(Node *node);
};


}  /* namespace Common */

#endif

