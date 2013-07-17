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

#ifndef M2C_COMMON_CTREE_H
#define M2C_COMMON_CTREE_H

#include <stdio.h>

#include <lib/util/class.h>

#include "node.h"


#ifdef HAVE_LLVM
#include <llvm-c/Core.h>
#endif


/*** Forward declarations ***/

CLASS_FORWARD_DECLARATION(BasicBlock);
struct config_t;



/*
 * Variables
 */

#define ctree_debug(...) debug(ctree_debug_category, __VA_ARGS__)

extern char *ctree_config_file_name;
extern char *ctree_debug_file_name;
extern int ctree_debug_category;



/*
 * Class 'CTree'
 */

CLASS_BEGIN(CTree, Object)
	
	/* Name of control tree */
	char *name;

	/* Counters used to assign names to new nodes. A different counter is
	 * used for each possible abstract node region. */
	unsigned int name_counter[AbstractNodeRegionCount];

	/* Nodes are kept in a linked list and a hash table */
	struct linked_list_t *node_list;
	struct hash_table_t *node_table;

	/* Root node.
	 * Read/Write access. */
	Node *entry_node;

	/* Flag indicating whether a structural analysis has been run on the
	 * control tree. */
	int structural_analysis_done;

CLASS_END(CTree)


void CTreeCreate(CTree *self, char *name);
void CTreeDestroy(CTree *self);

/* Virtual function from class Object */
void CTreeDump(Object *self, FILE *f);

/* Add a node to the control tree */
void CTreeAddNode(CTree *ctree, Node *node);

/* Given an LLVM function, create one node for each basic block. Nodes are then
 * connected following the same structure as the control flow graph of the LLVM
 * function, and they are inserted into the control tree. The node
 * corresponding to the LLVM entry basic block is returned. */
#if HAVE_LLVM
LeafNode *CTreeAddLlvmCFG(CTree *ctree, LLVMValueRef llfunction);
#endif

/* Search a node by its name */
Node *CTreeGetNode(CTree *ctree, char *name);

/* Free all nodes in the control tree and reset its entry. */
void CTreeClear(CTree *ctree);

/* Create the function control tree by performing a structural analysis on the
 * control flow graph of the function. */
void CTreeStructuralAnalysis(CTree *ctree);

/* Depth-first traversal of the control tree following the abstract nodes'
 * children (as opposed to successor/predecessor traversal of the control flow
 * graph). A structural analysis must have been run on the control tree first.
 * The function returns two lists with all tree nodes, listed in pre-order
 * and post-order, respectively. Either list can be NULL if that specific
 * ordering is of no interest to the caller. */
void CTreeTraverse(CTree *ctree, struct linked_list_t *preorder_list,
		struct linked_list_t *postorder_list);

/* Read/write the control tree from/to an INI file */
void CTreeWriteToConfig(CTree *ctree, struct config_t *config);
void CTreeReadFromConfig(CTree *ctree, struct config_t *config,
		char *name);

/* Compare two control trees */
void CTreeCompare(CTree *ctree1, CTree *ctree2);



/*
 * Public Functions
 * (Not related with 'ctree_t' object)
 */

void ctree_init(void);
void ctree_done(void);


#endif
