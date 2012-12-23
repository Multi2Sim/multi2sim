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

#ifndef RUNTIME_OPENCL_PLATFORM_H
#define RUNTIME_OPENCL_PLATFORM_H


/* Platform type. For name compatibility between OpenCL and Multi2Sim naming
 * conventions, a pseudo-type is created with a macro. */
#define opencl_platform_t _cl_platform_id
struct _cl_platform_id
{
	int num_device_types;
	struct clrt_device_type_entry_t *entries;
};



/* Global OpenCL platform */
extern struct opencl_platform_t *opencl_platform;


/* Create/free */
struct opencl_platform_t *opencl_platform_create(void);
void opencl_platform_free(struct opencl_platform_t *platform);


#endif

