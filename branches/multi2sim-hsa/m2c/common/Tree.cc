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
#include <memory>
#include <list>
#include <sstream>

#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>

#include "Node.h"
#include "Tree.h"


namespace comm
{


//
// Class 'TreeConfig'
//

Tree *TreeConfig::getTree(const std::string &name)
{
	// Find tree
	for (auto &tree : tree_list)
		if (tree->name == name)
			return tree.get();
	
	// Not found
	return nullptr;
}


void TreeConfig::ProcessCommand(const std::string &s)
{
	// Get list of tokens
	std::vector<std::string> tokens;
	misc::StringTokenize(s, tokens);
	if (!tokens.size())
		misc::fatal("%s: empty command", __FUNCTION__);
	
	// Process command
	std::string command = tokens[0];
	if (!strcasecmp(command.c_str(), "LoadTree"))
	{
		// Syntax: LoadTree <file> <name>
		if (tokens.size() != 3)
			misc::fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command.c_str());
		std::string file_name = tokens[1];
		std::string tree_name = tokens[2];

		// Load control tree
		misc::IniFile f(file_name);
		tree_list.emplace_back(new Tree(f, tree_name));
	}
	else if (!strcasecmp(command.c_str(), "SaveTree"))
	{
		// Syntax: SaveTree <file> <name>
		if (tokens.size() != 3)
			misc::fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command.c_str());
		std::string file_name = tokens[1];
		std::string tree_name = tokens[2];

		// Get control tree
		Tree *tree = getTree(tree_name);
		if (!tree)
			misc::fatal("%s: %s: invalid control tree",
					__FUNCTION__, tree_name.c_str());

		// Save control tree in INI file
		misc::IniFile f;
		tree->Write(f);
		f.Save(file_name);
	}
	else if (!strcasecmp(command.c_str(), "RenameTree"))
	{
		// Syntax: RenameTree <tree> <name>
		if (tokens.size() != 3)
			misc::fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command.c_str());
		std::string tree_name = tokens[1];
		std::string tree_name2 = tokens[2];

		// Get control tree
		Tree *tree = getTree(tree_name);
		if (!tree)
			misc::fatal("%s: %s: invalid control tree",
					__FUNCTION__, tree_name.c_str());

		// Rename
		tree->name = tree_name2;
	}
	else if (!strcasecmp(command.c_str(), "CompareTree"))
	{
		// Syntax: CompareTree <tree1> <tree2>
		if (tokens.size() != 3)
			misc::fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command.c_str());
		std::string tree_name1 = tokens[1];
		std::string tree_name2 = tokens[2];

		// Get first control tree
		Tree *tree1 = getTree(tree_name1);
		if (!tree1)
			misc::fatal("%s: %s: invalid control tree",
					__FUNCTION__, tree_name1.c_str());

		// Get second control tree
		Tree *tree2 = getTree(tree_name2);
		if (!tree2)
			misc::fatal("%s: %s: invalid control tree",
					__FUNCTION__, tree_name2.c_str());

		// Compare them
		tree1->Compare(tree2);
	}
	else if (!strcasecmp(command.c_str(), "StructuralAnalysis"))
	{
		// Syntax: StructuralAnalysis <tree>
		if (tokens.size() != 2)
			misc::fatal("%s: %s: invalid syntax",
					__FUNCTION__, command.c_str());
		std::string tree_name = tokens[1];

		// Get control tree
		Tree *tree = getTree(tree_name);
		if (!tree)
			misc::fatal("%s: %s: invalid control tree",
					__FUNCTION__, tree_name.c_str());

		// Structural analysis
		tree->StructuralAnalysis();
	}
	else
		misc::fatal("%s: invalid command: %s", __FUNCTION__,
				command.c_str());
}


void TreeConfig::setPath(const std::string &path)
{
	this->path = path;
	ini_file.reset(new misc::IniFile(path));
}


void TreeConfig::Run()
{
	// INI file must have been loaded
	if (!ini_file.get())
		misc::panic("%s: INI file not loaded", __FUNCTION__);

	// Process commands
	std::string section = "Commands";
	for (int index = 0;; index++)
	{
		// Read next command
		std::string var = misc::fmt("Command[%d]", index);
		std::string value = ini_file->ReadString(section, var);
		if (value.empty())
			break;

		// Process command
		ProcessCommand(value);
	}
	
	// Close configuration file
	ini_file->Check();
}




