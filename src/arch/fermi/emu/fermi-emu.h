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
#include <assert.h>
#include <string.h>
#include <list.h>
#include <linked-list.h>
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
        FRM_CUDA_OBJ_FUNCTION,
        FRM_CUDA_OBJ_MEMORY
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
	struct frm_grid_t *grid;
};

struct frm_cuda_function_t *frm_cuda_function_create(void);
void frm_cuda_function_free(struct frm_cuda_function_t *function);
struct frm_cuda_function_arg_t *frm_cuda_function_arg_create(char *name);
void frm_cuda_function_arg_free(struct frm_cuda_function_arg_t *arg);
void frm_cuda_function_load(struct frm_cuda_function_t *function, char *function_name);



/*
 * CUDA Grid 
 */

struct frm_grid_t
{
	/* ID */
	char name[MAX_STRING_SIZE];
	int id;  /* Sequential grid ID (given by frm_emu->grid_count counter) */

	/* OpenCL function associated */
	struct frm_cuda_function_t *function;

	/* Pointers to work-groups, warps, and threads */
	struct frm_threadblock_t **threadblocks;
	struct frm_warp_t **warps;
	struct frm_thread_t **threads;

	/* IDs of work-items contained */
	int thread_id_first;
	int thread_id_last;
	int thread_count;

	/* IDs of warps contained */
	int warp_id_first;
	int warp_id_last;
	int warp_count;

	/* IDs of work-groups contained */
	int threadblock_id_first;
	int threadblock_id_last;
	int threadblock_count;
	
	/* Size of work-groups */
	int warps_per_threadblock;  /* = ceil(local_size / frm_emu_warp_size) */

	/* List of pending work-groups */
	struct frm_threadblock_t *pending_list_head;
	struct frm_threadblock_t *pending_list_tail;
	int pending_list_count;
	int pending_list_max;

	/* List of running work-groups */
	struct frm_threadblock_t *running_list_head;
	struct frm_threadblock_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of finished work-groups */
	struct frm_threadblock_t *finished_list_head;
	struct frm_threadblock_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
};

struct frm_grid_t *frm_grid_create(struct frm_cuda_function_t *function);
void frm_grid_free(struct frm_grid_t *grid);
void frm_grid_dump(struct frm_grid_t *grid, FILE *f);

void frm_grid_setup_threads(struct frm_grid_t *grid);
void frm_grid_setup_const_mem(struct frm_grid_t *grid);
void frm_grid_setup_args(struct frm_grid_t *grid);
void frm_grid_run(struct frm_grid_t *grid);




/*
 * GPU Work-Group
 */

enum frm_threadblock_status_t
{
	frm_threadblock_pending		= 0x0001,
	frm_threadblock_running		= 0x0002,
	frm_threadblock_finished		= 0x0004
};

struct frm_threadblock_t
{
	/* ID */
	char name[MAX_STRING_SIZE];
	int id;  /* Group ID */
	int id_3d[3];  /* 3-dimensional Group ID */

	/* Status */
	enum frm_threadblock_status_t status;

	/* NDRange it belongs to */
	struct frm_grid_t *grid;

	/* IDs of work-items contained */
	int thread_id_first;
	int thread_id_last;
	int thread_count;

	/* IDs of warps contained */
	int warp_id_first;
	int warp_id_last;
	int warp_count;

	/* Pointers to warps and work-items */
	struct frm_thread_t **threads;  /* Pointer to first thread in 'function->threads' */
	struct frm_warp_t **warps;  /* Pointer to first warp in 'function->warps' */

	/* Double linked lists of work-groups */
	struct frm_threadblock_t *pending_list_prev;
	struct frm_threadblock_t *pending_list_next;
	struct frm_threadblock_t *running_list_prev;
	struct frm_threadblock_t *running_list_next;
	struct frm_threadblock_t *finished_list_prev;
	struct frm_threadblock_t *finished_list_next;

	/* List of running warps */
	struct frm_warp_t *running_list_head;
	struct frm_warp_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of warps in barrier */
	struct frm_warp_t *barrier_list_head;
	struct frm_warp_t *barrier_list_tail;
	int barrier_list_count;
	int barrier_list_max;

