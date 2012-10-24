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

#ifndef M2S_CLRT_H
#define M2S_CLRT_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <elf.h>

#include <util/debug.h>
#include <CL/cl.h>


/*
 * Runtime System Calls
 */

/* Version for the Multi2Sim Runtime library implementation */
#define M2S_CLRT_VERSION_MAJOR  1
#define M2S_CLRT_VERSION_MINOR  752

/* System call code to communicate with Multi2Sim */
#define M2S_CLRT_SYS_CODE  329

/* Function code to pass as a first argument of a system call */
enum m2s_clrt_call_t
{
	m2s_clrt_call_invalid,
#define X86_CLRT_DEFINE_CALL(name, code) m2s_clrt_call_##name = code,
#include "../../src/arch/x86/emu/clrt.dat"
#undef X86_CLRT_DEFINE_CALL
	m2s_clrt_call_count
};

/* Executing in native mode (vs. simulation mode on Multi2Sim) */
extern int m2s_clrt_native_mode;

/* Debug */
void m2s_clrt_debug(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));




/*
 * Error macros
 */

extern char *m2s_clrt_err_not_impl;
extern char *m2s_clrt_err_note;
extern char *m2s_clrt_err_param_note;

#define __M2S_CLRT_NOT_IMPL__  \
	fatal("%s: OpenCL call not implemented.\n%s", __FUNCTION__, m2s_clrt_err_not_impl);
