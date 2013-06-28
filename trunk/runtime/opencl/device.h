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
	cl_ulong global_mem_cache_size;
	cl_device_mem_cache_type global_mem_cache_type;
	cl_uint global_mem_cacheline_size;
	cl_ulong global_mem_size;
	cl_bool host_unified_memory;
	cl_bool image_support;
	cl_int image2d_max_height;
	cl_int image2d_max_width;
	cl_int image3d_max_depth;
	cl_int image3d_max_height;
	cl_int image3d_max_width;
	cl_bool linker_available;
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

	/* Call-back functions for an architecture-specific device */
	opencl_arch_device_free_func_t arch_device_free_func;
	opencl_arch_device_mem_alloc_func_t arch_device_mem_alloc_func;
	opencl_arch_device_mem_free_func_t arch_device_mem_free_func;
	opencl_arch_device_mem_read_func_t arch_device_mem_read_func;
	opencl_arch_device_mem_write_func_t arch_device_mem_write_func;
	opencl_arch_device_mem_copy_func_t arch_device_mem_copy_func;
	opencl_arch_device_preferred_workgroups_func_t arch_device_preferred_workgroups_func;

	/* Call-back functions for an architecture-specific program */
	opencl_arch_program_create_func_t arch_program_create_func;
	opencl_arch_program_free_func_t arch_program_free_func;
	opencl_arch_program_valid_binary_func_t arch_program_valid_binary_func;

	/* Call-back functions for an architecture-specific kernel */
	opencl_arch_kernel_create_func_t arch_kernel_create_func;
	opencl_arch_kernel_free_func_t arch_kernel_free_func;
	opencl_arch_kernel_set_arg_func_t arch_kernel_set_arg_func;

	/* Call-back functions for ND-range execution */
	opencl_arch_ndrange_create_func_t arch_ndrange_create_func;
	opencl_arch_ndrange_init_func_t arch_ndrange_init_func;
	opencl_arch_ndrange_run_func_t arch_ndrange_run_func;
	opencl_arch_ndrange_run_partial_func_t arch_ndrange_run_partial_func;
	opencl_arch_ndrange_free_func_t arch_ndrange_free_func;
	
	/* Architecture-specific device of type 'opencl_XXX_device_t'.
	 * This pointer is used to reference what would be a sub-class in an
	 * object-oriented language. */
	void *arch_device;
};


struct opencl_device_t *opencl_device_create(void);
void opencl_device_free(struct opencl_device_t *device);

int opencl_device_verify(struct opencl_device_t *device);


#endif

