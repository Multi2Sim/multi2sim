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

#ifndef GPUKERNEL_LOCAL_H
#define GPUKERNEL_LOCAL_H

#include <gpukernel.h>
#include <m2skernel.h>
#include <stdint.h>



/* Error messages */

extern char *err_opencl_note;
extern char *err_opencl_param_note;



/* OpenCL API Implementation */


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
};

struct opencl_command_queue_t *opencl_command_queue_create(void);
void opencl_command_queue_free(struct opencl_command_queue_t *command_queue);
void opencl_command_queue_read_properties(struct opencl_command_queue_t *command_queue, struct mem_t *mem, uint32_t addr);




/* OpenCL program */

struct opencl_program_t
{
	uint32_t id;
	int ref_count;

	uint32_t device_id;  /* Only one device allowed */
	uint32_t context_id;

	void *binary;  /* Main ELF binary  */
	int binary_size;
	int binary_loaded_from_file;  /* Flag */

	void *rodata;  /* Main ELF binary's '.rodata' section */
	int rodata_size;

	void *code;  /* Main ELF binary's 1st '.text' section's 2nd '.text' section */
	int code_size;
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

	/* Number of work dimensions */
	int work_dim;

	/* 3D Counters */
	int global_size3[3];  /* Total number of threads */
	int local_size3[3];  /* Number of threads in a group */
	int group_count3[3];  /* Number of thread groups */
	
	/* 1D Counters. Each counter is equal to the multiplication
	 * of each component in the corresponding 3D counter. */
	int global_size;
	int local_size;
	int group_count;
};

struct opencl_kernel_t *opencl_kernel_create(void);
void opencl_kernel_free(struct opencl_kernel_t *kernel);

struct opencl_kernel_arg_t *opencl_kernel_arg_create(char *name);
void opencl_kernel_arg_free(struct opencl_kernel_arg_t *arg);

void opencl_kernel_load_rodata(struct opencl_kernel_t *kernel, char *kernel_name);
uint32_t opencl_kernel_get_work_group_info(struct opencl_kernel_t *kernel, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size);




/* OpenCL mem */

struct opencl_mem_t
{
	uint32_t id;
	int ref_count;

	uint32_t size;
	uint32_t flags;

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




/*
 * GPU Write Tasks
 */

enum gpu_isa_write_task_kind_enum {
	GPU_ISA_WRITE_TASK_NONE = 0,
	GPU_ISA_WRITE_TASK_WRITE_DEST,
	GPU_ISA_WRITE_TASK_PUSH_BEFORE,
	GPU_ISA_WRITE_TASK_SET_PRED
};


struct gpu_isa_write_task_t {
	
	/* All */
	enum gpu_isa_write_task_kind_enum kind;
	struct amd_inst_t *inst;
	
	/* WRITE_DEST */
	int gpr, rel, chan, index_mode, write_mask;
	uint32_t value;

	/* PRED_SET */
	int cond;
};


/* Repository for 'struct gpu_isa_write_task_t' objects */
extern struct repos_t *gpu_isa_write_task_repos;


/* Functions to handle deferred tasks */
void gpu_isa_enqueue_write_dest(uint32_t value);
void gpu_isa_enqueue_write_dest_float(float value);
void gpu_isa_enqueue_push_before(void);
void gpu_isa_enqueue_pred_set(int cond);
void gpu_isa_write_task_commit(void);




/*
 * GPU Warp
 */

/* Type of clauses */
enum gpu_clause_kind_enum {
	GPU_CLAUSE_NONE = 0,
	GPU_CLAUSE_CF,  /* Control-flow */
	GPU_CLAUSE_ALU,  /* ALU clause */
	GPU_CLAUSE_TC,  /* Fetch trough a Texture Cache Clause */
	GPU_CLAUSE_VC  /* Fetch through a Vectex Cache Clause */
};


/* Warp */
#define GPU_MAX_STACK_SIZE  32
struct gpu_warp_t
{
	/* Number of threads and 'global_id' of first thread */
	int thread_count;
	int global_id;
	char name[20];

	/* Current clause kind and instruction pointers */
	enum gpu_clause_kind_enum clause_kind;

	/* Starting/current CF buffer and instruction */
	void *cf_buf_start;
	void *cf_buf;
	struct amd_inst_t *cf_inst;

	/* Current and end of clause buffer */
	void *clause_buf;
	void *clause_buf_end;

	/* Active mask stack */
	struct bit_map_t *active_stack;  /* GPU_MAX_STACK_SIZE * thread_count elements */
	int stack_top;

	/* Predicate mask */
	struct bit_map_t *pred;  /* thread_count elements */

	/* Flag indicating whether the stack has been pushed after a PRED_SET* instruction
	 * has executed. This is done within ALU_PUSH_BEFORE instructions. */
	int push_before_done;
};


struct gpu_warp_t *gpu_warp_create(int thread_count, int global_id);
void gpu_warp_free(struct gpu_warp_t *warp);

void gpu_warp_stack_push(struct gpu_warp_t *warp);
void gpu_warp_stack_pop(struct gpu_warp_t *warp, int count);




/*
 * GPU Thread (Pixel)
 */

#define GPU_MAX_GPR_ELEM  5
struct gpu_gpr_t
{
	uint32_t elem[GPU_MAX_GPR_ELEM];  /* x, y, z, w, t */
};

struct gpu_thread_t
{
	/* Warp where it belongs */
	struct gpu_warp_t *warp;

