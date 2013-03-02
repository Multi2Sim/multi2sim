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

#include "debug.h"
#include "device.h"
#include "mhandle.h"
#include "si-device.h"
#include "si-kernel.h"
#include "si-program.h"


struct opencl_si_device_t *opencl_si_device_create(struct opencl_device_t *parent)
{
	struct opencl_si_device_t *device;

	/* Initialize */
	device = xcalloc(1, sizeof(struct opencl_si_device_t));
	device->parent = parent;

	/* Initialize parent device */
	parent->name = "Multi2Sim Southern Islands GPU Model";
	parent->type = CL_DEVICE_TYPE_GPU;

	/* Call-back functions for device */
	parent->arch_device_free_func = (opencl_arch_device_free_func_t)
			opencl_si_device_free;
	parent->arch_device_mem_alloc_func = (opencl_arch_device_mem_alloc_func_t)
			opencl_si_device_mem_alloc;
	parent->arch_device_mem_free_func = (opencl_arch_device_mem_free_func_t)
			opencl_si_device_mem_free;
	parent->arch_device_mem_read_func = (opencl_arch_device_mem_read_func_t)
			opencl_si_device_mem_read;
	parent->arch_device_mem_write_func = (opencl_arch_device_mem_write_func_t)
			opencl_si_device_mem_write;
	parent->arch_device_mem_copy_func = (opencl_arch_device_mem_copy_func_t)
			opencl_si_device_mem_copy;

	/* Call-back functions for kernel */
	parent->arch_kernel_create_func = (opencl_arch_kernel_create_func_t)
			opencl_si_kernel_create;
	parent->arch_kernel_free_func = (opencl_arch_kernel_free_func_t)
			opencl_si_kernel_free;
	parent->arch_kernel_check_func = (opencl_arch_kernel_check_func_t)
			opencl_si_kernel_check;
	parent->arch_kernel_set_arg_func = (opencl_arch_kernel_set_arg_func_t)
			opencl_si_kernel_set_arg;
	parent->arch_kernel_run_func = (opencl_arch_kernel_run_func_t)
			opencl_si_kernel_run;
	
	/* Call-back functions for program */
	parent->arch_program_create_func = (opencl_arch_program_create_func_t)
			opencl_si_program_create;
	parent->arch_program_free_func = (opencl_arch_program_free_func_t)
			opencl_si_program_free;
	parent->arch_program_valid_binary_func = (opencl_arch_program_valid_binary_func_t)
			opencl_si_program_valid_binary;

	/* Return */
	return device;
}


void opencl_si_device_free(struct opencl_si_device_t *device)
{
	free(device);
}


void *opencl_si_device_mem_alloc(size_t size)
{
	warning("%s: not implemented", __FUNCTION__);
	return NULL;
}


void opencl_si_device_mem_free(void *ptr)
{
	warning("%s: not implemented", __FUNCTION__);
}


void opencl_si_device_mem_read(void *host_ptr, void *device_ptr, size_t size)
{
	warning("%s: not implemented", __FUNCTION__);
}


void opencl_si_device_mem_write(void *device_ptr, void *host_ptr, size_t size)
{
	warning("%s: not implemented", __FUNCTION__);
}


void opencl_si_device_mem_copy(void *device_dest_ptr, void *device_src_ptr, size_t size)
{
	warning("%s: not implemented", __FUNCTION__);
}
