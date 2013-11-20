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

#ifndef M2C_COMMON_TREE_H
#define M2C_COMMON_TREE_H

#include <list>
#include <memory>
#include <unordered_map>

#include <lib/cpp/Debug.h>
#include <lib/cpp/IniFile.h>

#include "Node.h"


namespace Common
{

/* Forward declarations */
class Tree;


/* Class managing the configuration file used to debug the control flow tree,
 * activated with option '--tree-debug <file>' in the command line. */
class TreeConfig
{
	/* File name */
	std::string path;
	std::unique_ptr<misc::IniFile> ini_file;

	/* List of trees loaded by configuration */
	std::list<std::unique_ptr<Tree>> tree_list;

	/* Return a created control tree given its name, or null if the tree
	 * does not exist. */
	Tree *GetTree(const std::string &name);

	/* Process command read from the configuration file. */
	void ProcessCommand(const std::string &command);
public:
	const std::string &GetPath() { return path; }
	void SetPath(const std::string &path);

	void Run();
};


/* Class representing a control flow tree, obtained from a program's control
 * flow graph after a structural analysis. */
class Tree
{
	friend class TreeConfig;

	/* Name of control tree */
	std::string name;

	/* Counters used to assign names to new nodes. A different counter is
	 * used for each possible abstract node region. */
	unsigned int name_counter[AbstractNodeRegionCount];

	/* The main container for nodes is this linked list with smart pointers.
	 * When a node is removed from this list, it will be automatically
	 * destructed. */
	std::list<std::unique_ptr<Node>> node_list;

	/* Additional hash table storing nodes */
	std::unordered_map<std::string, Node *> node_table;

	/* Root node. */
	Node *entry_node;

	/* Flag indicating whether a structural analysis has been run on the
	 * control tree. */
	bool structural_analysis_done;

	/* Depth-first search on function. This creates a depth-first spanning
	 * tree and classifies graph edges as tree-, forward-, cross-, and
	 * back-edges. Also, a post-order traversal of the graph is dumped in
	 * 'postorder_list'. We follow the algorithm presented in
	 * http://www.personal.kent.edu/~rmuhamma/Algorithms/MyAlgorithms/
	 *        GraphAlgor/depthSearch.htm
	 */
	int DFS(std::list<Node *> &postorder_list, Node *node, int time);
	void DFS(std::list<Node *> &postorder_list);
	void DFS();

	/* Discover the natural loop (interval) with header 'header_node'. The interval
	 * is composed of all those nodes with a path from the header to the tail that
	 * doesn't go through the header, where the tail is a node that is connected to
	 * the header with a back-edge. */
	void ReachUnder(Node *header_node, Node *node,
			std::list<Node *> &reach_under_list);
	void ReachUnder(Node *header_node,
			std::list<Node *> &reach_under_list);

	/* Given an abstract node of type 'block' that was just reduced, take its
	 * sub-block regions and flatten them to avoid hierarchical blocks. */
	void FlattenBlock(AbstractNode *abs_node);

	/* Reduce the list of nodes in 'list' with a newly created abstract
	 * node, returned as the function result. Argument 'name' gives the name
	 * of the new abstract node. All incoming edges to any of the nodes in
	 * the list will point to 'node'. Likewise, all outgoing edges from any
	 * node in the list will come from 'this'. */
	AbstractNode *Reduce(std::list<Node *> &list, AbstractNodeRegion region);

	/* Identify a region, and return it in 'list'. The list
	 * 'list' must be empty when the function is called. If a valid block
	 * region is identified, the function returns true. Otherwise, it returns
	 * false and 'list' remains empty.
	 * List 'list' is an output list. */
	AbstractNodeRegion Region(Node *node, std::list<Node *> &list);

	/* Tree traversal */
	void PreorderTraversal(Node *node, std::list<Node *> &list);
	void PostorderTraversal(Node *node, std::list<Node *> &list);

	/* Auxiliary function called by its public homonymous. */
	LeafNode *AddLlvmCFG(llvm::BasicBlock *llvm_basic_block);

	/* Given a list of nodes in a string format, return the nodes in the
	 * linked list. */
	void GetNodeList(std::list<Node *> &list, const std::string &list_str);

public:

	/* Constructors */
	explicit Tree(const std::string &name);

	/* Create the tree from the content of an INI file */
	Tree(misc::IniFile &f, const std::string &name) { Read(f, name); }

	/* Getters */
	const std::string &GetName() { return name; }
	bool IsStructuralAnalysisDone() { return structural_analysis_done; }

	/* Setters */
	void SetEntryNode(Node *node) { assert(node->InList(node_list));
			entry_node = node; }

	/* Dump */
	void Dump(std::ostream &os);
	friend std::ostream &operator<<(std::ostream &os, Tree &tree) {
			tree.Dump(os); return os; }

	/* Add a node to the control tree. The control tree will own the node
	 * from now on, and the node will be destructed when the control tree
	 * is destructed. */
	void AddNode(Node *node);

	/* Given an LLVM function, create one node for each basic block. Nodes
	 * are then connected following the same structure as the control flow
	 * graph of the LLVM function, and they are inserted into the control
	 * tree. The node corresponding to the LLVM entry basic block is
	 * returned. */
	LeafNode *AddLlvmCFG(llvm::Function *llvm_function);

	/* Search node by name. Return null if node not found. */
	Node *GetNode(const std::string &name);

	/* Search leaf node by name and return null if the node is not found,
	 * or if a node with the same name is not a leaf node. */
	LeafNode *GetLeafNode(const std::string &name) {
			return dynamic_cast<LeafNode *>(GetNode(name)); }

	/* Search abstract node by name and return null if the node is not
	 * found or if a node with the same name is not an abstract node. */
	AbstractNode *GetAbstractNode(const std::string &name) {
			return dynamic_cast<AbstractNode *>(GetNode(name)); }

	/* Remove all nodes from tree and reset its entry. */
	void Clear();

	/* Create the function control tree by performing a structural analysis
	 * on the control flow graph of the function. */
	void StructuralAnalysis();

	/* Depth-first traversal of the control tree following abstract nodes'
	 * children (not successors or predecessors). */
	void PreorderTraversal(std::list<Node *> &list);
	void PostorderTraversal(std::list<Node *> &list);

	/* Read/write the control tree from/to an INI file */
	void Write(misc::IniFile &f);
	void Read(misc::IniFile &f, const std::string &name);

	/* Compare two control trees */
	void Compare(Tree *tree);

	/* Debugger */
	static misc::Debug debug;

	/* Configuration */
	static TreeConfig config;
};



}  /* namespace Common */

#endif