//
// Class 'Tree'
//

misc::Debug Tree::debug;

TreeConfig Tree::config;


int Tree::DFS(std::list<Node *> &postorder_list, Node *node, int time)
{
	node->color = 1;  // Gray
	node->preorder_id = time++;
	for (auto &succ_node : node->succ_list)
	{
		if (succ_node->color == 2)  // Black
		{
			// Forward- or cross-edge
			if (node->preorder_id < succ_node->preorder_id)
				node->forward_edge_list.push_back(succ_node);
			else
				node->cross_edge_list.push_back(succ_node);
		}
		else if (succ_node->color == 1)  // Gray
		{
			// This is a back-edge
			node->back_edge_list.push_back(succ_node);
		}
		else  // White
		{
			// This is a tree-edge
			node->tree_edge_list.push_back(succ_node);
			time = DFS(postorder_list, succ_node, time);
		}
	}
	node->color = 2;  // Black
	node->postorder_id = time++;
	postorder_list.push_back(node);
	return time;
}


void Tree::DFS()
{
	std::list<Node *> postorder_list;
	DFS(postorder_list);
}


void Tree::DFS(std::list<Node *> &postorder_list)
{
	// Clear postorder list
	postorder_list.clear();

	// Initialize nodes
	for (auto &node : node_list)
	{
		node->preorder_id = -1;
		node->postorder_id = -1;
		node->color = 0;  // White
		node->back_edge_list.clear();
		node->cross_edge_list.clear();
		node->tree_edge_list.clear();
		node->forward_edge_list.clear();
	}

	// Initiate recursion
	assert(entry_node);
	DFS(postorder_list, entry_node, 0);
}


void Tree::ReachUnder(Node *header_node, Node *node,
		std::list<Node *> &reach_under_list)
{
	// Label as visited and add node
	node->color = 1;
	reach_under_list.push_back(node);

	// Header reached
	if (node == header_node)
		return;

	// Node with lower pre-order ID than the head reached. That means that
	// this is either a cross edge to another branch of the tree, or a
	// back-edge to a region on top of the tree. This indicates the
	// occurrence of an improper region.
	if (node->preorder_id < header_node->preorder_id)
		return;

	// Add predecessors recursively
	for (auto &pred_node : node->pred_list)
		if (!pred_node->color)
			ReachUnder(header_node, pred_node, reach_under_list);
}


void Tree::ReachUnder(Node *header_node, std::list<Node *> &reach_under_list)
{
	// Reset output list
	reach_under_list.clear();

	// Initialize nodes
	for (auto &node : node_list)
		node->color = 0;  // Not visited

	// For all back-edges entering 'header_node', follow edges backwards and
	// keep adding nodes.
	for (auto pred_node : header_node->pred_list)
		if (header_node->InList(pred_node->back_edge_list))
			ReachUnder(header_node, pred_node, reach_under_list);
}


void Tree::FlattenBlock(AbstractNode *abs_node)
{
	// Empty list of nodes
	std::list<Node *> tmp_node_list;

	// Get nodes
	assert(abs_node->getParent() == nullptr);
	assert(abs_node->getRegion() == AbstractNode::RegionBlock);
	assert(abs_node->getNumChildren() == 2);
	Node *in_node = abs_node->getFirstChild();
	Node *out_node = abs_node->getLastChild();

	// Remove existing connection between child nodes
	in_node->Disconnect(out_node);
	assert(!in_node->pred_list.size());
	assert(!in_node->succ_list.size());
	assert(!out_node->pred_list.size());
	assert(!out_node->succ_list.size());

	// Add elements of 'in_node' to 'tmp_node_list'
	AbstractNode *abs_in_node = dynamic_cast<AbstractNode *>(in_node);
	if (abs_in_node && abs_in_node->getRegion() ==
			AbstractNode::RegionBlock)
	{
		// Save child nodes
		for (auto &tmp_node : abs_in_node->getChildList())
			tmp_node_list.push_back(tmp_node);

		// Remove from parent node and tree
		abs_node->RemoveChild(in_node);
		Node::RemoveFromList(node_list, in_node);
	}
	else
	{
		// Save node and remove from children
		tmp_node_list.push_back(in_node);
		abs_node->RemoveChild(in_node);
	}

	// Add elements of 'out_node' to 'tmp_node_list'
	AbstractNode *abs_out_node = dynamic_cast<AbstractNode *>(out_node);
	if (abs_out_node && abs_out_node->getRegion() ==
			AbstractNode::RegionBlock)
	{
		// Save child nodes
		for (auto &tmp_node : abs_out_node->getChildList())
			tmp_node_list.push_back(tmp_node);

		// Remove from parent node and control tree
		abs_node->RemoveChild(out_node);
		Node::RemoveFromList(node_list, out_node);
	}
	else
	{
		// Save node and remove from children
		tmp_node_list.push_back(out_node);
		abs_node->RemoveChild(out_node);
	}

	// Adopt orphan nodes
	assert(abs_node->getNumChildren() == 0);
	for (auto &tmp_node : tmp_node_list)
		abs_node->AddChild(tmp_node);

	// Debug
	if (debug)
	{
		debug << "Flatten block region '" << abs_node->name << "' -> ";
		Node::DumpList(tmp_node_list, debug);
		debug << '\n';
	}
}


