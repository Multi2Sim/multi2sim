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
#include <lib/util/debug.h>
#include <mem-system/memory.h>

#include "platform.h"
#include "repo.h"


struct evg_opencl_platform_t *evg_opencl_platform_create(EvgEmu *emu)
{
	struct evg_opencl_platform_t *platform;

	/* Initialize */
	platform = xcalloc(1, sizeof(struct evg_opencl_platform_t));
	platform->id = evg_opencl_repo_new_object_id(emu->opencl_repo,
		evg_opencl_object_platform);
	platform->emu = emu;

	/* Return */
	evg_opencl_repo_add_object(emu->opencl_repo, platform);
	return platform;
}


void evg_opencl_platform_free(struct evg_opencl_platform_t *platform)
{
	EvgEmu *emu;

	emu = platform->emu;
	evg_opencl_repo_remove_object(emu->opencl_repo, platform);
	free(platform);
}


unsigned int evg_opencl_platform_get_info(struct evg_opencl_platform_t *platform, unsigned int name, struct mem_t *mem, unsigned int addr, unsigned int size)
{
	char *platform_profile = "FULL_PROFILE";
	char *platform_version = "OpenCL 1.1 Multi2Sim-v" VERSION;
	char *platform_name = "Multi2Sim";
	char *platform_vendor = "www.multi2sim.org";
	char *platform_extensions = "";

	unsigned int size_ret = 0;
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
			name, evg_err_opencl_param_note);
	}

	/* Write to memory and return size */
	assert(info);
	size_ret = strlen(info) + 1;
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}

