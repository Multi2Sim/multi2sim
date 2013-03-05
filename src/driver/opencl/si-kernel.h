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

#ifndef DRIVER_OPENCL_SI_KERNEL_H
#define DRIVER_OPENCL_SI_KERNEL_H

#include <lib/util/elf-format.h>


/*
 * Kernel List
 */

extern struct list_t *opencl_si_kernel_list;

void opencl_si_kernel_list_init(void);
void opencl_si_kernel_list_done(void);




/*
 * Argument
 */

enum opencl_si_arg_type_t
{
	opencl_si_arg_type_invalid = 0,
	opencl_si_arg_value,
	opencl_si_arg_pointer,
	opencl_si_arg_image,
	opencl_si_arg_sampler
};

enum opencl_si_arg_access_type_t
{
	opencl_si_arg_access_type_invalid = 0,
	opencl_si_arg_read_only,
	opencl_si_arg_write_only,
	opencl_si_arg_read_write
};

enum opencl_si_arg_scope_t
{
	opencl_si_arg_scope_invalid = 0,
	opencl_si_arg_global,
	opencl_si_arg_emu_private,
	opencl_si_arg_emu_local,
	opencl_si_arg_uav,
	opencl_si_arg_emu_constant,
	opencl_si_arg_emu_gds,
	opencl_si_arg_hw_local,
	opencl_si_arg_hw_private,
	opencl_si_arg_hw_constant,
	opencl_si_arg_hw_gds
};

enum opencl_si_arg_data_type_t
{
	opencl_si_arg_data_type_invalid = 0,
	opencl_si_arg_i1,
	opencl_si_arg_i8,
	opencl_si_arg_i16,
	opencl_si_arg_i32,
	opencl_si_arg_i64,
	opencl_si_arg_u1,
	opencl_si_arg_u8,
	opencl_si_arg_u16,
	opencl_si_arg_u32,
	opencl_si_arg_u64,
	opencl_si_arg_float,
	opencl_si_arg_double,
	opencl_si_arg_struct,
	opencl_si_arg_union,
	opencl_si_arg_event,
	opencl_si_arg_opaque
};

struct opencl_si_arg_value_t
{
	/* Metadata info */
	enum opencl_si_arg_data_type_t data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;

	/* Value set by user */
	void *value_ptr;
};

struct opencl_si_arg_pointer_t
{
	/* Metadata info */
	enum opencl_si_arg_data_type_t data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;
	enum opencl_si_arg_scope_t scope;
	int buffer_num;
	int alignment;
	enum opencl_si_arg_access_type_t access_type;

	/* Value set by user */
	unsigned int device_ptr;
};

struct opencl_si_arg_image_t
{
	int dimension;  /* 2 or 3 */
	enum opencl_si_arg_access_type_t access_type;
	int uav;
	int constant_buffer_num;
	int constant_offset;
};

struct opencl_si_arg_sampler_t
{
	int id;
	unsigned int location;
	int value;
};

struct opencl_si_arg_t
{
	enum opencl_si_arg_type_t type;
	char *name;
	int set;  /* Set to true when it is assigned */
	int size; /* Inferred from metadata or user calls */

	union
	{
		struct opencl_si_arg_value_t value;
		struct opencl_si_arg_pointer_t pointer;
		struct opencl_si_arg_image_t image;
		struct opencl_si_arg_sampler_t sampler;
	};
};

struct opencl_si_arg_t *opencl_si_arg_create(enum opencl_si_arg_type_t type,
		char *name);
void opencl_si_arg_free(struct opencl_si_arg_t *arg);



/*
 * Kernel
 */


struct opencl_si_kernel_t
{
	int id;
	char *name;

	/* Program that kernel belongs to */
	struct opencl_si_program_t *program;

	/* List of kernel arguments. Each element of the list is of type
	 * 'struct opencl_si_arg_t'. */
	struct list_t *arg_list;

	/* Excerpts of program binary */
	struct elf_buffer_t metadata_buffer;
	struct elf_buffer_t header_buffer;
	struct elf_buffer_t kernel_buffer;

	/* Memory requirements */
	int mem_size_local;
	int mem_size_private;

	/* Kernel function metadata */
	int func_uniqueid;  /* Id of kernel function */
};

struct opencl_si_kernel_t *opencl_si_kernel_create(struct opencl_si_program_t *program,
		char *name);
void opencl_si_kernel_free(struct opencl_si_kernel_t *kernel);

#endif

