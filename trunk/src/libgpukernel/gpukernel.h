/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#ifndef GPUKERNEL_H
#define GPUKERNEL_H

#include <mhandle.h>
#include <debug.h>
#include <config.h>
#include <list.h>
#include <lnlist.h>
#include <misc.h>
#include <stdio.h>
#include <unistd.h>
#include <elf.h>
#include <assert.h>
#include <gpudisasm.h>



/*
 * Global variables
 */


extern char *gpu_opencl_binary_name;
extern char *gpu_report_file_name;
extern FILE *gpu_report_file;

/* Architectural parameters that need to be introduced in the GPU functional
 * simulator, because they are returned by OpenCL functions or affect the
 * GPU emulation. */

extern int gpu_wavefront_size;
extern int gpu_max_work_group_size;


/* Error messages */

extern char *err_opencl_note;
extern char *err_opencl_param_note;




/*
 * CAL Application Binary Interface (ABI)
 */

struct cal_abi_t
{
	/* File as a string and elf file descriptor */
	char file_name[MAX_PATH_SIZE];
	struct elf_file_t *elf;

	/* Encoding dictionary */
	int enc_dict_phdr_idx;  /* Index for encoding dictionary in program header table */
	Elf32_Phdr *enc_dict_phdr;
	int enc_dict_entry_count;  /* Number of encoding dictionary entries */
	int enc_dict_entry_idx;  /* Index of encoding dictionary entry with d_machine=0x9 */

	/* PT_NOTE segment for encoding dictionary with d_machine=0x9 */
	Elf32_Phdr *pt_note_phdr;
	void *pt_note_buffer;

	/* PT_LOAD segment for encoding dictionary with d_machine=0x9*/
	Elf32_Phdr *pt_load_phdr;
	void *pt_load_buffer;

	/* Sections within PT_LOAD segment */
	Elf32_Shdr *text_shdr, *data_shdr, *symtab_shdr, *strtab_shdr;
	void *text_buffer, *data_buffer, *symtab_buffer, *strtab_buffer;  /* Pointers to 'pt_load_buffer' */
};

void cal_abi_parse_elf(struct cal_abi_t *cal_abi, char *file_name);
struct cal_abi_t *cal_abi_create();
void cal_abi_free(struct cal_abi_t *cal_abi);




/*
 * OpenCL API Implementation
 */

/* Debugging */
#define opencl_debug(...) debug(opencl_debug_category, __VA_ARGS__)
extern int opencl_debug_category;

/* Some constants */
#define OPENCL_FUNC_FIRST  1000
#define OPENCL_FUNC_LAST  1073
#define OPENCL_MAX_ARGS  14

/* An enumeration of the OpenCL functions */
enum opencl_func_enum {
#define DEF_OPENCL_FUNC(_name, _argc) OPENCL_FUNC_##_name,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	OPENCL_FUNC_COUNT
};

/* List of OpenCL functions and number of arguments */
extern char *opencl_func_names[];
extern int opencl_func_argc[];

/* Execute OpenCL call */
int opencl_func_run(int code, unsigned int *args);



/* OpenCL objects */

enum opencl_obj_enum {
	OPENCL_OBJ_PLATFORM = 1,
	OPENCL_OBJ_DEVICE,
	OPENCL_OBJ_CONTEXT,
	OPENCL_OBJ_COMMAND_QUEUE,
	OPENCL_OBJ_PROGRAM,
	OPENCL_OBJ_KERNEL,
	OPENCL_OBJ_MEM,
	OPENCL_OBJ_EVENT
};

extern struct lnlist_t *opencl_object_list;

void opencl_object_add(void *object);
void opencl_object_remove(void *object);
void *opencl_object_get(enum opencl_obj_enum type, uint32_t id);
void *opencl_object_get_type(enum opencl_obj_enum type);
uint32_t opencl_object_new_id(enum opencl_obj_enum type);
void opencl_object_free_all(void);




/* OpenCL platform */

