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

#ifndef RUNTIME_OPENCL_KERNEL_H
#define RUNTIME_OPENCL_KERNEL_H

#include "clrt.h"


/* Kernel entries (one per device type) */
struct opencl_kernel_entry_t
{
	struct opencl_device_type_t *device_type;
	void *kernel;
};


/* Kernel object */
struct opencl_kernel_t
{
	/* Program associated with kernel */
	struct opencl_program_t *program;

	/* List of kernel entries, including per-device-type info. Each element
	 * is of type 'opencl_kernel_entry_t'. */
	struct list_t *entry_list;
};


/* Create/free */
struct opencl_kernel_t *opencl_kernel_create(void);
void opencl_kernel_free(struct opencl_kernel_t *kernel);


#endif

