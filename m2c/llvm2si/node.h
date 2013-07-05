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

#ifndef M2C_LLVM2SI_NODE_H
#define M2C_LLVM2SI_NODE_H

struct linked_list_t;
struct llvm2si_ctree_t;


/* Kind of control tree node. The node can be a leaf representing a basic block
 * of the function, or an abstract node, representing a reduction of the control
 * flow graph. */
extern struct str_map_t llvm2si_node_kind_map;
enum llvm2si_node_kind_t
{
	llvm2si_node_kind_invalid,
	llvm2si_node_leaf,
	llvm2si_node_abstract
};

extern struct str_map_t llvm2si_node_region_map;
enum llvm2si_node_region_t
{
	llvm2si_node_region_invalid,
	llvm2si_node_block,
	llvm2si_node_if_then,
	llvm2si_node_if_then_else,
	llvm2si_node_while_loop,
	llvm2si_node_loop,
	llvm2si_node_proper_interval,
	llvm2si_node_improper_interval,
	llvm2si_node_proper_outer_interval,
	llvm2si_node_improper_outer_interval
};


/* Node of the control tree */
struct llvm2si_node_t
{
	enum llvm2si_node_kind_t kind;
	char *name;

	/* Control tree that the node belongs to */
	struct llvm2si_ctree_t *ctree;

	struct linked_list_t *succ_list;
	struct linked_list_t *pred_list;

	struct linked_list_t *forward_edge_list;
	struct linked_list_t *back_edge_list;
	struct linked_list_t *tree_edge_list;
	struct linked_list_t *cross_edge_list;

	/* If the node is part of a higher-level abstract node, this field
	 * points to it. If not, the field is NULL. */
	struct llvm2si_node_t *parent;

	/* Conditional fields depending on the node kind */
	union
	{
		struct
		{
			struct llvm2si_basic_block_t *basic_block;
		} leaf;

		struct
		{
			/* Type of region */
			enum llvm2si_node_region_t region;

			/* List of function nodes associated with the abstract
			 * node. Elements of type 'llvm2si_node_t'. */
			struct linked_list_t *child_list;
		} abstract;
	};

	/* Identifiers assigned during the depth-first search */
	int preorder_id;
	int postorder_id;

	/* Color used for traversal algorithms */
	int color;
};


/* Create node of type leaf */
struct llvm2si_node_t *llvm2si_node_create_leaf(char *name);

/* Create an abstract node containing the list of nodes in 'elem_list'. The list
 * 'elem_list' will be copied internally, and should be initialized and freed by
 * the caller. */
struct llvm2si_node_t *llvm2si_node_create_abstract(char *name,
		enum llvm2si_node_region_t region);

void llvm2si_node_free(struct llvm2si_node_t *node);
void llvm2si_node_dump(struct llvm2si_node_t *node, FILE *f);

int llvm2si_node_in_list(struct llvm2si_node_t *node,
		struct linked_list_t *list);

/* Try to create an edge between 'node' and 'node_dest'. If the edge already
 * exist, the function will ignore the call silently. */
void llvm2si_node_try_connect(struct llvm2si_node_t *node,
		struct llvm2si_node_t *node_dest);

/* Create an edge between 'node' and 'node_dest'. There should be no existing
 * edge for this source and destination when calling this function. */
void llvm2si_node_connect(struct llvm2si_node_t *node,
		struct llvm2si_node_t *node_dest);

/* Try to remove an edge between 'node' and 'node_dest'. If the edge does not
 * exist, the function exists silently. */
void llvm2si_node_try_disconnect(struct llvm2si_node_t *node,
		struct llvm2si_node_t *node_dest);

/* Disconnect 'node' and 'node_dest'. An edge must exist between both. */
void llvm2si_node_disconnect(struct llvm2si_node_t *node,
		struct llvm2si_node_t *node_dest);
		
/* Dumping lists of nodes */
void llvm2si_node_list_dump(struct linked_list_t *list, FILE *f);
void llvm2si_node_list_dump_buf(struct linked_list_t *list, char *buf,
		int size);
void llvm2si_node_list_dump_detail(struct linked_list_t *list, FILE *f);

#endif

