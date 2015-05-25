/*
 *  Multi2Sim
 *  Copyright (C) 2012  Yifan Sun (yifansun@coe.neu.edu)
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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "debug.h"
#include "hsa.h"

hsa_status_t HSA_API hsa_region_get_info(hsa_region_t region,
                                         hsa_region_info_t attribute,
                                         void *value)
{
	// Init arguments
	unsigned int args[6] = {0};
	memcpy(args + 1, &region, 8);
	memcpy(args + 3, &attribute, 4);
	memcpy(args + 4, &value, 4);

	// Check if HSA runtime initialized
	if(!hsa_runtime)
		return HSA_STATUS_ERROR_NOT_INITIALIZED;

	// Call driver function
	ioctl(hsa_runtime->fd, RegionGetInfo, args);

	// Return
	return args[0];
}


hsa_status_t HSA_API hsa_agent_iterate_regions(
	hsa_agent_t agent,
	hsa_status_t (*callback)(hsa_region_t region, void *data),
	void *data)
{
	// Init arguments
	unsigned long long args[3] = {0};
	args[0] = 1;
	args[1] = agent.handle;
	args[2] = 0;

	// Check if HSA runtime initialized
	if (!hsa_runtime)
		return HSA_STATUS_ERROR_NOT_INITIALIZED;

	// Iterate regions
	while (args[0] != 0)
	{
		ioctl(hsa_runtime->fd, NextRegion, args);
		if (args[0] != 0)
		{
			hsa_region_t region;
			region.handle = args[0];
			hsa_status_t status = callback(region, data);
			if (status != HSA_STATUS_SUCCESS)
				return status;
			args[2] = args[0];
		}
	}

	// Return
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API
	hsa_memory_allocate(hsa_region_t region, size_t size, void **ptr)
{
	// Init arguments
	unsigned int args[7] = {0};
	memcpy(args + 1, &region, 8);
	memcpy(args + 3, &size, 4);
	memcpy(args + 5, &ptr, 4);

	// Check if HSA runtime initialized
	if (!hsa_runtime)
		return HSA_STATUS_ERROR_NOT_INITIALIZED;

	// Iterate regions
	ioctl(hsa_runtime->fd, MemoryAllocate, args);

	return args[0];
}


hsa_status_t HSA_API hsa_memory_free(void *ptr)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_memory_copy(void *dst, const void *src, size_t size)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_memory_assign_agent(void *ptr, hsa_agent_t agent,
                                             hsa_access_permission_t access)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_memory_register(void *ptr, size_t size)
{
	return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_memory_deregister(void *ptr, size_t size)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}

