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

#ifndef SOUTHERN_ISLANDS_EMU_H
#define SOUTHERN_ISLANDS_EMU_H

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include <config.h>
#include <debug.h>
#include <elf-format.h>
#include <linked-list.h>
#include <list.h>
#include <mhandle.h>
#include <misc.h>

#include "southern-islands-asm.h"


/* 
 * Memory configuration 
 */

#define UAV_TABLE_START 0
#define UAV_TABLE_SIZE 1024
#define CONSTANT_MEMORY_START (UAV_TABLE_START + UAV_TABLE_SIZE)
#define CONSTANT_BUFFER_SIZE 1024
#define CONSTANT_BUFFERS 2
#define GLOBAL_MEMORY_START (CONSTANT_MEMORY_START + CONSTANT_BUFFERS*CONSTANT_BUFFER_SIZE)

/*
 * OpenCL API Implementation
 */

/* Forward declaration */
struct x86_ctx_t;

/* Function tables */
extern char *si_opencl_func_name[];
extern int si_opencl_func_argc[];

/* Debugging */
#define si_opencl_debug(...) debug(si_opencl_debug_category, __VA_ARGS__)
extern int si_opencl_debug_category;

/* Some constants */
#define SI_OPENCL_FUNC_FIRST  1000
#define SI_OPENCL_FUNC_LAST  1073
#define SI_OPENCL_FUNC_COUNT  (si_OPENCL_FUNC_LAST - si_OPENCL_FUNC_FIRST + 1)
#define SI_OPENCL_MAX_ARGS  14

int si_opencl_api_run(struct x86_ctx_t *ctx);

int si_opencl_api_read_args(struct x86_ctx_t *ctx, int *argc_ptr,
		void *argv_ptr, int argv_size);
void si_opencl_api_return(struct x86_ctx_t *ctx, int value);




/*
 * OpenCL objects
 */

enum si_opencl_object_type_t
{
	si_opencl_object_invalid,
	si_opencl_object_platform,
	si_opencl_object_device,
	si_opencl_object_context,
	si_opencl_object_command_queue,
	si_opencl_object_program,
	si_opencl_object_kernel,
	si_opencl_object_mem,
	si_opencl_object_event,
	si_opencl_object_sampler
};

struct si_opencl_repo_t;

struct si_opencl_repo_t *si_opencl_repo_create(void);
void si_opencl_repo_free(struct si_opencl_repo_t *repo);

void si_opencl_repo_add_object(struct si_opencl_repo_t *repo,
	void *object);
void si_opencl_repo_remove_object(struct si_opencl_repo_t *repo,
	void *object);
void *si_opencl_repo_get_object(struct si_opencl_repo_t *repo,
	enum si_opencl_object_type_t type, unsigned int object_id);

void *si_opencl_repo_get_object_of_type(struct si_opencl_repo_t *repo,
	enum si_opencl_object_type_t type);
unsigned int si_opencl_repo_new_object_id(struct si_opencl_repo_t *repo,
	enum si_opencl_object_type_t type);

void si_opencl_repo_free_all_objects(struct si_opencl_repo_t *repo);


/* OpenCL platform */

struct si_opencl_platform_t
{
	uint32_t id;
};

struct mem_t;  /* Forward declaration */

extern struct si_opencl_platform_t *si_opencl_platform;

struct si_opencl_platform_t *si_opencl_platform_create(void);
void si_opencl_platform_free(struct si_opencl_platform_t *platform);

uint32_t si_opencl_platform_get_info(struct si_opencl_platform_t *platform,
	uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);




/* OpenCL devices */

struct si_opencl_device_t
{
	uint32_t id;
};

struct si_opencl_device_t *si_opencl_device_create(void);
void si_opencl_device_free(struct si_opencl_device_t *device);

uint32_t si_opencl_device_get_info(struct si_opencl_device_t *device, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size);




/* OpenCL contexts */

struct si_opencl_context_t
{
	uint32_t id;
	int ref_count;

