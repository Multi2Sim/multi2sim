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

#include <arch/evergreen/emu/emu.h>
#include <lib/mhandle/mhandle.h>

#include "repo.h"
#include "sampler.h"


struct evg_opencl_sampler_t *evg_opencl_sampler_create()
{
	struct evg_opencl_sampler_t *sampler;

	/* Initialize */
	sampler = xcalloc(1, sizeof(struct evg_opencl_sampler_t));
	sampler->id = evg_opencl_repo_new_object_id(evg_emu->opencl_repo,
		evg_opencl_object_sampler);
	sampler->ref_count = 1;

	/* Return */
	evg_opencl_repo_add_object(evg_emu->opencl_repo, sampler);
	return sampler;
}

/* Free sampler */
void evg_opencl_sampler_free(struct evg_opencl_sampler_t *sampler)
{
	evg_opencl_repo_remove_object(evg_emu->opencl_repo, sampler);
	free(sampler);
}

