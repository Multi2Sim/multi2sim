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

#ifndef M2C_COMMON_BASIC_BLOCK_H
#define M2C_COMMON_BASIC_BLOCK_H

#include <stdio.h>

#include <lib/util/class.h>


/* Class-related macros */
#define BASIC_BLOCK_TYPE 0x4c1e17fa
#define BASIC_BLOCK(p) CLASS_REINTERPRET_CAST((p), BASIC_BLOCK_TYPE, struct basic_block_t)
#define BASIC_BLOCK_CLASS_OF(p) CLASS_OF((p), BASIC_BLOCK_TYPE)


/* Class: basic_block_t
 * Inherits: None
 */
struct basic_block_t
{
	/* Class information
	 * WARNING - must be the first field */
	struct class_t class_info;


	/*** Public fields ***/

	char *name;


	/*** Private fields ***/

	/* Predecessor and successor lists of basic blocks */
	struct linked_list_t *pred_list;
	struct linked_list_t *succ_list;

	/* Node associated in control tree */
	struct cnode_t *cnode;


	/*** Virtual methods ***/

	void (*free)(struct basic_block_t *basic_block);
	void (*dump)(struct basic_block_t *basic_block, FILE *f);
};


struct basic_block_t *basic_block_create(char *name);
void basic_block_free(struct basic_block_t *basic_block);

/* Change basic block name */
void basic_block_set_name(struct basic_block_t *basic_block, char *name);

/* Connect 'basic_block' to 'basic_block_dest' in control flow graph */
void basic_block_connect(struct basic_block_t *basic_block,
		struct basic_block_t *basic_block_dest);

#endif