	uint32_t platform_id;
	uint32_t device_id;
};

struct si_opencl_context_t *si_opencl_context_create(void);
void si_opencl_context_free(struct si_opencl_context_t *context);

uint32_t si_opencl_context_get_info(struct si_opencl_context_t *context,
	uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);
void si_opencl_context_set_properties(struct si_opencl_context_t *context,
	struct mem_t *mem, uint32_t addr);




/*
 * OpenCL Command Queue
 */

/* Forward declaration of x86 context used below in callback function. This
 * dependence would be removed if OpenCL API implementation was in 'arch/x86/emu'
 * instead. Is this a better option? */
struct x86_ctx_t;

enum si_opencl_command_type_t
{
	si_opencl_command_queue_task_invalid,
	si_opencl_command_queue_task_read_buffer,
	si_opencl_command_queue_task_write_buffer,
	si_opencl_command_queue_task_ndrange_kernel
};

struct si_opencl_command_t
{
	enum si_opencl_command_type_t type;
	union
	{
		struct
		{
			struct si_ndrange_t *ndrange;
		} ndrange_kernel;
	} u;
};

struct si_opencl_command_queue_t
{
	unsigned int id;
	int ref_count;

	unsigned int device_id;
	unsigned int context_id;
	unsigned int properties;

	struct linked_list_t *command_list;
};

struct si_opencl_command_queue_t *si_opencl_command_queue_create(void);
void si_opencl_command_queue_free(struct si_opencl_command_queue_t *command_queue);

struct si_opencl_command_t *si_opencl_command_create(enum
	si_opencl_command_type_t type);
void si_opencl_command_free(struct si_opencl_command_t *command);

void si_opencl_command_queue_submit(struct si_opencl_command_queue_t *command_queue,
	struct si_opencl_command_t *command);
void si_opencl_command_queue_complete(struct si_opencl_command_queue_t *command_queue,
	struct si_opencl_command_t *command);

/* Callback function of type 'x86_ctx_wakeup_callback_func_t'.
 * Argument 'data' is type-casted to 'struct si_opencl_command_queue_t' */
int si_opencl_command_queue_can_wakeup(struct x86_ctx_t *ctx, void *data);




/* OpenCL program */

struct si_opencl_program_t
{
	uint32_t id;
	int ref_count;

	uint32_t device_id;  /* Only one device allowed */
	uint32_t context_id;

	/* ELF binary */
	struct elf_file_t *elf_file;

	/* Constant buffers are shared by all kernels compiled in the
	 * same binary. This list is comprised of elf_buffers. */
	struct list_t *constant_buffer_list;
};

struct si_opencl_program_t *si_opencl_program_create(void);
void si_opencl_program_free(struct si_opencl_program_t *program);

void si_opencl_program_build(struct si_opencl_program_t *program);
void si_opencl_program_initialize_constant_buffers(struct si_opencl_program_t *program);

void si_opencl_program_read_symbol(struct si_opencl_program_t *program, char *symbol_name,
	struct elf_buffer_t *buffer);

void si_isa_init(void);
void si_isa_done(void);



/* OpenCL sampler */

struct si_opencl_sampler_t
{
	uint32_t id;
	int ref_count;

	uint32_t normalized_coords;
	uint32_t filter_mode;
	uint32_t addressing_mode;
};

struct si_opencl_sampler_t *si_opencl_sampler_create(void);
void si_opencl_sampler_free(struct si_opencl_sampler_t *sampler);



/* OpenCL mem */

struct si_opencl_mem_t
{
	uint32_t id;
	int ref_count;

	uint32_t type;  /* 0 buffer, 1 2D image, 2 3D image */

	uint32_t size;

	/* Used for images only */
	uint32_t height;
	uint32_t width;
	uint32_t depth;
	uint32_t num_pixels;
	uint32_t pixel_size;
	uint32_t num_channels_per_pixel;

	uint32_t flags;
	uint32_t host_ptr;

	uint32_t device_ptr;  /* Position assigned in device global memory */
};

