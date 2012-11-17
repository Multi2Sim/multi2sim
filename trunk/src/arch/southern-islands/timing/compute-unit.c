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

#include <lib/esim/trace.h>
#include <lib/util/misc.h>

#include "timing.h"

/* Front-end parameters */
int si_gpu_fetch_latency = 1;
int si_gpu_fetch_width = 10;
int si_gpu_fetch_buffer_size = 10;

int si_gpu_decode_latency = 1;
int si_gpu_decode_width = 5;
int si_gpu_decode_buffer_size = 10;

int si_gpu_issue_latency = 1;
int si_gpu_issue_width = 5;

int si_gpu_max_work_groups_per_inst_buffer = 10;
int si_gpu_max_wavefronts_per_inst_buffer = 10;


/*
 * Compute Unit
 */

struct si_compute_unit_t *si_compute_unit_create()
{
	struct si_compute_unit_t *compute_unit;
	char buf[MAX_STRING_SIZE];
	int i;

	/* Create */
	compute_unit = calloc(1, sizeof(struct si_compute_unit_t));
	if (!compute_unit)
		fatal("%s: out of memory", __FUNCTION__);

	/* Local memory */
	snprintf(buf, sizeof buf, "LocalMemory[%d]", compute_unit->id);
	compute_unit->local_memory = mod_create(buf, mod_kind_local_memory,
		si_gpu_local_mem_num_ports, si_gpu_local_mem_block_size, si_gpu_local_mem_latency);

	/* Hardware structures */
	compute_unit->num_inst_buffers = si_gpu_num_inst_buffers;
	compute_unit->inst_buffers = calloc(compute_unit->num_inst_buffers, 
		sizeof(struct si_inst_buffer_t*));
	compute_unit->fetch_buffers = calloc(compute_unit->num_inst_buffers,
			sizeof(struct list_t*));
	compute_unit->decode_buffers = calloc(compute_unit->num_inst_buffers,
			sizeof(struct list_t*));
	compute_unit->simds = calloc(compute_unit->num_inst_buffers, sizeof(struct si_simd_t*));

	compute_unit->scalar_unit.issue_buffer = list_create();
	compute_unit->scalar_unit.read_buffer = list_create();
	compute_unit->scalar_unit.exec_buffer = list_create();
	compute_unit->scalar_unit.inflight_buffer = list_create();
	compute_unit->scalar_unit.compute_unit = compute_unit;

	compute_unit->branch_unit.issue_buffer = list_create();
	compute_unit->branch_unit.read_buffer = list_create();
	compute_unit->branch_unit.exec_buffer = list_create();
	compute_unit->branch_unit.compute_unit = compute_unit;

	compute_unit->vector_mem_unit.issue_buffer = list_create();
	compute_unit->vector_mem_unit.read_buffer = list_create();
	compute_unit->vector_mem_unit.exec_buffer = list_create();
	compute_unit->vector_mem_unit.inflight_buffer = list_create();
	compute_unit->vector_mem_unit.compute_unit = compute_unit;

	compute_unit->lds.issue_buffer = list_create();
	compute_unit->lds.read_buffer = list_create();
	compute_unit->lds.exec_buffer = list_create();
	compute_unit->lds.inflight_buffer = list_create();
	compute_unit->lds.compute_unit = compute_unit;

	for (i = 0; i < compute_unit->num_inst_buffers; i++) 
	{
		/* Allocate and initialize instruction buffers */
		compute_unit->inst_buffers[i] = si_inst_buffer_create();
		compute_unit->inst_buffers[i]->id = i;
		compute_unit->inst_buffers[i]->compute_unit = compute_unit;
		compute_unit->fetch_buffers[i] = list_create();
		compute_unit->decode_buffers[i] = list_create();

		/* Allocate SIMD structures */
		compute_unit->simds[i] = calloc(1, sizeof(struct si_simd_t));
		if (!compute_unit->simds[i])
			fatal("%s: out of memory", __FUNCTION__);
		compute_unit->simds[i]->issue_buffer = list_create();
		compute_unit->simds[i]->read_buffer = list_create();
		compute_unit->simds[i]->exec_buffer = list_create();
		compute_unit->simds[i]->subwavefront_pool =
				calloc(1, sizeof(struct si_subwavefront_pool_t));
		compute_unit->simds[i]->compute_unit = compute_unit;
		compute_unit->simds[i]->wkg_util = calloc(1, sizeof(struct si_util_t));
		compute_unit->simds[i]->wvf_util = calloc(1, sizeof(struct si_util_t));
		compute_unit->simds[i]->rdy_util = calloc(1, sizeof(struct si_util_t));
		compute_unit->simds[i]->occ_util = calloc(1, sizeof(struct si_util_t));
		compute_unit->simds[i]->wki_util = calloc(1, sizeof(struct si_util_t));
		compute_unit->simds[i]->act_util = calloc(1, sizeof(struct si_util_t));
		compute_unit->simds[i]->tot_util = calloc(1, sizeof(struct si_util_t));
	}

