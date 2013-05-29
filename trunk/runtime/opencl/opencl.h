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

#ifndef RUNTIME_OPENCL_OPENCL_H
#define RUNTIME_OPENCL_OPENCL_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <elf.h>

#include "../include/CL/cl.h"


/* NOTE: The following re-declaration of 'pthread_setaffinity_np' is made to
 * avoid some 'implicit declaration' warnings in older pthread distributions.
 * Sometimes, the warning would still show up even after declaring macro
 * #define _GNU_SOURCE (notice that this macro is declared in the gcc command
 * line with the '-D' flag, see Makefile.am). */
#ifndef pthread_setaffinity_np
int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize,
		const cpu_set_t *cpuset);
#endif




/*
 * Types
 */

#define opencl_platform_t _cl_platform_id
#define opencl_device_t _cl_device_id
#define opencl_context_t _cl_context
#define opencl_command_queue_t _cl_command_queue
#define opencl_mem_t _cl_mem
#define opencl_program_t _cl_program
#define opencl_ndrange_t _cl_ndrange
#define opencl_kernel_t _cl_kernel
#define opencl_event_t _cl_event
#define opencl_sampler_t _cl_sampler

/* Generic call-back function */
typedef void *(*opencl_callback_t)(void *);

enum opencl_runtime_type_t
{
	opencl_runtime_type_x86=1,
	opencl_runtime_type_si,
	opencl_runtime_type_union
};


/*
 * Runtime System Calls
 */

/* System call code to communicate with Multi2Sim */
#define OPENCL_SYSCALL_CODE  329

/* Function code to pass as a first argument of a system call */
enum opencl_abi_call_t
{
	opencl_abi_invalid,
#define OPENCL_ABI_CALL(name, code) opencl_abi_##name = code,
#include "../../src/driver/opencl/opencl.dat"
#undef OPENCL_ABI_CALL
	opencl_abi_call_count
};





/*
 * Error macros
 */

extern char *opencl_err_not_impl;
extern char *opencl_err_note;
extern char *opencl_err_param_note;

#define __OPENCL_NOT_IMPL__  \
	fatal("%s: OpenCL call not implemented.\n%s", __FUNCTION__, opencl_err_not_impl);