struct si_opencl_mem_t *si_opencl_mem_create(void);
void si_opencl_mem_free(struct si_opencl_mem_t *mem);


/* OpenCL Image */

struct si_opencl_image_format_t
{
	uint32_t image_channel_order;
	uint32_t image_channel_data_type;
};





/* OpenCL kernel */

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
	uint32_t size;

	union
	{
		uint32_t ptr;
		uint32_t value[4];
	} data;

	/* Last field - memory assigned variably */
	char name[0];
};

struct si_opencl_kernel_t
{
	uint32_t id;
	int ref_count;
	uint32_t program_id;
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
	struct list_t *uav_list;
	struct list_t *constant_buffer_list;

	/* State of the running kernel */
	struct si_ndrange_t *ndrange;
};

struct si_opencl_kernel_t *si_opencl_kernel_create(void);
void si_opencl_kernel_free(struct si_opencl_kernel_t *kernel);

struct si_opencl_kernel_arg_t *si_opencl_kernel_arg_create(char *name);
void si_opencl_kernel_arg_free(struct si_opencl_kernel_arg_t *arg);

void si_opencl_kernel_load(struct si_opencl_kernel_t *kernel, char *kernel_name);
uint32_t si_opencl_kernel_get_work_group_info(struct si_opencl_kernel_t *kernel, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size);

void si_opencl_kernel_init_uav_table(struct si_opencl_kernel_t *kernel);




/* OpenCL Event */

enum si_opencl_event_kind_t
{
	SI_OPENCL_EVENT_NONE = 0,
	SI_OPENCL_EVENT_NDRANGE_KERNEL,
	SI_OPENCL_EVENT_TASK,
	SI_OPENCL_EVENT_NATIVE_KERNEL,
	SI_OPENCL_EVENT_READ_BUFFER,
	SI_OPENCL_EVENT_WRITE_BUFFER,
	SI_OPENCL_EVENT_MAP_BUFFER,
	SI_OPENCL_EVENT_UNMAP_MEM_OBJECT,
	SI_OPENCL_EVENT_READ_BUFFER_RECT,
	SI_OPENCL_EVENT_WRITE_BUFFER_RECT,
	SI_OPENCL_EVENT_READ_IMAGE,
	SI_OPENCL_EVENT_WRITE_IMAGE,
	SI_OPENCL_EVENT_MAP_IMAGE,
	SI_OPENCL_EVENT_COPY_BUFFER,
	SI_OPENCL_EVENT_COPY_IMAGE,
	SI_OPENCL_EVENT_COPY_BUFFER_RECT,
	SI_OPENCL_EVENT_COPY_BUFFER_TO_IMAGE,
	SI_OPENCL_EVENT_COPY_IMAGE_TO_BUFFER,
	SI_OPENCL_EVENT_MARKER,
	SI_OPENCL_EVENT_COUNT
};

enum si_opencl_event_status_t
{
	SI_OPENCL_EVENT_STATUS_NONE = 0,
	SI_OPENCL_EVENT_STATUS_QUEUED,
	SI_OPENCL_EVENT_STATUS_SUBMITTED,
	SI_OPENCL_EVENT_STATUS_RUNNING,
	SI_OPENCL_EVENT_STATUS_COMPLETE
};

struct si_opencl_event_t
{
	uint32_t id;
	int ref_count;
	enum si_opencl_event_kind_t kind;
	enum si_opencl_event_status_t status;

	long long time_queued;
	long long time_submit;
	long long time_start;
	long long time_end;
};

struct si_opencl_event_t *si_opencl_event_create(enum si_opencl_event_kind_t kind);
void si_opencl_event_free(struct si_opencl_event_t *event);

uint32_t si_opencl_event_get_profiling_info(struct si_opencl_event_t *event, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size);
long long si_opencl_event_timer(void);



/*
 * GPU Write Tasks
 */

