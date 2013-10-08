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

#include <lib/cpp/IniFile.h>
#include <lib/cpp/Misc.h>


namespace Common
{

/*
#define ctree_debug(...) debug(ctree_debug_category, __VA_ARGS__)

extern char *ctree_config_file_name;
extern char *ctree_debug_file_name;
extern int ctree_debug_category;
*/


class Tree
{
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

	/* Root node.
	 * Read/Write access. */
	Node *entry_node;

	/* Flag indicating whether a structural analysis has been run on the
	 * control tree. */
	bool structural_analysis_done;

public:

	/* Constructor and destructor */
	explicit Tree(const std::string &name);
	~Tree();

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
	LeafNode *AddLlvmCFG(llvm::Value *function);

	/* Search node by name. Return null if node not found. */
	Node *GetNode(const std::string &name);

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
	void Write(IniFile *f);
	void Read(IniFile *f, const std::string &name);

	/* Compare two control trees */
	void Compare(Tree *tree);
};



}  /* namespace Common */

#endif
