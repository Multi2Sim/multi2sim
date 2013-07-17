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
#include "debug.h"
#include "mhandle.h"
#include "program.h"


/*
 * Public Functions
 */

cl_program clCreateProgramWithSource(
	cl_context context,
	cl_uint count,
	const char **strings,
	const size_t *lengths,
	cl_int *errcode_ret)
{
	cl_program program;
	unsigned int sys_args[5];

	/* Create program */
	program = xcalloc(1, sizeof(struct _cl_program));
	program->context = context;

	/* FIXME
	 * New system call that returns the binary provided through "--opencl-kernel"
	 * in the simulator command line */

	/* System call */
	sys_args[0] = context->id;
	sys_args[1] = (unsigned int) count;
	sys_args[2] = (unsigned int) strings;
	sys_args[3] = (unsigned int) lengths;
	sys_args[4] = (unsigned int) errcode_ret;
	program->id = (unsigned int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateProgramWithSource, sys_args);

	/* Return */
	return program;
}


cl_program clCreateProgramWithBinary(
	cl_context context,
	cl_uint num_devices,
	const cl_device_id *device_list,
	const size_t *lengths,
	const unsigned char **binaries,
	cl_int *binary_status,
	cl_int *errcode_ret)
{
	cl_program program;
	unsigned int sys_args[7];

	/* Check arguments */
	OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);

	/* Create program */
	program = xcalloc(1, sizeof(struct _cl_program));
	program->context = context;

	/* Read binary */
	//program->bin_file = evg_bin_file_create((void *) *binaries, *lengths, "opencl-bin");  /* FIXME: any better name? */

	/* System call */
	sys_args[0] = context->id;
	sys_args[1] = (unsigned int) num_devices;
	sys_args[2] = (unsigned int) device_list;
	sys_args[3] = (unsigned int) lengths;
	sys_args[4] = (unsigned int) binaries;
	sys_args[5] = (unsigned int) binary_status;
	sys_args[6] = (unsigned int) errcode_ret;
	program->id = (unsigned int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateProgramWithBinary, sys_args);

	/* Return */
	return program;
}


cl_int clRetainProgram(
	cl_program program)
{
	unsigned int sys_args[1];
	sys_args[0] = program->id;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clRetainProgram, sys_args);
}


cl_int clReleaseProgram(
	cl_program program)
{
	unsigned int sys_args[1];

	/* FIXME: free program object */

	/* System call */
	sys_args[0] = program->id;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clReleaseProgram, sys_args);
}


cl_int clBuildProgram(
	cl_program program,
	cl_uint num_devices,
	const cl_device_id *device_list,
	const char *options,
	void (*pfn_notify)(cl_program program , void *user_data),
	void *user_data)
{
	unsigned int sys_args[6];
	sys_args[0] = program->id;
	sys_args[1] = (unsigned int) num_devices;
	sys_args[2] = (unsigned int) device_list;
	sys_args[3] = (unsigned int) options;
	sys_args[4] = (unsigned int) pfn_notify;
	sys_args[5] = (unsigned int) user_data;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clBuildProgram, sys_args);
}


cl_int clUnloadCompiler()
{
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clUnloadCompiler);
}


cl_int clGetProgramInfo(
	cl_program program,
	cl_program_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = program->id;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetProgramInfo, sys_args);
}


cl_int clGetProgramBuildInfo(
	cl_program program,
	cl_device_id device,
	cl_program_build_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[6];
	sys_args[0] = program->id;
	sys_args[1] = (unsigned int) device;
	sys_args[2] = (unsigned int) param_name;
	sys_args[3] = (unsigned int) param_value_size;
	sys_args[4] = (unsigned int) param_value;
	sys_args[5] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetProgramBuildInfo, sys_args);
}