AbstractNode *Tree::Reduce(std::list<Node *> &list,
		AbstractNode::Region region)
{
#ifndef NDEBUG

	// List of nodes must contain at least one node
	if (!list.size())
		misc::panic("%s: node list empty", __FUNCTION__);

	// All nodes in 'list' must be part of the control tree, and none
	// of them can have a parent yet.
	for (auto &tmp_node : list)
	{
		if (!tmp_node->InList(node_list))
			throw misc::Panic("Node not in control tree");
		if (tmp_node->getParent())
			throw misc::Panic("Node has a parent already");
	}
#endif

	// Figure out a name for the new abstract node
	assert(region);
	std::string abs_node_name = misc::fmt("__%s_%d",
			AbstractNode::RegionMap[region],
			name_counter[region]);
	name_counter[region]++;

	// Create new abstract node
	AbstractNode *abs_node = new AbstractNode(abs_node_name, region);
	AddNode(abs_node);

	// Debug
	if (debug)
	{
		debug << misc::fmt("\nReducing %s region: ",
				AbstractNode::RegionMap[region]);
		Node::DumpList(list, debug);
		debug << " -> '" << abs_node->name << "'\n";
	}

	// Special case of block regions: record whether there is an edge that
	// goes from the last node into the first. In this case, this edge
	// should stay outside of the reduced region.
	bool cyclic_block = false;
	if (region == AbstractNode::RegionBlock)
	{
		Node *in_node = list.front();
		Node *out_node = list.back();
		assert(in_node && out_node);
		if (in_node->InList(out_node->succ_list))
		{
			cyclic_block = true;
			out_node->Disconnect(in_node);
		}
	}

	// Create a list of incoming edges from the control tree into the
	// region given in 'list', and a list of outgoing edges from the
	// region in 'list' into the rest of the control tree.
	std::list<Node *> in_edge_src_list;
	std::list<Node *> in_edge_dest_list;
	std::list<Node *> out_edge_src_list;
	std::list<Node *> out_edge_dest_list;
	for (auto &tmp_node : list)
	{
		// Traverse incoming edges, and store those
		// that come from outside of 'list'.
		for (auto &in_node : tmp_node->pred_list)
		{
			if (!in_node->InList(list))
			{
				in_edge_src_list.push_back(in_node);
				in_edge_dest_list.push_back(tmp_node);
			}
		}

		// Traverse outgoing edges, and store those
		// that go outside of 'list'.
		for (auto &out_node : tmp_node->succ_list)
		{
			if (!out_node->InList(list))
			{
				out_edge_src_list.push_back(tmp_node);
				out_edge_dest_list.push_back(out_node);
			}
		}
	}

	// Reconnect incoming edges to the new abstract node
	while (in_edge_src_list.size() || in_edge_dest_list.size())
	{
		assert(in_edge_src_list.size() && in_edge_dest_list.size());
		auto it1 = in_edge_src_list.begin();
		auto it2 = in_edge_dest_list.begin();
		Node *src_node = *it1;
		Node *dest_node = *it2;
		in_edge_src_list.erase(it1);
		in_edge_dest_list.erase(it2);
		src_node->ReconnectDest(dest_node, abs_node);
	}

	// Reconnect outgoing edges from the new abstract node
	while (out_edge_src_list.size() || out_edge_dest_list.size())
	{
		assert(out_edge_src_list.size() && out_edge_dest_list.size());
		auto it1 = out_edge_src_list.begin();
		auto it2 = out_edge_dest_list.begin();
		Node *src_node = *it1;
		Node *dest_node = *it2;
		out_edge_src_list.erase(it1);
		out_edge_dest_list.erase(it2);
		src_node->ReconnectSource(dest_node, abs_node);
	}

	// Add all nodes as child nodes of the new abstract node
	assert(abs_node->getNumChildren() == 0);
	for (auto &tmp_node : list)
		abs_node->AddChild(tmp_node);

	// Special case for block regions: if a cyclic block was detected, now
	// the cycle must be inserted as a self-loop in the abstract node.
	if (cyclic_block && !abs_node->InList(abs_node->succ_list))
		abs_node->Connect(abs_node);

	// If entry node is part of the nodes that were replaced, set it to the
	// new abstract node.
	if (entry_node->InList(list))
		entry_node = abs_node;

	// Special case for block regions: in order to avoid nested blocks,
	// block regions are flattened when we detect that one block contains
	// another.
	if (region == AbstractNode::RegionBlock)
	{
		assert(list.size() == 2);
		Node *in_node = list.front();
		Node *out_node = list.back();
		assert(in_node && out_node);

		AbstractNode *abs_in_node = dynamic_cast<AbstractNode *>(in_node);
		AbstractNode *abs_out_node = dynamic_cast<AbstractNode *>(out_node);
		if ((abs_in_node &&
				abs_in_node->getRegion() ==
					AbstractNode::RegionBlock) ||
				(abs_out_node &&
				abs_out_node->getRegion() ==
					AbstractNode::RegionBlock))
			FlattenBlock(abs_node);
	}

	// Special case for while loops: a pre-header and exit blocks are added
	// into the region.
	if (region == AbstractNode::RegionWhileLoop)
	{
		// Get original nodes
		assert(list.size() == 2);
		Node *head_node = list.front();
		Node *tail_node = list.back();
		assert(head_node->kind == Node::KindLeaf);
		assert(head_node->role == Node::RoleHead);
		assert(tail_node->role == Node::RoleTail);

		// Create pre-header and exit nodes
		std::string pre_name = abs_node->name + "_pre";
		std::string exit_name = abs_node->name + "_exit";
		LeafNode *pre_node = new LeafNode(pre_name);
		LeafNode *exit_node = new LeafNode(exit_name);

		// Insert pre-header node into control tree
		AddNode(pre_node);
		pre_node->InsertBefore(head_node);
		pre_node->Connect(head_node);
		pre_node->role = Node::RolePre;

		// Insert exit node into control tree
		AddNode(exit_node);
		exit_node->InsertAfter(tail_node);
		head_node->Connect(exit_node);
		exit_node->role = Node::RoleExit;
	}

	// Return created abstract node
	return abs_node;
}


