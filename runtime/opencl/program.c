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
#include "elf-format.h"
#include "list.h"
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
	struct opencl_program_t *program;
	struct opencl_device_type_t **device_types;
	int num_device_types;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcontext = %p", context);
	opencl_debug("\tnum_devices = %u", num_devices);
	opencl_debug("\tdevice_list = %p", device_list);
	opencl_debug("\tlengths = %p", lengths);
	opencl_debug("\tbinaries = %p", binaries);
	opencl_debug("\tbinary_status = %p", binary_status);
	opencl_debug("\terrcode_ret = %p", errcode_ret);

	OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);

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

	/* Allocate enough room for all device types.
	 * We don't know how many of them there will be */
	device_types = xcalloc(opencl_platform->device_type_list->count, sizeof device_types[0]);
	num_device_types = 0;

	for (i = 0; i < num_devices; i++)
	{
		int j;
		int found;

		/* Device must be in context */
		if (!opencl_context_has_device(context, device_list[i]))
		{
			if (errcode_ret)
				*errcode_ret = CL_INVALID_DEVICE;
			return NULL;
		}

		/* Make sure the type of the binary matches */
		if (!lengths[i] || !binaries[i] || !device_list[i]->device_type->valid_binary(lengths[i], binaries[i]))
		{
			if (binary_status[i])
				binary_status[i] = CL_INVALID_VALUE;
			if (errcode_ret)
				*errcode_ret = CL_INVALID_VALUE;
			
			return NULL;
		}

		/* Add device type if not present already */
		found = 0;
		for (j = 0; j < num_device_types; j++)
		{
			if (device_types[j] == device_list[i]->device_type)
			{
				found = 1;
				break;
			}
		}
		if (!found)
			device_types[num_device_types++] = device_list[i]->device_type;
	}

	

	program = opencl_program_create();
	opencl_object_create(program, OPENCL_OBJECT_PROGRAM, clrt_program_free);
	program->num_entries = num_devices;
	program->entries = xmalloc(sizeof program->entries[0] * num_devices);
	for (i = 0; i < num_device_types; i++)
	{
		struct clrt_device_program_t *cur;
		struct elf_file_t *elf_file;
		struct elf_section_t *section;

		int section_index;
		int found;
		int f;

		cur = program->entries + i;

		/* Load ELF binary */
		elf_file = elf_file_create_from_buffer((void *) binaries[i], lengths[i],
			"OpenCL binary");

		/* Find '.text' section */
		found = 0;
		LIST_FOR_EACH(elf_file->section_list, section_index)
		{
			section = list_get(elf_file->section_list, section_index);
			if (!strcmp(section->name, ".text"))
			{
				found = 1;
				break;
			}
		}

		/* Section not found */
		if (!found)
			fatal("%s: no '.text' section in binary", __FUNCTION__);

		/* Extract section to temporary file */
		cur->filename = xstrdup("./XXXXXX.so");
		f = mkstemps(cur->filename, 3);
		if (f == -1)
			fatal("%s: could not create temporary file", __FUNCTION__);
		if (write(f, section->buffer.ptr, section->buffer.size) != section->buffer.size)
			fatal("%s: could not write to temporary file", __FUNCTION__);
		close(f);

		/* Close ELF external binary */
		elf_file_free(elf_file);

		/* Load internal binary for dynamic linking */
		cur->device_type = device_types[i];
		cur->handle = dlopen(cur->filename, RTLD_NOW);
		if (!cur->handle)
			fatal("%s: could not open ELF binary derived from program", __FUNCTION__);		

		/* Success */
		if (errcode_ret)
			*errcode_ret = CL_SUCCESS;
		if (binary_status)
			binary_status[0] = CL_SUCCESS;
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
	
	return opencl_object_retain(program, OPENCL_OBJECT_PROGRAM, CL_INVALID_PROGRAM);
}


cl_int clReleaseProgram(
	cl_program program)
{
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tprogram = %p", program);

	return opencl_object_release(program, OPENCL_OBJECT_PROGRAM, CL_INVALID_PROGRAM);
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

