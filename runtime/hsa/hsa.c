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

hsa_status_t HSA_API hsa_init()
{
	if (hsa_runtime == NULL)
	{
		hsa_runtime = calloc(1, sizeof(struct hsa_runtime_t));
		hsa_runtime->fd = open("/dev/hsa", O_RDWR);
		if (hsa_runtime->fd < 0)
			fatal("Cannot communicate with the HSA driver\n\n"
				"This error could be due to an incompatibility between the\n"
				"Multi2Sim HSA driver version and the version of the simulator.\n"
				"Please download the latest versions and retry.");

		// Call the driver init hsa environment
		ioctl(hsa_runtime->fd, InitFromX86, getpid());

		// Return success
		return HSA_STATUS_SUCCESS;
	}
	else
	{
		// Success silently
		return HSA_STATUS_SUCCESS;
	}
}


hsa_status_t HSA_API hsa_shut_down()
{
	return HSA_STATUS_SUCCESS;
}








