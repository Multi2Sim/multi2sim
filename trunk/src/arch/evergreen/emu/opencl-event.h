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

#ifndef ARCH_EVERGREEN_EMU_OPENCL_EVENT_H
#define ARCH_EVERGREEN_EMU_OPENCL_EVENT_H


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
	unsigned int id;
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

unsigned int evg_opencl_event_get_profiling_info(struct evg_opencl_event_t *event, unsigned int name,
	struct mem_t *mem, unsigned int addr, unsigned int size);
long long evg_opencl_event_timer(void);


#endif

