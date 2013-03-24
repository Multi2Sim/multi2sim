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


cl_command_queue clCreateCommandQueue(
	cl_context context,
	cl_device_id device,
	cl_command_queue_properties properties,
	cl_int *errcode_ret)
{
	unsigned int sys_args[4];
	sys_args[0] = context->id;
	sys_args[1] = (unsigned int) device;
	sys_args[2] = (unsigned int) properties;
	sys_args[3] = (unsigned int) errcode_ret;
	return (cl_command_queue) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clCreateCommandQueue, sys_args);
}


cl_int clRetainCommandQueue(
	cl_command_queue command_queue)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) command_queue;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clRetainCommandQueue, sys_args);
}


cl_int clReleaseCommandQueue(
	cl_command_queue command_queue)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) command_queue;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clReleaseCommandQueue, sys_args);
}


cl_int clGetCommandQueueInfo(
	cl_command_queue command_queue,
	cl_command_queue_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clGetCommandQueueInfo, sys_args);
}


cl_int clSetCommandQueueProperty(
	cl_command_queue command_queue,
	cl_command_queue_properties properties,
	cl_bool enable,
	cl_command_queue_properties *old_properties)
{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) properties;
	sys_args[2] = (unsigned int) enable;
	sys_args[3] = (unsigned int) old_properties;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clSetCommandQueueProperty, sys_args);
}


cl_int clEnqueueReadBuffer(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_read,
	size_t offset,
	size_t cb,
	void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) buffer;
	sys_args[2] = (unsigned int) blocking_read;
	sys_args[3] = (unsigned int) offset;
	sys_args[4] = (unsigned int) cb;
	sys_args[5] = (unsigned int) ptr;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueReadBuffer, sys_args);
}


cl_int clEnqueueReadBufferRect(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_read,
	const size_t *buffer_origin,
	const size_t *host_origin,
	const size_t *region,
	size_t buffer_row_pitch,
	size_t buffer_slice_pitch,
	size_t host_row_pitch,
	size_t host_slice_pitch,
	void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[14];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) buffer;
	sys_args[2] = (unsigned int) blocking_read;
	sys_args[3] = (unsigned int) buffer_origin;
	sys_args[4] = (unsigned int) host_origin;
	sys_args[5] = (unsigned int) region;
	sys_args[6] = (unsigned int) buffer_row_pitch;
	sys_args[7] = (unsigned int) buffer_slice_pitch;
	sys_args[8] = (unsigned int) host_row_pitch;
	sys_args[9] = (unsigned int) host_slice_pitch;
	sys_args[10] = (unsigned int) ptr;
	sys_args[11] = (unsigned int) num_events_in_wait_list;
	sys_args[12] = (unsigned int) event_wait_list;
	sys_args[13] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueReadBufferRect, sys_args);
}


cl_int clEnqueueWriteBuffer(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_write,
	size_t offset,
	size_t cb,
	const void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) buffer;
	sys_args[2] = (unsigned int) blocking_write;
	sys_args[3] = (unsigned int) offset;
	sys_args[4] = (unsigned int) cb;
	sys_args[5] = (unsigned int) ptr;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueWriteBuffer, sys_args);
}


cl_int clEnqueueWriteBufferRect(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_write,
	const size_t *buffer_origin,
	const size_t *host_origin,
	const size_t *region,
	size_t buffer_row_pitch,
	size_t buffer_slice_pitch,
	size_t host_row_pitch,
	size_t host_slice_pitch,
	const void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[14];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) buffer;
	sys_args[2] = (unsigned int) blocking_write;
	sys_args[3] = (unsigned int) buffer_origin;
	sys_args[4] = (unsigned int) host_origin;
	sys_args[5] = (unsigned int) region;
	sys_args[6] = (unsigned int) buffer_row_pitch;
	sys_args[7] = (unsigned int) buffer_slice_pitch;
	sys_args[8] = (unsigned int) host_row_pitch;
	sys_args[9] = (unsigned int) host_slice_pitch;
	sys_args[10] = (unsigned int) ptr;
	sys_args[11] = (unsigned int) num_events_in_wait_list;
	sys_args[12] = (unsigned int) event_wait_list;
	sys_args[13] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueWriteBufferRect, sys_args);
}


