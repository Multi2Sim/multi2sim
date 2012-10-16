/*
 *  Multi2Sim Tools
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef VISUAL_X86_CORE_H
#define VISUAL_X86_CORE_H


struct vi_x86_core_t
{
	char *name;

	/* Hash table of contexts.
	 * Only names are stored. The elements in the table are all
	 * 'VI_X86_CONTEXT_EMPTY'. */
	struct hash_table_t *context_table;

	/* Hash table of instructions.
	 * Each element is of type 'struct vi_x86_inst_t'. */
	struct hash_table_t *inst_table;

	/* Number of instructions */
	long long num_insts;
};

struct vi_x86_core_t *vi_x86_core_create(char *name);
void vi_x86_core_free(struct vi_x86_core_t *core);

void vi_x86_core_read_checkpoint(struct vi_x86_core_t *core, FILE *f);
void vi_x86_core_write_checkpoint(struct vi_x86_core_t *core, FILE *f);


#endif