	compute_unit->work_groups = calloc(si_gpu_max_work_groups_per_inst_buffer * 
		si_gpu_num_inst_buffers, sizeof(void *));
	if (!compute_unit->work_groups)
		fatal("%s: out of memory", __FUNCTION__);

	/* Return */
	return compute_unit;
}


void si_compute_unit_free(struct si_compute_unit_t *compute_unit)
{
	int i;

	/* Scalar Unit */
	assert(!list_count(compute_unit->scalar_unit.issue_buffer));
	assert(!list_count(compute_unit->scalar_unit.read_buffer));
	assert(!list_count(compute_unit->scalar_unit.exec_buffer));
	assert(!list_count(compute_unit->scalar_unit.inflight_buffer));
	list_free(compute_unit->scalar_unit.issue_buffer);
	list_free(compute_unit->scalar_unit.read_buffer);
	list_free(compute_unit->scalar_unit.exec_buffer);
	list_free(compute_unit->scalar_unit.inflight_buffer);

	/* Branch Unit */
	assert(!list_count(compute_unit->branch_unit.issue_buffer));
	assert(!list_count(compute_unit->branch_unit.read_buffer));
	assert(!list_count(compute_unit->branch_unit.exec_buffer));
	list_free(compute_unit->branch_unit.issue_buffer);
	list_free(compute_unit->branch_unit.read_buffer);
	list_free(compute_unit->branch_unit.exec_buffer);

	/* Vector Memory Unit */
	assert(!list_count(compute_unit->vector_mem_unit.issue_buffer));
	assert(!list_count(compute_unit->vector_mem_unit.read_buffer));
	assert(!list_count(compute_unit->vector_mem_unit.exec_buffer));
	assert(!list_count(compute_unit->vector_mem_unit.inflight_buffer));
	list_free(compute_unit->vector_mem_unit.issue_buffer);
	list_free(compute_unit->vector_mem_unit.read_buffer);
	list_free(compute_unit->vector_mem_unit.exec_buffer);
	list_free(compute_unit->vector_mem_unit.inflight_buffer);

	/* Local Data Share */
	assert(!list_count(compute_unit->lds.issue_buffer));
	assert(!list_count(compute_unit->lds.read_buffer));
	assert(!list_count(compute_unit->lds.exec_buffer));
	assert(!list_count(compute_unit->lds.inflight_buffer));
	list_free(compute_unit->lds.issue_buffer);
	list_free(compute_unit->lds.read_buffer);
	list_free(compute_unit->lds.exec_buffer);
	list_free(compute_unit->lds.inflight_buffer);

	/* Compute unit */
	for (i = 0; i < compute_unit->num_inst_buffers; i++)
	{
		assert(!list_count(compute_unit->simds[i]->issue_buffer));
		assert(!list_count(compute_unit->simds[i]->read_buffer));
		assert(!list_count(compute_unit->simds[i]->exec_buffer));
		list_free(compute_unit->simds[i]->issue_buffer);
		list_free(compute_unit->simds[i]->read_buffer);
		list_free(compute_unit->simds[i]->exec_buffer);
		free(compute_unit->simds[i]->subwavefront_pool);
		free(compute_unit->simds[i]->wkg_util);
		free(compute_unit->simds[i]->wvf_util);
		free(compute_unit->simds[i]->rdy_util);
		free(compute_unit->simds[i]->occ_util);
		free(compute_unit->simds[i]->wki_util);
		free(compute_unit->simds[i]->act_util);
		free(compute_unit->simds[i]->tot_util);
		free(compute_unit->simds[i]);

		si_inst_buffer_free(compute_unit->inst_buffers[i]);
		list_free(compute_unit->fetch_buffers[i]);
		list_free(compute_unit->decode_buffers[i]);
	}
	free(compute_unit->simds);
	free(compute_unit->inst_buffers);
	free(compute_unit->fetch_buffers);
	free(compute_unit->decode_buffers);
	free(compute_unit->work_groups);  /* List of mapped work-groups */
	mod_free(compute_unit->local_memory);
	free(compute_unit);
}