struct opencl_platform_t
{
	uint32_t id;
};

struct mem_t;  /* Forward declaration */

extern struct opencl_platform_t *opencl_platform;

struct opencl_platform_t *opencl_platform_create(void);
void opencl_platform_free(struct opencl_platform_t *platform);
uint32_t opencl_platform_get_info(struct opencl_platform_t *platform, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);




/* OpenCL devices */

struct opencl_device_t
{
	uint32_t id;
};

struct opencl_device_t *opencl_device_create(void);
void opencl_device_free(struct opencl_device_t *device);
uint32_t opencl_device_get_info(struct opencl_device_t *device, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);




/* OpenCL contexts */

struct opencl_context_t
{
	uint32_t id;
	int ref_count;

	uint32_t platform_id;
	uint32_t device_id;
};

struct opencl_context_t *opencl_context_create(void);
void opencl_context_free(struct opencl_context_t *context);
uint32_t opencl_context_get_info(struct opencl_context_t *context, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);
void opencl_context_set_properties(struct opencl_context_t *context, struct mem_t *mem, uint32_t addr);




/* OpenCL command queue */

struct opencl_command_queue_t
{
	uint32_t id;
	int ref_count;

	uint32_t device_id;
	uint32_t context_id;
	uint32_t properties;
};

struct opencl_command_queue_t *opencl_command_queue_create(void);
void opencl_command_queue_free(struct opencl_command_queue_t *command_queue);




/* OpenCL program */

struct opencl_program_t
{
	uint32_t id;
	int ref_count;

	uint32_t device_id;  /* Only one device allowed */
	uint32_t context_id;

	/* Binary file */
	FILE *binary_file;
	char binary_file_name[MAX_PATH_SIZE];
	struct elf_file_t *binary_file_elf;
};

struct opencl_program_t *opencl_program_create(void);
void opencl_program_free(struct opencl_program_t *program);
void opencl_program_build(struct opencl_program_t *program);




/* OpenCL kernel */

enum opencl_mem_scope_enum {
	OPENCL_MEM_SCOPE_NONE = 0,
	OPENCL_MEM_SCOPE_GLOBAL,
	OPENCL_MEM_SCOPE_LOCAL,
	OPENCL_MEM_SCOPE_PRIVATE,
	OPENCL_MEM_SCOPE_CONSTANT
};

enum opencl_kernel_arg_kind_enum {
	OPENCL_KERNEL_ARG_KIND_VALUE = 1,
	OPENCL_KERNEL_ARG_KIND_POINTER
};

struct opencl_kernel_arg_t
{
	/* Argument properties, as described in .rodata */
	enum opencl_kernel_arg_kind_enum kind;
	enum opencl_mem_scope_enum mem_scope;  /* For pointers */
	int elem_size;  /* For a pointer, size of element pointed to */

	/* Argument fields as set in clSetKernelArg */
	int set;  /* Set to true when it is assigned */
	uint32_t value;  /* 32-bit arguments supported */
	uint32_t size;

	/* Last field - memory assigned variably */
	char name[0];
};

struct opencl_kernel_t
{
	uint32_t id;
	int ref_count;
	uint32_t program_id;
	char name[MAX_STRING_SIZE];
	struct list_t *arg_list;

	/* Excerpts of program ELF binary */
	/* Kernel metadata */
	FILE *metadata_file;
	char metadata_file_name[MAX_PATH_SIZE];

	/* Kernel embedded ELF file */
	FILE *kernel_file;
	char kernel_file_name[MAX_PATH_SIZE];

	/* CAL ABI data read from 'kernel_file' */
	struct cal_abi_t *cal_abi;

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

	/* State of the running kernel */
	struct gpu_ndrange_t *ndrange;
};

struct opencl_kernel_t *opencl_kernel_create(void);
void opencl_kernel_free(struct opencl_kernel_t *kernel);

struct opencl_kernel_arg_t *opencl_kernel_arg_create(char *name);
void opencl_kernel_arg_free(struct opencl_kernel_arg_t *arg);