	/* List of finished warps */
	struct frm_warp_t *finished_list_head;
	struct frm_warp_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Fields introduced for architectural simulation */
	int id_in_compute_unit;
	int compute_unit_finished_count;  /* like 'finished_list_count', but when WF reaches Complete stage */

	/* Local memory */
	struct mem_t *local_mem;
};

#define FRM_FOR_EACH_THREADBLOCK_IN_GRID(GRID, THREADBLOCK_ID) \
	for ((THREADBLOCK_ID) = (GRID)->threadblock_id_first; \
		(THREADBLOCK_ID) <= (GRID)->threadblock_id_last; \
		(THREADBLOCK_ID)++)

struct frm_threadblock_t *frm_threadblock_create(void);
void frm_threadblock_free(struct frm_threadblock_t *threadblock);
void frm_threadblock_dump(struct frm_threadblock_t *threadblock, FILE *f);

void frm_threadblock_set_name(struct frm_threadblock_t *threadblock, char *name);

int frm_threadblock_get_status(struct frm_threadblock_t *threadblock, enum frm_threadblock_status_t status);
void frm_threadblock_set_status(struct frm_threadblock_t *threadblock, enum frm_threadblock_status_t status);
void frm_threadblock_clear_status(struct frm_threadblock_t *threadblock, enum frm_threadblock_status_t status);




/*
 * GPU Wavefront
 */

/* Type of clauses */
enum frm_clause_kind_t
{
	FRM_CLAUSE_NONE = 0,
	FRM_CLAUSE_CF,  /* Control-flow */
	FRM_CLAUSE_ALU,  /* ALU clause */
	FRM_CLAUSE_TEX,  /* Fetch trough a Texture Cache Clause */
	FRM_CLAUSE_VC  /* Fetch through a Vertex Cache Clause */
};


#define FRM_MAX_STACK_SIZE  32

/* Wavefront */
struct frm_warp_t
{
	/* ID */
	char name[MAX_STRING_SIZE];
	int id;
	int id_in_threadblock;

	/* IDs of work-items it contains */
	int thread_id_first;
	int thread_id_last;
	int thread_count;

	/* NDRange and Work-group it belongs to */
	struct frm_grid_t *grid;
	struct frm_threadblock_t *threadblock;

	/* Pointer to threads */
	struct frm_thread_t **threads;  /* Pointer to first work-items in 'function->threads' */

	/* Current instructions */
	struct frm_inst_t inst;

	/* Starting/current CF buffer */
	void *buf_start;
	void *buf;

	/* Secondary clause boundaries and current position */
	void *clause_buf;
	void *clause_buf_start;
	void *clause_buf_end;

	/* Active mask stack */
	struct bit_map_t *active_stack;  /* FRM_MAX_STACK_SIZE * thread_count elements */
	int stack_top;

	/* Predicate mask */
	struct bit_map_t *pred;  /* thread_count elements */

	/* Loop counters */
	/* FIXME: Include this as part of the stack to handle nested loops */
	int loop_depth;
	int loop_max_trip_count;
	int loop_trip_count;
	int loop_start;
	int loop_step;
	int loop_index;

	/* Flags updated during instruction execution */
	unsigned int global_mem_read : 1;
	unsigned int global_mem_write : 1;
	unsigned int local_mem_read : 1;
	unsigned int local_mem_write : 1;
	unsigned int pred_mask_update : 1;
	unsigned int push_before_done : 1;  /* Indicates whether the stack has been pushed after PRED_SET* instr. */
	unsigned int active_mask_update : 1;
	int active_mask_push;  /* Number of entries the stack was pushed */
	int active_mask_pop;  /* Number of entries the stack was popped */

	/* Linked lists */
	struct frm_warp_t *running_list_next;
	struct frm_warp_t *running_list_prev;
	struct frm_warp_t *barrier_list_next;
	struct frm_warp_t *barrier_list_prev;
	struct frm_warp_t *finished_list_next;
	struct frm_warp_t *finished_list_prev;

