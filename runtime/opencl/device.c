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
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

#include "debug.h"
#include "device.h"
#include "mhandle.h"
#include "opencl.h"
#include "platform.h"
#include "union-device.h"






/*
 * Private Functions
 */


int dataIsZero(const char *data, size_t size)
{
	int i;
	for (i = 0; i < size; i++)
		if (data[i])
			return 0;
	return 1;
}

size_t getPropertiesCount(const void *properties, size_t prop_size)
{
	size_t size = 1;  /* There is always a 0 at the end. */
	const char *prop = (const char *)properties;
	while (!dataIsZero(prop + size * prop_size, prop_size))
		size++;
	return size;
}

void copyProperties(void *dest, const void *src, size_t size, size_t numObjs)
{
	memcpy(dest, src, size * numObjs);
}


struct opencl_device_match_info_t
{
	cl_device_type device_type;
	cl_uint num_entries;
	cl_device_id *devices;
	cl_uint num_matches;
};


static void opencl_device_matcher(struct opencl_device_t *device, void *user_data)
{
	struct opencl_device_match_info_t *info = user_data;

	if (info->device_type & device->type)
	{
		if (info->num_matches < info->num_entries)
			info->devices[info->num_matches] = device;
		info->num_matches++;
	}
}


struct opencl_device_finder_info_t
{
	struct opencl_device_t *device;
	int found;
};


static void opencl_device_finder(struct opencl_device_t *device, void *user_data)
{
	struct opencl_device_finder_info_t *info = user_data;

	if (device == info->device)
		info->found = 1;
}





/*
 * Public Functions
 */

struct opencl_device_t *opencl_device_create(void)
{
	struct opencl_device_t *device;

	/* Initialize */
	device = xcalloc(1, sizeof(struct opencl_device_t));
	device->full_profile = "FULL_PROFILE";
	device->opencl_c_version = "OpenCL 1.1 Multi2Sim";
	device->vendor = "Multi2Sim";
	device->extensions = "";
	device->driver_version = "1.0";

	/* Return */
	return device;
}


void opencl_device_free(struct opencl_device_t *device)
{
	free(device);
}


int opencl_device_verify(struct opencl_device_t *device)
{
	struct opencl_device_finder_info_t info;

	info.device = device;
	info.found = 0;	
	opencl_platform_for_each_device(opencl_platform,
		opencl_device_finder, &info);
	return info.found;
}





/*
 * OpenCL API Functions
 */

cl_int clGetDeviceIDs(
	cl_platform_id platform,
	cl_device_type device_type,
	cl_uint num_entries,
	cl_device_id *devices,
	cl_uint *num_devices)
{
	struct opencl_device_match_info_t match_info;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tplatform = %p", platform);
	opencl_debug("\tdevice_type = 0x%x", (int) device_type);
	opencl_debug("\tnum_entries = %u", num_entries);
	opencl_debug("\tdevices = %p", devices);
	opencl_debug("\tnum_devices = %p", num_devices);

	if (platform != opencl_platform)
		return CL_INVALID_PLATFORM;

	if (!opencl_is_valid_device_type(device_type))
		return CL_INVALID_DEVICE_TYPE;

	/* If a device array is passed in, it must have a corresponding length and vice-versa
	 * The client must also want either a count of the number of devices or the devices themselves */
	if ((!num_entries && devices) || (num_entries && !devices) || (!num_devices && !devices))
		return CL_INVALID_VALUE;

	/* Find devices matching search criterion */
	match_info.device_type = device_type;
	match_info.num_entries = num_entries;
	match_info.devices = devices;
	match_info.num_matches = 0;
	opencl_platform_for_each_device(opencl_platform, opencl_device_matcher, &match_info);

	/* Return number of matches */
	if (num_devices)
		*num_devices = match_info.num_matches;

	/* No match found */
	if (!match_info.num_matches)
		return CL_DEVICE_NOT_FOUND;

	/* Success */
	return CL_SUCCESS;
}


