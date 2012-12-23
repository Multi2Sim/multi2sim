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

#include "clrt.h"
#include "context.h"
#include "debug.h"
#include "mhandle.h"
#include "platform.h"
#include "program.h"



/*
 * Private Functions
 */


void clrt_program_free(void *data)
{
	struct _cl_program *program;

	program = (struct _cl_program *) data;
	int i;
	for (i = 0; i < program->num_entries; i++)
	{
		dlclose(program->entries[i].handle);
		unlink(program->entries[i].filename);
		free(program->entries[i].filename);
	}
	free(program);
}




/*
 * Public Functions
 */


struct opencl_program_t *opencl_program_create(void)
{
	struct opencl_program_t *program;

	/* Initialize */
	program = xcalloc(1, sizeof(struct opencl_program_t));

	/* Return */
	return program;
}


void opencl_program_free(struct opencl_program_t *program)
{
	free(program);
}




/*
 * OpenCL API Functions
 */

cl_program clCreateProgramWithSource(
	cl_context context,
	cl_uint count,
	const char **strings,
	const size_t *lengths,
	cl_int *errcode_ret)
{
	__OPENCL_NOT_IMPL__
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
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcontext = %p", context);
	opencl_debug("\tnum_devices = %u", num_devices);
	opencl_debug("\tdevice_list = %p", device_list);
	opencl_debug("\tlengths = %p", lengths);
	opencl_debug("\tbinaries = %p", binaries);
	opencl_debug("\tbinary_status = %p", binary_status);
	opencl_debug("\terrcode_ret = %p", errcode_ret);

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

	/* allocate enough room for all device types.  We don't know how many of them there will be */
	struct clrt_device_type_t **device_types = xcalloc(opencl_platform->num_device_types, sizeof device_types[0]);
	int num_device_types = 0;

	for (i = 0; i < num_devices; i++)
	{
		int j;

		/* make sure that the device is in our context */
		int found = 0;
		for (j = 0; j < context->num_devices; j++)
			if (context->devices[j] == device_list[i])
			{
				found = 1;
				break;
			}
		
		if (!found)
		{
			if (errcode_ret)
				*errcode_ret = CL_INVALID_DEVICE;
			return NULL;
		}

		/* make sure the type of the binary matches */
		if (!lengths[i] || !binaries[i] || !device_list[i]->device_type->valid_binary(lengths[i], binaries[i]))
		{
			if (binary_status[i])
				binary_status[i] = CL_INVALID_VALUE;
			if (errcode_ret)
				*errcode_ret = CL_INVALID_VALUE;
			
			return NULL;
		}

		found = 0;
		for (j = 0; j < num_device_types; j++)
			if (device_types[j] == device_list[i]->device_type)
			{
				found = 1;
				break;
			}

		if (!found)
			device_types[num_device_types++] = device_list[i]->device_type;
	}

	

	program = xmalloc(sizeof (struct _cl_program));
	if (!program)
		fatal("%s: out of memory", __FUNCTION__);

	clrt_object_create(program, CLRT_OBJECT_PROGRAM, clrt_program_free);

	program->num_entries = num_devices;
	program->entries = xmalloc(sizeof program->entries[0] * num_devices);

	for (i = 0; i < num_device_types; i++)
	{
		uint32_t inner_size;
		void *inner_start;
		struct clrt_device_program_t *cur = program->entries + i;

		inner_start = get_inner_elf_addr(binaries[i], &inner_size);
		
		if ((char *)inner_start + inner_size > (char *)binaries[i] + lengths[i] || (unsigned char *)inner_start < binaries[i])
			fatal("%s: could not executable content", __FUNCTION__);

		cur->filename = xstrdup("./XXXXXX.so");
		if (!cur->filename)
			fatal("%s: out of memory", __FUNCTION__);

		int handle = mkstemps(cur->filename, 3);
		if (handle == -1)
			fatal("%s: could not create temporary file", __FUNCTION__);

		if (write(handle, inner_start, inner_size) != inner_size)
			fatal("%s: could not write to temporary file", __FUNCTION__);

		close(handle);

		
		cur->handle = dlopen(cur->filename, RTLD_NOW);
		if (!cur->handle)
			fatal("%s: could not open ELF binary derived from program", __FUNCTION__);		

		if (errcode_ret)
			*errcode_ret = CL_SUCCESS;

		if (binary_status)
			binary_status[0] = CL_SUCCESS;

		cur->device_type = device_types[i];
	}
	free(device_types);

	return program;
}


cl_int clRetainProgram(
	cl_program program)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tprogram = %p", program);
	
	return clrt_object_retain(program, CLRT_OBJECT_PROGRAM, CL_INVALID_PROGRAM);
}


cl_int clReleaseProgram(
	cl_program program)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tprogram = %p", program);

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
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tprogram = %p", program);
	opencl_debug("\tnum_devices = %u", num_devices);
	opencl_debug("\tdevice_list = %p", device_list);
	opencl_debug("\toptions = %p", options);
	opencl_debug("\tcall back = %p", pfn_notify);
	opencl_debug("\tuser_data = %p", user_data);

	/* We only support loading binaries, so this shouldn't ever fail. */
	return CL_SUCCESS;
}


cl_int clUnloadCompiler()
{
	__OPENCL_NOT_IMPL__
	return 0;
}


cl_int clGetProgramInfo(
	cl_program program,
	cl_program_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__OPENCL_NOT_IMPL__
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
	__OPENCL_NOT_IMPL__
	return 0;
}

