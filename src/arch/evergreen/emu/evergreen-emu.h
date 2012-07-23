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

#ifndef EVERGREEN_EMU_H
#define EVERGREEN_EMU_H

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

#include <evergreen-asm.h>
#include <mem-system.h>



/*
 * AMD Evergreen Binary File (Internal ELF)
 */


/* Encoding dictionary entry header (as encoded in ELF file) */
struct evg_bin_enc_dict_entry_header_t
{
	Elf32_Word d_machine;
	Elf32_Word d_type;
	Elf32_Off d_offset;  /* Offset for encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_size;  /* Size of encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_flags;
};


/* Constats embedded in the '.data' section */
struct evg_bin_enc_dict_entry_consts_t
{
	float float_consts[256][4];
	unsigned int int_consts[32][4];
	unsigned int bool_consts[32];
};


/* Encoding dictionary entry */
struct evg_bin_enc_dict_entry_t
{
	/* Header (pointer to ELF buffer contents) */
	struct evg_bin_enc_dict_entry_header_t *header;

	/* Buffers containing PT_LOAD and PT_NOTE segments */
	struct elf_buffer_t pt_load_buffer;
	struct elf_buffer_t pt_note_buffer;

	/* Buffers containing sections */
	struct elf_buffer_t sec_text_buffer;
	struct elf_buffer_t sec_data_buffer;
	struct elf_buffer_t sec_symtab_buffer;
	struct elf_buffer_t sec_strtab_buffer;

	/* Constants extract from '.data' section */
	struct evg_bin_enc_dict_entry_consts_t *consts;

	/* Info read from pt_notes */
	int num_gpr_used;
	int lds_size_used;
	int stack_size_used;
};


/* Binary file */
struct evg_bin_file_t
{
	/* Associated ELF file */
	struct elf_file_t *elf_file;

	/* Encoding dictionary.
	 * Elements are of type 'struct evg_bin_enc_dict_entry_t'
	 * Each element of the dictionary contains the binary for a different architecture
	 * (Evergreen, x86, etc.) */
	struct list_t *enc_dict;

	/* Encoding dictionary entry containing the Evergree kernel.
	 * This is a member of the 'enc_dict' list. */
	struct evg_bin_enc_dict_entry_t *enc_dict_entry_evergreen;
};

struct evg_bin_file_t *evg_bin_file_create(void *ptr, int size, char *name);
void evg_bin_file_free(struct evg_bin_file_t *bin);




/*
 * AMD OpenGL Binary File (Internal ELF)
 */

/* Shader types */
enum evg_opengl_shader_kind_t
{
	EVG_OPENGL_SHADER_VERTEX,
	EVG_OPENGL_SHADER_FRAGMENT,
	EVG_OPENGL_SHADER_GEOMETRY,
	EVG_OPENGL_SHADER_EVALUATION,
	EVG_OPENGL_SHADER_CONTROL
};

/* OpenGL shader binary */
struct evg_opengl_shader_t
{
	/* Shader kind */
	enum evg_opengl_shader_kind_t shader_kind;

	/* Associated ELF file */
	struct elf_file_t *external_elf_file;
	struct elf_file_t *internal_elf_file;

	/* ISA buffer, which ptr element points to .text section in internel_elf_file  */
	struct elf_buffer_t isa_buffer;
};

/* OpenGL shader binary */
struct evg_opengl_bin_file_t
{
	/* Name of the associated binary file */
	char *name;
	
	/* List of shaders associated with binary file.
	 * Elements are of type 'struct evg_opengl_shader_t' */
	struct list_t *shader_list;

	/* NEED or NOT ? */
	// struct evg_opengl_shader_t *amd_opengl_shader;
};

struct evg_opengl_bin_file_t *evg_opengl_bin_file_create(void *ptr, int size, char *name);
void evg_opengl_bin_file_free(struct evg_opengl_bin_file_t *bin_file);





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
 * ND-Range
 */

enum evg_ndrange_status_t
{
	evg_ndrange_pending		= 0x0001,
	evg_ndrange_running		= 0x0002,
	evg_ndrange_finished		= 0x0004
};

struct evg_ndrange_t
{
	/* ID */
	char *name;
	int id;  /* Sequential ndrange ID (given by evg_emu->ndrange_count counter) */

