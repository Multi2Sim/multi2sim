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

#ifndef ARCH_EVERGREEN_EMU_EMU_H
#define ARCH_EVERGREEN_EMU_EMU_H

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include <arch/evergreen/asm/asm.h>
#include <lib/mhandle/mhandle.h>
#include <lib/misc/misc.h>
#include <lib/struct/config.h>
#include <lib/struct/debug.h>
#include <lib/struct/elf-format.h>
#include <lib/struct/linked-list.h>
#include <lib/struct/list.h>
#include <mem-system/memory.h>

#include "work-item.h"



/*
 * OpenCL API Implementation
 */

/* Forward declaration */
struct x86_ctx_t;

/* Function tables */
extern char *evg_opencl_func_name[];
extern int evg_opencl_func_argc[];

/* Debugging */
#define evg_opencl_debug(...) debug(evg_opencl_debug_category, __VA_ARGS__)
extern int evg_opencl_debug_category;

/* Some constants */
#define EVG_OPENCL_FUNC_FIRST  1000
#define EVG_OPENCL_FUNC_LAST  1073
#define EVG_OPENCL_FUNC_COUNT  (EVG_OPENCL_FUNC_LAST - EVG_OPENCL_FUNC_FIRST + 1)
#define EVG_OPENCL_MAX_ARGS  14

int evg_opencl_api_run(struct x86_ctx_t *ctx);

int evg_opencl_api_read_args(struct x86_ctx_t *ctx, int *argc_ptr,
		void *argv_ptr, int argv_size);
void evg_opencl_api_return(struct x86_ctx_t *ctx, int value);




/*
 * OpenCL objects
 */

enum evg_opencl_object_type_t
{
	evg_opencl_object_invalid,
	evg_opencl_object_platform,
	evg_opencl_object_device,
	evg_opencl_object_context,
	evg_opencl_object_command_queue,
	evg_opencl_object_program,
	evg_opencl_object_kernel,
	evg_opencl_object_mem,
	evg_opencl_object_event,
	evg_opencl_object_sampler
};

struct evg_opencl_repo_t;

struct evg_opencl_repo_t *evg_opencl_repo_create(void);
void evg_opencl_repo_free(struct evg_opencl_repo_t *repo);

void evg_opencl_repo_add_object(struct evg_opencl_repo_t *repo,
	void *object);
void evg_opencl_repo_remove_object(struct evg_opencl_repo_t *repo,
	void *object);
void *evg_opencl_repo_get_object(struct evg_opencl_repo_t *repo,
	enum evg_opencl_object_type_t type, unsigned int object_id);

void *evg_opencl_repo_get_object_of_type(struct evg_opencl_repo_t *repo,
	enum evg_opencl_object_type_t type);
unsigned int evg_opencl_repo_new_object_id(struct evg_opencl_repo_t *repo,
	enum evg_opencl_object_type_t type);

void evg_opencl_repo_free_all_objects(struct evg_opencl_repo_t *repo);




/*
 * OpenCL Platform
 */

struct evg_opencl_platform_t
{
	uint32_t id;
};

struct evg_opencl_platform_t *evg_opencl_platform_create(void);
void evg_opencl_platform_free(struct evg_opencl_platform_t *platform);

uint32_t evg_opencl_platform_get_info(struct evg_opencl_platform_t *platform,
	uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);




/*
 * OpenCL Device
 */

struct evg_opencl_device_t
{
	uint32_t id;
};

struct evg_opencl_device_t *evg_opencl_device_create(void);
void evg_opencl_device_free(struct evg_opencl_device_t *device);

uint32_t evg_opencl_device_get_info(struct evg_opencl_device_t *device, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size);




/*
 * OpenCL Context
 */

struct evg_opencl_context_t
{
	uint32_t id;
	int ref_count;

	uint32_t platform_id;
	uint32_t device_id;
};

struct evg_opencl_context_t *evg_opencl_context_create(void);
void evg_opencl_context_free(struct evg_opencl_context_t *context);

uint32_t evg_opencl_context_get_info(struct evg_opencl_context_t *context,
	uint32_t name, struct mem_t *mem, uint32_t addr, uint32_t size);
void evg_opencl_context_set_properties(struct evg_opencl_context_t *context,
	struct mem_t *mem, uint32_t addr);







/*
 * OpenCL Command Queue
 */

/* Forward declaration of x86 context used below in callback function. This
 * dependence would be removed if OpenCL API implementation was in 'arch/x86/emu'
 * instead. Is this a better option? */
struct x86_ctx_t;

