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
#include <llvm/IR/BasicBlock.h>


namespace comm
{

// Forward declarations
class BasicBlock;
class Tree;


/// Node of the control tree
class Node
{
public:

	/// Kind of control tree node. The node can be a leaf representing a
	/// basic block of the function, or an abstract node, representing a
	/// reduction of the control flow graph.
	enum Kind
	{
		KindInvalid,
		KindLeaf,
		KindAbstract
	};

	/// String map for Kind
	static const misc::StringMap KindMap;

	/// Role of a node as a child of an abstract node
	enum NodeRole
	{
		NodeRoleInvalid,
	
		RoleIf,
		RoleThen,
		RoleElse,
		RoleHead,
		RoleTail,
		RolePre,  // Loop pre-header
		RoleExit,  // Loop exit
	
		RoleCount
	};

	/// String map for Role
	static const misc::StringMap RoleMap;

private:

	friend class Tree;

	// Name of the node
	std::string name;

	// Node kind (leaf or abstract)
	Kind kind;

	// Control tree that the node belongs to
	Tree *tree = nullptr;

	// List of successor nodes
	std::list<Node *> succ_list;
	
	// This list is a back up of the succ_list which is created when new
	// connections are initially laid in the tree. The succ_list is losing
	// some connections during the tree analysis which is not giving the correct
	// successors for every basic block. While that needs to be fixed, as a temporary
	// solution, use backup lists to get BB successors
	std::list<Node *> backup_succ_list;

	// List of predecessor nodes
	std::list<Node *> pred_list;

	// Same as backup_succ_list
	std::list<Node *> backup_pred_list;

	// List of nodes connected through forward edges
	std::list<Node *> forward_edge_list;

	// List of nodes connected through back edges
	std::list<Node *> back_edge_list;

	// List of nodes connected through tree edges
	std::list<Node *> tree_edge_list;

	// List of nodes connected through cross edges
	std::list<Node *> cross_edge_list;

	// Additional scalar edge inserted between a 'Then' and an 'Else' block
	// in an if-then-else region after structural analysis.
	Node *scalar_succ_node = nullptr;
	Node *scalar_pred_node = nullptr;

	// If the node is part of a higher-level abstract node, this field
	// points to it. If not, the field is null.
	Node *parent = nullptr;

	// Role that the node plays inside of its parent abstract node.
	// This field is other than 'NodeRoleInvalid' only when 'parent' is
	// not null.
	NodeRole role = NodeRoleInvalid;

	// Flags indicating when a node with role 'NodeRoleHead' belonging
	// to a parent region 'WhileLoop' exists the loop when its condition
	// is evaluated to true or false. Only one of these two flags can be
	// set.
	bool exit_if_true = false;
	bool exit_if_false = false;

	// Identifiers assigned during the depth-first search
	int preorder_id = -1;
	int postorder_id = -1;

	// Color used for traversal algorithms
	int color = 0;

public:

	/// Constructor
	Node(const std::string &name, Kind kind) :
			name(name),
			kind(kind)
	{
	}
	
	/// Return a reference to the list of successor nodes, which can be used
	/// for convenient traversal using:
	///
	/// \code
	///	for (auto &succ_node : node->getSuccList())
	/// \endcode
	const std::list<Node *> &getSuccList() const { return succ_list; }

	/// Return a reference to the list of predecessor nodes, which can be
	/// used for convenient traversal in a `for` loop:
	///
	/// \code
	///	for (auto &pred_node : node->getPredList())
	/// \endcode
	const std::list<Node *> &getPredList() const { return pred_list; }

	/// Return the reference to the list of successor nodes.
	const std::list<Node *> &getBackupSuccList() const { return backup_succ_list; }

	/// Return the reference to the list of successor nodes.
	const std::list<Node *> &getBackupPredList() const { return backup_pred_list; }

	/// Dumps the back up successor list
	void DumpSuccList();

	/// Virtual destructor
	virtual ~Node() { }

	/// Return the parent node, or `nullptr` if node has no parent.
	Node *getParent() const { return parent; }

	/// Set the parent node
	void setParent(Node *parent) { this->parent = parent; }

	/// Return the tree that the node belongs to
	Tree *getTree() const { return tree; }

	/// Return the name of the node
	const std::string &getName() const { return name; }

	/// Return the node kind
	Kind getKind() const { return kind; }

	/// Return the role of the node
	NodeRole getRole() const { return role; }

	/// For loop head nodes, return whether the loop exists if the condition
	/// is true.
	bool getExitIfTrue() { return exit_if_true; }

	/// For loop head nodes, return whether the loop exists if the condition
	/// is false.
	bool getExitIfFalse() { return exit_if_false; }

	// Dump node to an output stream
	virtual void Dump(std::ostream &os = std::cout);

	/// Alternative syntax for Dump()
	friend std::ostream &operator<<(std::ostream &os, Node &node)
	{
		node.Dump(os);
		return os;
	}

	/// Return `true` if the node is in the list of nodes passed in the
	/// first argument. The argument can be a linked list of pointers to
	/// nodes or a linked list of unique pointers to nodes.
	bool InList(std::list<Node *> &list);
	bool InList(std::list<std::unique_ptr<Node>> &list);

	/// Try to create an edge between `this` and \a node. If the edge
	/// already exist, the function will ignore the call silently.
	void TryConnect(Node *node);

	/// Create an edge between `this` and \a node. There should be no
	/// existing edge for this source and destination when calling this
	/// function.
	void Connect(Node *node);

	/// Create a scalar edge between 'then' and 'else' node. There should be
	/// no existing edge for this source and destination when calling this
	/// function.
	void ConnectScalar(Node *node);

	/// Try to remove an edge between `this` and \a node. If the edge does
	/// not exist, the function exists silently.
	void TryDisconnect(Node *node);

