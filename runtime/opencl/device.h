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

#include "opencl.h"


/*
 * Call-back functions for object 'opencl_xxx_device_t'
 */

/* Create an architecture-specific device. Returns an object of type
 * 'opencl_XXX_device_t'. */
typedef void *(*opencl_device_arch_device_create_func_t)(
		struct opencl_device_t *parent);

/* Free an architecture-specific device. */
typedef void *(*opencl_device_arch_device_free_func_t)(
		void *device);  /* Of type 'opencl_XXX_device_t' */



/*
 * Call-back functions for object 'opencl_xxx_program_t'
 */

/* Create an architecture-specific program. Returns an object of type
 * 'opencl_XXX_program_t'. */
typedef void *(*opencl_device_arch_program_create_func_t)(
		struct opencl_program_t *parent,
		void *arch_device,  /* Of type 'opencl_xxx_device_t' */
		void *binary,
		size_t length);

/* Free an architecture-specific program. */
typedef void *(*opencl_device_arch_program_free_func_t)(
		void *program);  /* Of type 'opencl_XXX_program_t' */

/* Check if a binary blob is a valid program */
typedef cl_bool (*opencl_device_arch_program_valid_binary_func_t)(
	void *binary,
	size_t length);



/*
 * Call-back functions for object 'opencl_xxx_kernel_t'
 */

/* Create an architecture-specific kernel. Returns an object of type
 * 'opencl_XXX_kernel_t'. */
typedef void *(*opencl_device_arch_kernel_create_func_t)(
		struct opencl_kernel_t *parent,
		void *arch_program,  /* Of type 'opencl_xxx_program_t' */
		const char *kernel_name);

/* Free an architecture-specific kernel. */
typedef void (*opencl_device_arch_kernel_free_func_t)(
		void *kernel);  /* Of type 'opencl_XXX_kernel_t' */

/* Verify that a kernel has properly set parameters */
typedef cl_int (*opencl_device_arch_kernel_check_func_t)(
		void *kernel);  /* Of type 'opencl_XXX_kernel_t' */

/* Set a kernel argument */
typedef cl_int (*opencl_device_arch_kernel_set_arg_func_t)(
		void *kernel,  /* Of type 'opencl_XXX_kernel_t' */
		cl_uint arg_index,
		size_t arg_size,
		const void *arg_value);

/* Run ND-Range on device */
typedef void (*opencl_device_arch_kernel_run_func_t)(
		void *kernel,  /* Of type 'opencl_XXX_kernel_t' */
		cl_uint work_dim,
		const size_t *global_work_offset,
		const size_t *global_work_size,
		const size_t *local_work_size);



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

	/* Call-back functions for an architecture-specific program */
	opencl_device_arch_program_create_func_t arch_program_create_func;
	opencl_device_arch_program_free_func_t arch_program_free_func;
	opencl_device_arch_program_valid_binary_func_t arch_program_valid_binary_func;

	/* Call-back functions for an architecture-specific kernel */
	opencl_device_arch_kernel_create_func_t arch_kernel_create_func;
	opencl_device_arch_kernel_free_func_t arch_kernel_free_func;
	opencl_device_arch_kernel_check_func_t arch_kernel_check_func;
	opencl_device_arch_kernel_set_arg_func_t arch_kernel_set_arg_func;
	opencl_device_arch_kernel_run_func_t arch_kernel_run_func;
	
	/* Architecture-specific device of type 'opencl_XXX_device_t'.
	 * This pointer is used to reference what would be a sub-class in an
	 * object-oriented language. */
	void *arch_device;
};


struct opencl_device_t *opencl_device_create(void);
void opencl_device_free(struct opencl_device_t *device);

int opencl_device_verify(struct opencl_device_t *device);


#endif

