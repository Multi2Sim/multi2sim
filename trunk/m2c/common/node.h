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

#include <lib/util/class.h>


#ifdef HAVE_LLVM
#include <llvm-c/Core.h>
#endif


/* Forward declarations */
CLASS_FORWARD_DECLARATION(BasicBlock);
CLASS_FORWARD_DECLARATION(CTree);
CLASS_FORWARD_DECLARATION(Node);
struct linked_list_t;



/*
 * Class 'Node'
 */

/* Kind of control tree node. The node can be a leaf representing a basic block
 * of the function, or an abstract node, representing a reduction of the control
 * flow graph. */
extern struct str_map_t node_kind_map;
enum node_kind_t
{
	node_kind_invalid,
	node_leaf,
	node_abstract
};

extern struct str_map_t node_region_map;
enum node_region_t
{
	node_region_invalid,

	node_region_block,
	node_region_if_then,
	node_region_if_then_else,
	node_region_while_loop,
	node_region_loop,
	node_region_proper_interval,
	node_region_improper_interval,
	node_region_proper_outer_interval,
	node_region_improper_outer_interval,

	node_region_count
};


extern struct str_map_t node_role_map;
enum node_role_t
{
	node_role_invalid,

	node_role_if,
	node_role_then,
	node_role_else,
	node_role_head,
	node_role_tail,
	node_role_pre,  /* Loop pre-header */
	node_role_exit,  /* Loop exit */

	node_role_count
};


/* Node of the control tree */
CLASS_BEGIN(Node, Object)
	
	enum node_kind_t kind;
	char *name;

	/* Control tree that the node belongs to */
	CTree *ctree;

	struct linked_list_t *succ_list;
	struct linked_list_t *pred_list;

	struct linked_list_t *forward_edge_list;
	struct linked_list_t *back_edge_list;
	struct linked_list_t *tree_edge_list;
	struct linked_list_t *cross_edge_list;

	/* If the node is part of a higher-level abstract node, this field
	 * points to it. If not, the field is NULL. */
	Node *parent;

	/* Role that the node plays inside of its parent abstract node.
	 * This field is other than 'node_role_invalid' only when 'parent' is
	 * not NULL. */
	enum node_role_t role;

	/* Flags indicating when a node with role 'node_role_head' belonging
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
			BasicBlock *basic_block;
		} leaf;

		struct
		{
			/* Type of region */
			enum node_region_t region;

			/* List of function nodes associated with the abstract
			 * node. Elements of type 'node_t'. */
			struct linked_list_t *child_list;
		} abstract;
	};

	/* Identifiers assigned during the depth-first search */
	int preorder_id;
	int postorder_id;

	/* Color used for traversal algorithms */
	int color;

#ifdef HAVE_LLVM
	/* When the node is created automatically from an LLVM function's
	 * control flow graph, this fields contains the associated LLVM
	 * basic block. */
	LLVMBasicBlockRef llbb;
#endif

CLASS_END(Node)


void NodeCreate(Node *self, enum node_kind_t kind, char *name,
		enum node_region_t region);
void NodeDestroy(Node *self);

/* Virtual function inherited from class Object */
void NodeDump(Object *self, FILE *f);

/* Return the basic block associated to the node. This function makes a sanity
 * check on the node type: it must be a leaf. */
BasicBlock *node_get_basic_block(Node *node);

/* Return true if 'node' is in the linked list of nodes passed as the second
 * argument. This function does not call 'linked_list_find'. Instead, it
 * traverses the list using a dedicated iterator, so that the current element of
 * the list is not lost. */
int node_in_list(Node *node, struct linked_list_t *list);

/* Try to create an edge between 'node' and 'node_dest'. If the edge already
 * exist, the function will ignore the call silently. */
void node_try_connect(Node *node, Node *node_dest);

/* Create an edge between 'node' and 'node_dest'. There should be no existing
 * edge for this source and destination when calling this function. */
void node_connect(Node *node, Node *node_dest);

/* Try to remove an edge between 'node' and 'node_dest'. If the edge does not
 * exist, the function exists silently. */
void node_try_disconnect(Node *node, Node *node_dest);

/* Disconnect 'node' and 'node_dest'. An edge must exist between both. */
void node_disconnect(Node *node, Node *node_dest);

/* Try to reconnect a source node with a new destination node. This is
 * equivalent to disconnecting and connecting it, except that the order
 * of the edge within the successor list of the source node is
 * guaranteed to stay the same. If an edge already exists between the
 * source and the new destination, the original edge will just be
 * completely removed. */
void node_reconnect_dest(Node *src_node,
		Node *dest_node,
		Node *new_dest_node);

/* Try to replace the source node of an edge. This is equivalent to
 * disconnecting and connecting it, except that the order of the
 * predecessor list of the destination node is guaranteed to stay
 * intact. If an edge already exists between the new source and the
 * destination, the original edge will just be completely removed. */
void node_reconnect_source(Node *src_node, Node *dest_node,
		Node *new_src_node);

/* Make 'node' take the same parent as 'before' and place it right before it in
 * its child list. Node 'before' must have a parent.
 * This does not insert the node into the control tree structures (an extra
 * call to 'node_add_node' is needed). */
void node_insert_before(Node *node, Node *before);

/* Make 'node' take the same parent as 'after' and place it right after it in
 * its child list. Node 'after' must have a parent.
 * This does not insert the node into the control tree structures (an extra
 * call to 'node_add_node' is needed). */
void node_insert_after(Node *node, Node *after);

/* Starting at 'node', traverse the syntax tree (not control tree) in depth-
 * first and return the first leaf node found (could be 'node' itself). */
Node *node_get_first_leaf(Node *node);

/* Starting at 'node', traverse the syntax tree (not control tree) in depth-
 * first and return the last leaf node found (could be 'node' itself). */
Node *node_get_last_leaf(Node *node);
		
/* Dumping lists of nodes */
void node_list_dump(struct linked_list_t *list, FILE *f);
void node_list_dump_buf(struct linked_list_t *list, char *buf, int size);
void node_list_dump_detail(struct linked_list_t *list, FILE *f);

/* Compare two nodes */
void node_compare(Node *node1, Node *node2);

#endif

