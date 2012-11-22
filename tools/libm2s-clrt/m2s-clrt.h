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

#include <CL/cl.h>

#include "clrt-object.h"
#include "device-interface.h"


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
 * Private ELF-related definitions 
 */

Elf32_Shdr *get_section_header(void *elf, char *name);
void *get_inner_elf_addr(const unsigned char *outer_elf, uint32_t *size);



/*
 * Private OpenCL implementation definitions
 */


void *clrt_buffer_allocate(size_t size);
void clrt_buffer_free(void *buffer);


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


struct clrt_device_type_entry_t
{
	struct clrt_device_type_t *device_type;
	cl_uint num_devices;
	cl_device_id *devices;
};


struct _cl_platform_id
{
	int num_device_types;
	struct clrt_device_type_entry_t *entries;
};


struct _cl_device_id
{
	/* OpenCL device properties */
	cl_int address_bits;
	cl_bool available;
	cl_bool compiler_available;
	cl_device_fp_config double_fp_config;
	cl_bool endian_little;
	cl_bool error_correction_support;
	cl_device_exec_capabilities execution_capabilities;
	const char *extensions;
	cl_int global_mem_cache_size;
	cl_device_mem_cache_type global_mem_cache_type;
	cl_uint global_mem_cacheline_size;
	cl_int global_mem_size;
	cl_bool host_unified_memory;
	cl_bool image_support;
	cl_int image2d_max_height;
	cl_int image2d_max_width;
	cl_int image3d_max_depth;
	cl_int image3d_max_height;
	cl_int image3d_max_width;
	cl_ulong local_mem_size;
	cl_device_local_mem_type local_mem_type;
	cl_int max_clock_frequency;
	cl_int max_compute_units;
	cl_uint max_constant_args;
	cl_ulong max_constant_buffer_size;
	cl_ulong max_mem_alloc_size;
	size_t max_parameter_size;
	cl_uint max_read_image_args;
	cl_int max_samplers;
	size_t max_work_group_size;
	cl_uint max_work_item_dimensions;
	size_t max_work_item_sizes[3];
	cl_uint max_write_image_args;
	cl_uint mem_base_addr_align;
	cl_uint min_data_type_align_size;
	const char *name;
	const char *opencl_c_version;
	cl_uint vector_width_char;
	cl_uint vector_width_short;
	cl_uint vector_width_int;
	cl_uint vector_width_long;
	cl_uint vector_width_float;
	cl_uint vector_width_double;
	cl_uint vector_width_half;
	const char *profile;
	size_t profiling_timer_resolution;
	cl_command_queue_properties queue_properties;
	cl_device_fp_config single_fp_config;
	cl_device_type type;
	const char *vendor;
	cl_uint vendor_id;
	const char *version;

	struct clrt_device_type_t *device_type;
	/* Device-dependent implementation */
	void *device;
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


struct clrt_device_program_t
{
	struct clrt_device_type_t *device_type;
	void *handle;
	void *filename;
};

struct _cl_program
{
	int num_entries;
	struct clrt_device_program_t *entries;
};

struct clrt_device_kernel_t
{
	struct clrt_device_type_t *device_type;
	void *kernel;
};

struct _cl_kernel
{
	int num_entries;
	struct clrt_device_kernel_t *entries;
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

/* Device Visitor Type */
typedef void (*device_visitor_t)(void *ctx, cl_device_id device, struct clrt_device_type_t *device_type);
void visit_devices(device_visitor_t visitor, void *ctx);
int verify_device(cl_device_id device);



#endif