	/* Thread status */
	struct gpu_gpr_t gpr[128];  /* General purpose registers */
	struct gpu_gpr_t pv;  /* Result of last computations */

	/* 1D identifiers */
	int warp_id;
	int local_id;
	int global_id;
	int group_id;

	/* 3D identifiers */
	int local_id3[3];
	int global_id3[3];
	int group_id3[3];

	/* Linked list of write tasks. They are enqueued by machine instructions
	 * and executed as a burst at the end of an ALU group. */
	struct lnlist_t *write_task_list;

	/* LDS (Local Data Share) OQs (Output Queues) */
	struct list_t *lds_oqa;
	struct list_t *lds_oqb;
};


struct gpu_thread_t *gpu_thread_create(void);
void gpu_thread_free(struct gpu_thread_t *thread);

/* Consult and change active/predicate bits */
void gpu_thread_set_active(struct gpu_thread_t *thread, int active);
int gpu_thread_get_active(struct gpu_thread_t *thread);
void gpu_thread_set_pred(struct gpu_thread_t *thread, int pred);
int gpu_thread_get_pred(struct gpu_thread_t *thread);




/*
 * GPU ISA
 */

/* Macros for quick access */
#define GPU_THR (*gpu_isa_thread)
#define GPU_THR_I(I)  (*gpu_isa_threads[(I)])

#define GPU_GPR_ELEM(_gpr, _elem)  (gpu_isa_thread->gpr[(_gpr)].elem[(_elem)])
#define GPU_GPR_X(_gpr)  GPU_GPR_ELEM((_gpr), 0)
#define GPU_GPR_Y(_gpr)  GPU_GPR_ELEM((_gpr), 1)
#define GPU_GPR_Z(_gpr)  GPU_GPR_ELEM((_gpr), 2)
#define GPU_GPR_W(_gpr)  GPU_GPR_ELEM((_gpr), 3)
#define GPU_GPR_T(_gpr)  GPU_GPR_ELEM((_gpr), 4)

#define GPU_GPR_FLOAT_ELEM(_gpr, _elem)  (* (float *) &gpu_isa_thread->gpr[(_gpr)].elem[(_elem)])
#define GPU_GPR_FLOAT_X(_gpr)  GPU_GPR_FLOAT_ELEM((_gpr), 0)
#define GPU_GPR_FLOAT_Y(_gpr)  GPU_GPR_FLOAT_ELEM((_gpr), 1)
#define GPU_GPR_FLOAT_Z(_gpr)  GPU_GPR_FLOAT_ELEM((_gpr), 2)
#define GPU_GPR_FLOAT_W(_gpr)  GPU_GPR_FLOAT_ELEM((_gpr), 3)
#define GPU_GPR_FLOAT_T(_gpr)  GPU_GPR_FLOAT_ELEM((_gpr), 4)


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


/* Global variables */
extern struct gpu_thread_t *gpu_isa_thread;
extern struct gpu_thread_t **gpu_isa_threads;
extern struct gpu_warp_t *gpu_isa_warp;
extern struct amd_inst_t *gpu_isa_inst;
extern struct amd_alu_group_t *gpu_isa_alu_group;

/* List of functions implementing GPU instructions 'amd_inst_XXX_impl' */
typedef void (*amd_inst_impl_t)(void);
extern amd_inst_impl_t *amd_inst_impl;

/* For ALU clauses */
void gpu_isa_alu_clause_start(void);
void gpu_isa_alu_clause_end(void);

/* For functional simulation */
uint32_t gpu_isa_read_gpr(int gpr, int rel, int chan, int im);
float gpu_isa_read_gpr_float(int gpr, int rel, int chan, int im);
void gpu_isa_write_gpr(int gpr, int rel, int chan, uint32_t value);
void gpu_isa_write_gpr_float(int gpr, int rel, int chan, float value);

uint32_t gpu_isa_read_op_src(int src_idx);
float gpu_isa_read_op_src_float(int src_idx);

void gpu_isa_init(void);
void gpu_isa_done(void);
void gpu_isa_run(struct opencl_kernel_t *kernel);




/*
 * GPU Kernel (gk)
 * This refers to the Multi2Sim object representing the GPU.
 */

#define GK_GLOBAL_MEM_BASE   0x1000000
#define GK_LOCAL_MEM_BASE    0x1000000

struct gk_t {
	
	/* Constant memory (constant buffers)
	 * There are 15 constant buffers, referenced as CB0 to CB14.
	 * Each buffer can hold up to 1024 four-component vectors.
	 * These buffers will be represented as a memory object indexed as
	 *   buffer_id * 1024 * 4 * 4 + vector_id * 4 * 4 + elem_id * 4
	 */
	struct mem_t *const_mem;

	/* Global memory */
	struct mem_t *global_mem;
	uint32_t global_mem_top;  /* Current top pointer assigned to 'device_ptr' in 'opencl_mem' objects */
	
	/* Array of local memories.
	 * This array is initialized when the kernel is created. */
	struct mem_t **local_mem;
	uint32_t local_mem_top;  /* Current top pointer assigned to '__local' arguments of variable size */
};

extern struct gk_t *gk;
extern char *gk_opencl_binary_name;

/* Macros to access constant memory */
#define GPU_CONST_MEM_ADDR(_bank, _vector, _elem)  ((_bank) * 16384 + (_vector) * 16 + (_elem * 4))


#endif

