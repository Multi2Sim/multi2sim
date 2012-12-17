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

#include "../include/CL/cl.h"
#include "api.h"
#include "context.h"


cl_mem clCreateBuffer(
	cl_context context,
	cl_mem_flags flags,
	size_t size,
	void *host_ptr,
	cl_int *errcode_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = context->id;
	sys_args[1] = (unsigned int) flags;
	sys_args[2] = (unsigned int) size;
	sys_args[3] = (unsigned int) host_ptr;
	sys_args[4] = (unsigned int) errcode_ret;
	return (cl_mem) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateBuffer, sys_args);
}


cl_mem clCreateSubBuffer(
	cl_mem buffer,
	cl_mem_flags flags,
	cl_buffer_create_type buffer_create_type,
	const void *buffer_create_info,
	cl_int *errcode_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) buffer;
	sys_args[1] = (unsigned int) flags;
	sys_args[2] = (unsigned int) buffer_create_type;
	sys_args[3] = (unsigned int) buffer_create_info;
	sys_args[4] = (unsigned int) errcode_ret;
	return (cl_mem) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateSubBuffer, sys_args);
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
	unsigned int sys_args[8];
	sys_args[0] = context->id;
	sys_args[1] = (unsigned int) flags;
	sys_args[2] = (unsigned int) image_format;
	sys_args[3] = (unsigned int) image_width;
	sys_args[4] = (unsigned int) image_height;
	sys_args[5] = (unsigned int) image_row_pitch;
	sys_args[6] = (unsigned int) host_ptr;
	sys_args[7] = (unsigned int) errcode_ret;
	return (cl_mem) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateImage2D, sys_args);
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
	unsigned int sys_args[10];
	sys_args[0] = context->id;
	sys_args[1] = (unsigned int) flags;
	sys_args[2] = (unsigned int) image_format;
	sys_args[3] = (unsigned int) image_width;
	sys_args[4] = (unsigned int) image_height;
	sys_args[5] = (unsigned int) image_depth;
	sys_args[6] = (unsigned int) image_row_pitch;
	sys_args[7] = (unsigned int) image_slice_pitch;
	sys_args[8] = (unsigned int) host_ptr;
	sys_args[9] = (unsigned int) errcode_ret;
	return (cl_mem) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateImage3D, sys_args);
}


cl_int clRetainMemObject(
	cl_mem memobj)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) memobj;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clRetainMemObject, sys_args);
}


cl_int clReleaseMemObject(
	cl_mem memobj)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) memobj;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clReleaseMemObject, sys_args);
}


cl_int clGetSupportedImageFormats(
	cl_context context,
	cl_mem_flags flags,
	cl_mem_object_type image_type,
	cl_uint num_entries,
	cl_image_format *image_formats,
	cl_uint *num_image_formats)
{
	unsigned int sys_args[6];
	sys_args[0] = context->id;
	sys_args[1] = (unsigned int) flags;
	sys_args[2] = (unsigned int) image_type;
	sys_args[3] = (unsigned int) num_entries;
	sys_args[4] = (unsigned int) image_formats;
	sys_args[5] = (unsigned int) num_image_formats;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetSupportedImageFormats, sys_args);
}


cl_int clGetMemObjectInfo(
	cl_mem memobj,
	cl_mem_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) memobj;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetMemObjectInfo, sys_args);
}


cl_int clGetImageInfo(
	cl_mem image,
	cl_image_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) image;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetImageInfo, sys_args);
}


cl_int clSetMemObjectDestructorCallback(
	cl_mem memobj,
	void (*pfn_notify)(cl_mem memobj , void *user_data),
	void *user_data)
{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) memobj;
	sys_args[1] = (unsigned int) pfn_notify;
	sys_args[2] = (unsigned int) user_data;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clSetMemObjectDestructorCallback, sys_args);
}