	/// Disconnect `this` and \a node. An edge must exist between both.
	void Disconnect(Node *node);

	/// Try to reconnect a source node with a new destination node. This is
	/// equivalent to disconnecting and connecting it, except that the order
	/// of the edge within the successor list of the source node is
	/// guaranteed to stay the same. If an edge already exists between the
	/// source and the new destination, the original edge will just be
	/// completely removed.
	void ReconnectDest(Node *dest_node, Node *new_dest_node);

	/// Try to replace the source node of an edge. This is equivalent to
	/// disconnecting and connecting it, except that the order of the
	/// predecessor list of the destination node is guaranteed to stay
	/// intact. If an edge already exists between the new source and the
	/// destination, the original edge will just be completely removed.
	void ReconnectSource(Node *dest_node, Node *new_src_node);

	/// Make `this` take the same parent as \a before and place it right
	/// before it in its child list. Node \a before must have a parent. This
	/// does not insert the node into the control tree structures (an extra
	/// call to AddNode() is needed).
	void InsertBefore(Node *before);

	/// Make `this` take the same parent as \a after and place it right after
	/// it in its child list. Node \a after must have a parent. This does not
	/// insert the node into the control tree structures (an extra call to
	/// AddNode() is needed).
	void InsertAfter(Node *after);

	/// Starting at `this`, traverse the syntax tree (not control tree) in
	/// depth-first and return the first leaf node found (could be `this`
	/// itself).
	Node *getFirstLeaf();

	/// Starting at `this`, traverse the syntax tree (not control tree) in
	/// depth-first and return the last leaf node found (could be `this`
	/// itself).
	Node *getLastLeaf();

	/// Compare `this` with \a node, and error out if not equal
	virtual void Compare(Node *node);

	/// Dump a list of nodes
	static void DumpList(std::list<Node *> &list,
			std::ostream &os = std::cout);

	/// Dump a list of nodes with a detailed format
	static void DumpListDetail(std::list<Node *> &list,
			std::ostream &os = std::cout);
	
	/// Remove node from a list. If the node is present, return true. Return
	/// false otherwise. The reason to make these static functions is that
	/// the second version (based on `std::unique_ptr`) would destroy the
	/// object instance while removing it from the list, which does not seem
	/// something safe to do while a member function is running.
	static bool RemoveFromList(std::list<Node *> &list, Node *node);
	static bool RemoveFromList(std::list<std::unique_ptr<Node>> &list,
			Node *node);
		
};


class LeafNode : public Node
{
	// Basic block associated with the node
	BasicBlock *basic_block = nullptr;

	// When the node is created automatically from an LLVM function's
	// control flow graph, this fields contains the associated LLVM
	// basic block.
	llvm::BasicBlock *llvm_basic_block = nullptr;

public:

	/// Constructor
	///
	/// \param name
	///	Name of the LLVM basic block that caused the creation of this
	///	node. This will be a label assigned to the block when translated
	///	to assembly code.
	LeafNode(const std::string &name) : Node(name, KindLeaf)
	{
	}

	/// Set the basic block associated with this leaf node.
	void setBasicBlock(BasicBlock *basic_block)
	{
		assert(this->basic_block == nullptr);
		this->basic_block = basic_block;
	}

	/// Return the basic block associated with the node
	BasicBlock *getBasicBlock() const { return basic_block; }

	/// Return the LLVM basic block associated with the node
	llvm::BasicBlock *getLLVMBasicBlock() const { return llvm_basic_block; }

	/// Update the LLVM basic block associated with the node
	void setLLVMBasicBlock(llvm::BasicBlock *llvm_basic_block)
	{
		this->llvm_basic_block = llvm_basic_block;
	}

	/// Dump node
	void Dump(std::ostream &os = std::cout);
};


/// Abstract node in the control tree, created after a reduction process during
/// the structural analysis pass.
class AbstractNode : public Node
{
public:

	/// Region types that an abstract node forms
	enum Region
	{
		RegionInvalid,

		RegionBlock,
		RegionIfThen,
		RegionIfThenElse,
		RegionWhileLoop,
		RegionLoop,
		RegionProperInterval,
		RegionImproperInterval,
		RegionProperOuterInterval,
		RegionImproperOuterInterval,
	
		RegionCount
	};

	/// String map for Region
	static const misc::StringMap RegionMap;

private:

	// Type of region
	Region region;

	// List of function nodes associated with the abstract node
	std::list<Node *> child_list;

public:
	
	/// Constructor
	///
	/// \param name
	///	Unique name given to the abstract node
	///
	/// \param region
	///	Region type formed by the abstract node
	AbstractNode(const std::string &name, Region region);

	/// Return the region type for this abstract node
	Region getRegion() const { return region; }

	/// Return the list of child nodes
	std::list<Node *> &getChildList() { return child_list; }

	/// Return a constant reference to the list of child nodes
	const std::list<Node *> &getChildList() const { return child_list; }

	/// Return the number of child nodes
	int getNumChildren() const { return child_list.size(); }

	/// Return the first child in the list
	Node *getFirstChild() const { return child_list.front(); }

	/// Return the last child in the list
	Node *getLastChild() const { return child_list.back(); }

	/// Add a node as a child
	void AddChild(Node *node)
	{
		child_list.push_back(node);
		node->setParent(this);
	}

	/// Remove a child node. The node must be part of the child list.
	void RemoveChild(Node *node)
	{
		Node::RemoveFromList(child_list, node);
		node->setParent(nullptr);
	}
	
	/// Dump node
	void Dump(std::ostream &os = std::cout);

	/// Compare `this` with \a node, and error out if not equal
	void Compare(Node *node);
};


}  // namespace comm

#endif

