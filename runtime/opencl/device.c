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

#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

#include "clrt.h"
#include "debug.h"



extern struct _cl_platform_id *m2s_platform;

const char *opencl_device_full_profile = "FULL_PROFILE";
const char *opencl_device_version = "OpenCL 1.1 Multi2Sim";
const char *opencl_device_name = "Multi2Sim OpenCL Runtime";
const char *opencl_device_vendor = "Multi2Sim";
const char *opencl_device_extensions = "";

/* FIXME - combine runtime version numbers here */
const char *opencl_device_driver_version = "1.0";





/*
 * Private Functions
 */


/* Used for clGet*Info functions, does standard checking */
cl_int populateParameter(const void *value, size_t actual, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
	if (param_value && actual > param_value_size)
		return CL_INVALID_VALUE;
	if (param_value_size_ret)
		*param_value_size_ret = actual;
	if (param_value)
		memcpy(param_value, value, actual);
	return CL_SUCCESS;
}

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
	size_t size = 1; // There is always a 0 at the end.
	const char *prop = (const char *)properties;
	while (!dataIsZero(prop + size * prop_size, prop_size))
		size++;
	return size;
}

void copyProperties(void *dest, const void *src, size_t size, size_t numObjs)
{
	memcpy(dest, src, size * numObjs);
}

struct device_match_info_t
{
	cl_device_type device_type;
	cl_uint num_entries;
	cl_device_id *devices;
	cl_uint matches;
};

void device_matcher(void *ctx, cl_device_id device, struct clrt_device_type_t *device_type)
{
	struct device_match_info_t *info = ctx;
	if (info->device_type & device->type)
	{
		if (info->matches < info->num_entries)
			info->devices[info->matches] = device;
		info->matches++;
	}
}

/*
 * Public Functions
 */

int clrt_is_valid_device_type(cl_device_type device_type)
{
	return device_type == CL_DEVICE_TYPE_ALL
		|| (device_type & 
			(CL_DEVICE_TYPE_GPU 
			| CL_DEVICE_TYPE_CPU 
			| CL_DEVICE_TYPE_ACCELERATOR 
			| CL_DEVICE_TYPE_DEFAULT));
}

cl_int clGetDeviceIDs(
	cl_platform_id platform,
	cl_device_type device_type,
	cl_uint num_entries,
	cl_device_id *devices,
	cl_uint *num_devices)
{
	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tplatform = %p", platform);
	m2s_clrt_debug("\tdevice_type = 0x%x", (int) device_type);
	m2s_clrt_debug("\tnum_entries = %u", num_entries);
	m2s_clrt_debug("\tdevices = %p", devices);
	m2s_clrt_debug("\tnum_devices = %p", num_devices);

	if (platform != m2s_platform)
		return CL_INVALID_PLATFORM;

	if (!clrt_is_valid_device_type(device_type))
		return CL_INVALID_DEVICE_TYPE;

	/* If a device array is passed in, it must have a corresponding length and vice-versa
	 * The client must also want either a count of the number of devices or the devices themselves */
	if ((!num_entries && devices) || (num_entries && !devices) || (!num_devices && !devices))
		return CL_INVALID_VALUE;

	struct device_match_info_t info;
	info.device_type = device_type;
	info.num_entries = num_entries;
	info.devices = devices;
	info.matches = 0;

	visit_devices(device_matcher, &info);

	if (num_devices)
		*num_devices = info.matches;

	if (info.matches == 0)
		return CL_DEVICE_NOT_FOUND;

	return CL_SUCCESS;
}

struct has_device_info_t
{
	cl_device_id device;
	int found;
};

void device_finder(void *ctx, cl_device_id device, struct clrt_device_type_t *device_type)
{
	struct has_device_info_t *info = ctx;
	if (device == info->device)
		info->found = 1;
}

int verify_device(cl_device_id device)
{
	struct has_device_info_t info;
	info.device = device;
	info.found = 0;	
	visit_devices(device_finder, &info);
	return info.found;
}