void opencl_kernel_load(struct opencl_kernel_t *kernel, char *kernel_name);
uint32_t opencl_kernel_get_work_group_info(struct opencl_kernel_t *kernel, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size);




/* OpenCL mem */

struct opencl_mem_t
{
	uint32_t id;
	int ref_count;

	uint32_t size;
	uint32_t flags;
	uint32_t host_ptr;

	uint32_t device_ptr;  /* Position assigned in device global memory */
};

struct opencl_mem_t *opencl_mem_create(void);
void opencl_mem_free(struct opencl_mem_t *mem);




/* OpenCL Event */

enum opencl_event_kind_enum {
	OPENCL_EVENT_NONE = 0,
	OPENCL_EVENT_NDRANGE_KERNEL,
	OPENCL_EVENT_TASK,
	OPENCL_EVENT_NATIVE_KERNEL,
	OPENCL_EVENT_READ_BUFFER,
	OPENCL_EVENT_WRITE_BUFFER,
	OPENCL_EVENT_MAP_BUFFER,
	OPENCL_EVENT_UNMAP_MEM_OBJECT,
	OPENCL_EVENT_READ_BUFFER_RECT,
	OPENCL_EVENT_WRITE_BUFFER_RECT,
	OPENCL_EVENT_READ_IMAGE,
	OPENCL_EVENT_WRITE_IMAGE,
	OPENCL_EVENT_MAP_IMAGE,
	OPENCL_EVENT_COPY_BUFFER,
	OPENCL_EVENT_COPY_IMAGE,
	OPENCL_EVENT_COPY_BUFFER_RECT,
	OPENCL_EVENT_COPY_BUFFER_TO_IMAGE,
	OPENCL_EVENT_COPY_IMAGE_TO_BUFFER,
	OPENCL_EVENT_MARKER,
	OPENCL_EVENT_COUNT
};

enum opencl_event_status_enum {
	OPENCL_EVENT_STATUS_NONE = 0,
	OPENCL_EVENT_STATUS_QUEUED,
	OPENCL_EVENT_STATUS_SUBMITTED,
	OPENCL_EVENT_STATUS_RUNNING,
	OPENCL_EVENT_STATUS_COMPLETE
};

struct opencl_event_t
{
	uint32_t id;
	int ref_count;
	enum opencl_event_kind_enum kind;
	enum opencl_event_status_enum status;

	uint64_t time_queued;
	uint64_t time_submit;
	uint64_t time_start;
	uint64_t time_end;
};

struct opencl_event_t *opencl_event_create(enum opencl_event_kind_enum kind);
void opencl_event_free(struct opencl_event_t *event);

uint32_t opencl_event_get_profiling_info(struct opencl_event_t *event, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size);
uint64_t opencl_event_timer(void);




/*
 * GPU Write Tasks
 */

enum gpu_isa_write_task_kind_enum {
	GPU_ISA_WRITE_TASK_NONE = 0,
	GPU_ISA_WRITE_TASK_WRITE_LDS,
	GPU_ISA_WRITE_TASK_WRITE_DEST,
	GPU_ISA_WRITE_TASK_PUSH_BEFORE,
	GPU_ISA_WRITE_TASK_SET_PRED
};


struct gpu_isa_write_task_t {
	
	/* All */
	enum gpu_isa_write_task_kind_enum kind;
	struct amd_inst_t *inst;
	
	/* When 'kind' == GPU_ISA_WRITE_TASK_WRITE_DEST */
	int gpr, rel, chan, index_mode, write_mask;
	uint32_t value;

	/* When 'kind' == GPU_ISA_WRITE_TASK_WRITE_LDS */
	uint32_t lds_addr;
	uint32_t lds_value;

	/* When 'kind' == GPU_ISA_WRITE_TASK_PRED_SET */
	int cond;
};


/* Repository for 'struct gpu_isa_write_task_t' objects */
extern struct repos_t *gpu_isa_write_task_repos;


