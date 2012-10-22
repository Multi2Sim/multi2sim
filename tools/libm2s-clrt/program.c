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
#include <sys/mman.h>
#include <string.h>
#include <dlfcn.h>

#include <m2s-clrt.h>





/*
 * Private Functions
 */

void clrt_program_free(void *data)
{
	struct _cl_program *program;

	program = (struct _cl_program *) data;
	dlclose(program->handle);
	unlink(program->filename);
	free(program->filename);
	free(program);
}





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
	int i;
	struct _cl_program *program;

	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tcontext = %p", context);
	m2s_clrt_debug("\tnum_devices = %u", num_devices);
	m2s_clrt_debug("\tdevice_list = %p", device_list);
	m2s_clrt_debug("\tlengths = %p", lengths);
	m2s_clrt_debug("\tbinaries = %p", binaries);
	m2s_clrt_debug("\tbinary_status = %p", binary_status);
	m2s_clrt_debug("\terrcode_ret = %p", errcode_ret);

	EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);

	if (!context)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_CONTEXT;
		return NULL;
	}

	if (!device_list || !num_devices || !lengths || !binaries)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_VALUE;
		return NULL;
	}

	/* Even though we don't support more than one device, we will in the future */
	for (i = 0; i < num_devices; i++)
	{
		if (!lengths[i] || !binaries[i])
		{
			if (errcode_ret)
				*errcode_ret = CL_INVALID_VALUE;
			if (binary_status)
				binary_status[i] = CL_INVALID_VALUE;

			return NULL;
		}
		if (!device_list[i])
		{
			if (errcode_ret)
				*errcode_ret = CL_INVALID_DEVICE;
			return NULL;
		}
	}

	program = (struct _cl_program *) malloc(sizeof (struct _cl_program));
	if (!program)
		fatal("%s: out of memory", __FUNCTION__);

	clrt_object_create(program, CLRT_OBJECT_PROGRAM, clrt_program_free);

	uint32_t inner_size;
	void *inner_start;
	inner_start = get_inner_elf_addr(binaries[0], &inner_size);
	
	if ((char *)inner_start + inner_size > (char *)binaries[0] + lengths[0] || (unsigned char *)inner_start < binaries[0])
		fatal("%s: could not executable content", __FUNCTION__);

	const char *tempname = "./XXXXXX.so"; /* initialize buffer for mkstemp to create a file in the cwd */
	program->filename = malloc(strlen(tempname) + 1);
	if (!program->filename)
		fatal("%s: out of memory", __FUNCTION__);
	strcpy(program->filename, tempname);

	int handle = mkstemps(program->filename, 3);
	if (handle == -1)
		fatal("%s: could not create temporary file", __FUNCTION__);

	if (write(handle, inner_start, inner_size) != inner_size)
		fatal("%s: could not write to temporary file", __FUNCTION__);

	close(handle);

	program->handle = dlopen(program->filename, RTLD_NOW);
	if (!program->handle)
		fatal("%s: could not open ELF binary derived from program", __FUNCTION__);		

	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;

	if (binary_status)
		binary_status[0] = CL_SUCCESS;

	return program;
}


cl_int clRetainProgram(
	cl_program program)
{
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tprogram = %p", program);
	
	return clrt_object_retain(program, CLRT_OBJECT_PROGRAM, CL_INVALID_PROGRAM);
}


cl_int clReleaseProgram(
	cl_program program)
{
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tprogram = %p", program);

	return clrt_object_release(program, CLRT_OBJECT_PROGRAM, CL_INVALID_PROGRAM);
}


cl_int clBuildProgram(
	cl_program program,
	cl_uint num_devices,
	const cl_device_id *device_list,
	const char *options,
	void (*pfn_notify)(cl_program program , void *user_data),
	void *user_data)
{
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tprogram = %p", program);
	m2s_clrt_debug("\tnum_devices = %u", num_devices);
	m2s_clrt_debug("\tdevice_list = %p", device_list);
	m2s_clrt_debug("\toptions = %p", options);
	m2s_clrt_debug("\tcall back = %p", pfn_notify);
	m2s_clrt_debug("\tuser_data = %p", user_data);

	/* We only support loading binaries, so this shouldn't ever fail. */
	return CL_SUCCESS;
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

