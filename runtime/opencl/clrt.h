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

#ifndef RUNTIME_OPENCL_CLRT_H
#define RUNTIME_OPENCL_CLRT_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <elf.h>

#include "../include/CL/cl.h"
#include "device.h"
#include "object.h"


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
#include "../../src/arch/x86/emu/clrt.dat"
#undef X86_CLRT_DEFINE_CALL
	opencl_call_count
};

/* Debug */
void opencl_debug(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

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




/*
 * OpenCL Types
 */


struct clrt_device_kernel_t
{
	struct clrt_device_type_t *device_type;
	void *kernel;
};

/* Device Visitor Type */
typedef void (*device_visitor_t)(void *ctx, cl_device_id device, struct clrt_device_type_t *device_type);
void visit_devices(device_visitor_t visitor, void *ctx);
int verify_device(cl_device_id device);



#endif