/* Functions to handle deferred tasks */
void gpu_isa_enqueue_write_lds(uint32_t addr, uint32_t value);
void gpu_isa_enqueue_write_dest(uint32_t value);
void gpu_isa_enqueue_write_dest_float(float value);
void gpu_isa_enqueue_push_before(void);
void gpu_isa_enqueue_pred_set(int cond);
void gpu_isa_write_task_commit(void);




/*
 * GPU NDRange (State of running kernel, grid of work_groups)
 */

struct gpu_ndrange_t
{
	/* ID */
	char name[MAX_STRING_SIZE];
	int id;  /* Sequential ndrange ID (given by gk->ndrange_count counter) */

	/* OpenCL kernel associated */
	struct opencl_kernel_t *kernel;

	/* Pointers to work-groups, wavefronts, and work_items */
	struct gpu_work_group_t **work_groups;
	struct gpu_wavefront_t **wavefronts;
	struct gpu_work_item_t **work_items;

	/* IDs of work-items contained */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* IDs of wavefronts contained */
	int wavefront_id_first;
	int wavefront_id_last;
	int wavefront_count;

	/* IDs of work-groups contained */
	int work_group_id_first;
	int work_group_id_last;
	int work_group_count;
	
	/* Size of work-groups */
	int wavefronts_per_work_group;  /* = ceil(local_size / gpu_wavefront_size) */

	/* Double linked lists of work-groups */
	struct gpu_work_group_t *pending_list_head, *pending_list_tail;
	struct gpu_work_group_t *running_list_head, *running_list_tail;
	struct gpu_work_group_t *finished_list_head, *finished_list_tail;
	int pending_count, pending_max;
	int running_count, running_max;
	int finished_count, finished_max;
	
	/* Local memory top to assign to local arguments.
	 * Initially it is equal to the size of local variables in kernel function. */
	uint32_t local_mem_top;
};

struct gpu_ndrange_t *gpu_ndrange_create(struct opencl_kernel_t *kernel);
void gpu_ndrange_free(struct gpu_ndrange_t *ndrange);
void gpu_ndrange_dump(struct gpu_ndrange_t *ndrange, FILE *f);

void gpu_ndrange_setup_work_items(struct gpu_ndrange_t *ndrange);
void gpu_ndrange_setup_const_mem(struct gpu_ndrange_t *ndrange);
void gpu_ndrange_setup_args(struct gpu_ndrange_t *ndrange);
void gpu_ndrange_run(struct gpu_ndrange_t *ndrange);




/*
 * GPU Work-Group
 */

enum gpu_work_group_status_enum {
	gpu_work_group_pending		= 0x0001,
	gpu_work_group_running		= 0x0002,
	gpu_work_group_finished		= 0x0004
};

struct gpu_work_group_t
{
	/* ID */
	char name[30];
	int id;  /* Group ID */
	int id_3d[3];  /* 3-dimensional Group ID */

	/* Status */
	enum gpu_work_group_status_enum status;

	/* NDRange it belongs to */
	struct gpu_ndrange_t *ndrange;

	/* IDs of work-items contained */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* IDs of wavefronts contained */
	int wavefront_id_first;
	int wavefront_id_last;
	int wavefront_count;

	/* Pointers to wavefronts and work-items */
	struct gpu_work_item_t **work_items;  /* Ptr to first work_item in 'kernel->work_items' */
	struct gpu_wavefront_t **wavefronts;  /* Pointer to first wavefront in 'kernel->wavefronts' */

	/* Double linked lists of work-groups */
	struct gpu_work_group_t *pending_prev, *pending_next;
	struct gpu_work_group_t *running_prev, *running_next;
	struct gpu_work_group_t *finished_prev, *finished_next;

	/* Double linked lists of wavefronts */
	struct gpu_wavefront_t *running_list_head, *running_list_tail;
	struct gpu_wavefront_t *barrier_list_head, *barrier_list_tail;
	struct gpu_wavefront_t *finished_list_head, *finished_list_tail;
	int running_count, running_max;
	int barrier_count, barrier_max;
	int finished_count, finished_max;
	