	/* Status */
	enum evg_ndrange_status_t status;

	/* OpenCL kernel associated */
	struct evg_opencl_kernel_t *kernel;

	/* Command queue and command queue task associated */
	struct evg_opencl_command_queue_t *command_queue;
	struct evg_opencl_command_t *command;

	/* Pointers to work-groups, wavefronts, and work_items */
	struct evg_work_group_t **work_groups;
	struct evg_wavefront_t **wavefronts;
	struct evg_work_item_t **work_items;

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
	int wavefronts_per_work_group;  /* = ceil(local_size / evg_emu_wavefront_size) */

	/* List of ND-Ranges */
	struct evg_ndrange_t *ndrange_list_prev;
	struct evg_ndrange_t *ndrange_list_next;
	struct evg_ndrange_t *pending_ndrange_list_prev;
	struct evg_ndrange_t *pending_ndrange_list_next;
	struct evg_ndrange_t *running_ndrange_list_prev;
	struct evg_ndrange_t *running_ndrange_list_next;
	struct evg_ndrange_t *finished_ndrange_list_prev;
	struct evg_ndrange_t *finished_ndrange_list_next;

	/* List of pending work-groups */
	struct evg_work_group_t *pending_list_head;
	struct evg_work_group_t *pending_list_tail;
	int pending_list_count;
	int pending_list_max;

	/* List of running work-groups */
	struct evg_work_group_t *running_list_head;
	struct evg_work_group_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of finished work-groups */
	struct evg_work_group_t *finished_list_head;
	struct evg_work_group_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Local memory top to assign to local arguments.
	 * Initially it is equal to the size of local variables in kernel function. */
	unsigned int local_mem_top;


	/* Statistics */

	/* Histogram of executed instructions. Only allocated if the kernel report
	 * option is active. */
	unsigned int *inst_histogram;
};

struct evg_ndrange_t *evg_ndrange_create(struct evg_opencl_kernel_t *kernel);
void evg_ndrange_free(struct evg_ndrange_t *ndrange);
void evg_ndrange_dump(struct evg_ndrange_t *ndrange, FILE *f);

int evg_ndrange_get_status(struct evg_ndrange_t *ndrange, enum evg_ndrange_status_t status);
void evg_ndrange_set_status(struct evg_ndrange_t *work_group, enum evg_ndrange_status_t status);
void evg_ndrange_clear_status(struct evg_ndrange_t *work_group, enum evg_ndrange_status_t status);

void evg_ndrange_setup_work_items(struct evg_ndrange_t *ndrange);
void evg_ndrange_setup_const_mem(struct evg_ndrange_t *ndrange);
void evg_ndrange_setup_args(struct evg_ndrange_t *ndrange);

void evg_ndrange_finish(struct evg_ndrange_t *ndrange);




/*
 * GPU Work-Group
 */

enum evg_work_group_status_t
{
	evg_work_group_pending		= 0x0001,
	evg_work_group_running		= 0x0002,
	evg_work_group_finished		= 0x0004
};

struct evg_work_group_t
{
	/* ID */
	char *name;
	int id;  /* Group ID */
	int id_3d[3];  /* 3-dimensional Group ID */

	/* Status */
	enum evg_work_group_status_t status;

	/* NDRange it belongs to */
	struct evg_ndrange_t *ndrange;

	/* IDs of work-items contained */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* IDs of wavefronts contained */
	int wavefront_id_first;
	int wavefront_id_last;
	int wavefront_count;

	/* Pointers to wavefronts and work-items */
	struct evg_work_item_t **work_items;  /* Pointer to first work_item in 'kernel->work_items' */
	struct evg_wavefront_t **wavefronts;  /* Pointer to first wavefront in 'kernel->wavefronts' */