enum evg_opencl_command_type_t
{
	evg_opencl_command_queue_task_invalid,
	evg_opencl_command_queue_task_read_buffer,
	evg_opencl_command_queue_task_write_buffer,
	evg_opencl_command_queue_task_ndrange_kernel
};

struct evg_opencl_command_t
{
	enum evg_opencl_command_type_t type;
	union
	{
		struct
		{
			struct evg_ndrange_t *ndrange;
		} ndrange_kernel;
	} u;
};

struct evg_opencl_command_queue_t
{
	unsigned int id;
	int ref_count;

	unsigned int device_id;
	unsigned int context_id;
	unsigned int properties;

	struct linked_list_t *command_list;
};

struct evg_opencl_command_queue_t *evg_opencl_command_queue_create(void);
void evg_opencl_command_queue_free(struct evg_opencl_command_queue_t *command_queue);

struct evg_opencl_command_t *evg_opencl_command_create(enum
	evg_opencl_command_type_t type);
void evg_opencl_command_free(struct evg_opencl_command_t *command);

void evg_opencl_command_queue_submit(struct evg_opencl_command_queue_t *command_queue,
	struct evg_opencl_command_t *command);
void evg_opencl_command_queue_complete(struct evg_opencl_command_queue_t *command_queue,
	struct evg_opencl_command_t *command);

/* Callback function of type 'x86_ctx_wakeup_callback_func_t'.
 * Argument 'data' is type-casted to 'struct evg_opencl_command_queue_t' */
int evg_opencl_command_queue_can_wakeup(struct x86_ctx_t *ctx, void *data);




/*
 * OpenCL Program
 */

struct evg_opencl_program_t
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

struct evg_opencl_program_t *evg_opencl_program_create(void);
void evg_opencl_program_free(struct evg_opencl_program_t *program);

void evg_opencl_program_build(struct evg_opencl_program_t *program);
void evg_opencl_program_initialize_constant_buffers(struct evg_opencl_program_t *program);

void evg_opencl_program_read_symbol(struct evg_opencl_program_t *program, char *symbol_name,
	struct elf_buffer_t *buffer);




/*
 * OpenCL Sampler
 */

struct evg_opencl_sampler_t
{
	uint32_t id;
	int ref_count;

	uint32_t normalized_coords;
	uint32_t filter_mode;
	uint32_t addressing_mode;
};

struct evg_opencl_sampler_t *evg_opencl_sampler_create(void);
void evg_opencl_sampler_free(struct evg_opencl_sampler_t *sampler);




/*
 * OpenCL Memory Object
 */

struct evg_opencl_mem_t
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

struct evg_opencl_mem_t *evg_opencl_mem_create(void);
void evg_opencl_mem_free(struct evg_opencl_mem_t *mem);




/*
 * OpenCL Image
 */

struct evg_opencl_image_format_t
{
	uint32_t image_channel_order;
	uint32_t image_channel_data_type;
};





/*
 * OpenCL kernel
 */

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


struct evg_opencl_kernel_arg_t
{
	/* Argument properties, as described in .rodata */
	enum evg_opencl_kernel_arg_kind_t kind;
	enum evg_opencl_mem_scope_t mem_scope;  /* For pointers */
	int uav;  /* For memory objects */
	enum evg_opencl_kernel_arg_access_type_t access_type;

	/* Argument fields as set in clSetKernelArg */
	int set;  /* Set to true when it is assigned */
	uint32_t value;  /* 32-bit arguments supported */
	uint32_t size;

	/* Last field - memory assigned variably */
	char name[0];
};

struct evg_opencl_kernel_t
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
	struct list_t *uav_read_list;
	struct list_t *uav_write_list;
	struct list_t *constant_buffer_list;

	/* State of the running kernel */
	struct evg_ndrange_t *ndrange;
};

struct evg_opencl_kernel_t *evg_opencl_kernel_create(void);
void evg_opencl_kernel_free(struct evg_opencl_kernel_t *kernel);

struct evg_opencl_kernel_arg_t *evg_opencl_kernel_arg_create(char *name);
void evg_opencl_kernel_arg_free(struct evg_opencl_kernel_arg_t *arg);

void evg_opencl_kernel_load(struct evg_opencl_kernel_t *kernel, char *kernel_name);
uint32_t evg_opencl_kernel_get_work_group_info(struct evg_opencl_kernel_t *kernel, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size);




/*
 * OpenCL Event
 */

