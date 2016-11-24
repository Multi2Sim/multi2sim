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
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include "debug.h"
#include "device.h"
#include "list.h"
#include "mhandle.h"
#include "platform.h"
#include "si-device.h"
#include "x86-device.h"
#include "union-device.h"

char *opencl_err_version =
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

/* Global OpenCL devices */
struct opencl_device_t *opencl_hsa_device;
struct opencl_device_t *opencl_si_device;
struct opencl_device_t *opencl_union_device;
struct opencl_device_t *opencl_x86_device;




/*
 * Private Functions
 */


static void opencl_platform_add_devices(struct opencl_platform_t *platform)
{
	struct opencl_device_t *device;

	char *devices;
	char devices_copy[200];
	char *token;
	char *saveptr;

	/* Create list of devices */
	platform->device_list = list_create();

	/* Read M2S_OPENCL_DEVICES configuration variable, containing default
	 * configuration for device order. If none specified, the default will
	 * show the devices in this order:
	 *
	 *   1) x86
	 *   2) Southern Islands
	 *   3) Union
	 *
	 * In native mode, only the x86 devices is available.
	 */
	devices = getenv("M2S_OPENCL_DEVICES");
	if (!devices || !devices[0])
	{
		if (opencl_native_mode)
			devices = "x86";
		else
			devices = "x86,southern-islands,union";
	}
	
	/* Process devices string */
	snprintf(devices_copy, sizeof devices_copy, "%s", devices);
	token = strtok_r(devices_copy, ", ", &saveptr);
	while (token)
	{
		/* Add device */
		if (!strcasecmp(token, "x86"))
		{
			/* Add x86 device */
			device = opencl_device_create();
			device->arch_device = opencl_x86_device_create(device);
			list_add(platform->device_list, device);
			opencl_x86_device = device;
			opencl_debug("\tx86 device added to the platform\n");
		}
		else if (!strcasecmp(token, "southern-islands") ||
				!strcasecmp(token, "SouthernIslands"))
		{
			/* Not available in native mode */
			if (opencl_native_mode)
				fatal("Southern Islands device not available in native mode");

			/* Add Southern Islands device */
			device = opencl_device_create();
			device->arch_device = opencl_si_device_create(device);
			list_add(platform->device_list, device);
			opencl_si_device = device;
			opencl_debug("\tSouthern Islands device added to the platform\n");
		}
		else if (!strcasecmp(token, "union"))
		{
			/* Not available in native mode */
			if (opencl_native_mode)
				fatal("Union device not available in native mode");

			
			/* Add fused device */
			device = opencl_device_create();
			device->arch_device = opencl_union_device_create(device,
					platform->device_list);
			list_add(platform->device_list, device);
			opencl_union_device = device;
			opencl_debug("\tUnion device added to the platform\n");
		}
		else
		{
			fatal("Invalid device name (%s)\n\n"
				"\tEnvironment variable M2S_OPENCL_DEVICES contains an invalid\n"
				"\tdevice name. Please check the Multi2Sim guide for a list of\n"
				"\tvalid OpenCL devices\n", token);
		}

		/* Next */
		token = strtok_r(NULL, ", ", &saveptr);
	}

	/* Check that at least one device was added */
	if (!platform->device_list->count)
		fatal("No device added\n\n"
			"\tThe list of OpenCL devices is empty. Please use environment variable\n"
			"\tM2S_OPENCL_DEVICES to select a list of devices that the OpenCL runtime\n"
			"\twill make visible to the application.\n");
}



/*
 * Public Functions
 */

struct opencl_platform_t *opencl_platform_create(void)
{
	struct opencl_platform_t *platform;

	/* Initialize */
	platform = xcalloc(1, sizeof(struct opencl_platform_t));
	platform->full_profile = "OpenCL Multi2Sim Platform Full Profile";
	platform->version = "1.1";
	platform->name = "Multi2Sim OpenCL Platform";
	platform->vendor = "Multi2Sim";
	platform->extensions = "";

	/* Initialize device list */
	opencl_platform_add_devices(platform);

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
 * OpenCL API Functions
 */

/* Version for the OpenCL runtime.
 * NOTE: when modifying the values of these two macros, the same values should
 * be reflected in 'src/driver/opencl/opencl.c'. */
#define OPENCL_VERSION_MAJOR  7 
#define OPENCL_VERSION_MINOR  2652

cl_int clGetPlatformIDs(
		cl_uint num_entries,
		cl_platform_id *platforms,
		cl_uint *num_platforms)
{
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

/*	m2s_active_dev = open("/dev/m2s-si-cl", O_RDWR);
	if (getenv("HSA"))
		m2s_active_dev = open("/dev/m2s-hsa-cl", O_RDWR);

	ret = ioctl(m2s_active_dev, DriverInit, &version);*/

	/* If the system call returns error, we are in native mode. */
	ret = access("/dev/multi2sim", F_OK);
	if (ret == -1 && !opencl_native_mode)
	{
		opencl_native_mode = 1;
		warning("Multi2Sim OpenCL library running on native mode.\n%s",
				opencl_err_native);
	}

	/* On simulation mode, check Multi2sim version and Multi2Sim OpenCL
	 * Runtime version compatibility. */
/*	if (!opencl_native_mode)
	{
		struct opencl_version_t version;
		if (version.major != OPENCL_VERSION_MAJOR
				|| version.minor < OPENCL_VERSION_MINOR)
			fatal("incompatible Multi2Sim Runtime version.\n"
				"\tRuntime library v. %d.%d / "
				"Host implementation v. %d.%d.\n%s",
				OPENCL_VERSION_MAJOR, OPENCL_VERSION_MINOR,
				version.major, version.minor, opencl_err_version);
	}*/

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

