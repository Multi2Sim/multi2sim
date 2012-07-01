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


cl_command_queue clCreateCommandQueue(
	cl_context context,
	cl_device_id device,
	cl_command_queue_properties properties,
	cl_int *errcode_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clRetainCommandQueue(
	cl_command_queue command_queue)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clReleaseCommandQueue(
	cl_command_queue command_queue)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clGetCommandQueueInfo(
	cl_command_queue command_queue,
	cl_command_queue_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clSetCommandQueueProperty(
	cl_command_queue command_queue,
	cl_command_queue_properties properties,
	cl_bool enable,
	cl_command_queue_properties *old_properties)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clEnqueueUnmapMemObject(
	cl_command_queue command_queue,
	cl_mem memobj,
	void *mapped_ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clEnqueueTask(
	cl_command_queue command_queue,
	cl_kernel kernel,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
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
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clEnqueueMarker(
	cl_command_queue command_queue,
	cl_event *event)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clEnqueueWaitForEvents(
	cl_command_queue command_queue,
	cl_uint num_events,
	const cl_event *event_list)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}


cl_int clEnqueueBarrier(
	cl_command_queue command_queue)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}

