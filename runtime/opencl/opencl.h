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



/*
 * Types
 */

#define opencl_platform_t _cl_platform_id
#define opencl_device_t _cl_device_id
#define opencl_context_t _cl_context
#define opencl_command_queue_t _cl_command_queue
#define opencl_mem_t _cl_mem
#define opencl_program_t _cl_program
#define opencl_kernel_t _cl_kernel
#define opencl_event_t _cl_event
#define opencl_sampler_t _cl_sampler

/* Generic call-back function */
typedef void *(*opencl_callback_t)(void *);



/*
 * Runtime System Calls
 */

/* Version for the Multi2Sim Runtime library implementation */
#define OPENCL_VERSION_MAJOR  1
#define OPENCL_VERSION_MINOR  752

/* System call code to communicate with Multi2Sim */
#define OPENCL_SYSCALL_CODE  329

/* Function code to pass as a first argument of a system call */
enum opencl_call_t
{
	opencl_call_invalid,
#define X86_CLRT_DEFINE_CALL(name, code) opencl_call_##name = code,
#include "../../src/driver/opencl/opencl.dat"
#undef X86_CLRT_DEFINE_CALL
	opencl_call_count
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