	/* Local memory */
	struct mem_t *local_mem;
};

#define FOREACH_WORK_GROUP_IN_NDRANGE(NDRANGE, WORK_GROUP_ID) \
	for ((WORK_GROUP_ID) = (NDRANGE)->work_group_id_first; \
		(WORK_GROUP_ID) <= (NDRANGE)->work_group_id_last; \
		(WORK_GROUP_ID)++)

struct gpu_work_group_t *gpu_work_group_create();
void gpu_work_group_free(struct gpu_work_group_t *work_group);
void gpu_work_group_dump(struct gpu_work_group_t *work_group, FILE *f);

int gpu_work_group_get_status(struct gpu_work_group_t *work_group, enum gpu_work_group_status_enum status);
void gpu_work_group_set_status(struct gpu_work_group_t *work_group, enum gpu_work_group_status_enum status);
void gpu_work_group_clear_status(struct gpu_work_group_t *work_group, enum gpu_work_group_status_enum status);




/*
 * GPU Wavefront
 */

/* Type of clauses */
enum gpu_clause_kind_enum {
	GPU_CLAUSE_NONE = 0,
	GPU_CLAUSE_CF,  /* Control-flow */
	GPU_CLAUSE_ALU,  /* ALU clause */
	GPU_CLAUSE_TC,  /* Fetch trough a Texture Cache Clause */
	GPU_CLAUSE_VC  /* Fetch through a Vertex Cache Clause */
};

/* Wavefront */
#define GPU_MAX_STACK_SIZE  32
struct gpu_wavefront_t
{
	/* ID */
	char name[30];
	int id;
	int id_in_work_group;

	/* IDs of work-items it contains */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* NDRange and Work-group it belongs to */
	struct gpu_ndrange_t *ndrange;
	struct gpu_work_group_t *work_group;

	/* Pointer to work_items */
	struct gpu_work_item_t **work_items;  /* Pointer to first work-items in 'kernel->work_items' */

	/* Current clause kind and instruction pointers */
	enum gpu_clause_kind_enum clause_kind;

	/* Current instructions */
	struct amd_inst_t cf_inst;
	struct amd_alu_group_t alu_group;
	struct amd_inst_t tc_inst;

	/* Starting/current CF buffer */
	void *cf_buf_start;
	void *cf_buf;

	/* Current and end of clause buffer */
	void *clause_buf;
	void *clause_buf_end;

	/* Active mask stack */
	struct bit_map_t *active_stack;  /* GPU_MAX_STACK_SIZE * work_item_count elements */
	int stack_top;

	/* Predicate mask */
	struct bit_map_t *pred;  /* work_item_count elements */

	/* Flag indicating whether the stack has been pushed after a PRED_SET* instruction
	 * has executed. This is done within ALU_PUSH_BEFORE instructions. */
	int push_before_done;

	/* Linked lists */
	struct gpu_wavefront_t *running_next, *running_prev;
	struct gpu_wavefront_t *barrier_next, *barrier_prev;
	struct gpu_wavefront_t *finished_next, *finished_prev;

	/* To measure simulation performance */
	uint64_t emu_inst_count;  /* Total emulated instructions */
	uint64_t emu_time_start;
	uint64_t emu_time_end;

	/* Statistics */
	uint64_t inst_count;  /* Total number of instructions */
	uint64_t global_mem_inst_count;  /* Instructions (CF or TC) accessing global memory */
	uint64_t local_mem_inst_count;  /* Instructions (ALU) accessing local memory */

	uint64_t cf_inst_count;  /* Number of CF inst executed */
	uint64_t cf_inst_global_mem_write_count;  /* Number of instructions writing to global mem (they are CF inst) */

