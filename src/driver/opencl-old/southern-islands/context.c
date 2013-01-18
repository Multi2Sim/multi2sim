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

#include <assert.h>

#include <arch/southern-islands/emu/emu.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <mem-system/memory.h>

#include "context.h"
#include "opencl.h"
#include "repo.h"


/* Create a context */
struct si_opencl_context_t *si_opencl_context_create()
{
	struct si_opencl_context_t *context;

	/* Initialize */
	context = xcalloc(1, sizeof(struct si_opencl_context_t));
	context->id = si_opencl_repo_new_object_id(si_emu->opencl_repo,
		si_opencl_object_context);
	context->ref_count = 1;

	/* Return */
	si_opencl_repo_add_object(si_emu->opencl_repo, context);
	return context;
}


/* Free context */
void si_opencl_context_free(struct si_opencl_context_t *context)
{
	si_opencl_repo_remove_object(si_emu->opencl_repo, context);
	free(context);
}


unsigned int si_opencl_context_get_info(struct si_opencl_context_t *context, unsigned int name, struct mem_t *mem, unsigned int addr, unsigned int size)
{
	unsigned int num_devices = 1;

	unsigned int size_ret = 0;
	void *info = NULL;

	switch (name)
	{

	case 0x1081:  /* CL_CONTEXT_DEVICES */
		size_ret = 4;
		info = &context->device_id;
		break;

	case 0x1083:  /* CL_CONTEXT_NUM_DEVICES */
		size_ret = 4;
		info = &num_devices;
		break;

	case 0x1080:  /* CL_CONTEXT_REFERENCE_COUNT */
	case 0x1082:  /* CL_CONTEXT_PROPERTIES */
	default:
		fatal("opencl_context_get_info: invalid or not implemented value for 'name' (0x%x)\n%s",
			name, si_err_opencl_param_note);
	}
	
	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}


/* Read context properties from a null-terminated sequence of
 * 'cl_context_properties' elements in guest memory */
void si_opencl_context_set_properties(struct si_opencl_context_t *context, struct mem_t *mem, unsigned int addr)
{
	unsigned int property;
	unsigned int value;

	while (addr)
	{
		/* Read property */
		mem_read(mem, addr, 4, &property);
		if (!property)
			break;
		mem_read(mem, addr + 4, 4, &value);
		addr += 8;

		/* Analyze property */
		switch (property)
		{

		case 0x1084:  /* CL_CONTEXT_PLATFORM */
			context->platform_id = value;
			si_opencl_repo_get_object(si_emu->opencl_repo, si_opencl_object_platform, value);
			si_opencl_debug("    property CL_CONTEXT_PLATFORM assigned: 0x%x\n", value);
			break;

		default:
			fatal("opencl_context_read_properties: invalid property (0x%x)\n", property);
		}
	}
}

