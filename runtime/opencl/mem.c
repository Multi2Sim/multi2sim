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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "debug.h"
#include "mem.h"
#include "mhandle.h"
#include "object.h"


/* Memory alignment for buffers */
#define MEMORY_ALIGN 16



/*
 * Public Functions
 */

struct opencl_mem_t *opencl_mem_create(void)
{
	struct opencl_mem_t *mem;

	/* Initialize */
	mem = xcalloc(1, sizeof(struct opencl_mem_t));
	
	/* Register OpenCL object */
	opencl_object_create(mem, OPENCL_OBJECT_MEM,
		(opencl_object_free_func_t) opencl_mem_free);

	/* Return */
	return mem;
}


void opencl_mem_free(struct opencl_mem_t *mem)
{
	/* Free buffer if initialized */
	if (mem->buffer)
		free(mem->buffer);
	
	/* Free memory object */
	free(mem);
}


void *opencl_mem_get_buffer(struct opencl_mem_t *mem)
{
	/* Check memory object */
	if (!opencl_object_verify(mem, OPENCL_OBJECT_MEM))
		return NULL;

	/* Return buffer */
	return mem->buffer;
}




/*
 * OpenCL API Functions
 */

cl_mem clCreateBuffer(
	cl_context context,
	cl_mem_flags flags,
	size_t size,
	void *host_ptr,
	cl_int *errcode_ret)
{
	struct opencl_mem_t *mem;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcontext = %p", context);
	opencl_debug("\tflags = 0x%x", (int) flags);
	opencl_debug("\tsize = %u", size);
	opencl_debug("\thost_ptr = %p", host_ptr);
	opencl_debug("\terrcode_ret = %p", errcode_ret);

	/* Check context */
	if (!opencl_object_verify(context, OPENCL_OBJECT_CONTEXT))
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_CONTEXT;
		return NULL;
	}

	/* Check combination of arguments */
	if (((flags & CL_MEM_USE_HOST_PTR) ||
		(flags & CL_MEM_COPY_HOST_PTR)) && !host_ptr)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_HOST_PTR;
		return NULL;
	}

	/* Create memory object */
	mem = opencl_mem_create();

	/* Because of alignment reasons, we are going to 'cache' buffers even
	 * when the user specifies CL_MEM_USE_HOST_PTR */
	mem->size = size;
	if (posix_memalign(&mem->buffer, MEMORY_ALIGN, size))
		fatal("%s: out of memory", __FUNCTION__);
	mhandle_register_ptr(mem->buffer, mem->size);

	/* Copy buffer contents */
	if ((flags & CL_MEM_USE_HOST_PTR) || (flags & CL_MEM_COPY_HOST_PTR))
		memcpy(mem->buffer, host_ptr, size);

	/* Success */
	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;
		
	/* Return memory object */
	return mem;
}


cl_mem clCreateSubBuffer(
	cl_mem buffer,
	cl_mem_flags flags,
	cl_buffer_create_type buffer_create_type,
	const void *buffer_create_info,
	cl_int *errcode_ret)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_mem clCreateImage2D(
	cl_context context,
	cl_mem_flags flags,
	const cl_image_format *image_format,
	size_t image_width,
	size_t image_height,
	size_t image_row_pitch,
	void *host_ptr,
	cl_int *errcode_ret)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_mem clCreateImage3D(
	cl_context context,
	cl_mem_flags flags,
	const cl_image_format *image_format,
	size_t image_width,
	size_t image_height,
	size_t image_depth,
	size_t image_row_pitch,
	size_t image_slice_pitch,
	void *host_ptr,
	cl_int *errcode_ret)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clRetainMemObject(
	cl_mem memobj)
{
	return opencl_object_retain(memobj, OPENCL_OBJECT_MEM, CL_INVALID_MEM_OBJECT);
}


cl_int clReleaseMemObject(
	cl_mem memobj)
{
	return opencl_object_release(memobj, OPENCL_OBJECT_MEM, CL_INVALID_MEM_OBJECT);
}


cl_int clGetSupportedImageFormats(
	cl_context context,
	cl_mem_flags flags,
	cl_mem_object_type image_type,
	cl_uint num_entries,
	cl_image_format *image_formats,
	cl_uint *num_image_formats)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clGetMemObjectInfo(
	cl_mem memobj,
	cl_mem_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clGetImageInfo(
	cl_mem image,
	cl_image_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clSetMemObjectDestructorCallback(
	cl_mem memobj,
	void (*pfn_notify)(cl_mem memobj , void *user_data),
	void *user_data)
{
	__OPENCL_NOT_IMPL__
	return 0;
}

