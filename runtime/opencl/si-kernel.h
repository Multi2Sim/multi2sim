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

#ifndef RUNTIME_OPENCL_SI_KERNEL_H
#define RUNTIME_OPENCL_SI_KERNEL_H

#include "opencl.h"


/* Forward declarations */
struct opencl_si_device_t;
struct opencl_si_program_t;


/*
 * Southern Islands Kernel Argument
 */

/* NOTE: when adding new fields, update string map 'opencl_si_arg_type_map'
 * in 'runtime/opencl/si-kernel.c'. */
enum opencl_si_arg_type_t
{
	opencl_si_arg_type_invalid = 0,
	opencl_si_arg_type_value,
	opencl_si_arg_type_pointer
};


/* NOTE: update string map 'opencl_si_arg_scope_map' in 'runtime/opencl/si-kernel.c'
 * when modifying this.
 * NOTE: enumeration with same name in 'src/driver/opencl' should match exactly.
 * Increase runtime/driver major version if changed. */
enum opencl_si_arg_scope_t
{
	opencl_si_arg_scope_invalid = 0,
	opencl_si_arg_scope_g,  /* Global memory */
	opencl_si_arg_scope_p,  /* Private memory */
	opencl_si_arg_scope_local,  /* Local memory */
	opencl_si_arg_scope_uav,  /* Global memory */
	opencl_si_arg_scope_c,  /* Constant memory */
	opencl_si_arg_scope_r,  /* GDS */
	opencl_si_arg_scope_hl,  /* Hardware local */
	opencl_si_arg_scope_hp,  /* Hardware private */
	opencl_si_arg_scope_hc,  /* Hardware constant */
	opencl_si_arg_scope_hr  /* Hardware GDS */
};


struct opencl_si_arg_t
{
	enum opencl_si_arg_type_t type;
	char *name;

	/* For argument of type 'pointer', scope. */
	enum opencl_si_arg_scope_t scope;
};


struct opencl_si_arg_t *opencl_si_arg_create(enum opencl_si_arg_type_t type,
		char *name);
void opencl_si_arg_free(struct opencl_si_arg_t *arg);




/*
 * Southern Islands Kernel
 */

/* Subclass of 'opencl_kernel_t' containing Southern Islands specific
 * information of the OpenCL kernel. */
struct opencl_si_kernel_t
{
	/* Kernel object acting as parent object. */
	struct opencl_kernel_t *parent;

	/* Architecture-specific program and device associated */
	struct opencl_si_program_t *program;
	struct opencl_si_device_t *device;

	/* List of arguments */
	struct list_t *arg_list;
};


struct opencl_si_kernel_t *opencl_si_kernel_create(
		struct opencl_kernel_t *parent,
		struct opencl_si_program_t *program,
		char *func_name);

void opencl_si_kernel_free(struct opencl_si_kernel_t *kernel);

void opencl_si_kernel_debug(struct opencl_si_kernel_t *kernel);

int opencl_si_kernel_check(struct opencl_si_kernel_t *kernel);

int opencl_si_kernel_set_arg(
		struct opencl_si_kernel_t *kernel,
		int arg_index,
		unsigned int arg_size,
		void *arg_value);

void opencl_si_kernel_run(
		struct opencl_si_kernel_t *kernel,
		cl_uint work_dim,
		const size_t *global_work_offset,
		const size_t *global_work_size,
		const size_t *local_work_size);


#endif

