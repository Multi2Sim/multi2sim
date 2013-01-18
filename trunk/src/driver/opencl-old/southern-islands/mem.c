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

#include <arch/southern-islands/emu/emu.h>
#include <lib/mhandle/mhandle.h>

#include "mem.h"
#include "repo.h"


struct si_opencl_mem_t *si_opencl_mem_create()
{
	struct si_opencl_mem_t *mem;

	/* Initialize */
	mem = xcalloc(1, sizeof(struct si_opencl_mem_t));
	mem->id = si_opencl_repo_new_object_id(si_emu->opencl_repo,
		si_opencl_object_mem);
	mem->ref_count = 1;

	/* Return */
	si_opencl_repo_add_object(si_emu->opencl_repo, mem);
	return mem;
}


void si_opencl_mem_free(struct si_opencl_mem_t *mem)
{
	si_opencl_repo_remove_object(si_emu->opencl_repo, mem);
	free(mem);
}