AbstractNode::Region Tree::Region(Node *node, std::list<Node *> &list)
{
	// Reset output region
	list.clear();


	//
	// Acyclic regions
	//

	// 1. Block region

	// Find two consecutive nodes A and B, where A is the only predecessor
	// of B and B is the only successor of A.
	if (node->succ_list.size() == 1)
	{
		Node *succ_node = node->succ_list.front();
		if (node != succ_node &&
				succ_node != entry_node &&
				succ_node->pred_list.size() == 1)
		{
			list.push_back(node);
			list.push_back(succ_node);
			return AbstractNode::RegionBlock;
		}
	}


	// 2. If-Then

	if (node->succ_list.size() == 2)
	{
		// Assume one order for 'then' and 'endif' blocks
		Node *then_node = node->succ_list.front();
		Node *endif_node = node->succ_list.back();
		assert(then_node && endif_node);

		// Reverse them if necessary
		if (then_node->InList(endif_node->succ_list))
			std::swap(then_node, endif_node);

		// Check conditions.
		// We don't allow 'endif_node' to be the same as 'node'. If they
		// are, we rather reduce such a scheme as a Loop + WhileLoop + Loop.
		if (then_node->pred_list.size() == 1 &&
				then_node->succ_list.size() == 1 &&
				endif_node->InList(then_node->succ_list) &&
				then_node != entry_node &&
				node != then_node &&
				node != endif_node)
		{
			// Create node list - order important!
			list.push_back(node);
			list.push_back(then_node);

			// Set node roles
			node->role = Node::RoleIf;
			then_node->role = Node::RoleThen;

			// Return region
			return AbstractNode::RegionIfThen;
		}
	}


	// 3. If-Then-Else

	if (node->succ_list.size() == 2)
	{
		Node *then_node = node->succ_list.front();
		Node *else_node = node->succ_list.back();
		assert(then_node && else_node);

		Node *then_succ_node = then_node->succ_list.front();
		Node *else_succ_node = else_node->succ_list.front();

		// As opposed to the 'If-Then' region, we allow here the
		// 'endif_node' to be the same as 'node'.
		if (then_node->pred_list.size() == 1 &&
			else_node->pred_list.size() == 1 &&
			then_node != entry_node &&
			else_node != entry_node &&
			then_node->succ_list.size() == 1 &&
			else_node->succ_list.size() == 1 &&
			then_succ_node == else_succ_node &&
			then_succ_node != entry_node &&
			else_succ_node != entry_node)
		{
			// Create list of nodes - notice order!
			list.push_back(node);
			list.push_back(then_node);
			list.push_back(else_node);

			// Assign roles
			node->role = Node::RoleIf;
			then_node->role = Node::RoleThen;
			else_node->role = Node::RoleElse;

			// Return region
			return AbstractNode::RegionIfThenElse;
		}
	}

	// 4. Loop
	if (node->InList(node->succ_list))
	{
		list.push_back(node);
		return AbstractNode::RegionLoop;
	}


	
	//
	// Cyclic regions
	//

	// Obtain the interval in 'list'
	ReachUnder(node, list);
	if (!list.size())
		return AbstractNode::RegionInvalid;
	

	// 1. While-loop
	if (list.size() == 2 && node->succ_list.size() == 2)
	{
		// Obtain head and tail nodes
		Node *head_node = node;
		Node *tail_node = list.front();
		if (tail_node == head_node)
			tail_node = list.back();
		assert(tail_node != head_node);

		// Obtain loop exit node
		Node *exit_node = node->succ_list.front();
		if (exit_node == tail_node)
			exit_node = node->succ_list.back();
		assert(exit_node != tail_node);

		// Check condition for while loop
		if (tail_node->succ_list.size() == 1 &&
				head_node->InList(tail_node->succ_list) &&
				tail_node->pred_list.size() == 1 &&
				head_node->InList(tail_node->pred_list) &&
				tail_node != entry_node &&
				exit_node != head_node)
		{
			// Create node list. The order is important, so we make
			// sure that head node is shown first
			list.clear();
			list.push_back(head_node);
			list.push_back(tail_node);

			// Set node roles
			head_node->role = Node::RoleHead;
			tail_node->role = Node::RoleTail;

			// Determine here whether the loop exists when the condition
			// in its head node is evaluated to true or false - we need
			// this info later!
			//
			// This is inferred from the order in which the head's
			// outgoing edges show up in its successor list. The edge
			// occurring first points to basic block 'if_true' of the
			// LLVM 'br' instruction, while the second edge points to
			// basic block 'if_false'.
			//
			// Thus, if edge head=>tail is the first, the loop exists
			// if the head condition is false. If edge head=>tail is
			// the second, it exists if the condition is true.
			head_node->exit_if_false = tail_node ==
					head_node->succ_list.front();
			head_node->exit_if_true = tail_node ==
					head_node->succ_list.back();

			// Return region
			return AbstractNode::RegionWhileLoop;
		}
	}

	
	// Nothing identified
	list.clear();
	return AbstractNode::RegionInvalid;
}