enum si_isa_write_task_kind_t
{
	SI_ISA_WRITE_TASK_NONE = 0,
	SI_ISA_WRITE_TASK_WRITE_LDS,
	SI_ISA_WRITE_TASK_WRITE_DEST,
	SI_ISA_WRITE_TASK_PUSH_BEFORE,
	SI_ISA_WRITE_TASK_SET_PRED
};


struct si_isa_write_task_t
{
	/* All */
	enum si_isa_write_task_kind_t kind;
	struct si_inst_t *inst;
	
	/* When 'kind' == SI_ISA_WRITE_TASK_WRITE_DEST */
	int gpr, rel, chan, index_mode, write_mask;
	uint32_t value;

	/* When 'kind' == SI_ISA_WRITE_TASK_WRITE_LDS */
	uint32_t lds_addr;
	uint32_t lds_value;
        size_t   lds_value_size;

	/* When 'kind' == GPU_ISA_WRITE_TASK_PRED_SET */
	int cond;
};


/* Repository for 'struct si_isa_write_task_t' objects */
extern struct repos_t *si_isa_write_task_repos;


/* Functions to handle deferred tasks */
void si_isa_enqueue_write_lds(uint32_t addr, uint32_t value, size_t value_size);
void si_isa_enqueue_write_dest(uint32_t value);
void si_isa_enqueue_write_dest_float(float value);
void si_isa_enqueue_push_before(void);
void si_isa_enqueue_pred_set(int cond);
void si_isa_write_task_commit(void);




/*
 * GPU NDRange (State of running kernel, grid of work_groups)
 */

enum si_ndrange_status_t
{
	si_ndrange_pending		= 0x0001,
	si_ndrange_running		= 0x0002,
	si_ndrange_finished		= 0x0004
};

struct si_ndrange_t
{
	/* ID */
	char *name;
	int id;  /* Sequential ndrange ID (given by si_emu->ndrange_count counter) */

	/* Status */
	enum si_ndrange_status_t status;

	/* OpenCL kernel associated */
	struct si_opencl_kernel_t *kernel;

	/* Command queue and command queue task associated */
	struct si_opencl_command_queue_t *command_queue;
	struct si_opencl_command_t *command;

	/* Pointers to work-groups, wavefronts, and work_items */
	struct si_work_group_t **work_groups;
	struct si_wavefront_t **wavefronts;
	struct si_work_item_t **work_items;
	struct si_work_item_t **scalar_work_items;

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
	int wavefronts_per_work_group;  /* = ceil(local_size / si_emu_wavefront_size) */

	/* List of ND-Ranges */
	struct si_ndrange_t *ndrange_list_prev;
	struct si_ndrange_t *ndrange_list_next;
	struct si_ndrange_t *pending_ndrange_list_prev;
	struct si_ndrange_t *pending_ndrange_list_next;
	struct si_ndrange_t *running_ndrange_list_prev;
	struct si_ndrange_t *running_ndrange_list_next;
	struct si_ndrange_t *finished_ndrange_list_prev;
	struct si_ndrange_t *finished_ndrange_list_next;

	/* List of pending work-groups */
	struct si_work_group_t *pending_list_head;
	struct si_work_group_t *pending_list_tail;
	int pending_list_count;
	int pending_list_max;

	/* List of running work-groups */
	struct si_work_group_t *running_list_head;
	struct si_work_group_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of finished work-groups */
	struct si_work_group_t *finished_list_head;
	struct si_work_group_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Local memory top to assign to local arguments.
	 * Initially it is equal to the size of local variables in kernel function. */
	uint32_t local_mem_top;

	/* Statistics */

	/* Histogram of executed instructions. Only allocated if the kernel report
	 * option is active. */
	unsigned int *inst_histogram;
};

struct si_ndrange_t *si_ndrange_create(struct si_opencl_kernel_t *kernel);
void si_ndrange_free(struct si_ndrange_t *ndrange);
void si_ndrange_dump(struct si_ndrange_t *ndrange, FILE *f);

