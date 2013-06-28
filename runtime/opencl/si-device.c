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

#include <limits.h>

#include "debug.h"
#include "device.h"
#include "list.h"
#include "mhandle.h"
#include "x86-device.h"
#include "si-device.h"
#include "si-kernel.h"
#include "si-program.h"

struct opencl_si_device_t *opencl_si_device_create(struct opencl_device_t *parent)
{
	struct opencl_si_device_t *device;

	/* Initialize */
	device = xcalloc(1, sizeof(struct opencl_si_device_t));
	device->type = opencl_runtime_type_si;
	device->parent = parent;

	/* Initialize parent device */
	parent->address_bits = 32;
	parent->available = CL_TRUE;
	parent->compiler_available = CL_FALSE;
	parent->double_fp_config = CL_FP_DENORM |
				CL_FP_INF_NAN | 
				CL_FP_ROUND_TO_NEAREST | 
				CL_FP_ROUND_TO_ZERO | 
				CL_FP_ROUND_TO_INF | 
				CL_FP_FMA | 
				CL_FP_SOFT_FLOAT;
	parent->endian_little = CL_TRUE;
	parent->error_correction_support = CL_FALSE;
	parent->execution_capabilities = CL_EXEC_KERNEL;
	parent->extensions = "cl_khr_fp64 cl_khr_byte_addressable_store "
			"cl_khr_global_int32_base_atomics "
			"cl_khr_local_int32_base_atomics";
	parent->global_mem_cache_size = 0;
	parent->global_mem_cache_type = CL_READ_WRITE_CACHE;
	parent->global_mem_cacheline_size = 0;
	parent->global_mem_size = 0;
	parent->host_unified_memory = CL_TRUE;
	parent->image_support = CL_FALSE;
	parent->image2d_max_height = 0;
	parent->image2d_max_width = 0;
	parent->image3d_max_depth = 0;
	parent->image3d_max_height = 0;
	parent->image3d_max_width = 0;
	parent->linker_available = CL_FALSE;
	parent->local_mem_size = INT_MAX;
	parent->local_mem_type = CL_GLOBAL;
	parent->max_clock_frequency = 0;
	parent->max_compute_units = 32;  /* FIXME - call timing simulator */
	parent->max_constant_args = 0;
	parent->max_constant_buffer_size = 0;
	parent->max_mem_alloc_size = INT_MAX;
	parent->max_parameter_size = sizeof (cl_ulong16);
	parent->max_read_image_args = 0;
	parent->max_samplers = 0;
	parent->max_work_group_size = 512;
	parent->max_work_item_dimensions = 3;
	parent->max_work_item_sizes[0] = 512;
	parent->max_work_item_sizes[1] = 512;
	parent->max_work_item_sizes[2] = 512;
	parent->max_write_image_args = 0;
	parent->mem_base_addr_align = sizeof (cl_float4);
	parent->min_data_type_align_size = 1;
	parent->name = "Multi2Sim Southern Islands GPU Model";
	parent->opencl_c_version = VERSION;
	parent->version = "1";
	parent->driver_version = "1";
	parent->vector_width_char = 16;
	parent->vector_width_short = 16 / sizeof (cl_short);
	parent->vector_width_int = 16 / sizeof (cl_int);
	parent->vector_width_long = 16 / sizeof (cl_long);
	parent->vector_width_float = 16 / sizeof (cl_float);
	parent->vector_width_double = 16 / sizeof (cl_double);
	parent->vector_width_half = 0;
	parent->profile = "PROFILE";
	parent->profiling_timer_resolution = 0;
	parent->queue_properties = CL_QUEUE_PROFILING_ENABLE;
	parent->single_fp_config = CL_FP_DENORM | 
				CL_FP_INF_NAN | 
				CL_FP_ROUND_TO_NEAREST | 
				CL_FP_ROUND_TO_ZERO | 
				CL_FP_ROUND_TO_INF | 
				CL_FP_FMA | 
				CL_FP_SOFT_FLOAT;
	parent->type = CL_DEVICE_TYPE_GPU;
	parent->vendor_id = 0;

	/* Call-back functions for device */
	parent->arch_device_free_func = (opencl_arch_device_free_func_t)
			opencl_si_device_free;