void si_compute_unit_map_work_group(struct si_compute_unit_t *compute_unit, 
	struct si_work_group_t *work_group)
{
	struct si_ndrange_t *ndrange = work_group->ndrange;
	struct si_wavefront_t *wavefront;
	int wavefront_id;
	int ib_id;

	assert(compute_unit->work_group_count < si_gpu->work_groups_per_compute_unit);
	assert(!work_group->id_in_compute_unit);

	/* Find an available slot */
	while (work_group->id_in_compute_unit < si_gpu->work_groups_per_compute_unit &&
		compute_unit->work_groups[work_group->id_in_compute_unit])
		work_group->id_in_compute_unit++;
	assert(work_group->id_in_compute_unit < si_gpu->work_groups_per_compute_unit);
	compute_unit->work_groups[work_group->id_in_compute_unit] = work_group;
	compute_unit->work_group_count++;

	/* If compute unit reached its maximum load, remove it from 'compute_unit_ready' list.
	 * Otherwise, move it to the end of the 'compute_unit_ready' list. */
	assert(DOUBLE_LINKED_LIST_MEMBER(si_gpu, compute_unit_ready, compute_unit));
	DOUBLE_LINKED_LIST_REMOVE(si_gpu, compute_unit_ready, compute_unit);
	if (compute_unit->work_group_count < si_gpu->work_groups_per_compute_unit)
	{
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_gpu, compute_unit_ready, compute_unit);
	}
	
	/* If this is the first scheduled work-group, insert to 'compute_unit_busy' list. */
	if (!DOUBLE_LINKED_LIST_MEMBER(si_gpu, compute_unit_busy, compute_unit))
	{
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_gpu, compute_unit_busy, compute_unit);
	}

	/* Assign wavefronts identifiers in compute unit */
	SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
	{
		wavefront = ndrange->wavefronts[wavefront_id];
		wavefront->id_in_compute_unit = work_group->id_in_compute_unit *
			ndrange->wavefronts_per_work_group + wavefront->id_in_work_group;
	}

	/* Set instruction buffer for work group */
	ib_id = work_group->id_in_compute_unit % si_gpu_num_inst_buffers;
	work_group->inst_buffer = compute_unit->inst_buffers[ib_id];

	/* Insert wavefronts into an instruction buffer */
	si_inst_buffer_map_wavefronts(work_group->inst_buffer, work_group);

	/* Change work-group status to running */
	si_work_group_clear_status(work_group, si_work_group_pending);
	si_work_group_set_status(work_group, si_work_group_running);

	/* Trace */
	si_trace("si.map_wg cu=%d wg=%d wi_first=%d wi_count=%d wf_first=%d wf_count=%d\n",
		compute_unit->id, work_group->id, work_group->work_item_id_first,
		work_group->work_item_count, work_group->wavefront_id_first,
		work_group->wavefront_count);

	/* Stats */
	compute_unit->mapped_work_groups++;
}


void si_compute_unit_unmap_work_group(struct si_compute_unit_t *compute_unit, struct si_work_group_t *work_group)
{
	/* Reset mapped work-group */
	assert(compute_unit->work_group_count > 0);
	assert(compute_unit->work_groups[work_group->id_in_compute_unit]);
	compute_unit->work_groups[work_group->id_in_compute_unit] = NULL;
	compute_unit->work_group_count--;

	/* Unmap wavefronts from instruction buffer */
	si_inst_buffer_unmap_wavefronts(work_group->inst_buffer, work_group);

	/* If compute unit accepts work-groups again, insert into 'compute_unit_ready' list */
	if (!DOUBLE_LINKED_LIST_MEMBER(si_gpu, compute_unit_ready, compute_unit))
	{
		DOUBLE_LINKED_LIST_INSERT_TAIL(si_gpu, compute_unit_ready, compute_unit);
	}
	
	/* If compute unit is not compute_unit_busy anymore, remove it from 
	 * 'compute_unit_busy' list */
	if (!compute_unit->work_group_count && DOUBLE_LINKED_LIST_MEMBER(si_gpu, compute_unit_busy,
		compute_unit))
	{
		DOUBLE_LINKED_LIST_REMOVE(si_gpu, compute_unit_busy, compute_unit);
	}

	/* Trace */
	si_trace("si.unmap_wg cu=%d wg=%d\n", compute_unit->id, work_group->id);
}

