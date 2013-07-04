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

#ifndef M2C_LLVM2SI_CTREE_H
#define M2C_LLVM2SI_CTREE_H

#include <stdio.h>

struct config_t;

struct llvm2si_ctree_t
{
	struct linked_list_t *node_list;
	struct llvm2si_node_t *node_entry;
};

struct llvm2si_ctree_t *llvm2si_ctree_create(void);
void llvm2si_ctree_free(struct llvm2si_ctree_t *ctree);
void llvm2si_ctree_dump(struct llvm2si_ctree_t *ctree, FILE *f);

/* Add a node to the control tree */
void llvm2si_ctree_add_node(struct llvm2si_ctree_t *ctree,
		struct llvm2si_node_t *node);

/* Free all nodes in the control tree and reset its entry. */
void llvm2si_ctree_clear(struct llvm2si_ctree_t *ctree);

/* Create the function control tree by performing a structural analysis on the
 * control flow graph of the function. */
void llvm2si_ctree_structural_analysis(struct llvm2si_ctree_t *ctree);

/* Read/write the control tree from/to an INI file */
void llvm2si_ctree_write_to_config(struct llvm2si_ctree_t *ctree,
		struct config_t *config);
void llvm2si_ctree_read_from_config(struct llvm2si_ctree_t *ctree,
			struct config_t *config);


#endif

