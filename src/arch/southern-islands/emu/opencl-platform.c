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
#include <stdlib.h>

#include <arch/x86/emu/emu.h>
#include <lib/struct/debug.h>
#include <mem-system/mem-system.h>

#include "emu.h"


struct si_opencl_platform_t *si_opencl_platform_create()
{
	struct si_opencl_platform_t *platform;

	/* Allocate */
	platform = calloc(1, sizeof(struct si_opencl_platform_t));
	if (!platform)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	platform->id = si_opencl_repo_new_object_id(si_emu->opencl_repo,
		si_opencl_object_platform);

	/* Return */
	si_opencl_repo_add_object(si_emu->opencl_repo, platform);
	return platform;
}


void si_opencl_platform_free(struct si_opencl_platform_t *platform)
{
	si_opencl_repo_remove_object(si_emu->opencl_repo, platform);
	free(platform);
}


uint32_t si_opencl_platform_get_info(struct si_opencl_platform_t *platform, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size)
{
	char *platform_profile = "FULL_PROFILE";
	char *platform_version = "OpenCL 1.1 Multi2Sim-v" VERSION;
	char *platform_name = "Multi2Sim";
	char *platform_vendor = "www.multi2sim.org";
	char *platform_extensions = "";

	uint32_t size_ret = 0;
	char *info;

	switch (name) {
	
	case 0x900:  /* CL_PLATFORM_PROFILE */
		info = platform_profile;
		break;

	case 0x901:  /* CL_PLATFORM_VERSION */
		info = platform_version;
		break;

	case 0x902:  /* CL_PLATFORM_NAME */
		info = platform_name;
		break;

	case 0x903:  /* CL_PLATFORM_VENDOR */
		info = platform_vendor;
		break;

	case 0x904:  /* CL_PLATFORM_EXTENSIONS */
		info = platform_extensions;
		break;

	default:
		info = NULL;
		fatal("opencl_platform_get_info: invalid value for 'name' (0x%x)\n%s",
			name, si_err_opencl_param_note);
	}

	/* Write to memory and return size */
	assert(info);
	size_ret = strlen(info) + 1;
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}

