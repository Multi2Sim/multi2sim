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

#include <assert.h>
#include <debug.h>
#include <stdlib.h>

#include <southern-islands-emu.h>
#include <mem-system.h>
#include <x86-emu.h>


struct si_opencl_event_t *si_opencl_event_create(enum si_opencl_event_kind_t kind)
{
	struct si_opencl_event_t *event;

	/* Allocate */
	event = calloc(1, sizeof(struct si_opencl_event_t));
	if (!event)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	event->id = si_opencl_repo_new_object_id(si_emu->opencl_repo,
		si_opencl_object_event);
	event->ref_count = 1;
	event->kind = kind;

	/* Return */
	si_opencl_repo_add_object(si_emu->opencl_repo, event);
	return event;
}


void si_opencl_event_free(struct si_opencl_event_t *event)
{
	si_opencl_repo_remove_object(si_emu->opencl_repo, event);
	free(event);
}


uint32_t si_opencl_event_get_profiling_info(struct si_opencl_event_t *event, uint32_t name,
	struct mem_t *mem, uint32_t addr, uint32_t size)
{
	uint32_t size_ret = 0;
	void *info = NULL;

	switch (name)
	{

	case 0x1280:  /* CL_PROFILING_COMMAND_QUEUED */
		size_ret = 8;
		info = &event->time_queued;
		break;
	
	case 0x1281:  /* CL_PROFILING_COMMAND_SUBMIT */
		size_ret = 8;
		info = &event->time_submit;
		break;
	
	case 0x1282:  /* CL_PROFILING_COMMAND_START */
		size_ret = 8;
		info = &event->time_start;
		break;
	
	case 0x1283:  /* CL_PROFILING_COMMAND_END */
		size_ret = 8;
		info = &event->time_end;
		break;

	default:
		fatal("%s: invalid or not implemented value for 'name' (0x%x)\n%s",
			__FUNCTION__, name, si_err_opencl_param_note);
	}
	
	/* Write to memory and return size */
	assert(info);
	if (addr && size >= size_ret)
		mem_write(mem, addr, size_ret, info);
	return size_ret;
}


long long si_opencl_event_timer(void)
{
	return esim_real_time() * 1000;
}

int si_opencl_event_can_wakeup(struct x86_ctx_t *ctx, void *data)
{
	struct si_opencl_event_t *event = data;

	return event->status == SI_OPENCL_EVENT_STATUS_COMPLETE;
}