#define OPENCL_ARG_NOT_SUPPORTED(p) \
	fatal("%s: not supported for '" #p "' = 0x%x\n%s", __FUNCTION__, p, opencl_err_note);
#define OPENCL_ARG_NOT_SUPPORTED_EQ(p, v) \
	{ if ((p) == (v)) fatal("%s: not supported for '" #p "' = 0x%x\n%s", __FUNCTION__, (v), opencl_err_param_note); }
#define OPENCL_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: not supported for '" #p "' != 0x%x\n%s", __FUNCTION__, (v), opencl_err_param_note); }
#define OPENCL_ARG_NOT_SUPPORTED_LT(p, v) \
	{ if ((p) < (v)) fatal("%s: not supported for '" #p "' < %d\n%s", __FUNCTION__, (v), opencl_err_param_note); }
#define OPENCL_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	__FUNCTION__, (min), (max), opencl_err_param_note); }
#define OPENCL_ARG_NOT_SUPPORTED_FLAG(p, flag, name) \
	{ if ((p) & (flag)) fatal("%s: flag '" name "' not supported\n%s", __FUNCTION__, opencl_err_param_note); }




/*
 * Call-back functions for object 'opencl_xxx_device_t'
 */

/* Create an architecture-specific device. Returns an object of type
 * 'opencl_XXX_device_t'. */
typedef void *(*opencl_arch_device_create_func_t)(
		struct opencl_device_t *parent);

/* Free an architecture-specific device. */
typedef void (*opencl_arch_device_free_func_t)(
		void *device);  /* Of type 'opencl_XXX_device_t' */

/* Allocate device memory. The function returns a device pointer.
 * If the device is the x86 CPU, the pointer can be dereferenced
 * normally. If not, the value is just a 32-bit address used by
 * device-specific functions. */
typedef void *(*opencl_arch_device_mem_alloc_func_t)(
		void *device,  /* Of type 'opencl_xxx_device_t' */
		size_t size);

/* Free device memory */
typedef void (*opencl_arch_device_mem_free_func_t)(
		void *device,  /* Of type 'opencl_xxx_device_t' */
		void *device_ptr);

/* Read from device memory */
typedef void (*opencl_arch_device_mem_read_func_t)(
		void *device,  /* Of type 'opencl_xxx_device_t' */
		void *host_ptr,
		void *device_ptr,
		unsigned int size);

/* Write into device memory */
typedef void (*opencl_arch_device_mem_write_func_t)(
		void *device,  /* Of type 'opencl_xxx_device_t' */
		void *device_ptr,
		void *host_ptr,
		unsigned int size);

/* Copy device memory */
typedef void (*opencl_arch_device_mem_copy_func_t)(
		void *device,  /* Of type 'opencl_xxx_device_t' */
		void *device_dest_ptr,
		void *device_src_ptr,
		unsigned int size);

/* Determine the preferred number of work-groups a device has */
typedef int (*opencl_arch_device_preferred_workgroups_func_t)(
		void *device); /* Of type opencl_xxx_device_t */



/*
 * Call-back functions for object 'opencl_xxx_program_t'
 */

/* Create an architecture-specific program. Returns an object of type
 * 'opencl_XXX_program_t'. */
typedef void *(*opencl_arch_program_create_func_t)(
		struct opencl_program_t *parent,
		void *arch_device,  /* Of type 'opencl_xxx_device_t' */
		void *binary,
		unsigned int length);

/* Free an architecture-specific program. */
typedef void (*opencl_arch_program_free_func_t)(
		void *program);  /* Of type 'opencl_XXX_program_t' */

/* Check if a binary blob is a valid program */
typedef int (*opencl_arch_program_valid_binary_func_t)(
	void *device,
	void *binary,
	unsigned int length);



/*
 * Call-back functions for object 'opencl_xxx_kernel_t'
 */

/* Create an architecture-specific kernel. Returns an object of type
 * 'opencl_XXX_kernel_t'. */
typedef void *(*opencl_arch_kernel_create_func_t)(
		struct opencl_kernel_t *parent,
		void *arch_program,  /* Of type 'opencl_xxx_program_t' */
		const char *kernel_name);

/* Free an architecture-specific kernel. */
typedef void (*opencl_arch_kernel_free_func_t)(
		void *kernel);  /* Of type 'opencl_XXX_kernel_t' */

/* Set a kernel argument */
typedef int (*opencl_arch_kernel_set_arg_func_t)(
		void *kernel,  /* Of type 'opencl_XXX_kernel_t' */
		int arg_index,
		unsigned int arg_size,
		void *arg_value);

/* Create an ND-Range */
typedef void *(*opencl_arch_ndrange_create_func_t)(
	struct opencl_ndrange_t *ndrange, void *arch_kernel,
	unsigned int work_dim, unsigned int *global_work_offset,
	unsigned int *global_work_size, unsigned int *local_work_size,
	unsigned int fused);

/* Initialize an ND-Range */
typedef void (*opencl_arch_ndrange_init_func_t)(void *ndrange);

/* Launch an ND-Range */
typedef void (*opencl_arch_ndrange_run_func_t)(void *ndrange);

/* Run an part of an ND-Range */
typedef void (*opencl_arch_ndrange_run_partial_func_t)(void *ndrange, 
		unsigned int *work_group_start, unsigned int *work_group_count);

/* Free an ND-Range */
typedef void (*opencl_arch_ndrange_free_func_t)(void *ndrange);

/*
 * Global Variables
 */

/* Execution in native/simulated mode */
extern int opencl_native_mode;



/*
 * Public Functions 
 */

void opencl_debug(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

/* Populate a parameter as a response to OpenCL's many clGet*Info functions */
cl_int opencl_set_param(const void *src_value, size_t src_size,
	size_t dest_size, void *dest_value, size_t *size_ret);
cl_int opencl_set_string(const char *src_string, size_t dest_size,
	void *dest_string, size_t *size_ret);
int opencl_is_valid_device_type(cl_device_type device_type);

int opencl_event_wait_list_check(unsigned int num_events, const cl_event *event_list);

/* get the number of properties in a properties list */
size_t getPropertiesCount(const void *properties, size_t prop_size);

/* copy a properties list */
void copyProperties(void *dest, const void *src, size_t size, size_t numObjs);

void *clrt_buffer_allocate(size_t size);
void clrt_buffer_free(void *buffer);


#endif

