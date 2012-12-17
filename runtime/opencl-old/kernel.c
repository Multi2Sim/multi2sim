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
#include "program.h"


cl_kernel clCreateKernel(
	cl_program program,
	const char *kernel_name,
	cl_int *errcode_ret)
{
	unsigned int sys_args[3];
	sys_args[0] = program->id;
	sys_args[1] = (unsigned int) kernel_name;
	sys_args[2] = (unsigned int) errcode_ret;
	return (cl_kernel) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateKernel, sys_args);
}


cl_int clCreateKernelsInProgram(
	cl_program program,
	cl_uint num_kernels,
	cl_kernel *kernels,
	cl_uint *num_kernels_ret)
{
	unsigned int sys_args[4];
	sys_args[0] = program->id;
	sys_args[1] = (unsigned int) num_kernels;
	sys_args[2] = (unsigned int) kernels;
	sys_args[3] = (unsigned int) num_kernels_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateKernelsInProgram, sys_args);
}


cl_int clRetainKernel(
	cl_kernel kernel)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) kernel;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clRetainKernel, sys_args);
}


cl_int clReleaseKernel(
	cl_kernel kernel)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) kernel;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clReleaseKernel, sys_args);
}


cl_int clSetKernelArg(
	cl_kernel kernel,
	cl_uint arg_index,
	size_t arg_size,
	const void *arg_value)
{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) kernel;
	sys_args[1] = (unsigned int) arg_index;
	sys_args[2] = (unsigned int) arg_size;
	sys_args[3] = (unsigned int) arg_value;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clSetKernelArg, sys_args);
}


cl_int clGetKernelInfo(
	cl_kernel kernel,
	cl_kernel_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) kernel;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetKernelInfo, sys_args);
}


cl_int clGetKernelWorkGroupInfo(
	cl_kernel kernel,
	cl_device_id device,
	cl_kernel_work_group_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) kernel;
	sys_args[1] = (unsigned int) device;
	sys_args[2] = (unsigned int) param_name;
	sys_args[3] = (unsigned int) param_value_size;
	sys_args[4] = (unsigned int) param_value;
	sys_args[5] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetKernelWorkGroupInfo, sys_args);
}

