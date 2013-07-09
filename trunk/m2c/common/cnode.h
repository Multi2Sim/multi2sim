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

	cnode_region_block,
	cnode_region_if_then,
	cnode_region_if_then_else,
	cnode_region_while_loop,
	cnode_region_loop,
	cnode_region_proper_interval,
	cnode_region_improper_interval,
	cnode_region_proper_outer_interval,
	cnode_region_improper_outer_interval,

	cnode_region_count
};


extern struct str_map_t cnode_role_map;
enum cnode_role_t
{
	cnode_role_invalid,

	cnode_role_if,
	cnode_role_then,
	cnode_role_else,
	cnode_role_head,
	cnode_role_tail,
	cnode_role_pre,  /* Loop pre-header */
	cnode_role_exit,  /* Loop exit */

	cnode_role_count
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

	/* Role that the node plays inside of its parent abstract node.
	 * This field is other than 'cnode_role_invalid' only when 'parent' is
	 * not NULL. */
	enum cnode_role_t role;

	/* Flags indicating when a node with role 'cnode_role_head' belonging
	 * to a parent region 'while_loop' exists the loop when its condition
	 * is evaluated to true or false. Only one of these two flags can be
	 * set. */
	int exit_if_true;
	int exit_if_false;

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

/* Try to reconnect a source node with a new destination node. This is
 * equivalent to disconnecting and connecting it, except that the order
 * of the edge within the successor list of the source node is
 * guaranteed to stay the same. If an edge already exists between the
 * source and the new destination, the original edge will just be
 * completely removed. */
void cnode_reconnect_dest(struct cnode_t *src_node,
		struct cnode_t *dest_node,
		struct cnode_t *new_dest_node);

/* Try to replace the source node of an edge. This is equivalent to
 * disconnecting and connecting it, except that the order of the
 * predecessor list of the destination node is guaranteed to stay
 * intact. If an edge already exists between the new source and the
 * destination, the original edge will just be completely removed. */
void cnode_reconnect_source(struct cnode_t *src_node,
		struct cnode_t *dest_node,
		struct cnode_t *new_src_node);

/* Insert node 'node' before node 'before', meaning that an edge
 * 'node'=>'before' is created, and that 'node' will appear right before node
 * 'before' in their common abstract parent node. If 'before' doesn't have a
 * parent node, this function is equivalent to 'cnode_connect'. */
void cnode_insert_before(struct cnode_t *node, struct cnode_t *before);

/* Create an edge 'after'=>'node'. Node 'node' will appear right after node
 * 'after' in their common parent's child list. If 'after doesn't have a parent
 * node, the call is equivalent to 'cnode_connect(after, node)'. */
void cnode_insert_after(struct cnode_t *node, struct cnode_t *after);

/* Starting at 'node', traverse the syntax tree (not control tree) in depth-
 * first and return the first leaf node found (could be 'node' itself). */
struct cnode_t *cnode_get_first_leaf(struct cnode_t *node);

/* Starting at 'node', traverse the syntax tree (not control tree) in depth-
 * first and return the last leaf node found (could be 'node' itself). */
struct cnode_t *cnode_get_last_leaf(struct cnode_t *node);
		
/* Dumping lists of nodes */
void cnode_list_dump(struct linked_list_t *list, FILE *f);
void cnode_list_dump_buf(struct linked_list_t *list, char *buf,
		int size);
void cnode_list_dump_detail(struct linked_list_t *list, FILE *f);

/* Compare two nodes */
void cnode_compare(struct cnode_t *node1,
		struct cnode_t *node2);

#endif