cl_int clEnqueueCopyBuffer(
	cl_command_queue command_queue,
	cl_mem src_buffer,
	cl_mem dst_buffer,
	size_t src_offset,
	size_t dst_offset,
	size_t cb,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) src_buffer;
	sys_args[2] = (unsigned int) dst_buffer;
	sys_args[3] = (unsigned int) src_offset;
	sys_args[4] = (unsigned int) dst_offset;
	sys_args[5] = (unsigned int) cb;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueCopyBuffer, sys_args);
}


cl_int clEnqueueCopyBufferRect(
	cl_command_queue command_queue,
	cl_mem src_buffer,
	cl_mem dst_buffer,
	const size_t *src_origin,
	const size_t *dst_origin,
	const size_t *region,
	size_t src_row_pitch,
	size_t src_slice_pitch,
	size_t dst_row_pitch,
	size_t dst_slice_pitch,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[13];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) src_buffer;
	sys_args[2] = (unsigned int) dst_buffer;
	sys_args[3] = (unsigned int) src_origin;
	sys_args[4] = (unsigned int) dst_origin;
	sys_args[5] = (unsigned int) region;
	sys_args[6] = (unsigned int) src_row_pitch;
	sys_args[7] = (unsigned int) src_slice_pitch;
	sys_args[8] = (unsigned int) dst_row_pitch;
	sys_args[9] = (unsigned int) dst_slice_pitch;
	sys_args[10] = (unsigned int) num_events_in_wait_list;
	sys_args[11] = (unsigned int) event_wait_list;
	sys_args[12] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueCopyBufferRect, sys_args);
}


cl_int clEnqueueReadImage(
	cl_command_queue command_queue,
	cl_mem image,
	cl_bool blocking_read,
	const size_t *origin3,
	const size_t *region3,
	size_t row_pitch,
	size_t slice_pitch,
	void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[11];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) image;
	sys_args[2] = (unsigned int) blocking_read;
	sys_args[3] = (unsigned int) origin3;
	sys_args[4] = (unsigned int) region3;
	sys_args[5] = (unsigned int) row_pitch;
	sys_args[6] = (unsigned int) slice_pitch;
	sys_args[7] = (unsigned int) ptr;
	sys_args[8] = (unsigned int) num_events_in_wait_list;
	sys_args[9] = (unsigned int) event_wait_list;
	sys_args[10] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueReadImage, sys_args);
}


cl_int clEnqueueWriteImage(
	cl_command_queue command_queue,
	cl_mem image,
	cl_bool blocking_write,
	const size_t *origin3,
	const size_t *region3,
	size_t input_row_pitch,
	size_t input_slice_pitch,
	const void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[11];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) image;
	sys_args[2] = (unsigned int) blocking_write;
	sys_args[3] = (unsigned int) origin3;
	sys_args[4] = (unsigned int) region3;
	sys_args[5] = (unsigned int) input_row_pitch;
	sys_args[6] = (unsigned int) input_slice_pitch;
	sys_args[7] = (unsigned int) ptr;
	sys_args[8] = (unsigned int) num_events_in_wait_list;
	sys_args[9] = (unsigned int) event_wait_list;
	sys_args[10] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueWriteImage, sys_args);
}


cl_int clEnqueueCopyImage(
	cl_command_queue command_queue,
	cl_mem src_image,
	cl_mem dst_image,
	const size_t *src_origin3,
	const size_t *dst_origin3,
	const size_t *region3,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) src_image;
	sys_args[2] = (unsigned int) dst_image;
	sys_args[3] = (unsigned int) src_origin3;
	sys_args[4] = (unsigned int) dst_origin3;
	sys_args[5] = (unsigned int) region3;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueCopyImage, sys_args);
}


cl_int clEnqueueCopyImageToBuffer(
	cl_command_queue command_queue,
	cl_mem src_image,
	cl_mem dst_buffer,
	const size_t *src_origin3,
	const size_t *region3,
	size_t dst_offset,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) src_image;
	sys_args[2] = (unsigned int) dst_buffer;
	sys_args[3] = (unsigned int) src_origin3;
	sys_args[4] = (unsigned int) region3;
	sys_args[5] = (unsigned int) dst_offset;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueCopyImageToBuffer, sys_args);
}


cl_int clEnqueueCopyBufferToImage(
	cl_command_queue command_queue,
	cl_mem src_buffer,
	cl_mem dst_image,
	size_t src_offset,
	const size_t *dst_origin3,
	const size_t *region3,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) src_buffer;
	sys_args[2] = (unsigned int) dst_image;
	sys_args[3] = (unsigned int) src_offset;
	sys_args[4] = (unsigned int) dst_origin3;
	sys_args[5] = (unsigned int) region3;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueCopyBufferToImage, sys_args);
}


