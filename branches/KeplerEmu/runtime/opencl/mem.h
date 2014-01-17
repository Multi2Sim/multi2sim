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

#ifndef RUNTIME_OPENCL_MEM_H
#define RUNTIME_OPENCL_MEM_H

#include "opencl.h"

/* Memory object */
struct _cl_mem
{
	/* Pointer in device memory. Technically, there should be an array of
	 * devices and device pointers, but only one allowed in this
	 * implementation. */
	struct opencl_device_t *device;
	void *device_ptr;

	/* Pointer in host memory. If the buffer is created with flag
	 * CL_USE_HOST_PTR, the value of this field is obtained from the user.
	 * Otherwise, it is allocated/deallocated with MapBuffer/UnmapBuffer. */
	void *host_ptr;
	int use_host_ptr;  /* True if passed by user in clCreateBuffer */

	/* Used by MapBuffer */
	int mapped;  /* True if buffer is MapBuffer'ed and not UnmapBuffer'ed */
	int map_flags;  /* Flags used when mapping it (needed for unamp actions) */
	unsigned int map_offset;
	unsigned int map_size;

	/* Size with which buffer is created */
	unsigned int size;
};


/* Create/free */
struct opencl_mem_t *opencl_mem_create(void);
void opencl_mem_free(struct opencl_mem_t *mem);

void *opencl_mem_get_buffer(struct opencl_mem_t *mem);



#endif