	/* Double linked lists of work-groups */
	struct evg_work_group_t *pending_list_prev;
	struct evg_work_group_t *pending_list_next;
	struct evg_work_group_t *running_list_prev;
	struct evg_work_group_t *running_list_next;
	struct evg_work_group_t *finished_list_prev;
	struct evg_work_group_t *finished_list_next;

	/* List of running wavefronts */
	struct evg_wavefront_t *running_list_head;
	struct evg_wavefront_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of wavefronts in barrier */
	struct evg_wavefront_t *barrier_list_head;
	struct evg_wavefront_t *barrier_list_tail;
	int barrier_list_count;
	int barrier_list_max;

	/* List of finished wavefronts */
	struct evg_wavefront_t *finished_list_head;
	struct evg_wavefront_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
	
	/* Fields introduced for architectural simulation */
	int id_in_compute_unit;
	int compute_unit_finished_count;  /* like 'finished_list_count', but when WF reaches Complete stage */

	/* Local memory */
	struct mem_t *local_mem;
};

#define EVG_FOR_EACH_WORK_GROUP_IN_NDRANGE(NDRANGE, WORK_GROUP_ID) \
	for ((WORK_GROUP_ID) = (NDRANGE)->work_group_id_first; \
		(WORK_GROUP_ID) <= (NDRANGE)->work_group_id_last; \
		(WORK_GROUP_ID)++)

struct evg_work_group_t *evg_work_group_create(void);
void evg_work_group_free(struct evg_work_group_t *work_group);
void evg_work_group_dump(struct evg_work_group_t *work_group, FILE *f);

void evg_work_group_set_name(struct evg_work_group_t *work_group, char *name);

int evg_work_group_get_status(struct evg_work_group_t *work_group, enum evg_work_group_status_t status);
void evg_work_group_set_status(struct evg_work_group_t *work_group, enum evg_work_group_status_t status);
void evg_work_group_clear_status(struct evg_work_group_t *work_group, enum evg_work_group_status_t status);




/*
 * GPU Wavefront
 */

/* Type of clauses */
enum evg_clause_kind_t
{
	EVG_CLAUSE_NONE = 0,
	EVG_CLAUSE_CF,  /* Control-flow */
	EVG_CLAUSE_ALU,  /* ALU clause */
	EVG_CLAUSE_TEX,  /* Fetch trough a Texture Cache Clause */
	EVG_CLAUSE_VC  /* Fetch through a Vertex Cache Clause */
};


#define EVG_MAX_STACK_SIZE  32

/* Wavefront */
struct evg_wavefront_t
{
	/* ID */
	char *name;
	int id;
	int id_in_work_group;

	/* IDs of work-items it contains */
	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	/* NDRange and Work-group it belongs to */
	struct evg_ndrange_t *ndrange;
	struct evg_work_group_t *work_group;

	/* Pointer to work_items */
	struct evg_work_item_t **work_items;  /* Pointer to first work-items in 'kernel->work_items' */

	/* Current clause kind and instruction pointers */
	enum evg_clause_kind_t clause_kind;

	/* Current instructions */
	struct evg_inst_t cf_inst;
	struct evg_alu_group_t alu_group;
	struct evg_inst_t tex_inst;

	/* Starting/current CF buffer */
	void *cf_buf_start;
	void *cf_buf;

	/* Secondary clause boundaries and current position */
	void *clause_buf;
	void *clause_buf_start;
	void *clause_buf_end;

	/* Active mask stack */
	struct bit_map_t *active_stack;  /* EVG_MAX_STACK_SIZE * work_item_count elements */
	int stack_top;

	/* Predicate mask */
	struct bit_map_t *pred;  /* work_item_count elements */

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
	struct evg_wavefront_t *running_list_next;
	struct evg_wavefront_t *running_list_prev;
	struct evg_wavefront_t *barrier_list_next;
	struct evg_wavefront_t *barrier_list_prev;
	struct evg_wavefront_t *finished_list_next;
	struct evg_wavefront_t *finished_list_prev;

