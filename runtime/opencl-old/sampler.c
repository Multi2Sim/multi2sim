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


cl_sampler clCreateSampler(
	cl_context context,
	cl_bool normalized_coords,
	cl_addressing_mode addressing_mode,
	cl_filter_mode filter_mode,
	cl_int *errcode_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = context->id;
	sys_args[1] = (unsigned int) normalized_coords;
	sys_args[2] = (unsigned int) addressing_mode;
	sys_args[3] = (unsigned int) filter_mode;
	sys_args[4] = (unsigned int) errcode_ret;
	return (cl_sampler) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateSampler, sys_args);
}


cl_int clRetainSampler(
	cl_sampler sampler)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) sampler;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clRetainSampler, sys_args);
}


cl_int clReleaseSampler(
	cl_sampler sampler)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) sampler;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clReleaseSampler, sys_args);
}


cl_int clGetSamplerInfo(
	cl_sampler sampler,
	cl_sampler_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) sampler;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetSamplerInfo, sys_args);
}