	uint64_t alu_clause_count;  /* Number of ALU clauses started */
	uint64_t alu_group_count;  /* Number of ALU instruction groups (VLIW) */
	uint64_t alu_group_size[5];  /* Distribution of group sizes (alu_group_size[0] is the number of groups with 1 inst) */
	uint64_t alu_inst_count;  /* Number of ALU instructions */
	uint64_t alu_inst_local_mem_count;  /* Instructions accessing local memory (ALU) */

	uint64_t tc_clause_count;
	uint64_t tc_inst_count;
	uint64_t tc_inst_global_mem_read_count;  /* Number of instructions reading from global mem (they are TC inst) */
};

#define FOREACH_WAVEFRONT_IN_NDRANGE(NDRANGE, WAVEFRONT_ID) \
	for ((WAVEFRONT_ID) = (NDRANGE)->wavefront_id_first; \
		(WAVEFRONT_ID) <= (NDRANGE)->wavefront_id_last; \
		(WAVEFRONT_ID)++)

#define FOREACH_WAVEFRONT_IN_WORK_GROUP(WORK_GROUP, WAVEFRONT_ID) \
	for ((WAVEFRONT_ID) = (WORK_GROUP)->wavefront_id_first; \
		(WAVEFRONT_ID) <= (WORK_GROUP)->wavefront_id_last; \
		(WAVEFRONT_ID)++)

struct gpu_wavefront_t *gpu_wavefront_create();
void gpu_wavefront_free(struct gpu_wavefront_t *wavefront);
void gpu_wavefront_dump(struct gpu_wavefront_t *wavefront, FILE *f);

void gpu_wavefront_stack_push(struct gpu_wavefront_t *wavefront);
void gpu_wavefront_stack_pop(struct gpu_wavefront_t *wavefront, int count);
void gpu_wavefront_execute(struct gpu_wavefront_t *wavefront);




/*
 * GPU work_item (Pixel)
 */

#define GPU_MAX_GPR_ELEM  5
struct gpu_gpr_t
{
	uint32_t elem[GPU_MAX_GPR_ELEM];  /* x, y, z, w, t */
};

struct gpu_work_item_t
{
	/* IDs */
	int id;  /* global ID */
	int id_in_wavefront;
	int id_in_work_group;  /* local ID */

	/* 3-dimensional IDs */
	int id_3d[3];  /* global 3D IDs */
	int id_in_work_group_3d[3];  /* local 3D IDs */

	/* Wavefront, work-group, and NDRange where it belongs */
	struct gpu_wavefront_t *wavefront;
	struct gpu_work_group_t *work_group;
	struct gpu_ndrange_t *ndrange;

	/* Work-item state */
	struct gpu_gpr_t gpr[128];  /* General purpose registers */
	struct gpu_gpr_t pv;  /* Result of last computations */

	/* Linked list of write tasks. They are enqueued by machine instructions
	 * and executed as a burst at the end of an ALU group. */
	struct lnlist_t *write_task_list;

	/* LDS (Local Data Share) OQs (Output Queues) */
	struct list_t *lds_oqa;
	struct list_t *lds_oqb;

	/* This is a digest of the active mask updates for this work_item. Every time
	 * an instruction updates the active mask of a wavefront, this digest is updated
	 * for active work_items by XORing a random number common for the wavefront.
	 * At the end, work_items with different 'branch_digest' numbers can be considered
	 * divergent work_items. */
	uint32_t branch_digest;

	/* Last global memory access */
	uint32_t global_mem_access_addr;
	uint32_t global_mem_access_size;
};

#define FOREACH_WORK_ITEM_IN_NDRANGE(NDRANGE, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (NDRANGE)->work_item_id_first; \
		(WORK_ITEM_ID) <= (NDRANGE)->work_item_id_last; \
		(WORK_ITEM_ID)++)

#define FOREACH_WORK_ITEM_IN_WORK_GROUP(WORK_GROUP, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WORK_GROUP)->work_item_id_first; \
		(WORK_ITEM_ID) <= (WORK_GROUP)->work_item_id_last; \
		(WORK_ITEM_ID)++)

