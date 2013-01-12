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
#include <stdlib.h>
#include <unistd.h>
#include <syscall.h>
#include <limits.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

#include "../include/CL/cl.h"
#include "clcpu.h"
#include "debug.h"
#include "device.h"
#include "mhandle.h"
#include "opencl.h"
#include "x86-device.h"
#include "x86-kernel.h"
#include "x86-program.h"


const char *DRIVER_VERSION = "0.0";
const char *DEVICE_VERSION = "OpenCL 1.1 Multi2Sim";


struct opencl_device_type_t *clcpu_create_device_type(void)
{
	struct opencl_device_type_t *device_type = xmalloc(sizeof *device_type);
	device_type->init_devices = clcpu_device_type_init_devices;
	device_type->is_valid_binary =
			(opencl_device_is_valid_binary_func_t)
			x86_program_is_valid_binary;

	device_type->arch_kernel_create_func =
			(opencl_device_arch_kernel_create_func_t)
			opencl_x86_kernel_create;
	device_type->arch_kernel_free_func =
			(opencl_device_arch_kernel_free_func_t)
			opencl_x86_kernel_free;
	device_type->arch_kernel_check_func =
			(opencl_device_arch_kernel_check_func_t)
			opencl_x86_kernel_check;
	device_type->arch_kernel_set_arg_func =
			(opencl_device_arch_kernel_set_arg_func_t)
			opencl_x86_kernel_set_arg;
	device_type->arch_kernel_run_func =
			(opencl_device_arch_kernel_run_func_t)
			opencl_x86_kernel_run;

	return device_type;
}


cl_int clcpu_device_type_init_devices(
	cl_uint num_entries, 
	cl_device_id *devices, 
	cl_uint *num_devices)
{
	if (num_devices)
		*num_devices = 1; /* there will always be one CPU */

	if (num_entries && devices)
	{
		cl_device_id cpu = xmalloc(sizeof *cpu);
		cpu->arch_device = opencl_x86_device_create(cpu);
		devices[0] = cpu;
	}

	return CL_SUCCESS;
}