void * clEnqueueMapBuffer(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_map,
	cl_map_flags map_flags,
	size_t offset,
	size_t cb,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event,
	cl_int *errcode_ret)
{
	unsigned int sys_args[10];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) buffer;
	sys_args[2] = (unsigned int) blocking_map;
	sys_args[3] = (unsigned int) map_flags;
	sys_args[4] = (unsigned int) offset;
	sys_args[5] = (unsigned int) cb;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	sys_args[9] = (unsigned int) errcode_ret;
	return (void *) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueMapBuffer, sys_args);
}


void * clEnqueueMapImage(
	cl_command_queue command_queue,
	cl_mem image,
	cl_bool blocking_map,
	cl_map_flags map_flags,
	const size_t *origin3,
	const size_t *region3,
	size_t *image_row_pitch,
	size_t *image_slice_pitch,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event,
	cl_int *errcode_ret)
{
	unsigned int sys_args[12];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) image;
	sys_args[2] = (unsigned int) blocking_map;
	sys_args[3] = (unsigned int) map_flags;
	sys_args[4] = (unsigned int) origin3;
	sys_args[5] = (unsigned int) region3;
	sys_args[6] = (unsigned int) image_row_pitch;
	sys_args[7] = (unsigned int) image_slice_pitch;
	sys_args[8] = (unsigned int) num_events_in_wait_list;
	sys_args[9] = (unsigned int) event_wait_list;
	sys_args[10] = (unsigned int) event;
	sys_args[11] = (unsigned int) errcode_ret;
	return (void *) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueMapImage, sys_args);
}


cl_int clEnqueueUnmapMemObject(
	cl_command_queue command_queue,
	cl_mem memobj,
	void *mapped_ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) memobj;
	sys_args[2] = (unsigned int) mapped_ptr;
	sys_args[3] = (unsigned int) num_events_in_wait_list;
	sys_args[4] = (unsigned int) event_wait_list;
	sys_args[5] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueUnmapMemObject, sys_args);
}


cl_int clEnqueueNDRangeKernel(
	cl_command_queue command_queue,
	cl_kernel kernel,
	cl_uint work_dim,
	const size_t *global_work_offset,
	const size_t *global_work_size,
	const size_t *local_work_size,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) kernel;
	sys_args[2] = (unsigned int) work_dim;
	sys_args[3] = (unsigned int) global_work_offset;
	sys_args[4] = (unsigned int) global_work_size;
	sys_args[5] = (unsigned int) local_work_size;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueNDRangeKernel, sys_args);
}


cl_int clEnqueueTask(
	cl_command_queue command_queue,
	cl_kernel kernel,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) kernel;
	sys_args[2] = (unsigned int) num_events_in_wait_list;
	sys_args[3] = (unsigned int) event_wait_list;
	sys_args[4] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueTask, sys_args);
}


cl_int clEnqueueNativeKernel(
	cl_command_queue command_queue,
	void (*user_func)(void *),
	void *args,
	size_t cb_args,
	cl_uint num_mem_objects,
	const cl_mem *mem_list,
	const void **args_mem_loc,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[10];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) user_func;
	sys_args[2] = (unsigned int) args;
	sys_args[3] = (unsigned int) cb_args;
	sys_args[4] = (unsigned int) num_mem_objects;
	sys_args[5] = (unsigned int) mem_list;
	sys_args[6] = (unsigned int) args_mem_loc;
	sys_args[7] = (unsigned int) num_events_in_wait_list;
	sys_args[8] = (unsigned int) event_wait_list;
	sys_args[9] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueNativeKernel, sys_args);
}


cl_int clEnqueueMarker(
	cl_command_queue command_queue,
	cl_event *event)
{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) event;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueMarker, sys_args);
}


cl_int clEnqueueWaitForEvents(
	cl_command_queue command_queue,
	cl_uint num_events,
	const cl_event *event_list)
{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) num_events;
	sys_args[2] = (unsigned int) event_list;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueWaitForEvents, sys_args);
}


cl_int clEnqueueBarrier(
	cl_command_queue command_queue)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) command_queue;
	return (cl_int) syscall(OPENCL_SYSCALL_CODE, OPENCL_FUNC_clEnqueueBarrier, sys_args);
}

