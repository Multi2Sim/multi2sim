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

#ifndef RUNTIME_OPENCL_EVG_KERNEL_H
#define RUNTIME_OPENCL_EVG_KERNEL_H

#include "opencl.h"


/* Forward declarations */
struct opencl_evg_device_t;
struct opencl_evg_program_t;


/* Subclass of 'opencl_kernel_t' containing Evergreen specific
 * information of the OpenCL kernel. */
struct opencl_evg_kernel_t
{
	/* Kernel object acting as parent object. */
	struct opencl_kernel_t *parent;

	/* Architecture-specific program and device associated */
	struct opencl_evg_program_t *program;
	struct opencl_evg_device_t *device;
};


struct opencl_evg_kernel_t *opencl_evg_kernel_create(
		struct opencl_kernel_t *parent,
		struct opencl_evg_program_t *program,
		char *func_name);

void opencl_evg_kernel_free(
		struct opencl_evg_kernel_t *kernel);

cl_int opencl_evg_kernel_check(
		struct opencl_evg_kernel_t *kernel);

cl_int opencl_evg_kernel_set_arg(
		struct opencl_evg_kernel_t *kernel,
		cl_uint arg_index,
		size_t arg_size,
		const void *arg_value);

void opencl_evg_kernel_run(
		struct opencl_evg_kernel_t *kernel,
		cl_uint work_dim,
		const size_t *global_work_offset,
		const size_t *global_work_size,
		const size_t *local_work_size);


#endif

