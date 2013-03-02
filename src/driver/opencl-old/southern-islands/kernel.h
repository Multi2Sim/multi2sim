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

#define SI_OPENCL_KERNEL_ARG_MAX_SIZE 64 

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

enum si_opencl_kernel_arg_mem_type_t
{
	SI_OPENCL_KERNEL_ARG_MEM_TYPE_GLOBAL = 1,
	SI_OPENCL_KERNEL_ARG_MEM_TYPE_EMU_PRIVATE,
	SI_OPENCL_KERNEL_ARG_MEM_TYPE_EMU_LOCAL,
	SI_OPENCL_KERNEL_ARG_MEM_TYPE_UAV,
	SI_OPENCL_KERNEL_ARG_MEM_TYPE_EMU_CONSTANT,
	SI_OPENCL_KERNEL_ARG_MEM_TYPE_EMU_GDS,
	SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_LOCAL,
	SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_PRIVATE,
	SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_CONSTANT,
	SI_OPENCL_KERNEL_ARG_MEM_TYPE_HW_GDS
};

enum si_opencl_kernel_arg_data_type_t
{
	SI_OPENCL_KERNEL_ARG_DATA_TYPE_I1 = 1,
	SI_OPENCL_KERNEL_ARG_DATA_TYPE_I8,
	SI_OPENCL_KERNEL_ARG_DATA_TYPE_I16,
	SI_OPENCL_KERNEL_ARG_DATA_TYPE_I32,
	SI_OPENCL_KERNEL_ARG_DATA_TYPE_I64,
	SI_OPENCL_KERNEL_ARG_DATA_TYPE_FLOAT,
	SI_OPENCL_KERNEL_ARG_DATA_TYPE_DOUBLE,
	SI_OPENCL_KERNEL_ARG_DATA_TYPE_STRUCT,
	SI_OPENCL_KERNEL_ARG_DATA_TYPE_UNION,
	SI_OPENCL_KERNEL_ARG_DATA_TYPE_EVENT,
	SI_OPENCL_KERNEL_ARG_DATA_TYPE_OPAQUE
};

struct si_opencl_image_metadata_7xx_t
{
	int width;
	int height;
	int depth;   /* (for 2D image, this must be 1) */
	int offset;
	int data_type;
	int channel_order;
	int row_pitch;
	int slice_pitch;
};

struct si_opencl_image_metadata_8xx_t
{
	int width;
	int height;
	int depth;   /* (for 2D image, this must be 1) */
	int data_type;
	int recip_width;
	int recip_height;
	int recip_depth;
	int channel_order;
};

struct si_opencl_image_kernel_argument_t
{
	int dimension;  /* 2 or 3 */
	enum si_opencl_kernel_arg_access_type_t access_type;
	int uav;
	int constant_buffer_num;
	int constant_offset;

	union 
	{
		struct si_opencl_image_metadata_7xx_t metadata_7xx;
		struct si_opencl_image_metadata_8xx_t metadata_8xx;
	} metadata;

	int mem_obj_id;
};

struct si_opencl_sampler_argument_t
{
	int id;
	unsigned int location;
	int value;
	int mem_obj_id;
};

struct si_opencl_pass_by_value_argument_t
{
	enum si_opencl_kernel_arg_data_type_t data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;
	unsigned int value[SI_OPENCL_KERNEL_ARG_MAX_SIZE];
};

struct si_opencl_pass_by_pointer_argument_t
{
	enum si_opencl_kernel_arg_data_type_t data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;
	enum si_opencl_kernel_arg_mem_type_t mem_type;
	int buffer_num;
	int alignment;
	int mem_obj_id;
	enum si_opencl_kernel_arg_access_type_t access_type;
};

struct si_opencl_kernel_arg_t
{
	/* Argument properties, as described in .rodata */
	enum si_opencl_kernel_arg_kind_t kind;
	int set;  /* Set to true when it is assigned */
	int size; /* Inferred from data */

	union
	{
		struct si_opencl_pass_by_value_argument_t value;
		struct si_opencl_pass_by_pointer_argument_t pointer;
		struct si_opencl_image_kernel_argument_t image;
		struct si_opencl_sampler_argument_t sampler;
	};

	/* Last field - memory assigned variably */
	char name[0];
};

struct si_opencl_kernel_t
{
	unsigned int id;
	int ref_count;
	unsigned int program_id;
	struct list_t *arg_list;

	/* Excerpts of program ELF binary */
	struct elf_buffer_t metadata_buffer;
	struct elf_buffer_t kernel_buffer;
	struct elf_buffer_t header_buffer;

	/* AMD Kernel binary (internal ELF) */
	struct si_bin_file_t *bin_file;

	/* Kernel function metadata */
	int func_uniqueid;  /* Id of kernel function */

	/* Memory requirements */
	int mem_size_local;
	int mem_size_private;  

	FILE *func_file;
	char func_file_name[MAX_PATH_SIZE];

	/* UAV lists */
	struct list_t *constant_buffer_list;

	/* State of the running kernel */
	struct si_ndrange_t *ndrange;
	
	/* Variable sized declaration */
	char name[0];
};

struct si_opencl_kernel_t *si_opencl_kernel_create(char *name);
void si_opencl_kernel_free(struct si_opencl_kernel_t *kernel);

struct si_opencl_kernel_arg_t *si_opencl_kernel_arg_create(char *name);
void si_opencl_kernel_arg_free(struct si_opencl_kernel_arg_t *arg);

void si_opencl_kernel_load(struct si_opencl_kernel_t *kernel, 
	char *kernel_name);

struct mem_t;
unsigned int si_opencl_kernel_get_work_group_info(
	struct si_opencl_kernel_t *kernel, unsigned int name, 
	struct mem_t *mem, unsigned int addr, unsigned int size);

#endif
