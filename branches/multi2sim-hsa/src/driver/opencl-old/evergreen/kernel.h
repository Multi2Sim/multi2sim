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

#ifndef ARCH_EVERGREEN_EMU_KERNEL_H
#define ARCH_EVERGREEN_EMU_KERNEL_H

#include <lib/class/class.h>
#include <lib/util/string.h>
#include <lib/util/elf-format.h>


enum evg_opencl_mem_scope_t
{
	EVG_OPENCL_MEM_SCOPE_NONE = 0,
	EVG_OPENCL_MEM_SCOPE_GLOBAL,
	EVG_OPENCL_MEM_SCOPE_LOCAL,
	EVG_OPENCL_MEM_SCOPE_PRIVATE,
	EVG_OPENCL_MEM_SCOPE_CONSTANT
};

enum evg_opencl_kernel_arg_kind_t
{
	EVG_OPENCL_KERNEL_ARG_KIND_VALUE = 1,
	EVG_OPENCL_KERNEL_ARG_KIND_POINTER,
	EVG_OPENCL_KERNEL_ARG_KIND_IMAGE,
	EVG_OPENCL_KERNEL_ARG_KIND_SAMPLER
};

enum evg_opencl_kernel_arg_access_type_t
{
	EVG_OPENCL_KERNEL_ARG_READ_ONLY = 1,
	EVG_OPENCL_KERNEL_ARG_WRITE_ONLY,
	EVG_OPENCL_KERNEL_ARG_READ_WRITE
};

enum evg_arg_data_type_t 
{               
        evg_arg_data_type_invalid = 0,
        evg_arg_i1,
        evg_arg_i8,
        evg_arg_i16,
        evg_arg_i32,
        evg_arg_i64,
        evg_arg_u1,
        evg_arg_u8,
        evg_arg_u16,
        evg_arg_u32,
        evg_arg_u64,
        evg_arg_float,
        evg_arg_double,
        evg_arg_struct,
        evg_arg_union,
        evg_arg_event,
        evg_arg_opaque
}; 


struct evg_opencl_kernel_arg_t
{
	/* Argument properties, as described in .rodata */
	enum evg_opencl_kernel_arg_kind_t kind;
	enum evg_opencl_mem_scope_t mem_scope;  /* For pointers */
	int uav;  /* For memory objects */
	enum evg_opencl_kernel_arg_access_type_t access_type;
	enum evg_arg_data_type_t data_type;

	/* Argument fields as set in clSetKernelArg */
	int set;  /* Set to true when it is assigned */
	unsigned int value[32];  /* Support up to 128-bit arguments */
	unsigned int size;

	/* Storage related */
	unsigned int constant_buffer_num;
	unsigned int constant_offset;

	/* Last field - memory assigned variably */
	char name[0];
};

struct evg_opencl_kernel_t
{
	unsigned int id;
	int ref_count;
	unsigned int program_id;
	char name[MAX_STRING_SIZE];
	struct list_t *arg_list;

	OpenclOldDriver *driver;

	/* Excerpts of program ELF binary */
	struct elf_buffer_t metadata_buffer;
	struct elf_buffer_t kernel_buffer;
	struct elf_buffer_t header_buffer;

	/* AMD Kernel binary (internal ELF) */
	struct evg_bin_file_t *bin_file;

	/* Kernel function metadata */
	int func_uniqueid;  /* Id of kernel function */
	int func_mem_local;  /* Local memory usage */
	FILE *func_file;
	char func_file_name[MAX_PATH_SIZE];

	/* Number of work dimensions */
	int work_dim;

	/* 3D Counters */
	int global_size3[3];  /* Total number of work_items */
	int local_size3[3];  /* Number of work_items in a group */
	int group_count3[3];  /* Number of work_item groups */

	/* 1D Counters. Each counter is equal to the multiplication
	 * of each component in the corresponding 3D counter. */
	int global_size;
	int local_size;
	int group_count;

	/* UAV lists */
	struct list_t *uav_list;
	struct list_t *constant_buffer_list;

	/* State of the running kernel */
	EvgNDRange *ndrange;
};

struct evg_opencl_kernel_t *evg_opencl_kernel_create(OpenclOldDriver *driver);
void evg_opencl_kernel_free(struct evg_opencl_kernel_t *kernel);

struct evg_opencl_kernel_arg_t *evg_opencl_kernel_arg_create(char *name);
void evg_opencl_kernel_arg_free(struct evg_opencl_kernel_arg_t *arg);

void evg_opencl_kernel_load(struct evg_opencl_kernel_t *kernel, char *kernel_name);

struct mem_t;
unsigned int evg_opencl_kernel_get_work_group_info(struct evg_opencl_kernel_t *kernel, unsigned int name,
	struct mem_t *mem, unsigned int addr, unsigned int size);


#endif