cl_int clGetDeviceInfo(
	cl_device_id device,
	cl_device_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	if (!verify_device(device))
		return CL_INVALID_DEVICE;

	switch (param_name)
	{
		case CL_DEVICE_ADDRESS_BITS:
		return populateParameter(
			&device->address_bits, 
			sizeof device->address_bits, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_AVAILABLE:
		return populateParameter(
			&device->available, 
			sizeof device->available, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_COMPILER_AVAILABLE:
		return populateParameter(
			&device->compiler_available, 
			sizeof device->compiler_available, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		/* 0x1032 reserved for CL_DEVICE_DOUBLE_FP_CONFIG */
		case 0x1032:
		return populateParameter(
			&device->double_fp_config, 
			sizeof device->double_fp_config, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_ENDIAN_LITTLE:
		return populateParameter(
			&device->endian_little, 
			sizeof device->endian_little, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
		return populateParameter(
			&device->error_correction_support, 
			sizeof device->error_correction_support, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_EXECUTION_CAPABILITIES:
		return populateParameter(
			&device->execution_capabilities, 
			sizeof device->execution_capabilities, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_EXTENSIONS:
		return populateParameter(
			device->extensions,
			strlen(device->extensions) + 1, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
		return populateParameter(
			&device->global_mem_cache_size, 
			sizeof device->global_mem_cache_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
		return populateParameter(
			&device->global_mem_cache_type, 
			sizeof device->global_mem_cache_type, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
		return populateParameter(
			&device->global_mem_cacheline_size, 
			sizeof device->global_mem_cacheline_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_GLOBAL_MEM_SIZE:
		return populateParameter(
			&device->global_mem_size, 
			sizeof device->global_mem_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

//		case CL_DEVICE_HALF_FP_CONFIG:
//		return CL_SUCCESS;

		case CL_DEVICE_HOST_UNIFIED_MEMORY:
		return populateParameter(
			&device->host_unified_memory, 
			sizeof device->host_unified_memory, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_IMAGE_SUPPORT:
		return populateParameter(
			&device->image_support, 
			sizeof device->image_support, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
		return populateParameter(
			&device->image2d_max_height, 
			sizeof device->image2d_max_height, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_IMAGE2D_MAX_WIDTH:
		return populateParameter(
			&device->image2d_max_width, 
			sizeof device->image2d_max_width, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_IMAGE3D_MAX_DEPTH:
		return populateParameter(
			&device->image3d_max_depth, 
			sizeof device->image3d_max_depth, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
		return populateParameter(
			&device->image3d_max_height, 
			sizeof device->image3d_max_height, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_IMAGE3D_MAX_WIDTH:
		return populateParameter(
			&device->image3d_max_width, 
			sizeof device->image3d_max_width, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_LOCAL_MEM_SIZE:
		return populateParameter(
			&device->local_mem_size, 
			sizeof device->local_mem_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_LOCAL_MEM_TYPE:
		return populateParameter(
			&device->local_mem_type, 
			sizeof device->local_mem_type, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_MAX_CLOCK_FREQUENCY:
		return populateParameter(
			&device->max_clock_frequency, 
			sizeof device->max_clock_frequency, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_MAX_COMPUTE_UNITS:
		return populateParameter(
			&device->max_compute_units, 
			sizeof device->max_compute_units, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_MAX_CONSTANT_ARGS:
		return populateParameter(
			&device->max_constant_args, 
			sizeof device->max_constant_args, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
		return populateParameter(
			&device->max_constant_buffer_size, 
			sizeof device->max_constant_buffer_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
		return populateParameter(
			&device->max_mem_alloc_size, 
			sizeof device->max_mem_alloc_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_MAX_PARAMETER_SIZE:
		return populateParameter(
			&device->max_parameter_size, 
			sizeof device->max_parameter_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_MAX_READ_IMAGE_ARGS:
		return populateParameter(
			&device->max_read_image_args, 
			sizeof device->max_read_image_args, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_MAX_SAMPLERS:
		return populateParameter(
			&device->max_samplers, 
			sizeof device->max_samplers, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_MAX_WORK_GROUP_SIZE:
		return populateParameter(
			&device->max_work_group_size, 
			sizeof device->max_work_group_size, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
		return populateParameter(
			&device->max_work_item_dimensions, 
			sizeof device->max_work_item_dimensions, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_MAX_WORK_ITEM_SIZES:
		return populateParameter(
			&device->max_work_item_sizes,
			sizeof device->max_work_item_sizes,
			param_value_size,
			param_value,
			param_value_size_ret);

		case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
		return populateParameter(
			&device->max_write_image_args,
			sizeof device->max_write_image_args, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
		return populateParameter(
			&device->mem_base_addr_align,
			sizeof device->mem_base_addr_align,
			param_value_size,
			param_value,
			param_value_size_ret);

		case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
		return populateParameter(
			&device->min_data_type_align_size,
			sizeof device->min_data_type_align_size,
			param_value_size,
			param_value,
			param_value_size_ret);

		case CL_DEVICE_NAME:
		return populateParameter(
			device->name, 
			strlen(device->name) + 1, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_OPENCL_C_VERSION:
		return populateParameter(
			opencl_device_opencl_c_version, 
			strlen(opencl_device_opencl_c_version) + 1, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_PLATFORM:
		return populateParameter(
			&m2s_platform, 
			sizeof m2s_platform, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
		return populateParameter(
			&device->vector_width_char, 
			sizeof device->vector_width_char, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
		return populateParameter(
			&device->vector_width_short, 
			sizeof device->vector_width_short, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
		return populateParameter(
			&device->vector_width_int, 
			sizeof device->vector_width_int, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
		return populateParameter(
			&device->vector_width_long, 
			sizeof device->vector_width_long, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
		return populateParameter(
			&device->vector_width_float, 
			sizeof device->vector_width_float, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
		return populateParameter(
			&device->vector_width_double, 
			sizeof device->vector_width_double, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
		return populateParameter(
			&device->vector_width_half, 
			sizeof device->vector_width_half, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_PROFILE:
		return populateParameter(
			opencl_device_full_profile, 
			strlen(opencl_device_full_profile) + 1, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
		return populateParameter(
			&device->profiling_timer_resolution, 
			sizeof device->profiling_timer_resolution, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

/* #define CL_DEVICE_QUEUE_PROPERTIES                  0x102A */
		case CL_DEVICE_QUEUE_PROPERTIES:
		return populateParameter(
			&device->queue_properties, 
			sizeof device->queue_properties, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_SINGLE_FP_CONFIG:
		return populateParameter(
			&device->single_fp_config, 
			sizeof device->single_fp_config, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_TYPE:
		return populateParameter(
			&device->type, 
			sizeof device->type, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_VENDOR:
		return populateParameter(
			device->vendor, 
			strlen(device->vendor) + 1, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_VENDOR_ID:
		return populateParameter(	
			&device->vendor_id, 
			sizeof device->vendor_id, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DEVICE_VERSION:
		return populateParameter(
			&device->version, 
			strlen(device->version) + 1, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		case CL_DRIVER_VERSION:
		return populateParameter(
			opencl_device_driver_version, 
			strlen(opencl_device_driver_version) + 1, 
			param_value_size, 
			param_value, 
			param_value_size_ret);

		default:
			EVG_OPENCL_ARG_NOT_SUPPORTED(param_name)
			return CL_INVALID_VALUE;
	}

	return 0;
}