int si_ndrange_get_status(struct si_ndrange_t *ndrange, enum si_ndrange_status_t status);
void si_ndrange_set_status(struct si_ndrange_t *work_group, enum si_ndrange_status_t status);
void si_ndrange_clear_status(struct si_ndrange_t *work_group, enum si_ndrange_status_t status);

void si_ndrange_setup_work_items(struct si_ndrange_t *ndrange);
void si_ndrange_setup_const_mem(struct si_ndrange_t *ndrange);
void si_ndrange_setup_args(struct si_ndrange_t *ndrange);

/* Access to constant memory */
void si_isa_const_mem_write(int buffer, int offset, void *pvalue);
void si_isa_const_mem_read(int buffer, int offset, void *pvalue);


/*
 * GPU Work-Group
 */

enum si_work_group_status_t
{
	si_work_group_pending		= 0x0001,
	si_work_group_running		= 0x0002,
	si_work_group_finished		= 0x0004
};

struct si_work_group_t
{
	char name[30];

	/* ID */
	int id;  /* Group ID */
	int id_3d[3];  /* 3-dimensional Group ID */

	/* Status */
	enum si_work_group_status_t status;

	/* NDRange it belongs to */
	struct si_ndrange_t *ndrange;

	/* IDs of work-items contained */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* IDs of wavefronts contained */
	int wavefront_id_first;
	int wavefront_id_last;
	int wavefront_count;

	/* Pointers to wavefronts and work-items */
	struct si_work_item_t **work_items;  /* Pointer to first work_item in 'kernel->work_items' */
	struct si_wavefront_t **wavefronts;  /* Pointer to first wavefront in 'kernel->wavefronts' */

	/* Double linked lists of work-groups */
	struct si_work_group_t *pending_list_prev;
	struct si_work_group_t *pending_list_next;
	struct si_work_group_t *running_list_prev;
	struct si_work_group_t *running_list_next;
	struct si_work_group_t *finished_list_prev;
	struct si_work_group_t *finished_list_next;

	/* List of running wavefronts */
	struct si_wavefront_t *running_list_head;
	struct si_wavefront_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of wavefronts in barrier */
	struct si_wavefront_t *barrier_list_head;
	struct si_wavefront_t *barrier_list_tail;
	int barrier_list_count;
	int barrier_list_max;

	/* List of finished wavefronts */
	struct si_wavefront_t *finished_list_head;
	struct si_wavefront_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Fields introduced for architectural simulation */
	int id_in_compute_unit;
	int compute_unit_finished_count;  /* like 'finished_list_count', but when WF reaches Complete stage */

	/* Local memory */
	struct mem_t *local_mem;
};

#define SI_FOR_EACH_WORK_GROUP_IN_NDRANGE(NDRANGE, WORK_GROUP_ID) \
	for ((WORK_GROUP_ID) = (NDRANGE)->work_group_id_first; \
		(WORK_GROUP_ID) <= (NDRANGE)->work_group_id_last; \
		(WORK_GROUP_ID)++)

struct si_work_group_t *si_work_group_create();
void si_work_group_free(struct si_work_group_t *work_group);
void si_work_group_dump(struct si_work_group_t *work_group, FILE *f);

int si_work_group_get_status(struct si_work_group_t *work_group, enum si_work_group_status_t status);
void si_work_group_set_status(struct si_work_group_t *work_group, enum si_work_group_status_t status);
void si_work_group_clear_status(struct si_work_group_t *work_group, enum si_work_group_status_t status);




/*
 * GPU Wavefront
 */