cl_int clGetDeviceInfo(
	cl_device_id device,
	cl_device_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	if (!opencl_device_verify(device))
		return CL_INVALID_DEVICE;

	switch (param_name)
	{

	case CL_DEVICE_ADDRESS_BITS:
		return opencl_set_param(
			&device->address_bits, 
			sizeof device->address_bits, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_AVAILABLE:
		return opencl_set_param(
			&device->available, 
			sizeof device->available, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_COMPILER_AVAILABLE:
		return opencl_set_param(
			&device->compiler_available, 
			sizeof device->compiler_available, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	/* 0x1032 reserved for CL_DEVICE_DOUBLE_FP_CONFIG */
	case 0x1032:
		return opencl_set_param(
			&device->double_fp_config, 
			sizeof device->double_fp_config, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_ENDIAN_LITTLE:
		return opencl_set_param(
			&device->endian_little, 
			sizeof device->endian_little, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
		return opencl_set_param(
			&device->error_correction_support, 
			sizeof device->error_correction_support, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_EXECUTION_CAPABILITIES:
		return opencl_set_param(
			&device->execution_capabilities, 
			sizeof device->execution_capabilities, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_EXTENSIONS:
		return opencl_set_param(
			device->extensions,
			strlen(device->extensions) + 1, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
		return opencl_set_param(
			&device->global_mem_cache_size, 
			sizeof device->global_mem_cache_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
		return opencl_set_param(
			&device->global_mem_cache_type, 
			sizeof device->global_mem_cache_type, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
		return opencl_set_param(
			&device->global_mem_cacheline_size, 
			sizeof device->global_mem_cacheline_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_GLOBAL_MEM_SIZE:
		return opencl_set_param(
			&device->global_mem_size, 
			sizeof device->global_mem_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_HOST_UNIFIED_MEMORY:
		return opencl_set_param(
			&device->host_unified_memory, 
			sizeof device->host_unified_memory, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_IMAGE_SUPPORT:
		return opencl_set_param(
			&device->image_support, 
			sizeof device->image_support, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
		return opencl_set_param(
			&device->image2d_max_height, 
			sizeof device->image2d_max_height, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_IMAGE2D_MAX_WIDTH:
		return opencl_set_param(
			&device->image2d_max_width, 
			sizeof device->image2d_max_width, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_IMAGE3D_MAX_DEPTH:
		return opencl_set_param(
			&device->image3d_max_depth, 
			sizeof device->image3d_max_depth, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
		return opencl_set_param(
			&device->image3d_max_height, 
			sizeof device->image3d_max_height, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_IMAGE3D_MAX_WIDTH:
		return opencl_set_param(
			&device->image3d_max_width, 
			sizeof device->image3d_max_width, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_LOCAL_MEM_SIZE:
		return opencl_set_param(
			&device->local_mem_size, 
			sizeof device->local_mem_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_LOCAL_MEM_TYPE:
		return opencl_set_param(
			&device->local_mem_type, 
			sizeof device->local_mem_type, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_MAX_CLOCK_FREQUENCY:
		return opencl_set_param(
			&device->max_clock_frequency, 
			sizeof device->max_clock_frequency, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_MAX_COMPUTE_UNITS:
		return opencl_set_param(
			&device->max_compute_units, 
			sizeof device->max_compute_units, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_MAX_CONSTANT_ARGS:
		return opencl_set_param(
			&device->max_constant_args, 
			sizeof device->max_constant_args, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
		return opencl_set_param(
			&device->max_constant_buffer_size, 
			sizeof device->max_constant_buffer_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
		return opencl_set_param(
			&device->max_mem_alloc_size, 
			sizeof device->max_mem_alloc_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_MAX_PARAMETER_SIZE:
		return opencl_set_param(
			&device->max_parameter_size, 
			sizeof device->max_parameter_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_MAX_READ_IMAGE_ARGS:
		return opencl_set_param(
			&device->max_read_image_args, 
			sizeof device->max_read_image_args, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_MAX_SAMPLERS:
		return opencl_set_param(
			&device->max_samplers, 
			sizeof device->max_samplers, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_MAX_WORK_GROUP_SIZE:
		return opencl_set_param(
			&device->max_work_group_size, 
			sizeof device->max_work_group_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
		return opencl_set_param(
			&device->max_work_item_dimensions, 
			sizeof device->max_work_item_dimensions, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_MAX_WORK_ITEM_SIZES:
		return opencl_set_param(
			&device->max_work_item_sizes,
			sizeof device->max_work_item_sizes,
			param_value_size,
			param_value,
			param_value_size_ret);

	case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
		return opencl_set_param(
			&device->max_write_image_args,
			sizeof device->max_write_image_args, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
		return opencl_set_param(
			&device->mem_base_addr_align,
			sizeof device->mem_base_addr_align,
			param_value_size,
			param_value,
			param_value_size_ret);

	case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
		return opencl_set_param(
			&device->min_data_type_align_size,
			sizeof device->min_data_type_align_size,
			param_value_size,
			param_value,
			param_value_size_ret);

	case CL_DEVICE_NAME:
		return opencl_set_param(
			device->name, 
			strlen(device->name) + 1, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_OPENCL_C_VERSION:
		return opencl_set_string(
			device->opencl_c_version, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_PLATFORM:
		return opencl_set_param(
			&opencl_platform, 
			sizeof opencl_platform, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
	case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
		return opencl_set_param(
			&device->vector_width_char, 
			sizeof device->vector_width_char, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
	case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
		return opencl_set_param(
			&device->vector_width_short, 
			sizeof device->vector_width_short, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
	case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
		return opencl_set_param(
			&device->vector_width_int, 
			sizeof device->vector_width_int, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
	case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
		return opencl_set_param(
			&device->vector_width_long, 
			sizeof device->vector_width_long, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
	case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
		return opencl_set_param(
			&device->vector_width_float, 
			sizeof device->vector_width_float, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
	case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
		return opencl_set_param(
			&device->vector_width_double, 
			sizeof device->vector_width_double, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
	case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
		return opencl_set_param(
			&device->vector_width_half, 
			sizeof device->vector_width_half, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_PROFILE:
		return opencl_set_string(
			device->full_profile, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
		return opencl_set_param(
			&device->profiling_timer_resolution, 
			sizeof device->profiling_timer_resolution, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_QUEUE_PROPERTIES:
		return opencl_set_param(
			&device->queue_properties, 
			sizeof device->queue_properties, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_SINGLE_FP_CONFIG:
		return opencl_set_param(
			&device->single_fp_config, 
			sizeof device->single_fp_config, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_TYPE:
		return opencl_set_param(
			&device->type, 
			sizeof device->type, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_VENDOR:
		return opencl_set_param(
			device->vendor, 
			strlen(device->vendor) + 1, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_VENDOR_ID:
		return opencl_set_param(	
			&device->vendor_id, 
			sizeof device->vendor_id, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DEVICE_VERSION:
		return opencl_set_string(
			device->version, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	case CL_DRIVER_VERSION:
		return opencl_set_string(
			device->driver_version, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

	default:
			OPENCL_ARG_NOT_SUPPORTED(param_name)
			return CL_INVALID_VALUE;
	}

	return 0;
}