#define FOREACH_WORK_ITEM_IN_WAVEFRONT(WAVEFRONT, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WAVEFRONT)->work_item_id_first; \
		(WORK_ITEM_ID) <= (WAVEFRONT)->work_item_id_last; \
		(WORK_ITEM_ID)++)

struct gpu_work_item_t *gpu_work_item_create(void);
void gpu_work_item_free(struct gpu_work_item_t *work_item);

/* Consult and change active/predicate bits */
void gpu_work_item_set_active(struct gpu_work_item_t *work_item, int active);
int gpu_work_item_get_active(struct gpu_work_item_t *work_item);
void gpu_work_item_set_pred(struct gpu_work_item_t *work_item, int pred);
int gpu_work_item_get_pred(struct gpu_work_item_t *work_item);
void gpu_work_item_update_branch_digest(struct gpu_work_item_t *work_item,
	uint64_t inst_count, uint32_t inst_addr);




/*
 * GPU ISA
 */

/* Macros for quick access */
#define GPU_GPR_ELEM(_gpr, _elem)  (gpu_isa_work_item->gpr[(_gpr)].elem[(_elem)])
#define GPU_GPR_X(_gpr)  GPU_GPR_ELEM((_gpr), 0)
#define GPU_GPR_Y(_gpr)  GPU_GPR_ELEM((_gpr), 1)
#define GPU_GPR_Z(_gpr)  GPU_GPR_ELEM((_gpr), 2)
#define GPU_GPR_W(_gpr)  GPU_GPR_ELEM((_gpr), 3)
#define GPU_GPR_T(_gpr)  GPU_GPR_ELEM((_gpr), 4)

#define GPU_GPR_FLOAT_ELEM(_gpr, _elem)  (* (float *) &gpu_isa_work_item->gpr[(_gpr)].elem[(_elem)])
#define GPU_GPR_FLOAT_X(_gpr)  GPU_GPR_FLOAT_ELEM((_gpr), 0)
#define GPU_GPR_FLOAT_Y(_gpr)  GPU_GPR_FLOAT_ELEM((_gpr), 1)
#define GPU_GPR_FLOAT_Z(_gpr)  GPU_GPR_FLOAT_ELEM((_gpr), 2)
#define GPU_GPR_FLOAT_W(_gpr)  GPU_GPR_FLOAT_ELEM((_gpr), 3)
#define GPU_GPR_FLOAT_T(_gpr)  GPU_GPR_FLOAT_ELEM((_gpr), 4)


/* Debugging */
#define gpu_isa_debugging() debug_status(gpu_isa_debug_category)
#define gpu_isa_debug(...) debug(gpu_isa_debug_category, __VA_ARGS__)
extern int gpu_isa_debug_category;


/* Macros for unsupported parameters */
extern char *err_gpu_machine_note;

#define GPU_PARAM_NOT_SUPPORTED(p) \
	fatal("%s: %s: not supported for '" #p "' = 0x%x\n%s", \
	__FUNCTION__, gpu_isa_inst->info->name, (p), err_gpu_machine_note);
#define GPU_PARAM_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: %s: not supported for '" #p "' != 0x%x\n%s", \
	__FUNCTION__, gpu_isa_inst->info->name, (v), err_gpu_machine_note); }
#define GPU_PARAM_NOT_SUPPORTED_OOR(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: %s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	__FUNCTION__, gpu_isa_inst->info->name, (min), (max), err_opencl_param_note); }


/* Macros for fast access of instruction words */
#define CF_WORD0			gpu_isa_inst->words[0].cf_word0
#define CF_GWS_WORD0			gpu_isa_inst->words[0].cf_gws_word0
#define CF_WORD1			gpu_isa_inst->words[1].cf_word1

#define CF_ALU_WORD0			gpu_isa_inst->words[0].cf_alu_word0
#define CF_ALU_WORD1			gpu_isa_inst->words[1].cf_alu_word1
#define CF_ALU_WORD0_EXT		gpu_isa_inst->words[0].cf_alu_word0_ext
#define CF_ALU_WORD1_EXT		gpu_isa_inst->words[1].cf_alu_word1_ext