/* Wavefront */
struct si_wavefront_t
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
	struct si_ndrange_t *ndrange;
	struct si_work_group_t *work_group;

	/* Instruction buffer */
	void *inst_buf;	
	void *inst_buf_start;	

	/* Current instruction */
	struct si_inst_t inst;
	int inst_size;

	/* Pointer to work_items */
	struct si_work_item_t *scalar_work_item;  
	struct si_work_item_t **work_items;  /* Pointer to first work-items in 'kernel->work_items' */
	union si_reg_t sreg[256];  /* Scalar registers--used by scalar work items */

	/* Predicate mask */
	struct bit_map_t *pred;  /* work_item_count elements */

	/* Flags updated during instruction execution */
	unsigned int global_mem_read : 1;
	unsigned int global_mem_write : 1;
	unsigned int local_mem_read : 1;
	unsigned int local_mem_write : 1;
	unsigned int pred_mask_update : 1;

	/* Linked lists */
	struct si_wavefront_t *running_list_next;
	struct si_wavefront_t *running_list_prev;
	struct si_wavefront_t *barrier_list_next;
	struct si_wavefront_t *barrier_list_prev;
	struct si_wavefront_t *finished_list_next;
	struct si_wavefront_t *finished_list_prev;

	/* To measure simulation performance */
	long long emu_inst_count;  /* Total emulated instructions */
	long long emu_time_start;
	long long emu_time_end;

	/* Fields introduced for architectural simulation */
	int id_in_compute_unit;
	long long sched_when;  /* GPU cycle when wavefront was last scheduled */

	/* Ready to fetch next instruction (for timing simulation) */
	unsigned int ready : 1;

	/* Statistics */
	long long inst_count;  /* Total number of instructions */
	long long scalar_mem_inst_count;
	long long scalar_alu_inst_count;
	long long vector_mem_inst_count;
	long long vector_alu_inst_count;
	long long global_mem_inst_count;  
	long long local_mem_inst_count;  

	int finished;
};

#define SI_FOREACH_WAVEFRONT_IN_NDRANGE(NDRANGE, WAVEFRONT_ID) \
	for ((WAVEFRONT_ID) = (NDRANGE)->wavefront_id_first; \
		(WAVEFRONT_ID) <= (NDRANGE)->wavefront_id_last; \
		(WAVEFRONT_ID)++)

#define SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(WORK_GROUP, WAVEFRONT_ID) \
	for ((WAVEFRONT_ID) = (WORK_GROUP)->wavefront_id_first; \
		(WAVEFRONT_ID) <= (WORK_GROUP)->wavefront_id_last; \
		(WAVEFRONT_ID)++)

struct si_wavefront_t *si_wavefront_create();
void si_wavefront_sreg_init(struct si_wavefront_t *wavefront);
void si_wavefront_free(struct si_wavefront_t *wavefront);
void si_wavefront_dump(struct si_wavefront_t *wavefront, FILE *f);

void si_wavefront_stack_push(struct si_wavefront_t *wavefront);
void si_wavefront_stack_pop(struct si_wavefront_t *wavefront, int count);
void si_wavefront_execute(struct si_wavefront_t *wavefront);

int si_wavefront_work_item_active(struct si_wavefront_t *wavefront, int id_in_wavefront);



/*
 * GPU work_item (Pixel)
 */

#define SI_MAX_LOCAL_MEM_ACCESSES_PER_INST  2

/* Structure describing a memory access definition */
struct si_mem_access_t
{
	int type;  /* 0-none, 1-read, 2-write */
	uint32_t addr;
	int size;
};

struct si_work_item_t
{
	/* IDs */
	int id;  /* global ID */
	int id_in_wavefront;
	int id_in_work_group;  /* local ID */

	/* 3-dimensional IDs */
	int id_3d[3];  /* global 3D IDs */
	int id_in_work_group_3d[3];  /* local 3D IDs */

	/* Wavefront, work-group, and NDRange where it belongs */
	struct si_wavefront_t *wavefront;
	struct si_work_group_t *work_group;
	struct si_ndrange_t *ndrange;

	/* Work-item state */
	union si_reg_t vreg[256];  /* Vector general purpose registers */

	/* Linked list of write tasks. They are enqueued by machine instructions
	 * and executed as a burst at the end of an ALU group. */
	struct linked_list_t *write_task_list;

	/* LDS (Local Data Share) OQs (Output Queues) */
	struct list_t *lds_oqa;
	struct list_t *lds_oqb;

	/* Last global memory access */
	uint32_t global_mem_access_addr;
	uint32_t global_mem_access_size;

