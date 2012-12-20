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

#include "clrt.h"
#include "debug.h"
#include "mhandle.h"



#define MEMORY_ALIGN 16 /* memory alignment for buffers */

/*
 * Private Functions
 */

void *clrt_buffer_allocate(size_t size)
{
	void *ptr;
	if (posix_memalign(&ptr, MEMORY_ALIGN, size) != 0)
		return NULL;
	return ptr;
}


void clrt_buffer_free(void *buffer)
{
	free(buffer);
}


void clrt_mem_free(void *data)
{
	struct _cl_mem *mem = (struct _cl_mem *) data;

	clrt_buffer_free(mem->buffer);
	free(mem);
}

void *clrt_get_address_of_buffer_object(cl_mem buffer)
{
	if (!clrt_object_verify(buffer, CLRT_OBJECT_MEM))
		return NULL;
	else
		return buffer->buffer;
}


/*
 * Public Functions
 */

cl_mem clCreateBuffer(
	cl_context context,
	cl_mem_flags flags,
	size_t size,
	void *host_ptr,
	cl_int *errcode_ret)
{
	struct _cl_mem  *mem;

	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tcontext = %p", context);
	m2s_clrt_debug("\tflags = 0x%x", (int) flags);
	m2s_clrt_debug("\tsize = %u", size);
	m2s_clrt_debug("\thost_ptr = %p", host_ptr);
	m2s_clrt_debug("\terrcode_ret = %p", errcode_ret);

	if (!clrt_object_verify(context, CLRT_OBJECT_CONTEXT))
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_CONTEXT;
		return NULL;
	}

	if (((flags & CL_MEM_USE_HOST_PTR) || (flags & CL_MEM_COPY_HOST_PTR)) && !host_ptr)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_HOST_PTR;
		return NULL;
	}

	mem = xmalloc(sizeof (struct _cl_mem));
	clrt_object_create(mem, CLRT_OBJECT_MEM, clrt_mem_free);

	/* Because of alignment reasons, we are going to 'cache' buffers even when the user
         * specifies CL_MEM_UES_HOST_PTR */

	mem->buffer = clrt_buffer_allocate(size);
	mem->size = size;

	if ((flags & CL_MEM_USE_HOST_PTR) || (flags & CL_MEM_COPY_HOST_PTR))
		memcpy(mem->buffer, host_ptr, size);

	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;
		
	return mem;
}


cl_mem clCreateSubBuffer(
	cl_mem buffer,
	cl_mem_flags flags,
	cl_buffer_create_type buffer_create_type,
	const void *buffer_create_info,
	cl_int *errcode_ret)
{
	__M2S_CLRT_NOT_IMPL__
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
	__M2S_CLRT_NOT_IMPL__
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clRetainMemObject(
	cl_mem memobj)
{
	return clrt_object_retain(memobj, CLRT_OBJECT_MEM, CL_INVALID_MEM_OBJECT);
}


cl_int clReleaseMemObject(
	cl_mem memobj)
{
	return clrt_object_release(memobj, CLRT_OBJECT_MEM, CL_INVALID_MEM_OBJECT);
}


cl_int clGetSupportedImageFormats(
	cl_context context,
	cl_mem_flags flags,
	cl_mem_object_type image_type,
	cl_uint num_entries,
	cl_image_format *image_formats,
	cl_uint *num_image_formats)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clGetMemObjectInfo(
	cl_mem memobj,
	cl_mem_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clGetImageInfo(
	cl_mem image,
	cl_image_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clSetMemObjectDestructorCallback(
	cl_mem memobj,
	void (*pfn_notify)(cl_mem memobj , void *user_data),
	void *user_data)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


