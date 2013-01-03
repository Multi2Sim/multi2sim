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

#ifndef RUNTIME_OPENCL_DEVICE_H
#define RUNTIME_OPENCL_DEVICE_H

#include "../include/CL/cl.h"


struct opencl_device_type_t;
struct opencl_device_type_entry_t
{
	struct opencl_device_type_t *device_type;
	cl_uint num_devices;
	cl_device_id *devices;
};





/* Device object */
#define opencl_device_t _cl_device_id
struct _cl_device_id
{
	cl_int address_bits;
	cl_bool available;
	cl_bool compiler_available;
	cl_device_fp_config double_fp_config;
	cl_bool endian_little;
	cl_bool error_correction_support;
	cl_device_exec_capabilities execution_capabilities;
	const char *extensions;
	cl_int global_mem_cache_size;
	cl_device_mem_cache_type global_mem_cache_type;
	cl_uint global_mem_cacheline_size;
	cl_int global_mem_size;
	cl_bool host_unified_memory;
	cl_bool image_support;
	cl_int image2d_max_height;
	cl_int image2d_max_width;
	cl_int image3d_max_depth;
	cl_int image3d_max_height;
	cl_int image3d_max_width;
	cl_ulong local_mem_size;
	cl_device_local_mem_type local_mem_type;
	cl_int max_clock_frequency;
	cl_int max_compute_units;
	cl_uint max_constant_args;
	cl_ulong max_constant_buffer_size;
	cl_ulong max_mem_alloc_size;
	size_t max_parameter_size;
	cl_uint max_read_image_args;
	cl_int max_samplers;
	size_t max_work_group_size;
	cl_uint max_work_item_dimensions;
	size_t max_work_item_sizes[3];
	cl_uint max_write_image_args;
	cl_uint mem_base_addr_align;
	cl_uint min_data_type_align_size;
	const char *full_profile;
	const char *name;
	const char *opencl_c_version;
	cl_uint vector_width_char;
	cl_uint vector_width_short;
	cl_uint vector_width_int;
	cl_uint vector_width_long;
	cl_uint vector_width_float;
	cl_uint vector_width_double;
	cl_uint vector_width_half;
	const char *profile;
	size_t profiling_timer_resolution;
	cl_command_queue_properties queue_properties;
	cl_device_fp_config single_fp_config;
	cl_device_type type;
	const char *vendor;
	cl_uint vendor_id;
	const char *version;
	const char *driver_version;

	/* Device-dependent implementation */
	void *device;

	/* Don't set in driver.  Will be set by framework */
	struct opencl_device_type_t *device_type;
};


struct opencl_device_t *opencl_device_create(void);
void opencl_device_free(struct opencl_device_t *device);



/* Interface that each device driver must implement */

/* Same style as clGetDeviceIDs, except this time 
 * it is the framework querying a driver */
typedef cl_int (*opencl_device_type_init_devices_t)(
	cl_uint num_entries, 
	cl_device_id *devices, 
	cl_uint *num_devices);

/* check if a binary blob is a valid program */
typedef cl_bool (*opencl_device_type_is_valid_binary_t)(
	size_t length,
	const unsigned char *binary);

/* create a kernel object who's parameters can be set */
typedef void *(*opencl_device_type_create_kernel_t)(
	void *handle, 
	const char *kernel_name, 
	cl_int *errcode_ret);

/* set the argument of a kernel */
typedef cl_int (*opencl_device_type_set_kernel_arg_t)(
	void *kernel, 
	cl_uint arg_index, 
	size_t arg_size, 
	const void *arg_value);

/* execute on a device */
typedef void (*opencl_device_execute_ndrange_t)(
	void *device,
	void *kernel, 
	cl_uint work_dim, 
	const size_t *global_work_offset, 
	const size_t *global_work_size, 
	const size_t *local_work_size);

/* validate that a kernel has properly set parameters */
typedef cl_int (*opencl_device_check_kernel_t)(
	void *kernel);

/* destroy a kernel object */
typedef void (*opencl_device_kernel_destroy_t)(
	void *kernel);

struct opencl_device_type_t
{
	opencl_device_type_init_devices_t init_devices;	
	opencl_device_type_is_valid_binary_t valid_binary;
	opencl_device_type_create_kernel_t create_kernel;
	opencl_device_type_set_kernel_arg_t set_kernel_arg;
	opencl_device_execute_ndrange_t execute_ndrange;
	opencl_device_check_kernel_t check_kernel;
	opencl_device_kernel_destroy_t kernel_destroy;
};

/* create a device type */
typedef struct opencl_device_type_t *(*opencl_device_type_create_t)(void);

/* Calls that the device can make to the OpenCL Runtime */

void *clrt_get_address_of_buffer_object(cl_mem buffer);


#endif

