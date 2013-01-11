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

#ifndef RUNTIME_OPENCL_X86_KERNEL_H
#define RUNTIME_OPENCL_X86_KERNEL_H

#include "clrt.h"


/* Function executing x86 OpenCL kernel */
typedef void (*opencl_x86_kernel_func_t)(void);


enum opencl_x86_kernel_arg_type_t
{
	OPENCL_X86_KERNEL_ARG_INVALID = 0,
	OPENCL_X86_KERNEL_ARG_CHAR = 1,
	OPENCL_X86_KERNEL_ARG_SHORT = 2,
	OPENCL_X86_KERNEL_ARG_INT = 3,
	OPENCL_X86_KERNEL_ARG_LONG = 4,
	OPENCL_X86_KERNEL_ARG_FLOAT = 5,
	OPENCL_X86_KERNEL_ARG_DOUBLE = 6,
	OPENCL_X86_KERNEL_ARG_POINTER = 7,
	OPENCL_X86_KERNEL_ARG_CHAR2 = 8,
	OPENCL_X86_KERNEL_ARG_CHAR3 = 9,
	OPENCL_X86_KERNEL_ARG_CHAR4 = 10,
	OPENCL_X86_KERNEL_ARG_CHAR8 = 11,
	OPENCL_X86_KERNEL_ARG_CHAR16 = 12,
	OPENCL_X86_KERNEL_ARG_SHORT2 = 13,
	OPENCL_X86_KERNEL_ARG_SHORT3 = 14,
	OPENCL_X86_KERNEL_ARG_SHORT4 = 15,
	OPENCL_X86_KERNEL_ARG_SHORT8 = 16,
	OPENCL_X86_KERNEL_ARG_SHORT16 = 17,
	OPENCL_X86_KERNEL_ARG_INT2 = 18,
	OPENCL_X86_KERNEL_ARG_INT3 = 19,
	OPENCL_X86_KERNEL_ARG_INT4 = 20,
	OPENCL_X86_KERNEL_ARG_INT8 = 21,
	OPENCL_X86_KERNEL_ARG_INT16 = 22,
	OPENCL_X86_KERNEL_ARG_LONG2 = 23,
	OPENCL_X86_KERNEL_ARG_LONG3 = 24,
	OPENCL_X86_KERNEL_ARG_LONG4 = 25,
	OPENCL_X86_KERNEL_ARG_LONG8 = 26,
	OPENCL_X86_KERNEL_ARG_LONG16 = 27,
	OPENCL_X86_KERNEL_ARG_FLOAT2 = 28,
	OPENCL_X86_KERNEL_ARG_FLOAT3 = 29,
	OPENCL_X86_KERNEL_ARG_FLOAT4 = 30,
	OPENCL_X86_KERNEL_ARG_FLOAT8 = 31,
	OPENCL_X86_KERNEL_ARG_FLOAT16 = 32,
	OPENCL_X86_KERNEL_ARG_DOUBLE2 = 33,
	OPENCL_X86_KERNEL_ARG_DOUBLE3 = 34,
	OPENCL_X86_KERNEL_ARG_DOUBLE4 = 35,
	OPENCL_X86_KERNEL_ARG_DOUBLE8 = 36,
	OPENCL_X86_KERNEL_ARG_DOUBLE16 = 37
};


/*
* Does not start with invalid as this is based on the ELF produced
* by the AMD APP SDK.
*/

enum opencl_x86_kernel_mem_arg_type_t
{
	OPENCL_X86_KERNEL_MEM_ARG_VALUE = 0,
	OPENCL_X86_KERNEL_MEM_ARG_LOCAL = 1,
	OPENCL_X86_KERNEL_MEM_ARG_CONSTANT = 2,
	OPENCL_X86_KERNEL_MEM_ARG_GLOBAL = 3
};


struct opencl_x86_kernel_reg_arg_t
{
	int reg[4];
};


struct opencl_x86_kernel_arg_t
{
	enum opencl_x86_kernel_arg_type_t arg_type;
	enum opencl_x86_kernel_mem_arg_type_t mem_arg_type;
	int is_set;
	int stack_offset;
	int is_stack;
	int reg_offset;
	int size;
};


/* x86 kernel object. This structure contains the information that extends structure
 * opencl_kernel_t with information specific to x86. */
struct opencl_x86_kernel_t
{
	opencl_x86_kernel_func_t func;

	size_t *metadata;
	unsigned int num_params;
	struct opencl_x86_kernel_arg_t *param_info;
	int stack_param_words;
	size_t *stack_params;
	struct opencl_x86_kernel_reg_arg_t *register_params;
	size_t local_reserved_bytes;
};


struct opencl_x86_kernel_t *opencl_x86_kernel_create(
		void *handle,
		char *name,
		cl_int *err_ptr);

void opencl_x86_kernel_free(
		struct opencl_x86_kernel_t *kernel);

cl_int opencl_x86_kernel_check(
		struct opencl_x86_kernel_t *kernel);

cl_int opencl_x86_kernel_set_arg(
		struct opencl_x86_kernel_t *kernel,
		cl_uint arg_index,
		size_t arg_size,
		const void *arg_value);

void opencl_x86_kernel_run(
		struct clcpu_device_t *device,
		struct opencl_x86_kernel_t *kernel,
		cl_uint work_dim,
		const size_t *global_work_offset,
		const size_t *global_work_size,
		const size_t *local_work_size);


#endif
