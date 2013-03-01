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
#include "mhandle.h"
#include "si-kernel.h"


struct opencl_si_kernel_t *opencl_si_kernel_create(
		struct opencl_kernel_t *parent,
		struct opencl_si_program_t *program,
		char *func_name)
{
	struct opencl_si_kernel_t *kernel;

	/* Initialize */
	kernel = xcalloc(1, sizeof(struct opencl_si_kernel_t));
	kernel->parent = parent;
	kernel->program = program;

	/* Return */
	return kernel;
}


void opencl_si_kernel_free(struct opencl_si_kernel_t *kernel)
{
	free(kernel);
}


cl_int opencl_si_kernel_check(struct opencl_si_kernel_t *kernel)
{
	warning("%s: not implemented", __FUNCTION__);
	return 0;
}


cl_int opencl_si_kernel_set_arg(
		struct opencl_si_kernel_t *kernel,
		cl_uint arg_index,
		size_t arg_size,
		const void *arg_value)
{
	warning("%s: not implemented", __FUNCTION__);
	return 0;
}


void opencl_si_kernel_run(
		struct opencl_si_kernel_t *kernel,
		cl_uint work_dim,
		const size_t *global_work_offset,
		const size_t *global_work_size,
		const size_t *local_work_size)
{
	warning("%s: not implemented", __FUNCTION__);
}

