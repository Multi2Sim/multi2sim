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

#include <m2s-clrt.h>


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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clRetainProgram(
	cl_program program)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clReleaseProgram(
	cl_program program)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clBuildProgram(
	cl_program program,
	cl_uint num_devices,
	const cl_device_id *device_list,
	const char *options,
	void (*pfn_notify)(cl_program program , void *user_data),
	void *user_data)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clUnloadCompiler()
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clGetProgramInfo(
	cl_program program,
	cl_program_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clGetProgramBuildInfo(
	cl_program program,
	cl_device_id device,
	cl_program_build_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}