	/* Fields introduced for architectural simulation */
	int id_in_compute_unit;
	int alu_engine_in_flight;  /* Number of in-flight uops in ALU engine */
	long long sched_when;  /* GPU cycle when wavefront was last scheduled */


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

#define EVG_FOREACH_WAVEFRONT_IN_NDRANGE(NDRANGE, WAVEFRONT_ID) \
	for ((WAVEFRONT_ID) = (NDRANGE)->wavefront_id_first; \
		(WAVEFRONT_ID) <= (NDRANGE)->wavefront_id_last; \
		(WAVEFRONT_ID)++)

#define EVG_FOREACH_WAVEFRONT_IN_WORK_GROUP(WORK_GROUP, WAVEFRONT_ID) \
	for ((WAVEFRONT_ID) = (WORK_GROUP)->wavefront_id_first; \
		(WAVEFRONT_ID) <= (WORK_GROUP)->wavefront_id_last; \
		(WAVEFRONT_ID)++)

struct evg_wavefront_t *evg_wavefront_create(void);
void evg_wavefront_free(struct evg_wavefront_t *wavefront);
void evg_wavefront_dump(struct evg_wavefront_t *wavefront, FILE *f);

void evg_wavefront_set_name(struct evg_wavefront_t *wavefront, char *name);

void evg_wavefront_stack_push(struct evg_wavefront_t *wavefront);
void evg_wavefront_stack_pop(struct evg_wavefront_t *wavefront, int count);
void evg_wavefront_execute(struct evg_wavefront_t *wavefront);




/*
 * GPU work_item (Pixel)
 */

#define EVG_MAX_GPR_ELEM  5
#define EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST  2

struct evg_gpr_t
{
	uint32_t elem[EVG_MAX_GPR_ELEM];  /* x, y, z, w, t */
};

/* Structure describing a memory access definition */
struct evg_mem_access_t
{
	int type;  /* 0-none, 1-read, 2-write */
	uint32_t addr;
	int size;
};

struct evg_work_item_t
{
	/* IDs */
	int id;  /* global ID */
	int id_in_wavefront;
	int id_in_work_group;  /* local ID */

	/* 3-dimensional IDs */
	int id_3d[3];  /* global 3D IDs */
	int id_in_work_group_3d[3];  /* local 3D IDs */

	/* Wavefront, work-group, and NDRange where it belongs */
	struct evg_wavefront_t *wavefront;
	struct evg_work_group_t *work_group;
	struct evg_ndrange_t *ndrange;

	/* Work-item state */
	struct evg_gpr_t gpr[128];  /* General purpose registers */
	struct evg_gpr_t pv;  /* Result of last computations */

	/* Linked list of write tasks. They are enqueued by machine instructions
	 * and executed as a burst at the end of an ALU group. */
	struct linked_list_t *write_task_list;

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

	/* Last local memory access */
	int local_mem_access_count;  /* Number of local memory access performed by last instruction */
	uint32_t local_mem_access_addr[EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	uint32_t local_mem_access_size[EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	int local_mem_access_type[EVG_MAX_LOCAL_MEM_ACCESSES_PER_INST];  /* 0-none, 1-read, 2-write */
};

#define EVG_FOREACH_WORK_ITEM_IN_NDRANGE(NDRANGE, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (NDRANGE)->work_item_id_first; \
		(WORK_ITEM_ID) <= (NDRANGE)->work_item_id_last; \
		(WORK_ITEM_ID)++)

#define EVG_FOREACH_WORK_ITEM_IN_WORK_GROUP(WORK_GROUP, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WORK_GROUP)->work_item_id_first; \
		(WORK_ITEM_ID) <= (WORK_GROUP)->work_item_id_last; \
		(WORK_ITEM_ID)++)

#define EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(WAVEFRONT, WORK_ITEM_ID) \
	for ((WORK_ITEM_ID) = (WAVEFRONT)->work_item_id_first; \
		(WORK_ITEM_ID) <= (WAVEFRONT)->work_item_id_last; \
		(WORK_ITEM_ID)++)

struct evg_work_item_t *evg_work_item_create(void);
void evg_work_item_free(struct evg_work_item_t *work_item);

/* Consult and change active/predicate bits */
void evg_work_item_set_active(struct evg_work_item_t *work_item, int active);
int evg_work_item_get_active(struct evg_work_item_t *work_item);
void evg_work_item_set_pred(struct evg_work_item_t *work_item, int pred);
int evg_work_item_get_pred(struct evg_work_item_t *work_item);
void evg_work_item_update_branch_digest(struct evg_work_item_t *work_item,
	long long inst_count, uint32_t inst_addr);




/*
 * GPU Write Tasks
 */

enum evg_isa_write_task_kind_t
{
	EVG_ISA_WRITE_TASK_NONE = 0,
	EVG_ISA_WRITE_TASK_WRITE_LDS,
	EVG_ISA_WRITE_TASK_WRITE_DEST,
	EVG_ISA_WRITE_TASK_PUSH_BEFORE,
	EVG_ISA_WRITE_TASK_SET_PRED
};


struct evg_isa_write_task_t
{
	/* Work-item affected */
	struct evg_work_item_t *work_item;

