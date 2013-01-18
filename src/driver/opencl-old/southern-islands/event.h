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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_OPENCL_EVENT_H
#define ARCH_SOUTHERN_ISLANDS_EMU_OPENCL_EVENT_H


/* Forward declarations */
struct x86_ctx_t;
struct mem_t;


enum si_opencl_event_kind_t
{
	SI_OPENCL_EVENT_NONE = 0,
	SI_OPENCL_EVENT_NDRANGE_KERNEL,
	SI_OPENCL_EVENT_TASK,
	SI_OPENCL_EVENT_NATIVE_KERNEL,
	SI_OPENCL_EVENT_READ_BUFFER,
	SI_OPENCL_EVENT_WRITE_BUFFER,
	SI_OPENCL_EVENT_MAP_BUFFER,
	SI_OPENCL_EVENT_UNMAP_MEM_OBJECT,
	SI_OPENCL_EVENT_READ_BUFFER_RECT,
	SI_OPENCL_EVENT_WRITE_BUFFER_RECT,
	SI_OPENCL_EVENT_READ_IMAGE,
	SI_OPENCL_EVENT_WRITE_IMAGE,
	SI_OPENCL_EVENT_MAP_IMAGE,
	SI_OPENCL_EVENT_COPY_BUFFER,
	SI_OPENCL_EVENT_COPY_IMAGE,
	SI_OPENCL_EVENT_COPY_BUFFER_RECT,
	SI_OPENCL_EVENT_COPY_BUFFER_TO_IMAGE,
	SI_OPENCL_EVENT_COPY_IMAGE_TO_BUFFER,
	SI_OPENCL_EVENT_MARKER,
	SI_OPENCL_EVENT_COUNT
};

enum si_opencl_event_status_t
{
	SI_OPENCL_EVENT_STATUS_NONE = 0,
	SI_OPENCL_EVENT_STATUS_QUEUED,
	SI_OPENCL_EVENT_STATUS_SUBMITTED,
	SI_OPENCL_EVENT_STATUS_RUNNING,
	SI_OPENCL_EVENT_STATUS_COMPLETE
};

struct si_opencl_event_t
{
	unsigned int id;
	int ref_count;
	enum si_opencl_event_kind_t kind;
	enum si_opencl_event_status_t status;

	long long time_queued;
	long long time_submit;
	long long time_start;
	long long time_end;
};

struct si_opencl_event_t *si_opencl_event_create(enum si_opencl_event_kind_t kind);
void si_opencl_event_free(struct si_opencl_event_t *event);

unsigned int si_opencl_event_get_profiling_info(struct si_opencl_event_t *event, unsigned int name,
	struct mem_t *mem, unsigned int addr, unsigned int size);
long long si_opencl_event_timer(void);

int si_opencl_event_can_wakeup(struct x86_ctx_t *ctx, void *data);

#endif