	/* To measure simulation performance */
	long long emu_inst_count;  /* Total emulated instructions */
	long long emu_time_start;
	long long emu_time_end;

	/* Fields introduced for architectural simulation */
	int id_in_compute_unit;
	int alu_engine_in_flight;  /* Number of in-flight uops in ALU engine */
	long long sched_when;  /* GPU cycle when warp was last scheduled */


	/* Periodic report - used by architectural simulation */

	FILE *periodic_report_file;  /* File where report is dumped */
	long long periodic_report_vliw_bundle_count;  /* Number of VLIW bundles (or non-ALU instructions) reported */
	long long periodic_report_cycle;  /* Last cycle when periodic report was updated */
	int periodic_report_inst_count;  /* Number of instructions (VLIW slots) in this interval */
	int periodic_report_local_mem_accesses;  /* Number of local memory accesses in this interval */
	int periodic_report_global_mem_writes;  /* Number of Global memory writes in this interval */
	int periodic_report_global_mem_reads;  /* Number of Global memory reads in this interval */


	/* Statistics */

	long long inst_count;  /* Total number of instructions */
	long long global_mem_inst_count;  /* Instructions (CF or TC) accessing global memory */
	long long local_mem_inst_count;  /* Instructions (ALU) accessing local memory */

	long long cf_inst_count;  /* Number of CF inst executed */
	long long cf_inst_global_mem_write_count;  /* Number of instructions writing to global mem (they are CF inst) */

	long long alu_clause_count;  /* Number of ALU clauses started */
	long long alu_group_count;  /* Number of ALU instruction groups (VLIW) */
	long long alu_group_size[5];  /* Distribution of group sizes (alu_group_size[0] is the number of groups with 1 inst) */
	long long alu_inst_count;  /* Number of ALU instructions */
	long long alu_inst_local_mem_count;  /* Instructions accessing local memory (ALU) */

	long long tc_clause_count;
	long long tc_inst_count;
	long long tc_inst_global_mem_read_count;  /* Number of instructions reading from global mem (they are TC inst) */
};

#define FRM_FOREACH_WARP_IN_GRID(GRID, WARP_ID) \
	for ((WARP_ID) = (GRID)->warp_id_first; \
		(WARP_ID) <= (GRID)->warp_id_last; \
		(WARP_ID)++)

#define FRM_FOREACH_WARP_IN_THREADBLOCK(THREADBLOCK, WARP_ID) \
	for ((WARP_ID) = (THREADBLOCK)->warp_id_first; \
		(WARP_ID) <= (THREADBLOCK)->warp_id_last; \
		(WARP_ID)++)

struct frm_warp_t *frm_warp_create(void);
void frm_warp_free(struct frm_warp_t *warp);
void frm_warp_dump(struct frm_warp_t *warp, FILE *f);

void frm_warp_set_name(struct frm_warp_t *warp, char *name);

void frm_warp_stack_push(struct frm_warp_t *warp);
void frm_warp_stack_pop(struct frm_warp_t *warp, int count);
void frm_warp_execute(struct frm_warp_t *warp);




/*
 * GPU thread (Pixel)
 */

#define FRM_MAX_GPR_ELEM  5
#define FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST  2

struct frm_gpr_t
{
	uint32_t elem[FRM_MAX_GPR_ELEM];  /* x, y, z, w, t */
};

/* Structure describing a memory access definition */
struct frm_mem_access_t
{
	int type;  /* 0-none, 1-read, 2-write */
	uint32_t addr;
	int size;
};

struct frm_thread_t
{
	/* IDs */
	int id;  /* global ID */
	int id_in_warp;
	int id_in_threadblock;  /* local ID */

	/* 3-dimensional IDs */
	int id_3d[3];  /* global 3D IDs */
	int id_in_threadblock_3d[3];  /* local 3D IDs */

	/* Wavefront, work-group, and NDRange where it belongs */
	struct frm_warp_t *warp;
	struct frm_threadblock_t *threadblock;
	struct frm_grid_t *grid;

