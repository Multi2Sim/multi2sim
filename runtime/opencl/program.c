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

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <dlfcn.h>

#include "context.h"
#include "debug.h"
#include "device.h"
#include "elf-format.h"
#include "misc.h"
#include "list.h"
#include "mhandle.h"
#include "object.h"
#include "platform.h"
#include "program.h"
#include "string.h"



/*
 * Public Functions
 */


struct opencl_program_t *opencl_program_create(struct opencl_context_t *context)
{
	struct opencl_program_t *program;

	/* Initialize */
	program = xcalloc(1, sizeof(struct opencl_program_t));
	program->context = context;
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
	struct opencl_device_t *device;
	int index;

	/* Free program entries (per-device-type info) */
	LIST_FOR_EACH(program->entry_list, index)
	{
		/* Get entry */
		entry = list_get(program->entry_list, index);
		device = entry->device;

		/* Free architecture-specific program and entry*/
		assert(device->arch_program_free_func);
		device->arch_program_free_func(entry->arch_program);
		free(entry);
	}

	/* Free source if set */
	if (program->source)
		free(program->source);

	/* Free program */
	list_free(program->entry_list);
	free(program);
}


int opencl_program_has_device(struct opencl_program_t *program,
		struct opencl_device_t *device)
{
	struct opencl_program_entry_t *entry;
	int index;

	/* Find device in program entries */
	LIST_FOR_EACH(program->entry_list, index)
	{
		entry = list_get(program->entry_list, index);
		if (entry->device == device)
			return 1;
	}

	/* Device not found */
	return 0;
}


struct opencl_program_entry_t *opencl_program_add(struct opencl_program_t *program,
		struct opencl_device_t *device, void *arch_program)
{
	struct opencl_program_entry_t *entry;

	/* Initialize new entry */
	entry = xcalloc(1, sizeof(struct opencl_program_entry_t));
	entry->device = device;
	entry->arch_program = arch_program;

	/* Add entry to the list and return */
	list_add(program->entry_list, entry);
	return entry;
}


void opencl_program_set_source(struct opencl_program_t *program,
	unsigned int count, const char **strings, const size_t *lengths)
{
	char string[MAX_LONG_STRING_SIZE];
	char source[MAX_LONG_STRING_SIZE];
	char *source_ptr;

	int length;
	int source_size;
	int i;
	
	/* Reconstruct source in one single null-terminated string */
	source[0] = '\0';
	source_ptr = source;
	source_size = sizeof source;
	for (i = 0; i < count; i++)
	{
		/* Reconstruct null-terminated string */
		length = lengths && lengths[i] ? lengths[i] : strlen(strings[i]);
		if (length >= sizeof string)
			fatal("%s: buffer too small", __FUNCTION__);
		memcpy(string, strings[i], length);
		string[length] = '\0';

		/* Add string to source */
		str_printf(&source_ptr, &source_size, "%s\n", string);
		if (!source_size)
			fatal("%s: buffer too small", __FUNCTION__);
	}

	/* Set in program */
	assert(!program->source);
	program->source = xstrdup(source);
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
	struct opencl_program_t *program;
	int i;
	
	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tcontext = %p", context);
	opencl_debug("\tcount = %u", count);
	opencl_debug("\tstrings = %p", strings);
	opencl_debug("\tlengths = %p", lengths);
	opencl_debug("\terrcode_ret = %p", errcode_ret);

	/* Check valid context */
	if (!context)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_CONTEXT;
		return NULL;
	}

	/* Check valid 'count' and 'strings' */
	if (!count || !strings)
	{
		if (errcode_ret)
			*errcode_ret = CL_INVALID_VALUE;
		return NULL;
	}

	/* Check valid elements in 'strings' */
	for (i = 0; i < count; i++)
	{
		if (!strings[i])
		{
			if (errcode_ret)
				*errcode_ret = CL_INVALID_VALUE;
			return NULL;
		}
	}

	/* Create program */
	program = opencl_program_create(context);
	opencl_program_set_source(program, count, strings, lengths);

	/* Success */
	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;
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
	program = opencl_program_create(context);

	/* Initialize program entries (per-device-type info) */
	for (i = 0; i < num_devices; i++)
	{
		struct opencl_device_t *device;
		void *arch_program;  /* Of type 'opencl_xxx_program_t' */

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
		assert(device->arch_program_valid_binary_func);
		if (!lengths[i] || !binaries[i] ||
			!device->arch_program_valid_binary_func(
			device->arch_device, (void *) binaries[i], lengths[i]))
		{
			if (binary_status)
			{
				binary_status[i] = CL_INVALID_VALUE;
			}
			if (errcode_ret)
				*errcode_ret = CL_INVALID_VALUE;
			opencl_program_free(program);
			return NULL;
		}

		/* If the device is already present, skip */
		if (opencl_program_has_device(program, device))
			continue;

		/* Create the architecture-specific program object. */
		assert(device->arch_program_create_func);
		arch_program = device->arch_program_create_func(program,
			device->arch_device, (void *) binaries[i], lengths[i]);

		/* Add pair of architecture-specific program and device to the
		 * generic program object. */
		opencl_program_add(program, device, arch_program);

		/* Success */
		if (errcode_ret)
			*errcode_ret = CL_SUCCESS;
		if (binary_status)
			binary_status[0] = CL_SUCCESS;
	}

	/* Return program */
	if (errcode_ret)
		*errcode_ret = CL_SUCCESS;
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


