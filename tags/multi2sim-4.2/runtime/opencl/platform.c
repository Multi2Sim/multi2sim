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
#include <string.h>

#include "debug.h"
#include "device.h"
#include "list.h"
#include "mhandle.h"
#include "platform.h"
#include "si-device.h"
#include "x86-device.h"
#include "union-device.h"

static char *opencl_err_version =
	"\tYour OpenCL program is using a version of the Multi2Sim Runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tMulti2Sim OpenCL Runtime library ('libm2s-opencl.so').\n"
	"\n"
	"\tIf you are using statically compiled benchmarks from a pre-compiled\n"
	"\tbenchmark suite, please download the latest compatible package from the\n"
	"\tMulti2Sim website, section 'Benchmarks'.\n";

static char *opencl_err_native =
	"\tYou are running on the real machine an OpenCL program linked with\n"
	"\tMulti2Sim's OpenCL runtime (libm2s-opencl.so). In native mode, the only\n"
	"\tdevice visible to the application is the x86 CPU. If you want to see\n"
	"\tadditional devices, please run your application on Multi2Sim. If you want\n"
	"\tto run your program using native libraries, please compile it with flag\n"
	"\t'-lOpenCL' instead of '-lm2s-opencl'.\n";

struct opencl_version_t
{
	int major;
	int minor;
};

/* Global OpenCL platform */
struct opencl_platform_t *opencl_platform;




/*
 * Private Functions
 */


void opencl_platform_for_each_device(
	struct opencl_platform_t *platform,
	opencl_platform_for_each_device_func_t for_each_device_func,
	void *user_data)
{
	struct opencl_device_t *device;
	int i;

	/* Execute function for each device */
	assert(for_each_device_func);
	LIST_FOR_EACH(platform->device_list, i)
	{
		device = list_get(platform->device_list, i);
		for_each_device_func(device, user_data);
	}
}



/*
 * Public Functions
 */

struct opencl_platform_t *opencl_platform_create(void)
{
	struct opencl_platform_t *platform;
	struct opencl_device_t *device;

	/* Initialize */
	platform = xcalloc(1, sizeof(struct opencl_platform_t));
	platform->full_profile = "OpenCL Multi2Sim Platform Full Profile";
	platform->version = "1.1";
	platform->name = "Multi2Sim OpenCL Platform";
	platform->vendor = "Multi2Sim";
	platform->extensions = "";

	/* Initialize device list */
	platform->device_list = list_create();

	/* Add x86 device */
	device = opencl_device_create();
	device->arch_device = opencl_x86_device_create(device);
	list_add(platform->device_list, device);

	/* Devices other than x86 CPU are added only on simulated mode */
	if (!opencl_native_mode)
	{
		/* Add Southern Islands device */
		device = opencl_device_create();
		device->arch_device = opencl_si_device_create(device);
		list_add(platform->device_list, device);
	}

	/* Add Union device */
	device = opencl_device_create();
	device->arch_device = opencl_union_device_create(device, platform->device_list);
	list_add(platform->device_list, device);

	/* Return */
	return platform;
}


void opencl_platform_free(struct opencl_platform_t *platform)
{
	struct opencl_device_t *device;
	int index;

	/* Free devices */
	LIST_FOR_EACH(platform->device_list, index)
	{
		device = list_get(platform->device_list, index);
		opencl_device_free(device);
	}

	/* Free platform */
	list_free(platform->device_list);
	free(platform);
}




/*
 * OpenCL API Functions
 */

/* Version for the OpenCL runtime.
 * NOTE: when modifying the values of these two macros, the same values should
 * be reflected in 'src/driver/opencl/opencl.c'. */
#define OPENCL_VERSION_MAJOR  3
#define OPENCL_VERSION_MINOR  1664

cl_int clGetPlatformIDs(
	cl_uint num_entries,
	cl_platform_id *platforms,
	cl_uint *num_platforms)
{
	struct opencl_version_t version;
	int ret;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tnum_entries = %d", num_entries);
	opencl_debug("\tplatforms = %p", platforms);
	opencl_debug("\tnum_platforms = %p", num_platforms);

	/* It can be assumed that this is the first OpenCL function called by
	 * the host program. It is checked here whether we're running in native
	 * or simulation mode. If it's simulation mode, Multi2Sim's version is
	 * checked for compatibility with the runtime library version. */
	ret = syscall(OPENCL_SYSCALL_CODE, opencl_abi_init, &version);

	/* If the system call returns error, we are in native mode. */
	if (ret == -1 && !opencl_native_mode)
	{
		opencl_native_mode = 1;
		warning("Multi2Sim OpenCL library running on native mode.\n%s",
				opencl_err_native);
	}

	/* On simulation mode, check Multi2sim version and Multi2Sim OpenCL
	 * Runtime version compatibility. */
	if (!opencl_native_mode)
	{
		if (version.major != OPENCL_VERSION_MAJOR
				|| version.minor < OPENCL_VERSION_MINOR)
			fatal("incompatible Multi2Sim Runtime version.\n"
				"\tRuntime library v. %d.%d / "
				"Host implementation v. %d.%d.\n%s",
				OPENCL_VERSION_MAJOR, OPENCL_VERSION_MINOR,
				version.major, version.minor, opencl_err_version);
	}

	/* Create platform if it doesn't exist yet */
	if (!opencl_platform)
		opencl_platform = opencl_platform_create();

	/* If an array is passed in, it must have a corresponding length
	 * and the client must either want a count or a list of platforms */
	if ((!num_entries && platforms) || (num_entries && !platforms) || (!num_platforms && !platforms))
		return CL_INVALID_VALUE;

	/* If they just want to know how many platforms there are, tell them */
	if (!num_entries && num_platforms)
	{
		*num_platforms = 1;
		return CL_SUCCESS;
	}

	/* The client wants the platform itself. Also return the number of platforms inserted if they asked for it */
	if (num_platforms)
		*num_platforms = 1;
	*platforms = opencl_platform;
	
	/* Success */
	return CL_SUCCESS;
}


cl_int clGetPlatformInfo(
	cl_platform_id platform,
	cl_platform_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	if (platform != opencl_platform || !opencl_platform)
		return CL_INVALID_PLATFORM;

	switch (param_name)
	{

	case CL_PLATFORM_PROFILE:

		return opencl_set_string(opencl_platform->full_profile, param_value_size,
			param_value, param_value_size_ret);

	case CL_PLATFORM_VERSION:

		return opencl_set_string(opencl_platform->version, param_value_size,
			param_value, param_value_size_ret);
		
	case CL_PLATFORM_NAME:

		return opencl_set_string(opencl_platform->name, param_value_size,
			param_value, param_value_size_ret);

	case CL_PLATFORM_VENDOR:

		return opencl_set_string(opencl_platform->vendor, param_value_size,
			param_value, param_value_size_ret);

	case CL_PLATFORM_EXTENSIONS:

		return opencl_set_string(opencl_platform->extensions, param_value_size,
			param_value, param_value_size_ret);
	
	default:
		return CL_INVALID_VALUE;
	}

	return 0;
}