void si_compute_unit_fetch(struct si_compute_unit_t *compute_unit, int active_ib)
{
	int i, j;
	int instructions_processed = 0;
	int work_item_id;
	struct si_wavefront_t *wavefront;
	struct si_work_item_t *work_item;
	struct si_uop_t *uop;
	struct si_work_item_uop_t *work_item_uop;
	struct si_inst_buffer_entry_t *inst_buffer_entry;
	unsigned int rel_addr;
	char inst_str[MAX_INST_STR_SIZE];
	char inst_str_trimmed[MAX_INST_STR_SIZE];

	assert(active_ib < compute_unit->num_inst_buffers);

	for (i = 0; i < si_gpu_max_wavefronts_per_inst_buffer; i++)
	{
		wavefront = compute_unit->inst_buffers[active_ib]->entries[i]->wavefront;

		/* No wavefront */
		if (!wavefront) 
			continue;

		/* Fetch buffer full */
		if (list_count(compute_unit->fetch_buffers[active_ib]) >= si_gpu_fetch_buffer_size)
			break;

		/* Only fetch a fixed number of instructions per cycle */
		if (instructions_processed == si_gpu_fetch_width)
			continue;

		/* Sanity check wavefront */
		assert(wavefront->inst_buffer_entry);
		assert(wavefront->inst_buffer_entry ==
			compute_unit->inst_buffers[active_ib]->entries[i]);

		/* If the wavefront finishes, there still may be outstanding memory 
		 * operations, so if the entry is marked finished the wavefront must
		 * also be finished, but not vice-versa */
		if (wavefront->inst_buffer_entry->wavefront_finished)
		{
			assert(wavefront->finished);
			continue;
		}

		/* Wavefront is finished but other wavefronts from workgroup remain.
		 * There may still be outstanding memory operations, but no more
		 * instructions should be fetched. */
		if (wavefront->finished)
			continue;

		/* Wavefront isn't ready (previous instruction is still in flight) */
		if (!wavefront->inst_buffer_entry->ready)
			continue;

		/* Wavefront is ready but waiting on outstanding memory instructions */
		if (wavefront->inst_buffer_entry->wait_for_mem)
		{
			if (!wavefront->inst_buffer_entry->lgkm_cnt &&
				!wavefront->inst_buffer_entry->vm_cnt)
			{
				wavefront->inst_buffer_entry->wait_for_mem = 0;	
			}
			else
			{
				/* TODO Show a waiting state in visualization tool */
				continue;
			}
		}

		/* Wavefront is ready but waiting at barrier */
		if (wavefront->inst_buffer_entry->wait_for_barrier)
		{
			/* TODO Show a waiting state in visualization tool */
			continue;
		}

		/* If wavefront is ready, there should be no uop in the instruction buffer */
		assert(!compute_unit->inst_buffers[active_ib]->entries[i]->uop);

		/* XXX Check for if the wavefront is at a barrier. 
		 *        - If so, check to see if all wavefronts have reached the barrier
		 *           - If so, clear them all and proceed
		 *        - If not, continue to wait */

		/* The inst buffer entry is empty, so fetch another instruction */

		/* Emulate instruction */
		si_wavefront_execute(wavefront);

		inst_buffer_entry = wavefront->inst_buffer_entry;
		inst_buffer_entry->ready = 0;

		/* Create uop */
		uop = si_uop_create();
		uop->wavefront = wavefront;
		uop->work_group = wavefront->work_group;
		uop->compute_unit = compute_unit;
		uop->id_in_compute_unit = compute_unit->uop_id_counter++;
		uop->inst_buffer_id = active_ib;
		uop->vector_mem_read = wavefront->vector_mem_read;
		uop->vector_mem_write = wavefront->vector_mem_write;
		uop->scalar_mem_read = wavefront->scalar_mem_read;
		uop->local_mem_read = wavefront->local_mem_read;
		uop->local_mem_write = wavefront->local_mem_write;
		uop->inst_buffer_entry = wavefront->inst_buffer_entry;
		uop->wavefront_last_inst = wavefront->finished;
		uop->mem_wait_inst = wavefront->mem_wait;
		uop->barrier_wait_inst = wavefront->barrier;
		assert(wavefront->work_group && uop->work_group);

		/* Trace */
		if (si_tracing())
		{
			rel_addr = wavefront->inst_buf - wavefront->inst_buf_start;
			si_inst_dump(&wavefront->inst, wavefront->inst_size, wavefront->inst_buf, 
				rel_addr, inst_str, sizeof inst_str);
			str_single_spaces(inst_str_trimmed, sizeof inst_str_trimmed, inst_str);
			si_trace("si.new_inst id=%lld cu=%d ib=%d wg=%d wf=%d "
				"stg=\"f\" asm=\"%s\"\n", uop->id_in_compute_unit, compute_unit->id,
				uop->inst_buffer_id, uop->work_group->id, wavefront->id, 
				inst_str_trimmed);
		}
		
		/* Update last memory accesses */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
		{
			work_item = si_gpu->ndrange->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];

			/* global */
			work_item_uop->global_mem_access_addr = work_item->global_mem_access_addr;
			work_item_uop->global_mem_access_size = work_item->global_mem_access_size;

			/* local */
			work_item_uop->local_mem_access_count = work_item->local_mem_access_count;
			for (j = 0; j < work_item->local_mem_access_count; j++)
			{
				work_item_uop->local_mem_access_kind[j] = 
					work_item->local_mem_access_type[j];
				work_item_uop->local_mem_access_addr[j] = 
					work_item->local_mem_access_addr[j];
				work_item_uop->local_mem_access_size[j] = 
					work_item->local_mem_access_size[j];
			}
		}

		/* Access instruction cache. Record the time when the instruction will have 
		 * been fetched, as per the latency of the instruction memory. */
		uop->fetch_ready = si_gpu->cycle + si_gpu_fetch_latency;

		/* Insert uop into instruction buffer */
		inst_buffer_entry->uop = uop;
		inst_buffer_entry->cycle_fetched = si_gpu->cycle;
		/* Insert into fetch buffer */
		list_enqueue(compute_unit->fetch_buffers[active_ib], uop);

		instructions_processed++;
	}
}