	/* All */
	enum evg_isa_write_task_kind_t kind;
	struct evg_inst_t *inst;
	
	/* When 'kind' == EVG_ISA_WRITE_TASK_WRITE_DEST */
	int gpr, rel, chan, index_mode, write_mask;
	unsigned int value;

	/* When 'kind' == EVG_ISA_WRITE_TASK_WRITE_LDS */
	unsigned int lds_addr;
	unsigned int lds_value;
        int lds_value_size;

	/* When 'kind' == GPU_ISA_WRITE_TASK_PRED_SET */
	int cond;
};


/* Repository for 'struct evg_isa_write_task_t' objects */
extern struct repos_t *evg_isa_write_task_repos;


/* Functions to handle deferred tasks */
void evg_isa_enqueue_write_lds(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, unsigned int addr, unsigned int value,
	int value_size);
void evg_isa_enqueue_write_dest(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, unsigned int value);
void evg_isa_enqueue_write_dest_float(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, float value);
void evg_isa_enqueue_push_before(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst);
void evg_isa_enqueue_pred_set(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, int cond);

void evg_isa_write_task_commit(struct evg_work_item_t *work_item);




/*
 * Evergreen ISA
 */

/* Debugging */
#define evg_isa_debugging() debug_status(evg_isa_debug_category)
#define evg_isa_debug(...) debug(evg_isa_debug_category, __VA_ARGS__)
extern int evg_isa_debug_category;


/* Macros for unsupported parameters */
extern char *evg_err_isa_note;

#define EVG_ISA_ARG_NOT_SUPPORTED(p) \
	fatal("%s: %s: not supported for '" #p "' = 0x%x\n%s", \
	__FUNCTION__, inst->info->name, (p), evg_err_isa_note);
#define EVG_ISA_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: %s: not supported for '" #p "' != 0x%x\n%s", \
	__FUNCTION__, inst->info->name, (v), evg_err_isa_note); }
#define EVG_ISA_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: %s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	__FUNCTION__, inst->info->name, (min), (max), evg_err_opencl_param_note); }


/* Macros for fast access of instruction words */
#define EVG_CF_WORD0			inst->words[0].cf_word0
#define EVG_CF_GWS_WORD0		inst->words[0].cf_gws_word0
#define EVG_CF_WORD1			inst->words[1].cf_word1

#define EVG_CF_ALU_WORD0		inst->words[0].cf_alu_word0
#define EVG_CF_ALU_WORD1		inst->words[1].cf_alu_word1
#define EVG_CF_ALU_WORD0_EXT		inst->words[0].cf_alu_word0_ext
#define EVG_CF_ALU_WORD1_EXT		inst->words[1].cf_alu_word1_ext