	/* Last local memory access */
	int local_mem_access_count;  /* Number of local memory access performed by last instruction */
	uint32_t local_mem_access_addr[SI_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	uint32_t local_mem_access_size[SI_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	int local_mem_access_type[SI_MAX_LOCAL_MEM_ACCESSES_PER_INST];  /* 0-none, 1-read, 2-write */
};

#define SI_FOREACH_WORK_ITEM_IN_NDRANGE(NDRANGE, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (NDRANGE)->work_item_id_first; \
		(WORK_ITEM_ID) <= (NDRANGE)->work_item_id_last; \
		(WORK_ITEM_ID)++)

#define SI_FOREACH_WORK_ITEM_IN_WORK_GROUP(WORK_GROUP, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WORK_GROUP)->work_item_id_first; \
		(WORK_ITEM_ID) <= (WORK_GROUP)->work_item_id_last; \
		(WORK_ITEM_ID)++)

#define SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(WAVEFRONT, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WAVEFRONT)->work_item_id_first; \
		(WORK_ITEM_ID) <= (WAVEFRONT)->work_item_id_last; \
		(WORK_ITEM_ID)++)

struct si_work_item_t *si_work_item_create(void);
void si_work_item_free(struct si_work_item_t *work_item);

/* Consult and change predicate bits */  /* FIXME Remove */
void si_work_item_set_pred(struct si_work_item_t *work_item, int pred);
int si_work_item_get_pred(struct si_work_item_t *work_item);

void si_wavefront_init_sreg_with_value(struct si_wavefront_t *wavefront, int sreg, unsigned int value);
void si_wavefront_init_sreg_with_cb(struct si_wavefront_t *wavefront, int first_reg, int num_regs, 
	int cb);
void si_wavefront_init_sreg_with_uav_table(struct si_wavefront_t *wavefront, int first_reg, 
	int num_regs);




/*
 * Southern Islands ISA
 */


/* Macros for special registers */
#define SI_VCC 106
#define SI_VCCZ 251
#define SI_EXEC 126
#define SI_EXECZ 252
#define SI_SCC 253


/* Debugging */
#define si_isa_debugging() debug_status(si_isa_debug_category)
#define si_isa_debug(...) debug(si_isa_debug_category, __VA_ARGS__)
extern int si_isa_debug_category;


/* Macros for unsupported parameters */
extern char *err_si_isa_note;

#define SI_ISA_ARG_NOT_SUPPORTED(p) \
	fatal("%s: %s: not supported for '" #p "' = 0x%x\n%s", \
	__FUNCTION__, si_isa_inst->info->name, (p), err_si_isa_note);
#define SI_ISA_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: %s: not supported for '" #p "' != 0x%x\n%s", \
	__FUNCTION__, si_isa_inst->info->name, (v), err_si_isa_note); }
#define SI_ISA_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: %s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	__FUNCTION__, si_isa_inst->info->name, (min), (max), err_si_opencl_param_note); }


/* Macros for fast access of instruction words */
#define SI_INST_SMRD		inst->micro_inst.smrd
#define SI_INST_SOPP		inst->micro_inst.sopp
#define SI_INST_SOPK		inst->micro_inst.sopk
#define SI_INST_SOPC		inst->micro_inst.sopc
#define SI_INST_SOP1		inst->micro_inst.sop1
#define SI_INST_SOP2		inst->micro_inst.sop2
#define SI_INST_VOP1		inst->micro_inst.vop1
#define SI_INST_VOP2		inst->micro_inst.vop2
#define SI_INST_VOPC		inst->micro_inst.vopc
#define SI_INST_VOP3b		inst->micro_inst.vop3b
#define SI_INST_VOP3a		inst->micro_inst.vop3a
#define SI_INST_DS			inst->micro_inst.ds
#define SI_INST_MTBUF		inst->micro_inst.mtbuf
/* FIXME Finish filling these in */


