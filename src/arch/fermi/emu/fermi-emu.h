/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FERMI_EMU_H
#define FERMI_EMU_H

#include <stdio.h>
#include <string.h>
#include <list.h>
#include <debug.h>
#include <misc.h>
#include <elf-format.h>

#include <fermi-asm.h>



/*
 * CUDA Data Structures
 */

/* CUDA objects */

enum frm_cuda_obj_t
{
        FRM_CUDA_OBJ_DEVICE = 1,
        FRM_CUDA_OBJ_CONTEXT,
        FRM_CUDA_OBJ_STREAM,
        FRM_CUDA_OBJ_MODULE,
        FRM_CUDA_OBJ_FUNCTION
};

extern struct linked_list_t *frm_cuda_object_list;

void frm_cuda_object_add(void *object);
void frm_cuda_object_remove(void *object);
void *frm_cuda_object_get(enum frm_cuda_obj_t type, unsigned int id);
void *frm_cuda_object_get_type(enum frm_cuda_obj_t type);
unsigned int frm_cuda_object_new_id(enum frm_cuda_obj_t type);
void frm_cuda_object_free_all(void);


/* Device */

struct frm_cuda_device_t
{
	unsigned int id;
};

struct frm_cuda_device_t *frm_cuda_device_create(void);
void frm_cuda_device_free(struct frm_cuda_device_t *device);


/* Context */

struct frm_cuda_context_t
{
	unsigned int id;
	int ref_count;

	unsigned int device_id;
};

struct frm_cuda_context_t *frm_cuda_context_create(void);
void frm_cuda_context_free(struct frm_cuda_context_t *context);


/* Stream */

struct frm_cuda_stream_t
{
	unsigned int id;
	int ref_count;

	unsigned int device_id;
	unsigned int context_id;
	unsigned int properties;
};

struct frm_cuda_stream_t *frm_cuda_stream_create(void);
void frm_cuda_stream_free(struct frm_cuda_stream_t *stream);


/* Module */

struct frm_cuda_module_t
{
	unsigned int id;
	int ref_count;

	unsigned int device_id;  /* Only one device allowed */
	unsigned int context_id;

	/* ELF binary */
	struct elf_file_t *elf_file;
};

struct frm_cuda_module_t *frm_cuda_module_create(void);
void frm_cuda_module_free(struct frm_cuda_module_t *module);

void frm_cuda_module_build(struct frm_cuda_module_t *module);


/* Function argument */

enum frm_cuda_mem_scope_t
{
	FRM_CUDA_MEM_SCOPE_NONE = 0,
	FRM_CUDA_MEM_SCOPE_GLOBAL,
	FRM_CUDA_MEM_SCOPE_LOCAL,
	FRM_CUDA_MEM_SCOPE_PRIVATE,
	FRM_CUDA_MEM_SCOPE_CONSTANT
};

enum frm_cuda_function_arg_kind_t
{
	FRM_CUDA_FUNCTION_ARG_KIND_VALUE = 1,
	FRM_CUDA_FUNCTION_ARG_KIND_POINTER,
	FRM_CUDA_FUNCTION_ARG_KIND_IMAGE,
	FRM_CUDA_FUNCTION_ARG_KIND_SAMPLER
};

enum frm_cuda_function_arg_access_type_t
{
	FRM_CUDA_FUNCTION_ARG_READ_ONLY = 1,
	FRM_CUDA_FUNCTION_ARG_WRITE_ONLY,
	FRM_CUDA_FUNCTION_ARG_READ_WRITE
};

struct frm_cuda_function_arg_t
{
	/* Argument properties, as described in .rodata */
	enum frm_cuda_function_arg_kind_t kind;
	enum frm_cuda_mem_scope_t mem_scope;  /* For pointers */
	int uav;  /* For memory objects */
	enum frm_cuda_function_arg_access_type_t access_type;

	/* Argument fields as set in clSetKernelArg */
	int set;  /* Set to true when it is assigned */
	uint32_t value;  /* 32-bit arguments supported */
	uint32_t size;

	/* Last field - memory assigned variably */
	char name[0];
};


/* Function */

struct frm_cuda_function_t
{
	int id;
	int ref_count;
	unsigned int module_id;
	char name[MAX_STRING_SIZE];
	struct list_t *arg_list;

	/* Excerpts of module ELF binary */
	struct elf_buffer_t function_buffer;

	/* Kernel function metadata */
	int func_uniqueid;  /* Id of function function */
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

	/* State of the running function */
	struct frm_ndrange_t *ndrange;
};

struct frm_cuda_function_t *frm_cuda_function_create(void);
void frm_cuda_function_free(struct frm_cuda_function_t *function);
struct frm_cuda_function_arg_t *frm_cuda_function_arg_create(char *name);
void frm_cuda_function_arg_free(struct frm_cuda_function_arg_t *arg);
void frm_cuda_function_load(struct frm_cuda_function_t *function, char *function_name);



void frm_emu_init(void);
void frm_emu_done(void);
void frm_emu_disasm(char *path);

#endif

