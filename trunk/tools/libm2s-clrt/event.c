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

#include <m2s-clrt.h>


cl_int clWaitForEvents(
	cl_uint num_events,
	const cl_event *event_list)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clGetEventInfo(
	cl_event event,
	cl_event_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_event clCreateUserEvent(
	cl_context context,
	cl_int *errcode_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clRetainEvent(
	cl_event event)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clReleaseEvent(
	cl_event event)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clSetUserEventStatus(
	cl_event event,
	cl_int execution_status)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clSetEventCallback(
	cl_event event,
	cl_int command_exec_callback_type,
	void (*pfn_notify)(cl_event , cl_int , void *),
	void *user_data)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clGetEventProfilingInfo(
	cl_event event,
	cl_profiling_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clFlush(
	cl_command_queue command_queue)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clFinish(
	cl_command_queue command_queue)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}

