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
#include "mhandle.h"


cl_context clCreateContext(
	const cl_context_properties *properties,
	cl_uint num_devices,
	const cl_device_id *devices,
	void (*pfn_notify)(const char *, const void *, size_t , void *),
	void *user_data,
	cl_int *errcode_ret)
{
	cl_context context;

	/* Create context */
	context = xcalloc(1, sizeof(struct _cl_context));

	/* System call */
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) properties;
	sys_args[1] = (unsigned int) num_devices;
	sys_args[2] = (unsigned int) devices;
	sys_args[3] = (unsigned int) pfn_notify;
	sys_args[4] = (unsigned int) user_data;
	sys_args[5] = (unsigned int) errcode_ret;
	context->id = (unsigned int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateContext, sys_args);

	/* Return */
	return context;
}


cl_context clCreateContextFromType(
	const cl_context_properties *properties,
	cl_device_type device_type,
	void (*pfn_notify)(const char *, const void *, size_t , void *),
	void *user_data,
	cl_int *errcode_ret)
{
	cl_context context;

	/* Create context */
	context = xcalloc(1, sizeof(struct _cl_context));

	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) properties;
	sys_args[1] = (unsigned int) device_type;
	sys_args[2] = (unsigned int) pfn_notify;
	sys_args[3] = (unsigned int) user_data;
	sys_args[4] = (unsigned int) errcode_ret;
	context->id = (unsigned int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateContextFromType, sys_args);

	/* Return */
	return context;
}


cl_int clRetainContext(
	cl_context context)
{
	unsigned int sys_args[1];
	sys_args[0] = context->id;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clRetainContext, sys_args);
}


cl_int clReleaseContext(
	cl_context context)
{
	unsigned int sys_args[1];

	/* FIXME: free context object */

	/* System call */
	sys_args[0] = context->id;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clReleaseContext, sys_args);
}


cl_int clGetContextInfo(
	cl_context context,
	cl_context_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = context->id;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetContextInfo, sys_args);
}