#define EVG_CF_ALLOC_EXPORT_WORD0	inst->words[0].cf_alloc_export_word0
#define EVG_CF_ALLOC_EXPORT_WORD0_RAT	inst->words[0].cf_alloc_export_word0_rat
#define EVG_CF_ALLOC_EXPORT_WORD1_BUF	inst->words[1].cf_alloc_export_word1_buf
#define EVG_CF_ALLOC_EXPORT_WORD1_SWIZ	inst->words[1].cf_alloc_export_word1_swiz

#define EVG_ALU_WORD0			inst->words[0].alu_word0
#define EVG_ALU_WORD1_OP2		inst->words[1].alu_word1_op2
#define EVG_ALU_WORD1_OP3		inst->words[1].alu_word1_op3

#define EVG_ALU_WORD0_LDS_IDX_OP	inst->words[0].alu_word0_lds_idx_op
#define EVG_ALU_WORD1_LDS_IDX_OP	inst->words[1].alu_word1_lds_idx_op

#define EVG_VTX_WORD0			inst->words[0].vtx_word0
#define EVG_VTX_WORD1_GPR		inst->words[1].vtx_word1_gpr
#define EVG_VTX_WORD1_SEM		inst->words[1].vtx_word1_sem
#define EVG_VTX_WORD2			inst->words[2].vtx_word2

#define EVG_TEX_WORD0			inst->words[0].tex_word0
#define EVG_TEX_WORD1			inst->words[1].tex_word1
#define EVG_TEX_WORD2			inst->words[2].tex_word2

#define EVG_MEM_RD_WORD0		inst->words[0].mem_rd_word0
#define EVG_MEM_RD_WORD1		inst->words[1].mem_rd_word1
#define EVG_MEM_RD_WORD2		inst->words[2].mem_rd_word2

#define EVG_MEM_GDS_WORD0		inst->words[0].mem_gds_word0
#define EVG_MEM_GDS_WORD1		inst->words[1].mem_gds_word1
#define EVG_MEM_GDS_WORD2		inst->words[2].mem_gds_word2


/* Table of functions implementing implementing the Evergreen ISA */
typedef void (*evg_isa_inst_func_t)(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst);
extern evg_isa_inst_func_t *evg_isa_inst_func;

/* Declarations of function prototypes implementing Evergreen ISA */
#define DEFINST(_name, _fmt_str, _fmt0, _fmt1, _fmt2, _category, _opcode, _flags) \
	extern void evg_isa_##_name##_impl(struct evg_work_item_t *work_item, \
			struct evg_inst_t *inst);
#include <evergreen-asm.dat>
#undef DEFINST

/* Access to constant memory */
void evg_isa_const_mem_write(int bank, int vector, int elem, void *value_ptr);
void evg_isa_const_mem_read(int bank, int vector, int elem, void *value_ptr);

/* For ALU clauses */
void evg_isa_alu_clause_start(struct evg_wavefront_t *wavefront);
void evg_isa_alu_clause_end(struct evg_wavefront_t *wavefront);

/* For TC clauses */
void evg_isa_tc_clause_start(struct evg_wavefront_t *wavefront);
void evg_isa_tc_clause_end(struct evg_wavefront_t *wavefront);

/* Read from source register */
unsigned int evg_isa_read_gpr(struct evg_work_item_t *work_item,
	int gpr, int rel, int chan, int im);
float evg_isa_read_gpr_float(struct evg_work_item_t *work_item,
	int gpr, int rel, int chan, int im);

/* Write into destination register */
void evg_isa_write_gpr(struct evg_work_item_t *work_item,
	int gpr, int rel, int chan, unsigned int value);
void evg_isa_write_gpr_float(struct evg_work_item_t *work_item,
	int gpr, int rel, int chan, float value);

/* Read input operands */
unsigned int evg_isa_read_op_src_int(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, int src_idx);
float evg_isa_read_op_src_float(struct evg_work_item_t *work_item,
	struct evg_inst_t *inst, int src_idx);

struct evg_inst_t *evg_isa_get_alu_inst(struct evg_alu_group_t *alu_group,
	enum evg_alu_enum alu);

void evg_isa_init(void);
void evg_isa_done(void);





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

