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

#ifndef M2C_LLVM2SI_STRUCT_ANALYSIS_H
#define M2C_LLVM2SI_STRUCT_ANALYSIS_H

#include <stdio.h>

#include <lib/util/string.h>


/* Forward declarations */
struct llvm2si_basic_block_t;
struct llvm2si_function_t;
struct list_t;
struct linked_list_t;


/* Kind of control tree node. The node can be a leaf representing a basic block
 * of the function, or an abstract node, representing a reduction of the control
 * flow graph. */
enum llvm2si_function_node_kind_t
{
	llvm2si_function_node_kind_invalid,
	llvm2si_function_node_leaf,
	llvm2si_function_node_abstract
};

/* Node of the control tree */
struct llvm2si_function_node_t
{
	enum llvm2si_function_node_kind_t kind;
	char *name;

	/* Function that the node belongs to */
	struct llvm2si_function_t *function;

	struct linked_list_t *succ_list;
	struct linked_list_t *pred_list;

	struct linked_list_t *forward_edge_list;
	struct linked_list_t *back_edge_list;
	struct linked_list_t *tree_edge_list;
	struct linked_list_t *cross_edge_list;

	/* If the node is part of a higher-level abstract node, this field
	 * points to it. If not, the field is NULL. */
	struct llvm2si_function_node_t *parent;

	/* Conditional fields depending on the node kind */
	union
	{
		struct
		{
			struct llvm2si_basic_block_t *basic_block;
		} leaf;

		struct
		{
			/* List of function nodes associated with the abstract
			 * node. Elements of type 'llvm2si_function_node_t'. */
			struct linked_list_t *child_list;
		} abstract;
	};

	/* Identifiers assigned during the depth-first search */
	int preorder_id;
	int postorder_id;

	/* Color used for traversal algorithms */
	int color;
};

struct llvm2si_function_node_t *llvm2si_function_node_create_leaf(
		struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block);

/* Create an abstract node containing the list of nodes in 'elem_list'. The list
 * 'elem_list' will be copied internally, and should be initialized and freed by
 * the caller. */
struct llvm2si_function_node_t *llvm2si_function_node_create_abstract(
		struct llvm2si_function_t *function, char *name);

void llvm2si_function_node_free(struct llvm2si_function_node_t *node);
void llvm2si_function_node_dump(struct llvm2si_function_node_t *node, FILE *f);

/* Dumping lists of nodes */
void llvm2si_function_node_list_dump(struct linked_list_t *list, FILE *f);
void llvm2si_function_node_list_dump_detail(struct linked_list_t *list, FILE *f);




/*
 * Function object
 */

extern struct str_map_t llvm2si_function_region_map;
enum llvm2si_function_region_t
{
	llvm2si_function_region_invalid,
	llvm2si_function_region_block,
	llvm2si_function_region_if_then,
	llvm2si_function_region_if_then_else,
	llvm2si_function_region_self_loop,
	llvm2si_function_region_while_loop,
	llvm2si_function_region_natural_loop,
	llvm2si_function_region_improper
};


/* Create the function control tree by performing a structural analysis on the
 * control flow graph of the function. */
void llvm2si_function_struct_analysis(struct llvm2si_function_t *function);

/* Dump the control tree created by the structural analysis. */
void llvm2si_function_dump_control_tree(struct llvm2si_function_t *function,
		FILE *f);

#endif
