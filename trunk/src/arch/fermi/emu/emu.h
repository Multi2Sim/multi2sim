/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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
#include <assert.h>
#include <string.h>

#include <arch/fermi/asm/asm.h>
#include <arch/x86/emu/context.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/string.h>



#define frm_cuda_debug(...) debug(frm_cuda_debug_category, __VA_ARGS__)
extern int frm_cuda_debug_category;




/*
 * CUDA call
 */

int frm_cuda_call(struct x86_ctx_t *ctx);





/*
 * CUDA Data Structures
 */


/* Objects */

enum frm_cuda_obj_t
{
        FRM_CUDA_OBJ_DEVICE = 1,
        FRM_CUDA_OBJ_CONTEXT,
        FRM_CUDA_OBJ_MODULE,
        FRM_CUDA_OBJ_FUNCTION,
        FRM_CUDA_OBJ_MEMORY,
        FRM_CUDA_OBJ_STREAM
};

extern struct linked_list_t *frm_cuda_object_list;

void frm_cuda_object_add(void *object);
void frm_cuda_object_remove(void *object);
void *frm_cuda_object_get(enum frm_cuda_obj_t type, unsigned int id);
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
	FRM_CUDA_FUNCTION_ARG_KIND_POINTER
};

enum frm_cuda_function_arg_access_type_t
{
	FRM_CUDA_FUNCTION_ARG_READ_ONLY = 1,
	FRM_CUDA_FUNCTION_ARG_WRITE_ONLY,
	FRM_CUDA_FUNCTION_ARG_READ_WRITE
};

struct frm_cuda_function_arg_t
{
	int id;
	char name[MAX_STRING_SIZE];

	enum frm_cuda_function_arg_kind_t kind;
	enum frm_cuda_mem_scope_t mem_scope;
	enum frm_cuda_function_arg_access_type_t access_type;

	unsigned int value;
};

struct frm_cuda_function_arg_t *frm_cuda_function_arg_create(char *name);
void frm_cuda_function_arg_free(struct frm_cuda_function_arg_t *arg);


/* Function */

struct frm_cuda_function_t
{
	int id;
	char name[MAX_STRING_SIZE];
	int ref_count;

	unsigned int module_id;
	struct list_t *arg_list;

	/* FIXME */
	struct elf_buffer_t function_buffer;

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
	struct frm_grid_t *grid;
};

struct frm_cuda_function_t *frm_cuda_function_create(void);
void frm_cuda_function_free(struct frm_cuda_function_t *function);
void frm_cuda_function_load(struct frm_cuda_function_t *function, char *function_name);


/* Memory */

struct frm_cuda_memory_t
{
        unsigned int id;
        int ref_count;

        unsigned int size;
        unsigned int host_ptr;
        unsigned int device_ptr;
};

struct frm_cuda_memory_t *frm_cuda_memory_create(void);
void frm_cuda_memory_free(struct frm_cuda_memory_t *mem);


/* Stream */

struct frm_cuda_stream_t
{
	unsigned int id;
	int ref_count;

	unsigned int device_id;
	unsigned int context_id;
};

struct frm_cuda_stream_t *frm_cuda_stream_create(void);
void frm_cuda_stream_free(struct frm_cuda_stream_t *stream);



/* Write */
void frm_isa_enqueue_write_dest(unsigned int value);





/*
 * Fermi GPU Emulator
 */


struct frm_emu_t
{
        /* List of ND-Ranges */
        struct frm_grid_t *grid_list_head;
        struct frm_grid_t *grid_list_tail;
        int grid_list_count;
        int grid_list_max;

        /* List of pending ND-Ranges */
        struct frm_grid_t *pending_grid_list_head;
        struct frm_grid_t *pending_grid_list_tail;
        int pending_grid_list_count;
        int pending_grid_list_max;

        /* List of running ND-Ranges */
        struct frm_grid_t *running_grid_list_head;
        struct frm_grid_t *running_grid_list_tail;
        int running_grid_list_count;
        int running_grid_list_max;

        /* List of finished ND-Ranges */
        struct frm_grid_t *finished_grid_list_head;
        struct frm_grid_t *finished_grid_list_tail;
        int finished_grid_list_count;
        int finished_grid_list_max;

	/* Constant memory, which is organized as 16 banks of 64KB each. */
	struct mem_t *const_mem;

	/* Flags indicating whether the first 32 bytes of constant memory
	 * are initialized. A warning will be issued by the simulator
	 * if an uninitialized element is used by the kernel. */
	int const_mem_init[32];

	/* Global memory */
	struct mem_t *global_mem;
	unsigned int global_mem_top;
	unsigned int free_global_mem_size;
	unsigned int total_global_mem_size;

	/* Stats */
	int grid_count;  /* Number of CUDA functions executed */
	long long inst_count;  /* Number of instructions executed by warps */
};

extern enum frm_emu_kind_t
{
	frm_emu_kind_functional,
	frm_emu_kind_detailed
} frm_emu_kind;

extern long long frm_emu_max_cycles;
extern long long frm_emu_max_inst;
extern int frm_emu_max_kernels;
extern char *frm_emu_cuda_binary_name;
extern char *frm_emu_report_file_name;
extern FILE *frm_emu_report_file;
extern int frm_emu_warp_size;
extern char *err_frm_cuda_note;
extern struct frm_emu_t *frm_emu;

void frm_emu_init(void);
void frm_emu_done(void);

void frm_emu_libcuda_redirect(char *path, int size);
void frm_emu_libcuda_failed(int pid);

void frm_emu_disasm(char *path);

#endif