void Tree::PreorderTraversal(Node *node, std::list<Node *> &list)
{
	// Preorder visit
	list.push_back(node);

	// Visit children
	AbstractNode *abs_node = dynamic_cast<AbstractNode *>(node);
	if (abs_node)
		for (auto &child : abs_node->getChildList())
			PreorderTraversal(child, list);
}


void Tree::PostorderTraversal(Node *node, std::list<Node *> &list)
{
	// Visit children
	AbstractNode *abs_node = dynamic_cast<AbstractNode *>(node);
	if (abs_node)
		for (auto &child : abs_node->getChildList())
			PreorderTraversal(child, list);

	// Postorder visit
	list.push_back(node);
}




//
// Public Functions
//

Tree::Tree(const std::string &name)
{
	// No anonymous
	if (name.empty())
		misc::panic("%s: no name given", __FUNCTION__);

	// Initialize
	this->name = name;
	Clear();
}


void Tree::Dump(std::ostream &os)
{
	// Legend
	os << "\nControl tree (edges: +forward, -back, *cross, #scalar, "
			<< "|tree, =>entry)\n";
	
	// Dump all nodes
	for (auto &node : node_list)
	{
		// Entry
		if (node.get() == entry_node)
			os << "=>";

		// Print node
		os << *node << '\n';
	}
	os << '\n';
}