	/* Work-item state */
	struct frm_gpr_t gpr[128];  /* General purpose registers */
	struct frm_gpr_t pv;  /* Result of last computations */

	/* Linked list of write tasks. They are enqueued by machine instructions
	 * and executed as a burst at the end of an ALU group. */
	struct linked_list_t *write_task_list;

	/* LDS (Local Data Share) OQs (Output Queues) */
	struct list_t *lds_oqa;
	struct list_t *lds_oqb;

	/* This is a digest of the active mask updates for this thread. Every time
	 * an instruction updates the active mask of a warp, this digest is updated
	 * for active threads by XORing a random number common for the warp.
	 * At the end, threads with different 'branch_digest' numbers can be considered
	 * divergent threads. */
	uint32_t branch_digest;

	/* Last global memory access */
	uint32_t global_mem_access_addr;
	uint32_t global_mem_access_size;

	/* Last local memory access */
	int local_mem_access_count;  /* Number of local memory access performed by last instruction */
	uint32_t local_mem_access_addr[FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	uint32_t local_mem_access_size[FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	int local_mem_access_type[FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST];  /* 0-none, 1-read, 2-write */
};

#define FRM_FOREACH_THREAD_IN_GRID(GRID, THREAD_ID) \
	for ((THREAD_ID) = (GRID)->thread_id_first; \
		(THREAD_ID) <= (GRID)->thread_id_last; \
		(THREAD_ID)++)

#define FRM_FOREACH_THREAD_IN_THREADBLOCK(THREADBLOCK, THREAD_ID) \
	for ((THREAD_ID) = (THREADBLOCK)->thread_id_first; \
		(THREAD_ID) <= (THREADBLOCK)->thread_id_last; \
		(THREAD_ID)++)

#define FRM_FOREACH_THREAD_IN_WARP(WARP, THREAD_ID) \
	for ((THREAD_ID) = (WARP)->thread_id_first; \
		(THREAD_ID) <= (WARP)->thread_id_last; \
		(THREAD_ID)++)

struct frm_thread_t *frm_thread_create(void);
void frm_thread_free(struct frm_thread_t *thread);

/* Consult and change active/predicate bits */
void frm_thread_set_active(struct frm_thread_t *thread, int active);
int frm_thread_get_active(struct frm_thread_t *thread);
void frm_thread_set_pred(struct frm_thread_t *thread, int pred);
int frm_thread_get_pred(struct frm_thread_t *thread);
void frm_thread_update_branch_digest(struct frm_thread_t *thread,
	long long inst_count, uint32_t inst_addr);



/*
 * Fermi ISA
 */

///* Global variables referring to the instruction that is currently being emulated.
// * There variables are set before calling the instruction emulation function in
// * 'machine.c' to avoid passing pointers. */
//extern struct frm_ndrange_t *frm_isa_ndrange;
//extern struct frm_work_group_t *frm_isa_work_group;
//extern struct frm_wavefront_t *frm_isa_wavefront;
//extern struct frm_work_item_t *frm_isa_work_item;
//extern struct frm_inst_t *frm_isa_cf_inst;
//extern struct frm_inst_t *frm_isa_inst;
//extern struct frm_alu_group_t *frm_isa_alu_group;
//
///* Macros for quick access */
//#define EVG_GPR_ELEM(_gpr, _elem)  (frm_isa_work_item->gpr[(_gpr)].elem[(_elem)])
//#define EVG_GPR_X(_gpr)  EVG_GPR_ELEM((_gpr), 0)
//#define EVG_GPR_Y(_gpr)  EVG_GPR_ELEM((_gpr), 1)
//#define EVG_GPR_Z(_gpr)  EVG_GPR_ELEM((_gpr), 2)
//#define EVG_GPR_W(_gpr)  EVG_GPR_ELEM((_gpr), 3)
//#define EVG_GPR_T(_gpr)  EVG_GPR_ELEM((_gpr), 4)
//
//#define EVG_GPR_FLOAT_ELEM(_gpr, _elem)  (* (float *) &frm_isa_work_item->gpr[(_gpr)].elem[(_elem)])
//#define EVG_GPR_FLOAT_X(_gpr)  EVG_GPR_FLOAT_ELEM((_gpr), 0)
//#define EVG_GPR_FLOAT_Y(_gpr)  EVG_GPR_FLOAT_ELEM((_gpr), 1)
//#define EVG_GPR_FLOAT_Z(_gpr)  EVG_GPR_FLOAT_ELEM((_gpr), 2)
//#define EVG_GPR_FLOAT_W(_gpr)  EVG_GPR_FLOAT_ELEM((_gpr), 3)
//#define EVG_GPR_FLOAT_T(_gpr)  EVG_GPR_FLOAT_ELEM((_gpr), 4)
//
//
///* Debugging */
//#define frm_isa_debugging() debug_status(frm_isa_debug_category)
//#define frm_isa_debug(...) debug(frm_isa_debug_category, __VA_ARGS__)
//extern int frm_isa_debug_category;
//
//
///* Macros for unsupported parameters */
//extern char *err_frm_isa_note;
/*
#define EVG_ISA_ARG_NOT_SUPPORTED(p) \
	fatal("%s: %s: not supported for '" #p "' = 0x%x\n%s", \
	__FUNCTION__, frm_isa_inst->info->name, (p), err_frm_isa_note);
#define EVG_ISA_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: %s: not supported for '" #p "' != 0x%x\n%s", \
	__FUNCTION__, frm_isa_inst->info->name, (v), err_frm_isa_note); }
#define EVG_ISA_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: %s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	__FUNCTION__, frm_isa_inst->info->name, (min), (max), err_frm_opencl_param_note); }
*/
//
///* Macros for fast access of instruction words */
//#define EVG_CF_WORD0			frm_isa_inst->words[0].cf_word0
//#define EVG_CF_GWS_WORD0		frm_isa_inst->words[0].cf_gws_word0
//#define EVG_CF_WORD1			frm_isa_inst->words[1].cf_word1
//
//#define EVG_CF_ALU_WORD0		frm_isa_inst->words[0].cf_alu_word0
//#define EVG_CF_ALU_WORD1		frm_isa_inst->words[1].cf_alu_word1
//#define EVG_CF_ALU_WORD0_EXT		frm_isa_inst->words[0].cf_alu_word0_ext
//#define EVG_CF_ALU_WORD1_EXT		frm_isa_inst->words[1].cf_alu_word1_ext
//
//#define EVG_CF_ALLOC_EXPORT_WORD0	frm_isa_inst->words[0].cf_alloc_export_word0
//#define EVG_CF_ALLOC_EXPORT_WORD0_RAT	frm_isa_inst->words[0].cf_alloc_export_word0_rat
//#define EVG_CF_ALLOC_EXPORT_WORD1_BUF	frm_isa_inst->words[1].cf_alloc_export_word1_buf
//#define EVG_CF_ALLOC_EXPORT_WORD1_SWIZ	frm_isa_inst->words[1].cf_alloc_export_word1_swiz
//
//#define EVG_ALU_WORD0			frm_isa_inst->words[0].alu_word0
//#define EVG_ALU_WORD1_OP2		frm_isa_inst->words[1].alu_word1_op2
//#define EVG_ALU_WORD1_OP3		frm_isa_inst->words[1].alu_word1_op3
//
//#define EVG_ALU_WORD0_LDS_IDX_OP	frm_isa_inst->words[0].alu_word0_lds_idx_op
//#define EVG_ALU_WORD1_LDS_IDX_OP	frm_isa_inst->words[1].alu_word1_lds_idx_op
//
//#define EVG_VTX_WORD0			frm_isa_inst->words[0].vtx_word0
//#define EVG_VTX_WORD1_GPR		frm_isa_inst->words[1].vtx_word1_gpr
//#define EVG_VTX_WORD1_SEM		frm_isa_inst->words[1].vtx_word1_sem
//#define EVG_VTX_WORD2			frm_isa_inst->words[2].vtx_word2
//
//#define EVG_TEX_WORD0			frm_isa_inst->words[0].tex_word0
//#define EVG_TEX_WORD1			frm_isa_inst->words[1].tex_word1
//#define EVG_TEX_WORD2			frm_isa_inst->words[2].tex_word2
//
//#define EVG_MEM_RD_WORD0		frm_isa_inst->words[0].mem_rd_word0
//#define EVG_MEM_RD_WORD1		frm_isa_inst->words[1].mem_rd_word1
//#define EVG_MEM_RD_WORD2		frm_isa_inst->words[2].mem_rd_word2
//
//#define EVG_MEM_GDS_WORD0		frm_isa_inst->words[0].mem_gds_word0
//#define EVG_MEM_GDS_WORD1		frm_isa_inst->words[1].mem_gds_word1
//#define EVG_MEM_GDS_WORD2		frm_isa_inst->words[2].mem_gds_word2
//
//
///* List of functions implementing GPU instructions 'amd_inst_XXX_impl' */
//typedef void (*frm_isa_inst_func_t)(void);
//extern frm_isa_inst_func_t *frm_isa_inst_func;
//
/* Access to constant memory */
void frm_isa_const_mem_write(int bank, int vector, int elem, void *pvalue);
void frm_isa_const_mem_read(int bank, int vector, int elem, void *pvalue);
//
///* For ALU clauses */
//void frm_isa_alu_clause_start(void);
//void frm_isa_alu_clause_end(void);
//
///* For TC clauses */
//void frm_isa_tc_clause_start(void);
//void frm_isa_tc_clause_end(void);
//
///* For functional simulation */
//unsigned int frm_isa_read_gpr(int gpr, int rel, int chan, int im);
//float frm_isa_read_gpr_float(int gpr, int rel, int chan, int im);
//void frm_isa_write_gpr(int gpr, int rel, int chan, uint32_t value);
//void frm_isa_write_gpr_float(int gpr, int rel, int chan, float value);
//
//unsigned int frm_isa_read_op_src_int(int src_idx);
//float frm_isa_read_op_src_float(int src_idx);
//
//void frm_isa_init(void);
//void frm_isa_done(void);















/*
 * Evergreen GPU Emulator
 */

struct frm_emu_t
{
	/* Constant memory (constant buffers)
	 * There are 15 constant buffers, referenced as CB0 to CB14.
	 * Each buffer can hold up to 1024 four-component vectors.
	 * These buffers will be represented as a memory object indexed as
	 *   buffer_id * 1024 * 4 * 4 + vector_id * 4 * 4 + elem_id * 4
	 */
	struct mem_t *const_mem;

	/* Flags indicating whether the first 9 vector positions of CB0
	 * are initialized. A warning will be issued by the simulator
	 * if an uninitialized element is used by the kernel. */
	int const_mem_cb0_init[9 * 4];

	/* Global memory */
	struct mem_t *global_mem;
	unsigned int global_mem_top;

	/* Timer */
	int timer_running;  /* Current timer state */
	long long timer_start_time;  /* Last time (as per x86_emu_timer) when on */
	long long timer_acc;  /* Accumulated time in previous on-off cycles */

	/* Stats */
	int grid_count;  /* Number of OpenCL kernels executed */
	long long inst_count;  /* Number of instructions executed by warps */
};


extern enum frm_emu_kind_t
{
	frm_emu_functional,
	frm_emu_detailed
} frm_emu_kind;

extern long long frm_emu_max_cycles;
extern long long frm_emu_max_inst;
extern int frm_emu_max_kernels;

extern char *frm_emu_cuda_binary_name;
extern char *frm_emu_report_file_name;
extern FILE *frm_emu_report_file;

extern int frm_emu_warp_size;

extern char *err_frm_cuda_note;
extern char *err_frm_cuda_param_note;


extern struct frm_emu_t *frm_emu;

void frm_emu_init(void);
void frm_emu_done(void);

void frm_emu_timer_start(void);
void frm_emu_timer_stop(void);
long long frm_emu_timer(void);

void frm_emu_libcuda_redirect(char *path, int size);
void frm_emu_libcuda_failed(int pid);

void frm_emu_disasm(char *path);

#endif

