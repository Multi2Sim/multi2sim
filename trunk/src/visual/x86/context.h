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

#ifndef VISUAL_X86_CONTEXT_H
#define VISUAL_X86_CONTEXT_H


#define VI_X86_CONTEXT_EMPTY   ((void *) 1)

struct vi_x86_context_t
{
	char *name;
	int id;

	/* Map */
	int core_id;
	int thread_id;
	int parent_id;

	long long creation_cycle;
};

struct vi_x86_context_t *vi_x86_context_create(char *name, int id, int parent_id);
void vi_x86_context_free(struct vi_x86_context_t *context);

void vi_x86_context_get_name_short(char *context_name, char *buf, int size);
void vi_x86_context_get_desc(char *context_name, char *buf, int size);

void vi_x86_context_read_checkpoint(struct vi_x86_context_t *context, FILE *f);
void vi_x86_context_write_checkpoint(struct vi_x86_context_t *context, FILE *f);


#endif

