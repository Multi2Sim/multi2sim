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

#ifndef VISUAL_X86_CPU_H
#define VISUAL_X86_CPU_H


struct hash_table_t;
struct list_t;

struct vi_x86_cpu_t
{
	struct list_t *core_list;

	/* Hash table of contexts.
	 * Elements are of type 'struct vi_x86_context_t' */
	struct hash_table_t *context_table;

	/* True if the trace file contains x86 information at all */
	int active;
};


extern struct vi_x86_cpu_t *vi_x86_cpu;

void vi_x86_cpu_init(void);
void vi_x86_cpu_done(void);


#endif

