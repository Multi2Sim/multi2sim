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

#ifndef M2C_COMMON_CNODE_H
#define M2C_COMMON_CNODE_H

struct basic_block_t;
struct linked_list_t;
struct ctree_t;


/* Kind of control tree node. The node can be a leaf representing a basic block
 * of the function, or an abstract node, representing a reduction of the control
 * flow graph. */
extern struct str_map_t cnode_kind_map;
enum cnode_kind_t
{
	cnode_kind_invalid,
	cnode_leaf,
	cnode_abstract
};

extern struct str_map_t cnode_region_map;
enum cnode_region_t
{
	cnode_region_invalid,
	cnode_block,
	cnode_if_then,
	cnode_if_then_else,
	cnode_while_loop,
	cnode_loop,
	cnode_proper_interval,
	cnode_improper_interval,
	cnode_proper_outer_interval,
	cnode_improper_outer_interval
};


/* Node of the control tree */
struct cnode_t
{
	enum cnode_kind_t kind;
	char *name;

	/* Control tree that the node belongs to */
	struct ctree_t *ctree;

	struct linked_list_t *succ_list;
	struct linked_list_t *pred_list;

	struct linked_list_t *forward_edge_list;
	struct linked_list_t *back_edge_list;
	struct linked_list_t *tree_edge_list;
	struct linked_list_t *cross_edge_list;

	/* If the node is part of a higher-level abstract node, this field
	 * points to it. If not, the field is NULL. */
	struct cnode_t *parent;

	/* Conditional fields depending on the node kind */
	union
	{
		struct
		{
			struct basic_block_t *basic_block;
		} leaf;

		struct
		{
			/* Type of region */
			enum cnode_region_t region;

			/* List of function nodes associated with the abstract
			 * node. Elements of type 'cnode_t'. */
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
struct cnode_t *cnode_create_leaf(char *name,
		struct basic_block_t *basic_block);

/* Create an abstract node containing the list of nodes in 'elem_list'. The list
 * 'elem_list' will be copied internally, and should be initialized and freed by
 * the caller. */
struct cnode_t *cnode_create_abstract(char *name,
		enum cnode_region_t region);

void cnode_free(struct cnode_t *node);
void cnode_dump(struct cnode_t *node, FILE *f);

int cnode_in_list(struct cnode_t *node,
		struct linked_list_t *list);

/* Try to create an edge between 'node' and 'node_dest'. If the edge already
 * exist, the function will ignore the call silently. */
void cnode_try_connect(struct cnode_t *node,
		struct cnode_t *node_dest);

/* Create an edge between 'node' and 'node_dest'. There should be no existing
 * edge for this source and destination when calling this function. */
void cnode_connect(struct cnode_t *node,
		struct cnode_t *node_dest);

/* Try to remove an edge between 'node' and 'node_dest'. If the edge does not
 * exist, the function exists silently. */
void cnode_try_disconnect(struct cnode_t *node,
		struct cnode_t *node_dest);

/* Disconnect 'node' and 'node_dest'. An edge must exist between both. */
void cnode_disconnect(struct cnode_t *node,
		struct cnode_t *node_dest);
		
/* Dumping lists of nodes */
void cnode_list_dump(struct linked_list_t *list, FILE *f);
void cnode_list_dump_buf(struct linked_list_t *list, char *buf,
		int size);
void cnode_list_dump_detail(struct linked_list_t *list, FILE *f);

/* Compare two nodes */
void cnode_compare(struct cnode_t *node1,
		struct cnode_t *node2);

#endif