#define EVG_OPENCL_ARG_NOT_SUPPORTED(p) \
	fatal("%s: not supported for '" #p "' = 0x%x\n%s", __FUNCTION__, p, m2s_clrt_err_note);
#define EVG_OPENCL_ARG_NOT_SUPPORTED_EQ(p, v) \
	{ if ((p) == (v)) fatal("%s: not supported for '" #p "' = 0x%x\n%s", __FUNCTION__, (v), m2s_clrt_err_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: not supported for '" #p "' != 0x%x\n%s", __FUNCTION__, (v), m2s_clrt_err_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_LT(p, v) \
	{ if ((p) < (v)) fatal("%s: not supported for '" #p "' < %d\n%s", __FUNCTION__, (v), m2s_clrt_err_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	__FUNCTION__, (min), (max), m2s_clrt_err_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_FLAG(p, flag, name) \
	{ if ((p) & (flag)) fatal("%s: flag '" name "' not supported\n%s", __FUNCTION__, m2s_clrt_err_param_note); }




/* 
 * Private Fiber definitions 
 */

typedef void (*fiber_proc_t)(void);
typedef void (*return_address_t)(void);

struct fiber_t
{
	void *esp;
	void *eip;
	void *stack_bottom;
	size_t stack_size;
};

void make_fiber(struct fiber_t *fiber, fiber_proc_t fiber_proc, int num_args, ...);
void make_fiber_ex(struct fiber_t *fiber, fiber_proc_t fiber_proc, return_address_t return_proc, int arg_size, void *args);
void switch_fiber(struct fiber_t *current, struct fiber_t *dest);
void switch_fiber_cl(struct fiber_t *current, struct fiber_t *dest, void *reg_values);
void exit_fiber(struct fiber_t *dest);




/*
 * Private ELF-related definitions 
 */

Elf32_Shdr *get_section_header(void *elf, char *name);
void *get_inner_elf_addr(const unsigned char *outer_elf, uint32_t *size);
void *get_function_info(void *handle, const char *name, size_t **metadata);




/*
 * Private OpenCL implementation definitions
 */

typedef void (*clrt_function_t)(void);
typedef void (*clrt_barrier_t)(int option);

enum clrt_param_type_t
{
	CLRT_TYPE_INVALID = 0,
	CLRT_TYPE_CHAR = 1,
	CLRT_TYPE_SHORT = 2,
	CLRT_TYPE_INT = 3,
	CLRT_TYPE_LONG = 4,
	CLRT_TYPE_FLOAT = 5,
	CLRT_TYPE_DOUBLE = 6,
	CLRT_TYPE_POINTER = 7,
	CLRT_TYPE_CHAR2 = 8,
	CLRT_TYPE_CHAR3 = 9,
	CLRT_TYPE_CHAR4 = 10,
	CLRT_TYPE_CHAR8 = 11,
	CLRT_TYPE_CHAR16 = 12,
	CLRT_TYPE_SHORT2 = 13,
	CLRT_TYPE_SHORT3 = 14,
	CLRT_TYPE_SHORT4 = 15,
	CLRT_TYPE_SHORT8 = 16,
	CLRT_TYPE_SHORT16 = 17,
	CLRT_TYPE_INT2 = 18,
	CLRT_TYPE_INT3 = 19,
	CLRT_TYPE_INT4 = 20,
	CLRT_TYPE_INT8 = 21,
	CLRT_TYPE_INT16 = 22,
	CLRT_TYPE_LONG2 = 23,
	CLRT_TYPE_LONG3 = 24,
	CLRT_TYPE_LONG4 = 25,
	CLRT_TYPE_LONG8 = 26,
	CLRT_TYPE_LONG16 = 27,
	CLRT_TYPE_FLOAT2 = 28,
	CLRT_TYPE_FLOAT3 = 29,
	CLRT_TYPE_FLOAT4 = 30,
	CLRT_TYPE_FLOAT8 = 31,
	CLRT_TYPE_FLOAT16 = 32,
	CLRT_TYPE_DOUBLE2 = 33,
	CLRT_TYPE_DOUBLE3 = 34,
	CLRT_TYPE_DOUBLE4 = 35,
	CLRT_TYPE_DOUBLE8 = 36,
	CLRT_TYPE_DOUBLE16 = 37
};

/*
* Does not start with invalid as this is based on the ELF produced 
* by the AMD APP SDK.
*/

enum clrt_memory_t
{
	CLRT_MEM_VALUE = 0,
	CLRT_MEM_LOCAL = 1,
	CLRT_MEM_CONSTANT = 2,
	CLRT_MEM_GLOBAL = 3
};

struct clrt_parameter_t
{
	enum clrt_param_type_t param_type;
	enum clrt_memory_t mem_type;
	int is_set;
	int stack_offset;
	int is_stack;
	int reg_offset;
	int size;
};

struct clrt_reg_param_t
{
	int reg[4];
};

struct clrt_workitem_data_t
{
	int workgroup_data;  /* 0x60 (Not actually part of AMD runtime, padding_0) */
	int barrier_func;  /* 0x5c (function *) */
	int padding_1;  /* 0x58 */
	int work_dim;  /* 0x54 */
	int group_global[4];  /* [0x50, 0x44] */
	int global_size[4];  /* [0x40, 0x34] */
	int local_size[4];  /* [0x30, 0x24] */
	int group_id[4];  /* [0x20, 0x14] */
	int global_id[4];  /* [0x10, 0x04] */
};

struct clrt_workgroup_data_t
{
	int num_done;
	int num_items;
	int cur_item;
	struct fiber_t main_ctx;
	struct fiber_t *cur_ctx;
	struct fiber_t *workitems;
	struct clrt_workitem_data_t **workitem_data;
	unsigned int *stack_params;
	char *aligned_stacks;
};

struct clrt_workgroup_data_t *get_workgroup_data(); 

struct clrt_execution_t
{
	struct _cl_kernel *kernel;
	int dims;
	size_t *global;
	size_t *local;

	pthread_mutex_t mutex;
	int num_groups;
	volatile int next_group;
	size_t *group_starts;
};

struct clrt_buffer_list_t
{
	struct clrt_buffer_list_t *next;
	void *aligned;
	void *raw;
};

void *clrt_buffer_allocate(size_t size);
void clrt_buffer_free(void *buffer);




/*
 * OpenCL Objects
 */

typedef void (*clrt_object_destroy_func_t)(void *);

enum clrt_object_type_t
{
	CLRT_OBJECT_INVALID,
	CLRT_OBJECT_CONTEXT,
	CLRT_OBJECT_COMMAND_QUEUE,
	CLRT_OBJECT_MEM,
	CLRT_OBJECT_PROGRAM,
	CLRT_OBJECT_KERNEL,
	CLRT_OBJECT_EVENT,
	CLRT_OBJECT_SAMPLER
};

struct clrt_object_t
{
	struct clrt_object_t *next;
	enum clrt_object_type_t type;

	/* Callback function to destroy data */
	clrt_object_destroy_func_t destroy_func;

	/* Number of references to the data */
	int ref_count;
	pthread_mutex_t ref_mutex;

	/* Object itself */
	void *data;
};

struct clrt_object_t *clrt_object_create(void *data, enum clrt_object_type_t type,
		clrt_object_destroy_func_t destroy);
void clrt_object_free(struct clrt_object_t *object);

struct clrt_object_t *clrt_object_enumerate(struct clrt_object_t *prev,
		enum clrt_object_type_t type);
struct clrt_object_t *clrt_object_find(void *object, struct clrt_object_t **prev_item);

int clrt_object_verify(void *object, enum clrt_object_type_t type);

int clrt_object_ref_update(void *object, enum clrt_object_type_t type, int change);
int clrt_object_retain(void *object, enum clrt_object_type_t type, int err_code);
int clrt_object_release(void *object, enum clrt_object_type_t type, int err_code);







/*
 * Private Command Queue Items
 */

typedef void (*queue_action_t)(void *data);

struct clrt_queue_item_t
{
	struct clrt_queue_item_t *next;
	void *data;
	queue_action_t action;
	cl_event done_event;
	int num_wait_events;
	cl_event *wait_events;
};

struct clrt_queue_item_t *clrt_queue_item_create(
	struct _cl_command_queue *queue, 
	void *data, 
	queue_action_t action, 
	cl_event *done, 
	int num_wait, 
	cl_event *waits);
void clrt_command_queue_enqueue(struct _cl_command_queue *queue, struct clrt_queue_item_t *item);




/*
 * Event function declrations
 */

void clrt_event_set_status(struct _cl_event *event, cl_int status);
struct _cl_event *clrt_event_create(struct _cl_command_queue *queue);
int clrt_event_wait_list_check(unsigned int num_events, struct _cl_event * const *event_list);

/*
 * Helper Functions 
 */

/* populate a parameter as a response to OpenCL's many clGet*Info functions */
cl_int populateParameter(
	const void *value, 
	size_t actual, 
	size_t param_value_size, 
	void *param_value, 
	size_t *param_value_size_ret);

/* get the number of properties in a properties list */
size_t getPropertiesCount(const void *properties, size_t prop_size);

/* copy a properties list */
void copyProperties(void *dest, const void *src, size_t size, size_t numObjs);


/* Platform strings (defined in platform.c) */
extern const char *FULL_PROFILE;
extern const char *VERSION;
extern const char *NAME;
extern const char *VENDOR;
extern const char *EXTENSIONS;
extern const char *DEVICE_NAME;
extern const char *DRIVER_VERSION;
extern const char *DEVICE_VERSION;



/*
 * OpenCL Types
 */

struct _cl_platform_id
{
	unsigned int empty;
};


struct _cl_device_id
{
	volatile int num_kernels;
	volatile int num_done;

	volatile int num_cores;
	pthread_t *threads;

	pthread_mutex_t lock;
	pthread_cond_t ready;
	pthread_cond_t done;

	struct clrt_execution_t *exec;
};


struct _cl_context
{
	int num_devices;
	struct _cl_device_id **devices;
	size_t prop_count;
	cl_context_properties *props;
};


struct _cl_command_queue
{
	struct _cl_device_id *device;
	struct clrt_queue_item_t *head;
	struct clrt_queue_item_t *tail;
	cl_command_queue_properties properties;
	pthread_t queue_thread;
	pthread_mutex_t lock;
	pthread_cond_t cond_process;
	volatile int process;
};


struct _cl_mem
{
	void *buffer;
	size_t size;
};


struct _cl_program
{
	void *handle;
	char *filename;
};


struct _cl_kernel
{
	clrt_function_t function;
	size_t *metadata;
	unsigned int num_params;
	struct clrt_parameter_t *param_info;
	int stack_param_words;
	size_t *stack_params;
	struct clrt_reg_param_t *register_params;
};


struct _cl_event
{
	cl_int status;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	struct _cl_command_queue *queue;
	struct _cl_context *context;

	/* Profiling Information */
	cl_ulong time_queued;
	cl_ulong time_submit;
	cl_ulong time_start;
	cl_ulong time_end;
};


struct _cl_sampler
{
	unsigned int id;
};

#endif

