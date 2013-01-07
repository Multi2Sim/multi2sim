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
 * Public Functions
 */


struct opencl_program_t *opencl_program_create(void)
{
	struct opencl_program_t *program;

	/* Initialize */
	program = xcalloc(1, sizeof(struct opencl_program_t));
	program->entry_list = list_create();

	/* Register OpenCL object */
	opencl_object_create(program, OPENCL_OBJECT_PROGRAM,
		(opencl_object_free_func_t) opencl_program_free);

	/* Return */
	return program;
}


void opencl_program_free(struct opencl_program_t *program)
{
	struct opencl_program_entry_t *entry;
	int index;

	/* Free program entries (per-device-type info) */
	LIST_FOR_EACH(program->entry_list, index)
	{
		entry = list_get(program->entry_list, index);
		dlclose(entry->handle);
		unlink(entry->file_name);
		free(entry->file_name);
	}

	/* Free program */
	list_free(program->entry_list);
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
	struct opencl_program_t *program;
	
	int i;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcontext = %p", context);
	opencl_debug("\tnum_devices = %u", num_devices);
	opencl_debug("\tdevice_list = %p", device_list);
	opencl_debug("\tlengths = %p", lengths);
	opencl_debug("\tbinaries = %p", binaries);
	opencl_debug("\tbinary_status = %p", binary_status);
	opencl_debug("\terrcode_ret = %p", errcode_ret);

	/* Supported arguments */
	OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);

	/* Check valid context */
	if (!context)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_CONTEXT;
		return NULL;
	}

	/* Check valid arguments */
	if (!device_list || !num_devices || !lengths || !binaries)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_VALUE;
		return NULL;
	}

	/* Create program */
	program = opencl_program_create();

	/* Initialize program entries (per-device-type info) */
	for (i = 0; i < num_devices; i++)
	{
		struct opencl_device_t *device;
		struct opencl_program_entry_t *entry;

		struct elf_file_t *elf_file;
		struct elf_section_t *section;

		int f;
		int found;
		int section_index;

		/* Get current device */
		device = device_list[i];

		/* Device must be in context */
		if (!opencl_context_has_device(context, device))
		{
			if (errcode_ret)
				*errcode_ret = CL_INVALID_DEVICE;
			opencl_program_free(program);
			return NULL;
		}

		/* Make sure the type of the binary matches */
		if (!lengths[i] || !binaries[i] || !device->device_type->valid_binary(lengths[i], binaries[i]))
		{
			if (binary_status[i])
				binary_status[i] = CL_INVALID_VALUE;
			if (errcode_ret)
				*errcode_ret = CL_INVALID_VALUE;
			opencl_program_free(program);
			return NULL;
		}

		/* If the device type if already present, skip */
		if (list_index_of(program->entry_list, device->device_type) >= 0)
			continue;

		/* Add a new program entry */
		entry = xcalloc(1, sizeof(struct opencl_program_entry_t));
		entry->device_type = device->device_type;
		list_add(program->entry_list, entry);

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
		entry->file_name = xstrdup("./XXXXXX.so");
		f = mkstemps(entry->file_name, 3);
		if (f == -1)
			fatal("%s: could not create temporary file", __FUNCTION__);
		if (write(f, section->buffer.ptr, section->buffer.size) != section->buffer.size)
			fatal("%s: could not write to temporary file", __FUNCTION__);
		close(f);

		/* Close ELF external binary */
		elf_file_free(elf_file);

		/* Load internal binary for dynamic linking */
		entry->handle = dlopen(entry->file_name, RTLD_NOW);
		if (!entry->handle)
			fatal("%s: could not open ELF binary derived from program", __FUNCTION__);		

		/* Success */
		if (errcode_ret)
			*errcode_ret = CL_SUCCESS;
		if (binary_status)
			binary_status[0] = CL_SUCCESS;
	}

	/* Return program */
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

