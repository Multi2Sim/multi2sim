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
	OPENCL_OBJ_MEM
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

struct opencl_platform_t *opencl_platform_create();
void opencl_platform_free(struct opencl_platform_t *platform);
uint32_t opencl_platform_get_info(struct opencl_platform_t *platform, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);




/* OpenCL devices */

struct opencl_device_t
{
	uint32_t id;
};

struct opencl_device_t *opencl_device_create();
void opencl_device_free(struct opencl_device_t *device);
uint32_t opencl_device_get_info(struct opencl_device_t *device, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);




/* OpenCL contexts */

struct opencl_context_t
{
	uint32_t id;
	uint32_t platform_id;
	uint32_t device_id;
};

struct opencl_context_t *opencl_context_create();
void opencl_context_free(struct opencl_context_t *context);
uint32_t opencl_context_get_info(struct opencl_context_t *context, uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);
void opencl_context_set_properties(struct opencl_context_t *context, struct mem_t *mem, uint32_t addr);




/* OpenCL command queue */

struct opencl_command_queue_t
{
	uint32_t id;
	uint32_t device_id;
	uint32_t context_id;
};

struct opencl_command_queue_t *opencl_command_queue_create();
void opencl_command_queue_free(struct opencl_command_queue_t *command_queue);
void opencl_command_queue_read_properties(struct opencl_command_queue_t *command_queue, struct mem_t *mem, uint32_t addr);




/* OpenCL program */

struct opencl_program_t
{
	uint32_t id;
	uint32_t device_id;  /* Only one device allowed */
	uint32_t context_id;

	void *binary;  /* Main ELF binary  */
	int binary_size;

	void *rodata;  /* Main ELF binary's '.rodata' section */
	int rodata_size;

	void *code;  /* Main ELF binary's 1st '.text' section's 2nd '.text' section */
	int code_size;
};

struct opencl_program_t *opencl_program_create();
void opencl_program_free(struct opencl_program_t *program);
void opencl_program_build(struct opencl_program_t *program);




/* OpenCL kernel */

enum opencl_kernel_arg_kind_enum {
	OPENCL_KERNEL_ARG_KIND_VALUE = 1,
	OPENCL_KERNEL_ARG_KIND_POINTER
};

struct opencl_kernel_arg_t
{
	enum opencl_kernel_arg_kind_enum kind;
	int elem_size;  /* For a pointer, size of element pointed to */
	uint32_t value;  /* 32-bit arguments supported */

	/* Last field - memory assigned variably */
	char name[0];
};

struct opencl_kernel_t
{
	uint32_t id;
	uint32_t program_id;
	char kernel_name[MAX_STRING_SIZE];
	struct list_t *arg_list;

	int memory_hwprivate;
	int memory_hwlocal;

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

struct opencl_kernel_t *opencl_kernel_create();
void opencl_kernel_free(struct opencl_kernel_t *kernel);

struct opencl_kernel_arg_t *opencl_kernel_arg_create(char *name);
void opencl_kernel_arg_free(struct opencl_kernel_arg_t *arg);

void opencl_kernel_load_rodata(struct opencl_kernel_t *kernel, char *kernel_name);




/* OpenCL mem */

struct opencl_mem_t
{
	uint32_t id;
	uint32_t size;
	uint32_t flags;

	uint32_t device_ptr;  /* Position assigned in device global memory */
};

struct opencl_mem_t *opencl_mem_create();
void opencl_mem_free(struct opencl_mem_t *mem);




/* GPU thread */

#define GPU_MAX_GPR_ELEM  5
struct gpu_gpr_t {
	uint32_t elem[GPU_MAX_GPR_ELEM];  /* x, y, z, w, t */
};


#define GPU_MAX_WRITE_TASKS  5
struct gpu_write_task_t {
	int alu;  /* ALU generating result [0..GPU_MAX_GPR_ELEM-1] */
	uint32_t value;  /* Computed value */
	int gpr, rel, chan, im, wm;
};


struct gpu_thread_t {
	
	/* Thread status */
	struct gpu_gpr_t gpr[128];  /* General purpose registers */
	struct gpu_gpr_t pv;  /* Result of last computations */

	/* 1D identifiers */
	int local_id;
	int global_id;
	int group_id;

	/* 3D identifiers */
	int local_id3[3];
	int global_id3[3];
	int group_id3[3];

	/* Instruction pointers */
	uint32_t cf_ip;
	uint32_t clause_ip;

	/* Write tasks. These are writes to registers scheduled by processing elements,
	 * which are performed as a burst all together. */
	int write_task_count;
	struct gpu_write_task_t write_tasks[GPU_MAX_WRITE_TASKS];
};


struct gpu_thread_t *gpu_thread_create();
void gpu_thread_free(struct gpu_thread_t *thread);




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


/* Global variables */
extern struct gpu_thread_t *gpu_isa_thread;
extern struct gpu_thread_t **gpu_isa_threads;
extern struct amd_inst_t *gpu_isa_inst;
extern struct amd_alu_group_t *gpu_isa_alu_group;

/* List of functions implementing GPU instructions 'amd_inst_XXX_impl' */
typedef void (*amd_inst_impl_t)(void);
extern amd_inst_impl_t *amd_inst_impl;

/* For functional simulation */
uint32_t gpu_isa_read_gpr(int gpr, int rel, int chan, int im);
void gpu_isa_write_gpr(int gpr, int rel, int chan, uint32_t value);

uint32_t gpu_isa_read_op_src(int src_idx);
void gpu_isa_write_op_dst(uint32_t value);
void gpu_alu_group_commit(void);

void gpu_isa_init();
void gpu_isa_done();
void gpu_isa_run(struct opencl_kernel_t *kernel);




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

	/* Global memory */
	struct mem_t *global_mem;
	uint32_t global_mem_top;  /* Current top pointer assigned to 'device_ptr' in 'opencl_mem' objects */
};

extern struct gk_t *gk;
extern char *gk_opencl_binary_name;

/* Macros to access constant memory */
#define GPU_CONST_MEM_ADDR(_bank, _vector, _elem)  ((_bank) * 16384 + (_vector) * 16 + (_elem * 4))


#endif