/* Decode one instruction per hardware unit */
void si_compute_unit_decode(struct si_compute_unit_t *compute_unit, int active_ib)
{
	struct si_uop_t *uop;
	struct si_wavefront_t *wavefront;
	int instructions_processed = 0;
	int i;

	for (i = 0; i < list_count(compute_unit->fetch_buffers[active_ib]); i++)
	{
		uop = list_head(compute_unit->fetch_buffers[active_ib]);
		assert(uop);

		/* Skip all uops that have not yet completed the fetch */
		if (si_gpu->cycle < uop->fetch_ready)
			break;

		/* Only decode a fixed number of instructions per cycle */
		if (instructions_processed == si_gpu_decode_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n",
				uop->id_in_compute_unit, compute_unit->id, uop->wavefront->id);
			continue;
		}

		/* Stop if decode buffer is full */
		if (list_count(compute_unit->decode_buffers[active_ib]) ==
			si_gpu_decode_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n",
				uop->id_in_compute_unit, compute_unit->id,
				uop->wavefront->id);
			break;
		}

		wavefront = uop->wavefront;
		assert(wavefront);

		/* Decode uop */
		uop->decode_ready = si_gpu->cycle + si_gpu_decode_latency;
		list_remove(compute_unit->fetch_buffers[active_ib], uop);
		list_enqueue(compute_unit->decode_buffers[active_ib], uop);

		instructions_processed++;
	}
}