/* List of functions implementing GPU instructions 'amd_inst_XXX_impl' */
typedef void (*si_isa_inst_func_t)(struct si_work_item_t *work_item, struct si_inst_t *inst);
extern si_isa_inst_func_t *si_isa_inst_func;



/*
 * Southern Islands GPU Emulator
 */

struct si_emu_t
{
	/* Timer */
	struct m2s_timer_t *timer;

	/* OpenCL objects */
	struct si_opencl_repo_t *opencl_repo;
	struct si_opencl_platform_t *opencl_platform;
	struct si_opencl_device_t *opencl_device;

	/* List of ND-Ranges */
	struct si_ndrange_t *ndrange_list_head;
	struct si_ndrange_t *ndrange_list_tail;
	int ndrange_list_count;
	int ndrange_list_max;

	/* List of pending ND-Ranges */
	struct si_ndrange_t *pending_ndrange_list_head;
	struct si_ndrange_t *pending_ndrange_list_tail;
	int pending_ndrange_list_count;
	int pending_ndrange_list_max;

	/* List of running ND-Ranges */
	struct si_ndrange_t *running_ndrange_list_head;
	struct si_ndrange_t *running_ndrange_list_tail;
	int running_ndrange_list_count;
	int running_ndrange_list_max;

	/* List of finished ND-Ranges */
	struct si_ndrange_t *finished_ndrange_list_head;
	struct si_ndrange_t *finished_ndrange_list_tail;
	int finished_ndrange_list_count;
	int finished_ndrange_list_max;

	/* Global memory */
	struct mem_t *global_mem;
	unsigned int global_mem_top;

	/* Statistics */
	int ndrange_count;  /* Number of OpenCL kernels executed */
	long long inst_count;  /* Number of instructions executed by wavefronts */
	long long scalar_alu_inst_count;  /* Number of scalar ALU instructions executed */
	long long scalar_mem_inst_count;  /* Number of scalar memory instructions executed */
	long long vector_alu_inst_count;  /* Number of vector ALU instructions executed */
	long long vector_mem_inst_count;  /* Number of scalar memory instructions executed */
};


extern enum si_emu_kind_t
{
	si_emu_kind_functional,
	si_emu_kind_detailed
} si_emu_kind;

extern long long si_emu_max_cycles;
extern long long si_emu_max_inst;
extern int si_emu_max_kernels;

extern char *si_emu_opencl_binary_name;
extern char *si_emu_report_file_name;
extern FILE *si_emu_report_file;

extern int si_emu_wavefront_size;

extern char *si_err_opencl_note;
extern char *si_err_opencl_param_note;


extern struct si_emu_t *si_emu;

void si_emu_init(void);
void si_emu_done(void);

void si_emu_run(void);

void si_emu_libopencl_redirect(struct x86_ctx_t *ctx, char *path, int size);
void si_emu_libopencl_failed(int pid);

void si_emu_disasm(char *path);
void si_emu_opengl_disasm(char *path, int opengl_shader_index);

union si_reg_t si_isa_read_sreg(struct si_work_item_t *work_item, int sreg);
void si_isa_write_sreg(struct si_work_item_t *work_item, int sreg, union si_reg_t value);
union si_reg_t si_isa_read_vreg(struct si_work_item_t *work_item, int vreg);
void si_isa_write_vreg(struct si_work_item_t *work_item, int vreg, union si_reg_t value);
union si_reg_t si_isa_read_reg(struct si_work_item_t *work_item, int reg);
void si_isa_bitmask_sreg(struct si_work_item_t *work_item, int sreg, union si_reg_t value);
int si_isa_read_bitmask_sreg(struct si_work_item_t *work_item, int sreg);

void si_isa_read_buf_res(struct si_work_item_t *work_item, struct si_buffer_resource_t *buf_desc, int sreg);
void si_isa_read_mem_ptr(struct si_work_item_t *work_item, struct si_mem_ptr_t *mem_ptr, int sreg);
int si_isa_get_num_elems(int data_format);
int si_isa_get_elem_size(int data_format);

#endif

