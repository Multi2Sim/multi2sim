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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_OPENCL_KERNEL_H
#define ARCH_SOUTHERN_ISLANDS_EMU_OPENCL_KERNEL_H

#include <lib/util/string.h>
#include <lib/util/elf-format.h>


enum si_opencl_mem_scope_t
{
	SI_OPENCL_MEM_SCOPE_NONE = 0,
	SI_OPENCL_MEM_SCOPE_GLOBAL,
	SI_OPENCL_MEM_SCOPE_LOCAL,
	SI_OPENCL_MEM_SCOPE_PRIVATE,
	SI_OPENCL_MEM_SCOPE_CONSTANT
};

enum si_opencl_kernel_arg_kind_t
{
	SI_OPENCL_KERNEL_ARG_KIND_VALUE = 1,
	SI_OPENCL_KERNEL_ARG_KIND_POINTER,
	SI_OPENCL_KERNEL_ARG_KIND_IMAGE,
	SI_OPENCL_KERNEL_ARG_KIND_SAMPLER
};

enum si_opencl_kernel_arg_access_type_t
{
	SI_OPENCL_KERNEL_ARG_READ_ONLY = 1,
	SI_OPENCL_KERNEL_ARG_WRITE_ONLY,
	SI_OPENCL_KERNEL_ARG_READ_WRITE
};


struct si_opencl_kernel_arg_t
{
	/* Argument properties, as described in .rodata */
	enum si_opencl_kernel_arg_kind_t kind;
	enum si_opencl_mem_scope_t mem_scope;  /* For pointers */
	int uav;  /* For memory objects */
	enum si_opencl_kernel_arg_access_type_t access_type;

	/* Argument fields as set in clSetKernelArg */
	int set;  /* Set to true when it is assigned */
	unsigned int size;

	union
	{
		unsigned int ptr;
		unsigned int value[4];
	} data;

	/* Last field - memory assigned variably */
	char name[0];
};

struct si_opencl_kernel_t
{
	unsigned int id;
	int ref_count;
	unsigned int program_id;
	char name[MAX_STRING_SIZE];
	struct list_t *arg_list;

	/* Excerpts of program ELF binary */
	struct elf_buffer_t metadata_buffer;
	struct elf_buffer_t kernel_buffer;
	struct elf_buffer_t header_buffer;

	/* AMD Kernel binary (internal ELF) */
	struct si_bin_file_t *bin_file;

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
	struct list_t *constant_buffer_list;

	/* State of the running kernel */
	struct si_ndrange_t *ndrange;
};

struct si_opencl_kernel_t *si_opencl_kernel_create(void);
void si_opencl_kernel_free(struct si_opencl_kernel_t *kernel);

struct si_opencl_kernel_arg_t *si_opencl_kernel_arg_create(char *name);
void si_opencl_kernel_arg_free(struct si_opencl_kernel_arg_t *arg);

void si_opencl_kernel_load(struct si_opencl_kernel_t *kernel, char *kernel_name);

struct mem_t;
unsigned int si_opencl_kernel_get_work_group_info(struct si_opencl_kernel_t *kernel, unsigned int name,
	struct mem_t *mem, unsigned int addr, unsigned int size);

#endif