/* Issue one instruction per hardware unit */
void si_compute_unit_issue(struct si_compute_unit_t *compute_unit, int active_ib)
{
	struct si_scalar_unit_t *scalar_unit;
	struct si_uop_t *uop;
	struct si_wavefront_t *wavefront;
	int instructions_processed = 0;
	int i;

	scalar_unit = &compute_unit->scalar_unit;

	for (i = 0; i < list_count(compute_unit->decode_buffers[active_ib]); i++)
	{
		uop = list_head(compute_unit->decode_buffers[active_ib]);
		assert(uop);

		/* Skip all uops that have not yet completed the decode */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Only decode a fixed number of instructions per cycle */
		if (instructions_processed == si_gpu_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
				uop->id_in_compute_unit, compute_unit->id, uop->wavefront->id);
			continue;
		}

		wavefront = uop->wavefront;
		assert(wavefront);

		/* Issue */
		switch (wavefront->inst.info->fmt)
		{

		/* Scalar ALU */
		case SI_FMT_SOPP:
		{
			/* Branch Unit */
			if (wavefront->inst.micro_inst.sopp.op > 1 &&
					wavefront->inst.micro_inst.sopp.op < 10)
			{
				/* Continue if branch unit decode buffer is full */
				if (list_count(compute_unit->branch_unit.issue_buffer) ==
					si_gpu_branch_unit_issue_buffer_size)
				{
					si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
						uop->id_in_compute_unit, compute_unit->id, 
						uop->wavefront->id);
					continue;
				}

				/* Issue uop */
				uop->issue_ready = si_gpu->cycle + si_gpu_issue_latency;
				list_remove(compute_unit->decode_buffers[active_ib], uop);
				list_enqueue(compute_unit->branch_unit.issue_buffer, uop);

				/* Statistics */
				compute_unit->branch_inst_count++;
			}
			/* Scalar Unit */
			else
			{
				/* Continue if scalar unit decode buffer is full */
				if (list_count(scalar_unit->issue_buffer) == 
						si_gpu_scalar_unit_issue_buffer_size)
				{
					si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
						uop->id_in_compute_unit, compute_unit->id, 
						uop->wavefront->id);
					continue;
				}

				/* Issue uop */
				uop->issue_ready = si_gpu->cycle + si_gpu_issue_latency;
				list_remove(compute_unit->decode_buffers[active_ib], uop);
				list_enqueue(scalar_unit->issue_buffer, uop);

				/* Statistics */
				compute_unit->scalar_alu_inst_count++;
			}

			break;
		}
		case SI_FMT_SOP1:
		case SI_FMT_SOP2:
		case SI_FMT_SOPC:
		case SI_FMT_SOPK:
		{
			/* Continue if scalar unit decode buffer is full */
			if (list_count(scalar_unit->issue_buffer) ==
					si_gpu_scalar_unit_issue_buffer_size)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
					uop->id_in_compute_unit, compute_unit->id, 
					uop->wavefront->id);
				continue;
			}

			/* Issue uop */
			uop->issue_ready = si_gpu->cycle + si_gpu_issue_latency;
			list_remove(compute_unit->decode_buffers[active_ib], uop);
			list_enqueue(scalar_unit->issue_buffer, uop);

			/* Statistics */
			compute_unit->scalar_alu_inst_count++;

			break;
		}

		/* Scalar memory */
		case SI_FMT_SMRD:
		{
			/* Continue if scalar unit decode buffer is full */
			if (list_count(scalar_unit->issue_buffer) ==
					si_gpu_scalar_unit_issue_buffer_size)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
					uop->id_in_compute_unit, compute_unit->id, 
					uop->wavefront->id);
				continue;
			}

			/* Issue uop */
			uop->issue_ready = si_gpu->cycle + si_gpu_issue_latency;
			list_remove(compute_unit->decode_buffers[active_ib], uop);
			list_enqueue(scalar_unit->issue_buffer, uop);

			/* Statistics */
			compute_unit->scalar_mem_inst_count++;

			break;
		}

		/* Vector ALU */
		case SI_FMT_VOP2:
		case SI_FMT_VOP1:
		case SI_FMT_VOPC:
		case SI_FMT_VOP3a:
		case SI_FMT_VOP3b:
		{
			/* Continue if SIMD decode buffer is full */
			if (list_count(compute_unit->simds[active_ib]->issue_buffer) ==
					si_gpu_simd_issue_buffer_size)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
					uop->id_in_compute_unit, compute_unit->id, 
					uop->wavefront->id);
				continue;
			}

			/* Issue uop */
			uop->issue_ready = si_gpu->cycle + si_gpu_issue_latency;
			list_remove(compute_unit->decode_buffers[active_ib], uop);
			list_enqueue(compute_unit->simds[active_ib]->issue_buffer, uop);

			/* Statistics */
			compute_unit->simd_inst_count++;

			break;
		}

		/* Vector memory */
		case SI_FMT_MTBUF:
		{
			/* Continue if vector memory decode buffer is full */
			if (list_count(compute_unit->vector_mem_unit.issue_buffer) ==
					si_gpu_vector_mem_issue_buffer_size)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
					uop->id_in_compute_unit, compute_unit->id, 
					uop->wavefront->id);
				continue;
			}

			/* Issue uop */
			uop->issue_ready = si_gpu->cycle + si_gpu_issue_latency;
			list_remove(compute_unit->decode_buffers[active_ib], uop);
			list_enqueue(compute_unit->vector_mem_unit.issue_buffer, uop);

			/* Statistics */
			compute_unit->vector_mem_inst_count++;

			break;
		}

		/* Local memory */
		case SI_FMT_DS:
		{
			/* Continue if LDS decode buffer is full */
			if (list_count(compute_unit->lds.issue_buffer) ==
					si_gpu_lds_issue_buffer_size)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d stg=\"s\"\n", 
					uop->id_in_compute_unit, compute_unit->id, 
					uop->wavefront->id);
				continue;
			}

			/* Issue uop */
			uop->issue_ready = si_gpu->cycle + si_gpu_issue_latency;
			list_remove(compute_unit->decode_buffers[active_ib], uop);
			list_enqueue(compute_unit->lds.issue_buffer, uop);

			/* Statistics */
			compute_unit->local_mem_inst_count++;

			break;
		}

		default:
		{
			fatal("%s: instruction type not implemented", __FUNCTION__);
		}
		}

		instructions_processed++;  /* This cycle */

		/* Statistics */
		compute_unit->inst_count++;

		/* Trace */
		si_trace("si.inst id=%lld cu=%d wf=%d stg=\"d\"\n", uop->id_in_compute_unit, 
			compute_unit->id, uop->wavefront->id);
	}
}