#define CF_ALLOC_EXPORT_WORD0		gpu_isa_inst->words[0].cf_alloc_export_word0
#define CF_ALLOC_EXPORT_WORD0_RAT	gpu_isa_inst->words[0].cf_alloc_export_word0_rat
#define CF_ALLOC_EXPORT_WORD1_BUF	gpu_isa_inst->words[1].cf_alloc_export_word1_buf
#define CF_ALLOC_EXPORT_WORD1_SWIZ	gpu_isa_inst->words[1].cf_alloc_export_word1_swiz

#define ALU_WORD0			gpu_isa_inst->words[0].alu_word0
#define ALU_WORD1_OP2			gpu_isa_inst->words[1].alu_word1_op2
#define ALU_WORD1_OP3			gpu_isa_inst->words[1].alu_word1_op3

#define ALU_WORD0_LDS_IDX_OP		gpu_isa_inst->words[0].alu_word0_lds_idx_op
#define ALU_WORD1_LDS_IDX_OP		gpu_isa_inst->words[1].alu_word1_lds_idx_op

#define VTX_WORD0			gpu_isa_inst->words[0].vtx_word0
#define VTX_WORD1_GPR			gpu_isa_inst->words[1].vtx_word1_gpr
#define VTX_WORD1_SEM			gpu_isa_inst->words[1].vtx_word1_sem
#define VTX_WORD2			gpu_isa_inst->words[2].vtx_word2

#define TEX_WORD0			gpu_isa_inst->words[0].tex_word0
#define TEX_WORD1			gpu_isa_inst->words[1].tex_word1
#define TEX_WORD2			gpu_isa_inst->words[2].tex_word2

#define MEM_RD_WORD0			gpu_isa_inst->words[0].mem_rd_word0
#define MEM_RD_WORD1			gpu_isa_inst->words[1].mem_rd_word1
#define MEM_RD_WORD2			gpu_isa_inst->words[2].mem_rd_word2

#define MEM_GDS_WORD0			gpu_isa_inst->words[0].mem_gds_word0
#define MEM_GDS_WORD1			gpu_isa_inst->words[1].mem_gds_word1
#define MEM_GDS_WORD2			gpu_isa_inst->words[2].mem_gds_word2


/* List of functions implementing GPU instructions 'amd_inst_XXX_impl' */
typedef void (*amd_inst_impl_t)(void);
extern amd_inst_impl_t *amd_inst_impl;

/* Access to constant memory */
void gpu_isa_const_mem_write(int bank, int vector, int elem, void *pvalue);
void gpu_isa_const_mem_read(int bank, int vector, int elem, void *pvalue);

/* For ALU clauses */
void gpu_isa_alu_clause_start(void);
void gpu_isa_alu_clause_end(void);

/* For TC clauses */
void gpu_isa_tc_clause_start(void);
void gpu_isa_tc_clause_end(void);

/* For functional simulation */
uint32_t gpu_isa_read_gpr(int gpr, int rel, int chan, int im);
float gpu_isa_read_gpr_float(int gpr, int rel, int chan, int im);
void gpu_isa_write_gpr(int gpr, int rel, int chan, uint32_t value);
void gpu_isa_write_gpr_float(int gpr, int rel, int chan, float value);

uint32_t gpu_isa_read_op_src_int(int src_idx);
float gpu_isa_read_op_src_float(int src_idx);

void gpu_isa_init(void);
void gpu_isa_done(void);




/*
 * GPU Kernel (gk)
 * This refers to the Multi2Sim object representing the GPU.
 */

struct gk_t {
	
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
	uint32_t global_mem_top;

	/* Number of OpenCL kernels executed */
	int ndrange_count;
};

extern struct gk_t *gk;

void gk_init(void);
void gk_done(void);

void gk_libopencl_redirect(char *path, int size);
void gk_libopencl_failed(int pid);


#endif