	parent->arch_device_mem_alloc_func = 
		(opencl_arch_device_mem_alloc_func_t)
		opencl_si_device_mem_alloc;
	parent->arch_device_mem_free_func = (opencl_arch_device_mem_free_func_t)
		opencl_si_device_mem_free;
	parent->arch_device_mem_read_func = (opencl_arch_device_mem_read_func_t)
		opencl_si_device_mem_read;
	parent->arch_device_mem_write_func = 
		(opencl_arch_device_mem_write_func_t)
		opencl_si_device_mem_write;
	parent->arch_device_mem_copy_func = (opencl_arch_device_mem_copy_func_t)
		opencl_si_device_mem_copy;
	parent->arch_device_preferred_workgroups_func = 
		(opencl_arch_device_preferred_workgroups_func_t)
		opencl_si_device_preferred_workgroups;

	/* Call-back functions for kernel */
	parent->arch_kernel_create_func = (opencl_arch_kernel_create_func_t)
		opencl_si_kernel_create;
	parent->arch_kernel_free_func = (opencl_arch_kernel_free_func_t)
		opencl_si_kernel_free;
	parent->arch_kernel_set_arg_func = (opencl_arch_kernel_set_arg_func_t)
		opencl_si_kernel_set_arg;

	/* Call-back functions for ND-Range */
	parent->arch_ndrange_create_func = (opencl_arch_ndrange_create_func_t)
		opencl_si_ndrange_create;
	parent->arch_ndrange_free_func = (opencl_arch_ndrange_free_func_t) 
		opencl_si_ndrange_free;
	parent->arch_ndrange_init_func = (opencl_arch_ndrange_init_func_t) 
		opencl_si_ndrange_init;
	parent->arch_ndrange_run_func = (opencl_arch_ndrange_run_func_t) 
		opencl_si_ndrange_run;
	parent->arch_ndrange_run_partial_func = 
		(opencl_arch_ndrange_run_partial_func_t) 
		opencl_si_ndrange_run_partial;
	
	/* Call-back functions for program */
	parent->arch_program_create_func = (opencl_arch_program_create_func_t)
		opencl_si_program_create;
	parent->arch_program_free_func = (opencl_arch_program_free_func_t)
		opencl_si_program_free;
	parent->arch_program_valid_binary_func = 
		(opencl_arch_program_valid_binary_func_t)
		opencl_si_program_valid_binary;

	opencl_debug("[%s] opencl_si_device_t device = %p", __FUNCTION__, 
		device);

	/* Return */
	return device;
}


void opencl_si_device_free(struct opencl_si_device_t *device)
{
	free(device);
}


void *opencl_si_device_mem_alloc(struct opencl_si_device_t *device,
		unsigned int size)
{
	void *device_ptr;

	/* Request device memory to driver */
	device_ptr = (void *) syscall(OPENCL_SYSCALL_CODE,
			opencl_abi_si_mem_alloc, size);

	return device_ptr;
}


void opencl_si_device_mem_free(struct opencl_si_device_t *device,
		void *ptr)
{
	/* Invoke 'mem_free' ABI call */
	syscall(OPENCL_SYSCALL_CODE, opencl_abi_si_mem_free, ptr);
}


void opencl_si_device_mem_read(struct opencl_si_device_t *device,
		void *host_ptr, void *device_ptr, unsigned int size)
{
	/* Invoke 'mem_read' ABI call */
	syscall(OPENCL_SYSCALL_CODE, opencl_abi_si_mem_read,
			host_ptr, device_ptr, size);
}


void opencl_si_device_mem_write(struct opencl_si_device_t *device,
		void *device_ptr, void *host_ptr, unsigned int size)
{
	/* Invoke 'mem_write' ABI call */
	syscall(OPENCL_SYSCALL_CODE, opencl_abi_si_mem_write,
			device_ptr, host_ptr, size);
}


void opencl_si_device_mem_copy(struct opencl_si_device_t *device,
		void *device_dest_ptr, void *device_src_ptr, unsigned int size)
{
	fatal("%s: not implemented", __FUNCTION__);
}

int opencl_si_device_preferred_workgroups(struct opencl_si_device_t *device)
{
	return 32; /* TODO: Determine core count */
}