/* Advance one cycle in the compute unit by running every stage from last to first */
void si_compute_unit_run(struct si_compute_unit_t *compute_unit)
{
	int i;
	int num_simds;
	int active_fetch_ib;  /* Wavefront pool chosen to fetch this cycle */
	int active_decode_ib;  /* Wavefront pool chosen to decode this cycle */
	int active_issue_ib;  /* Wavefront pool chosen to issue this cycle */

	active_fetch_ib = si_gpu->cycle % compute_unit->num_inst_buffers;
	active_decode_ib = active_fetch_ib - (si_gpu_fetch_latency % 
			compute_unit->num_inst_buffers);
	active_decode_ib = (active_decode_ib < 0) ? 
		compute_unit->num_inst_buffers + active_decode_ib : active_decode_ib;
	active_issue_ib = active_decode_ib - (si_gpu_decode_latency %
				compute_unit->num_inst_buffers);
	active_issue_ib = (active_issue_ib < 0) ?
			compute_unit->num_inst_buffers + active_issue_ib : active_issue_ib;

	assert(active_fetch_ib >= 0 && active_fetch_ib < compute_unit->num_inst_buffers);
	assert(active_decode_ib >= 0 && active_decode_ib < compute_unit->num_inst_buffers);
	assert(active_issue_ib >= 0 && active_issue_ib < compute_unit->num_inst_buffers);

	/* Run Engines */
	num_simds = compute_unit->num_inst_buffers;
	for (i = 0; i < num_simds; i++)
		si_simd_run(compute_unit->simds[i]);

	si_vector_mem_run(&compute_unit->vector_mem_unit);

	si_lds_run(&compute_unit->lds);

	si_scalar_unit_run(&compute_unit->scalar_unit);

	si_branch_unit_run(&compute_unit->branch_unit);

	/* Run Issue */
	si_compute_unit_issue(compute_unit, active_issue_ib);

	/* Run Decode */
	si_compute_unit_decode(compute_unit, active_decode_ib);

	/* Run Fetch */
	si_compute_unit_fetch(compute_unit, active_fetch_ib);

	/* Stats */
	compute_unit->cycle++;
}

