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
#include "debug.h"


cl_int clGetPlatformIDs(
	cl_uint num_entries,
	cl_platform_id *platforms,
	cl_uint *num_platforms)
{
	unsigned int sys_args[4];
	int ret;

	sys_args[0] = (unsigned int) num_entries;
	sys_args[1] = (unsigned int) platforms;
	sys_args[2] = (unsigned int) num_platforms;
	
	/* An additional argument is added with the version information of this
	 * OpenCL implementation. If Multi2Sim expects a later version, the
	 * system call with fail and cause a fatal error. */
	sys_args[3] = OPENCL_RUNTIME_VERSION;

	/* Perform system call */
	ret = syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetPlatformIDs, sys_args);

	/* Detect the case where an OpenCL program linked with 'libm2s-opencl' is
	 * being run natively. */
	if (ret == -1)
		fatal("OpenCL program cannot be run natively.\n"
			"\tThis is an error message provided by the Multi2Sim OpenCL library\n"
			"\t(libm2s-opencl). Apparently, you are attempting to run natively a\n"
			"\tprogram that was linked with this library. You should either run\n"
			"\tit on top of Multi2Sim, or link it with the OpenCL library provided\n"
			"\tin the ATI Stream SDK if you want to use your physical GPU device.\n");

	/* Result */
	return (cl_int) ret;
}


cl_int clGetPlatformInfo(
	cl_platform_id platform,
	cl_platform_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) platform;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetPlatformInfo, sys_args);
}