static char *opencl_err_program_compile =
	"\tYour guest OpenCL application is trying to compile an OpenCL kernel\n"
	"\tby calling function 'clCreateProgramWithSource'. Dynamic kernel\n"
	"\tcompilation is currently not supported. The following two\n"
	"\talternatives are suggested instead:\n"
	"\n"
	"\t1) If you are running a benchmark from the AMD APPSDK suite, you can\n"
	"\t   add option '--load <file>' to the benchmark command line. This\n"
	"\t   option is supported by all benchmarks in the suite, and it allows\n"
	"\t   you to specify a precompiled kernel binary that will be loaded by\n"
	"\t   the application using 'clCreateProgramWithBinary'. Notice that\n"
	"\t   this is an option for the benchmark, and not for the simulator.\n"
	"\n"
	"\t2) You can force the Multi2Sim runtime to load a specific kernel\n"
	"\t   binary every time the application performs a call to\n"
	"\t   'clCreateProgramWithSource' by setting environment varaible\n"
	"\t   'M2S_OPENCL_BINARY' to the path of a pre-compiled binary that\n"
	"\t   will be passed to the application.\n";

static char *opencl_err_program_not_found =
	"\tYour guest OpenCL application has executed function\n"
	"\t'clCreateProgramWithSource', while the Multi2Sim runtime found that\n"
	"\tenvironment variable 'M2S_OPENCL_BINARY' was set. However, the\n"
	"\tbinary file pointed to by this variable was not found.\n";

static char *opencl_err_program_invalid =
	"\tThe file specified in environment variable 'M2S_OPENCL_BINARY' is\n"
	"\tnot a valid OpenCL kernel binary or does not contain ISA code for\n"
	"\tthe intended architecture.\n";

static char *opencl_note_program_binary =
	"\tYour guest OpenCL application has successfully loaded the program\n"
	"\tbinary passed through environment variable 'M2S_OPENCL_BINARY'\n"
	"\tafter a call to 'clCreateProgramWithSource'. While this works,\n"
	"\ta safer option is loading directly from the application with a\n"
	"\tcall to 'clCreateProgramWithBinary'.\n";

cl_int clBuildProgram(
	cl_program program,
	cl_uint num_devices,
	const cl_device_id *device_list,
	const char *options,
	void (*pfn_notify)(cl_program program , void *user_data),
	void *user_data)
{
	char *binary_name;
	void *binary;
	unsigned int size;
	FILE *f;

	int ret;
	int i;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tprogram = %p", program);
	opencl_debug("\tnum_devices = %u", num_devices);
	opencl_debug("\tdevice_list = %p", device_list);
	opencl_debug("\toptions = %p", options);
	opencl_debug("\tcall back = %p", pfn_notify);
	opencl_debug("\tuser_data = %p", user_data);

	/* Supported arguments */
	OPENCL_ARG_NOT_SUPPORTED_NEQ(num_devices, 1);

	/* Check valid program */
	if (!program)
		return CL_INVALID_PROGRAM;

	/* If program was created with 'clCreateProgramWithBinary', there is no
	 * action needed to build it. */
	if (!program->source)
		return CL_SUCCESS;

	/* Runtime compilation is not supported, but the user can have activated
	 * environment variable 'M2S_OPENCL_BINARY', which will make the runtime
	 * load that specific pre-compiled binary. */
	binary_name = getenv("M2S_OPENCL_BINARY");
	if (!binary_name || !*binary_name)
		fatal("%s: runtime kernel compilation not supported.\n%s",
				__FUNCTION__, opencl_err_program_compile);

	/* Load binary */
	f = fopen(binary_name, "rb");
	if (!f)
		fatal("%s: %s: cannot open file.\n%s", __FUNCTION__,
			binary_name, opencl_err_program_not_found);
	
	/* Allocate buffer */
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	binary = xmalloc(size);

	/* Read binary */
	fseek(f, 0, SEEK_SET);
	fread(binary, size, 1, f);
	fclose(f);
	
	/* Initialize program entries (per-device-type info) */
	ret = CL_SUCCESS;
	for (i = 0; i < num_devices; i++)
	{
		struct opencl_device_t *device;
		void *arch_program;  /* Of type 'opencl_xxx_program_t' */

		/* Get current device */
		device = device_list[i];

		/* Device must be in context */
		if (!opencl_context_has_device(program->context, device))
		{
			ret = CL_INVALID_DEVICE;
			goto out;
		}

		/* Make sure the type of the binary matches */
		assert(device->arch_program_valid_binary_func);
		if (!device->arch_program_valid_binary_func(device->arch_device, binary, size))
			fatal("%s: invalid binary for device '%s'.\n%s",
				__FUNCTION__, device->name, opencl_err_program_invalid);

		/* If the device is already present, skip */
		if (opencl_program_has_device(program, device))
			continue;

		/* Create the architecture-specific program object. */
		assert(device->arch_program_create_func);
		arch_program = device->arch_program_create_func(program,
				device->arch_device, binary, size);

		/* Add pair of architecture-specific program and device to the
		 * generic program object. */
		opencl_program_add(program, device, arch_program);
	
		/* Show warning */
		warning("%s: %s: program binary loaded for device '%s'.\n%s",
			__FUNCTION__, binary_name, device->name,
			opencl_note_program_binary);
	}

out:
	free(binary);
	return ret;
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

