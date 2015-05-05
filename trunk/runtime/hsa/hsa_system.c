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

hsa_status_t HSA_API hsa_system_get_info(
		hsa_system_info_t attribute, void *value)
{
	unsigned int args[3];
	args[1] = (unsigned int)attribute;
	args[2] = (unsigned int)value;

	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	else
	{
		ioctl(hsa_runtime->fd, SystemGetInfo, args);
		return (hsa_status_t)args[0];
	}
}


hsa_status_t HSA_API
    hsa_system_extension_supported(uint16_t extension, uint16_t version_major,
                                   uint16_t version_minor, bool *result)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API
    hsa_system_get_extension_table(uint16_t extension, uint16_t version_major,
                                   uint16_t version_minor, void *table)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}