void Tree::AddNode(Node *node)
{
	// Insert node in list
	assert(!node->InList(node_list));
	node_list.emplace_back(node);

	// Insert in hash table
	auto ret = node_table.insert(std::make_pair(node->name, node));
	if (!ret.second)
		misc::fatal("%s: duplicate node name ('%s')",
				__FUNCTION__, node->name.c_str());

	// Record tree in node
	assert(!node->tree);
	node->tree = this;
}


void Tree::Clear()
{
	node_list.clear();
	node_table.clear();
	entry_node = nullptr;
	structural_analysis_done = false;
	memset(name_counter, 0, sizeof name_counter);
}


void Tree::StructuralAnalysis()
{
	// Debug
	debug << "Starting structural analysis on tree '"
			<< name << "'\n\n";
	debug << "Initial control flow graph:\n";

	// Obtain the DFS spanning tree first, and a post-order traversal of
	// the CFG in 'postorder_list'. This list will be used for progressive
	// reduction steps.
	std::list<Node *> postorder_list;
	DFS(postorder_list);

	// Debug
	if (debug)
		Dump(debug);

	// Sharir's algorithm
	std::list<Node *> region_list;
	while (postorder_list.size())
	{
		// Extract next node in post-order
		auto it = postorder_list.begin();
		Node *node = *it;
		assert(node);
		postorder_list.erase(it);
		debug << "Processing node '" << node->name << "'\n";

		// Identify a region starting at 'node'. If a valid region is
		// found, reduce it into a new abstract node and reconstruct
		// DFS spanning tree.
		AbstractNode::Region region = Region(node, region_list);
		if (region)
		{
			// Reduce and reconstruct DFS
			AbstractNode *abs_node = Reduce(region_list, region);
			DFS();

			// Insert new abstract node in post-order list, to make
			// it be the next one to be processed.
			postorder_list.push_front(abs_node);

			// Debug
			if (debug)
				Dump(debug);
		}
	}

	// Remember that we have run a structural analysis
	structural_analysis_done = true;

	// Debug
	debug << "Done.\n\n";
}


void Tree::PreorderTraversal(std::list<Node *> &list)
{
	// A structural analysis must have been run first
	if (!structural_analysis_done)
		misc::panic("%s: %s: tree traversal requires structural analysis",
				__FUNCTION__, name.c_str());

	// Traverse tree recursively
	list.clear();
	PreorderTraversal(entry_node, list);

	// Debug
	if (debug)
	{
		debug << "Pre-order traversal of tree '" << name << "':\n";
		Node::DumpList(list, debug);
		debug << "\n\n";
	}
}


void Tree::PostorderTraversal(std::list<Node *> &list)
{
	// A structural analysis must have been run first
	if (!structural_analysis_done)
		misc::panic("%s: %s: tree traversal requires structural analysis",
				__FUNCTION__, name.c_str());

	// Traverse tree recursively
	list.clear();
	PostorderTraversal(entry_node, list);

	// Debug
	if (debug)
	{
		debug << "Post-order traversal of tree '" << name << "':\n";
		Node::DumpList(list, debug);
		debug << "\n\n";
	}
}


