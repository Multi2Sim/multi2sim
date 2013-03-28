/*
 *  Multi2Sim
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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "mem.h"

unsigned int mem_id = 1;

struct opengl_mem_t *opengl_mem_create()
{
	struct opengl_mem_t *mem;

	/* Allocate */
	mem = xcalloc(1, sizeof(struct opengl_mem_t));

	/* Initialize */
	mem->id = mem_id;
	pthread_mutex_init(&mem->ref_mutex, NULL);

	/* Update global mem id */
	mem_id += 1;

	/* Return */
	return mem;
}

void opengl_mem_free(struct opengl_mem_t *mem)
{
	pthread_mutex_destroy(&mem->ref_mutex);
	free(mem);
}
