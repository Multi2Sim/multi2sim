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

#include <unistd.h>
#include <stdio.h>

#include "../include/CL/cl.h"
#include "api.h"
#include "context.h"


cl_int clWaitForEvents(
	cl_uint num_events,
	const cl_event *event_list)
{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) num_events;
	sys_args[1] = (unsigned int) event_list;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clWaitForEvents, sys_args);
}


cl_int clGetEventInfo(
	cl_event event,
	cl_event_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) event;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetEventInfo, sys_args);
}


cl_event clCreateUserEvent(
	cl_context context,
	cl_int *errcode_ret)
{
	unsigned int sys_args[2];
	sys_args[0] = context->id;
	sys_args[1] = (unsigned int) errcode_ret;
	return (cl_event) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateUserEvent, sys_args);
}


cl_int clRetainEvent(
	cl_event event)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clRetainEvent, sys_args);
}


cl_int clReleaseEvent(
	cl_event event)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clReleaseEvent, sys_args);
}


cl_int clSetUserEventStatus(
	cl_event event,
	cl_int execution_status)
{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) event;
	sys_args[1] = (unsigned int) execution_status;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clSetUserEventStatus, sys_args);
}


cl_int clSetEventCallback(
	cl_event event,
	cl_int command_exec_callback_type,
	void (*pfn_notify)(cl_event , cl_int , void *),
	void *user_data)
{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) event;
	sys_args[1] = (unsigned int) command_exec_callback_type;
	sys_args[2] = (unsigned int) pfn_notify;
	sys_args[3] = (unsigned int) user_data;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clSetEventCallback, sys_args);
}


cl_int clGetEventProfilingInfo(
	cl_event event,
	cl_profiling_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) event;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetEventProfilingInfo, sys_args);
}


cl_int clFlush(
	cl_command_queue command_queue)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) command_queue;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clFlush, sys_args);
}


cl_int clFinish(
	cl_command_queue command_queue)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) command_queue;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clFinish, sys_args);
}