enum evg_opencl_event_kind_t
{
	EVG_OPENCL_EVENT_NONE = 0,
	EVG_OPENCL_EVENT_NDRANGE_KERNEL,
	EVG_OPENCL_EVENT_TASK,
	EVG_OPENCL_EVENT_NATIVE_KERNEL,
	EVG_OPENCL_EVENT_READ_BUFFER,
	EVG_OPENCL_EVENT_WRITE_BUFFER,
	EVG_OPENCL_EVENT_MAP_BUFFER,
	EVG_OPENCL_EVENT_UNMAP_MEM_OBJECT,
	EVG_OPENCL_EVENT_READ_BUFFER_RECT,
	EVG_OPENCL_EVENT_WRITE_BUFFER_RECT,
	EVG_OPENCL_EVENT_READ_IMAGE,
	EVG_OPENCL_EVENT_WRITE_IMAGE,
	EVG_OPENCL_EVENT_MAP_IMAGE,
	EVG_OPENCL_EVENT_COPY_BUFFER,
	EVG_OPENCL_EVENT_COPY_IMAGE,
	EVG_OPENCL_EVENT_COPY_BUFFER_RECT,
	EVG_OPENCL_EVENT_COPY_BUFFER_TO_IMAGE,
	EVG_OPENCL_EVENT_COPY_IMAGE_TO_BUFFER,
	EVG_OPENCL_EVENT_MARKER,
	EVG_OPENCL_EVENT_COUNT
};

enum evg_opencl_event_status_t
{
	EVG_OPENCL_EVENT_STATUS_NONE = 0,
	EVG_OPENCL_EVENT_STATUS_QUEUED,
	EVG_OPENCL_EVENT_STATUS_SUBMITTED,
	EVG_OPENCL_EVENT_STATUS_RUNNING,
	EVG_OPENCL_EVENT_STATUS_COMPLETE
};

struct evg_opencl_event_t
{
	uint32_t id;
	int ref_count;
	enum evg_opencl_event_kind_t kind;
	enum evg_opencl_event_status_t status;

	long long time_queued;
	long long time_submit;
	long long time_start;
	long long time_end;
};

struct evg_opencl_event_t *evg_opencl_event_create(enum evg_opencl_event_kind_t kind);
void evg_opencl_event_free(struct evg_opencl_event_t *event);

uint32_t evg_opencl_event_get_profiling_info(struct evg_opencl_event_t *event, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size);
long long evg_opencl_event_timer(void);






/*
 * Evergreen GPU Emulator
 */

struct evg_emu_t
{
	/* Timer */
	struct m2s_timer_t *timer;

	/* OpenCL objects */
	struct evg_opencl_repo_t *opencl_repo;
	struct evg_opencl_platform_t *opencl_platform;
	struct evg_opencl_device_t *opencl_device;

	/* List of ND-Ranges */
	struct evg_ndrange_t *ndrange_list_head;
	struct evg_ndrange_t *ndrange_list_tail;
	int ndrange_list_count;
	int ndrange_list_max;

	/* List of pending ND-Ranges */
	struct evg_ndrange_t *pending_ndrange_list_head;
	struct evg_ndrange_t *pending_ndrange_list_tail;
	int pending_ndrange_list_count;
	int pending_ndrange_list_max;

	/* List of running ND-Ranges */
	struct evg_ndrange_t *running_ndrange_list_head;
	struct evg_ndrange_t *running_ndrange_list_tail;
	int running_ndrange_list_count;
	int running_ndrange_list_max;

	/* List of finished ND-Ranges */
	struct evg_ndrange_t *finished_ndrange_list_head;
	struct evg_ndrange_t *finished_ndrange_list_tail;
	int finished_ndrange_list_count;
	int finished_ndrange_list_max;

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

	/* Stats */
	int ndrange_count;  /* Number of OpenCL kernels executed */
	long long inst_count;  /* Number of instructions executed by wavefronts */
};


extern enum evg_emu_kind_t
{
	evg_emu_kind_functional,
	evg_emu_kind_detailed
} evg_emu_kind;

extern long long evg_emu_max_cycles;
extern long long evg_emu_max_inst;
extern int evg_emu_max_kernels;

extern char *evg_emu_opencl_binary_name;
extern char *evg_emu_report_file_name;
extern FILE *evg_emu_report_file;

extern int evg_emu_wavefront_size;

extern char *evg_err_opencl_note;
extern char *evg_err_opencl_param_note;


extern struct evg_emu_t *evg_emu;

void evg_emu_init(void);
void evg_emu_done(void);

void evg_emu_dump_summary(FILE *f);

int evg_emu_run(void);

void evg_emu_libopencl_redirect(struct x86_ctx_t *ctx, char *path, int size);
void evg_emu_libopencl_failed(int pid);

void evg_emu_disasm(char *path);
void evg_emu_opengl_disasm(char *path, int opengl_shader_index);

#endif