LeafNode *Tree::AddLlvmCFG(llvm::BasicBlock *llvm_basic_block)
{
	// Empty name not allowed
	if (llvm_basic_block->getName().empty())
		misc::fatal("%s: anonymous LLVM basic blocks not allowed",
			__FUNCTION__);

	// If node already exists, just return it
	auto it = node_table.find(llvm_basic_block->getName());
	if (it != node_table.end())
	{
		LeafNode *leaf_node = dynamic_cast<LeafNode *>(it->second);
		assert(leaf_node);
		return leaf_node;
	}

	// Create node
	LeafNode *node = new LeafNode(llvm_basic_block->getName());
	AddNode(node);
	node->setLLVMBasicBlock(llvm_basic_block);

	// Get basic block terminator
	llvm::TerminatorInst *terminator = llvm_basic_block->getTerminator();

	// Unconditional branch: br label <dest>
	if (llvm::BranchInst::classof(terminator) &&
			terminator->getNumSuccessors() == 1)
	{
		llvm::BasicBlock *succ_llvm_basic_block = terminator->getSuccessor(0);
		LeafNode *succ_node = AddLlvmCFG(succ_llvm_basic_block);
		node->Connect(succ_node);
		return node;
	}

	// Conditional branch: br i1 <cond>, label <iftrue>, label <iffalse>
	if (llvm::BranchInst::classof(terminator) &&
			terminator->getNumSuccessors() == 2)
	{
		llvm::BasicBlock *true_llvm_basic_block = terminator->getSuccessor(0);
		LeafNode *true_node = AddLlvmCFG(true_llvm_basic_block);
		node->Connect(true_node);

		llvm::BasicBlock *false_llvm_basic_block = terminator->getSuccessor(1);
		LeafNode *false_node = AddLlvmCFG(false_llvm_basic_block);
		node->Connect(false_node);

		return node;
	}

	// Function exit: ret
	if (llvm::ReturnInst::classof(terminator))
		return node;

	// Invalid terminator
	misc::fatal("%s: %s: block terminator not supported",
		__FUNCTION__, terminator->getName().data());
	return NULL;
}


LeafNode *Tree::AddLlvmCFG(llvm::Function *llvm_function)
{
	llvm::BasicBlock &basic_block = llvm_function->getEntryBlock();
	return AddLlvmCFG(&basic_block);
}


Node *Tree::getNode(const std::string &name)
{
	auto it = node_table.find(name);
	return it == node_table.end() ? nullptr : it->second;
}


void Tree::getNodeList(std::list<Node *> &list, const std::string &list_str)
{
	// Clear list
	list.clear();

	// Extract nodes
	std::vector<std::string> tokens;
	misc::StringTokenize(list_str, tokens);
	for (auto &token : tokens)
	{
		Node *node = getNode(token);
		if (!node)
			misc::fatal("%s: invalid node name", token.c_str());
		list.push_back(node);
	}
}


void Tree::Write(misc::IniFile &f)
{
	// Control tree must have entry node
	if (!entry_node)
		misc::fatal("%s: control tree without entry node", __FUNCTION__);

	// Dump control tree section
	std::string section = "Tree." + name;
	f.WriteString(section, "Entry", entry_node->name);

	// Write information about the node
	for (auto &node : node_list)
	{
		section = "Tree." + name + ".Node." + node->name;
		if (f.Exists(section))
			misc::fatal("%s: duplicate node name ('%s')", __FUNCTION__,
					node->name.c_str());

		// Dump node properties
		if (node->kind == Node::KindAbstract)
			f.WriteString(section, "Kind", "Abstract");
		else if (node->kind == Node::KindLeaf)
			f.WriteString(section, "Kind", "Leaf");
		else
			misc::fatal("%s: unknown type of node '%s'", __FUNCTION__,
					node->name.c_str());

		// Successors
		std::stringstream stream;
		Node::DumpList(node->succ_list, stream);
		f.WriteString(section, "Succ", stream.str());

		// Abstract node
		AbstractNode *abs_node = dynamic_cast<AbstractNode *>(node.get());
		if (abs_node)
		{
			// Children
			stream.str("");
			Node::DumpList(abs_node->getChildList(), stream);
			f.WriteString(section, "Child", stream.str());

			// Region
			f.WriteString(section, "Region",
					AbstractNode::RegionMap[
					abs_node->getRegion()]);
		}

	}


}


