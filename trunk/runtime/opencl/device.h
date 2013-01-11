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

int opencl_device_verify(struct opencl_device_t *device);



/* Interface that each device driver must implement */

/* Same style as clGetDeviceIDs, except this time 
 * it is the framework querying a driver */
typedef cl_int (*opencl_device_type_init_devices_t)(
	cl_uint num_entries, 
	cl_device_id *devices, 
	cl_uint *num_devices);

/* check if a binary blob is a valid program */
typedef cl_bool (*opencl_device_is_valid_binary_func_t)(
	size_t length,
	const unsigned char *binary);

/* execute on a device */
typedef void (*opencl_device_arch_kernel_run_func_t)(
	void *device,
	void *kernel, 
	cl_uint work_dim, 
	const size_t *global_work_offset, 
	const size_t *global_work_size, 
	const size_t *local_work_size);

/* create a kernel object who's parameters can be set */
typedef void *(*opencl_device_arch_kernel_create_func_t)(
	void *dlhandle,
	const char *kernel_name,
	cl_int *errcode_ret);

/* Free an architecture-specific kernel. */
typedef void (*opencl_device_arch_kernel_free_func_t)(
	void *kernel);

/* Verify that a kernel has properly set parameters */
typedef cl_int (*opencl_device_arch_kernel_check_func_t)(
	void *kernel);

/* Set a kernel argument */
typedef cl_int (*opencl_device_arch_kernel_set_arg_func_t)(
	void *kernel,
	cl_uint arg_index,
	size_t arg_size,
	const void *arg_value);

struct opencl_device_type_t
{
	opencl_device_type_init_devices_t init_devices;	
	opencl_device_is_valid_binary_func_t is_valid_binary;

	/* Call-back functions for an architecture-specific kernel */
	opencl_device_arch_kernel_create_func_t arch_kernel_create_func;
	opencl_device_arch_kernel_free_func_t arch_kernel_free_func;
	opencl_device_arch_kernel_check_func_t arch_kernel_check_func;
	opencl_device_arch_kernel_set_arg_func_t arch_kernel_set_arg_func;
	opencl_device_arch_kernel_run_func_t arch_kernel_run_func;
};

/* create a device type */
typedef struct opencl_device_type_t *(*opencl_device_type_create_t)(void);


#endif