void Tree::Read(misc::IniFile &f, const std::string &name)
{
	// Clear existing tree
	Clear();

	// Set tree name
	this->name = name;
	if (name.empty())
		misc::fatal("%s: empty name", __FUNCTION__);

	// Check that it exists in configuration file
	std::string section = "Tree." + name;
	if (!f.Exists(section))
		misc::fatal("%s: %s: tree not found", __FUNCTION__, name.c_str());

	// Read nodes
	std::string path = f.getPath();
	for (int i = 0; i < f.getNumSections(); i++)
	{
		// Get section
		section = f.getSection(i);

		// Section name must be "Tree.<tree>.Node.<node>"
		std::vector<std::string> tokens;
		misc::StringTokenize(section, tokens, ".");
		if (tokens.size() != 4 ||
				strcasecmp(tokens[0].c_str(), "Tree") ||
				strcasecmp(tokens[1].c_str(), name.c_str()) ||
				strcasecmp(tokens[2].c_str(), "Node"))
			continue;
		
		// Get node properties
		std::string node_name = tokens[3];
		std::string kind_str = f.ReadString(section, "Kind", "Leaf");
		Node::Kind kind = (Node::Kind)
				Node::KindMap.MapStringCase(kind_str);
		if (!kind)
			misc::fatal("%s: %s: invalid value for 'Kind'",
					path.c_str(), section.c_str());

		// Create node
		Node *node;
		if (kind == Node::KindLeaf)
		{
			node = new LeafNode(node_name);
		}
		else
		{
			// Read region
			std::string region_str = f.ReadString(section, "Region");
			AbstractNode::Region region = (AbstractNode::Region)
					AbstractNode::RegionMap.MapStringCase(
					region_str);
			if (!region)
				misc::fatal("[%s] Invalid or missing region: "
						"%s", path.c_str(),
						node_name.c_str());

			// Create node
			node = new AbstractNode(node_name, region);
		}

		// Add node
		AddNode(node);
	}

	// Read node properties
	for (auto &node : node_list)
	{
		// Get section name
		section = "Tree." + name + ".Node." + node->name;

		// Successors
		std::string list_str = f.ReadString(section, "Succ");
		std::list<Node *> list;
		getNodeList(list, list_str);
		for (auto &tmp_node : list)
		{
			if (tmp_node->InList(node->succ_list))
				misc::fatal("%s.%s: duplicate successor", name.c_str(),
						node->name.c_str());
			node->Connect(tmp_node);
		}

		// Abstract node
		AbstractNode *abs_node = dynamic_cast<AbstractNode *>(node.get());
		if (abs_node)
		{
			// Children
			list_str = f.ReadString(section, "Child");
			getNodeList(list, list_str);
			for (auto &tmp_node : list)
				abs_node->AddChild(tmp_node);
		}
	}

	// Read entry node name
	section = "Tree." + name;
	std::string node_name = f.ReadString(section, "Entry");
	if (node_name.empty())
		misc::fatal("%s: %s: no entry node", __FUNCTION__, name.c_str());
	entry_node = getNode(node_name);
	if (!entry_node)
		misc::fatal("%s: %s: invalid node name", __FUNCTION__,
				node_name.c_str());

	// Check configuration file syntax
	f.Check();
}


void Tree::Compare(Tree *tree2)
{
	// Compare entry nodes
	assert(entry_node);
	assert(tree2->entry_node);
	if (entry_node->name != tree2->entry_node->name)
		misc::fatal("'%s' vs '%s': entry nodes differ", name.c_str(),
				tree2->name.c_str());
	
	// Check that all nodes in tree 1 are in tree 2
	for (auto &node : node_list)
		if (!tree2->getNode(node->name))
			misc::fatal("node '%s.%s' not present in tree '%s'",
				name.c_str(), node->name.c_str(),
				tree2->name.c_str());

	// Check that all nodes in tree 2 are in tree 1
	for (auto &node : tree2->node_list)
		if (!getNode(node->name))
			misc::fatal("node '%s.%s' not present in tree '%s'",
				tree2->name.c_str(), node->name.c_str(),
				name.c_str());

	// Compare all nodes
	for (auto &node : node_list)
	{
		Node *node2 = tree2->getNode(node->name);
		assert(node2);
		node->Compare(node2);
	}
}


}  // namespace comm

