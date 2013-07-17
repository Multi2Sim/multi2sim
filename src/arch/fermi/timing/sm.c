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


#include <arch/fermi/emu/grid.h>
#include <arch/fermi/emu/warp.h>
#include <arch/fermi/emu/thread-block.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "sm.h"
#include "gpu.h"
#include "simd-unit.h"
#include "uop.h"
#include "warp-inst-queue.h"

#include "cycle-interval-report.h"

/*
 * Compute Unit
 */

struct frm_sm_t *frm_sm_create()
{
	struct frm_sm_t *sm;
	char buf[MAX_STRING_SIZE];
	int i;

	/* Initialize */
	sm = xcalloc(1, sizeof(struct frm_sm_t));

	/* Local memory */
	snprintf(buf, sizeof buf, "LDS[%d]", sm->id);
	sm->lds_module = mod_create(buf, mod_kind_local_memory,
		frm_gpu_lds_num_ports, frm_gpu_lds_block_size, 
		frm_gpu_lds_latency);

	/* Hardware structures */
	sm->num_warp_inst_queues = 2;
	sm->warp_inst_queues = xcalloc(sm->num_warp_inst_queues,
		sizeof(struct list_t *));
	sm->fetch_buffers = xcalloc(sm->num_warp_inst_queues,
		sizeof(struct list_t *));
	sm->num_simd_units = 2;
	sm->simd_units = xcalloc(sm->num_simd_units, 
		sizeof(struct frm_simd_t*));

	sm->branch_unit.issue_buffer = list_create();
	sm->branch_unit.decode_buffer = list_create();
	sm->branch_unit.read_buffer = list_create();
	sm->branch_unit.exec_buffer = list_create();
	sm->branch_unit.write_buffer = list_create();
	sm->branch_unit.sm = sm;

	sm->vector_mem_unit.issue_buffer = list_create();
	sm->vector_mem_unit.decode_buffer = list_create();
	sm->vector_mem_unit.read_buffer = list_create();
	sm->vector_mem_unit.mem_buffer = list_create();
	sm->vector_mem_unit.write_buffer = list_create();
	sm->vector_mem_unit.sm = sm;

	sm->lds_unit.issue_buffer = list_create();
	sm->lds_unit.decode_buffer = list_create();
	sm->lds_unit.read_buffer = list_create();
	sm->lds_unit.mem_buffer = list_create();
	sm->lds_unit.write_buffer = list_create();
	sm->lds_unit.sm = sm;

	/* Allocate and initialize instruction buffers */
	for (i = 0; i < sm->num_warp_inst_queues; i++)
	{
		sm->warp_inst_queues[i] = frm_warp_inst_queue_create();
		sm->warp_inst_queues[i]->sm = sm;
	}
	for (i = 0; i < sm->num_warp_inst_queues; i++)
		sm->fetch_buffers[i] = list_create();

	/* Allocate SIMD structures */
	assert(sm->num_warp_inst_queues == sm->num_simd_units);
	for (i = 0; i < sm->num_simd_units; i++)
	{
		sm->simd_units[i] = xcalloc(1,
			sizeof(struct frm_simd_t));
		sm->simd_units[i]->id_in_sm = i;
		sm->simd_units[i]->sm = sm;
		sm->simd_units[i]->warp_inst_queue =
			sm->warp_inst_queues[i];
		sm->simd_units[i]->issue_buffer = list_create();
		sm->simd_units[i]->decode_buffer = list_create();
		sm->simd_units[i]->exec_buffer = list_create();
		sm->simd_units[i]->subwarp_inst_queue =
			xcalloc(1, sizeof(struct frm_subwarp_inst_queue_t));

		sm->simd_units[i]->sm = sm;
		sm->simd_units[i]->wkg_util = xcalloc(1,
			sizeof(struct frm_util_t));
		sm->simd_units[i]->wvf_util = xcalloc(1,
			sizeof(struct frm_util_t));
		sm->simd_units[i]->rdy_util = xcalloc(1,
			sizeof(struct frm_util_t));
		sm->simd_units[i]->occ_util = xcalloc(1,
			sizeof(struct frm_util_t));
		sm->simd_units[i]->wki_util = xcalloc(1,
			sizeof(struct frm_util_t));
		sm->simd_units[i]->act_util = xcalloc(1,
			sizeof(struct frm_util_t));
		sm->simd_units[i]->tot_util = xcalloc(1,
			sizeof(struct frm_util_t));
	}

	sm->thread_blocks = 
		xcalloc(frm_gpu_max_thread_blocks_per_sm, sizeof(void *));

	/* Return */
	return sm;
}


void frm_sm_free(struct frm_sm_t *sm)
{
	int i;

	/* Branch Unit */
	frm_uop_list_free(sm->branch_unit.issue_buffer);
	frm_uop_list_free(sm->branch_unit.decode_buffer);
	frm_uop_list_free(sm->branch_unit.read_buffer);
	frm_uop_list_free(sm->branch_unit.exec_buffer);
	frm_uop_list_free(sm->branch_unit.write_buffer);
	list_free(sm->branch_unit.issue_buffer);
	list_free(sm->branch_unit.decode_buffer);
	list_free(sm->branch_unit.read_buffer);
	list_free(sm->branch_unit.exec_buffer);
	list_free(sm->branch_unit.write_buffer);

	/* Global Memory */
	frm_uop_list_free(sm->vector_mem_unit.issue_buffer);
	frm_uop_list_free(sm->vector_mem_unit.decode_buffer);
	frm_uop_list_free(sm->vector_mem_unit.read_buffer);
	frm_uop_list_free(sm->vector_mem_unit.mem_buffer);
	frm_uop_list_free(sm->vector_mem_unit.write_buffer);
	list_free(sm->vector_mem_unit.issue_buffer);
	list_free(sm->vector_mem_unit.decode_buffer);
	list_free(sm->vector_mem_unit.read_buffer);
	list_free(sm->vector_mem_unit.mem_buffer);
	list_free(sm->vector_mem_unit.write_buffer);

	/* Shared Memory */
	frm_uop_list_free(sm->lds_unit.issue_buffer);
	frm_uop_list_free(sm->lds_unit.decode_buffer);
	frm_uop_list_free(sm->lds_unit.read_buffer);
	frm_uop_list_free(sm->lds_unit.mem_buffer);
	frm_uop_list_free(sm->lds_unit.write_buffer);
	list_free(sm->lds_unit.issue_buffer);
	list_free(sm->lds_unit.decode_buffer);
	list_free(sm->lds_unit.read_buffer);
	list_free(sm->lds_unit.mem_buffer);
	list_free(sm->lds_unit.write_buffer);

	/* SIMDs */
	for (i = 0; i < sm->num_simd_units; i++)
	{
		frm_uop_list_free(sm->simd_units[i]->issue_buffer);
		frm_uop_list_free(sm->simd_units[i]->decode_buffer);
		frm_uop_list_free(sm->simd_units[i]->exec_buffer);
		list_free(sm->simd_units[i]->issue_buffer);
		list_free(sm->simd_units[i]->decode_buffer);
		list_free(sm->simd_units[i]->exec_buffer);

		free(sm->simd_units[i]->subwarp_inst_queue);
		free(sm->simd_units[i]->wkg_util);
		free(sm->simd_units[i]->wvf_util);
		free(sm->simd_units[i]->rdy_util);
		free(sm->simd_units[i]->occ_util);
		free(sm->simd_units[i]->wki_util);
		free(sm->simd_units[i]->act_util);
		free(sm->simd_units[i]->tot_util);
		free(sm->simd_units[i]);
	}
	free(sm->simd_units);

	/* Fetch buffers */
	for (i = 0; i < sm->num_warp_inst_queues; i++)
	{
		frm_uop_list_free(sm->fetch_buffers[i]);
		list_free(sm->fetch_buffers[i]);
	}
	free(sm->fetch_buffers);

	/* Others */
	for (i = 0; i < sm->num_warp_inst_queues; i++)
		frm_warp_inst_queue_free(sm->warp_inst_queues[i]);
	free(sm->warp_inst_queues);
	free(sm->thread_blocks);
	mod_free(sm->lds_module);
	free(sm);
}


void frm_sm_map_thread_block(struct frm_sm_t *sm, 
		struct frm_thread_block_t *thread_block)
{
	struct frm_grid_t *grid;
	struct frm_warp_t *warp;

	int warp_id;
	int wiq_id;
	int entry_index;

	assert(sm->thread_block_count < frm_gpu->thread_blocks_per_sm);
	assert(!thread_block->id_in_sm);

	/* Assign thread block ID */
	while (thread_block->id_in_sm < frm_gpu->thread_blocks_per_sm &&
			sm->thread_blocks[thread_block->id_in_sm])
		thread_block->id_in_sm++;

	/* Assign thread block to a SM */
	sm->thread_blocks[thread_block->id_in_sm] = thread_block;
	sm->thread_block_count++;

	/* Remove fully-loaded SM to 'sm_ready' list so that thread blocks
	 * cannot be assigned to it any more */
	if (sm->thread_block_count == frm_gpu->thread_blocks_per_sm)
		list_remove(frm_gpu->sm_ready_list, sm);

	/* Add SM to 'sm_busy' list if it has not been there */
	if (list_index_of(frm_gpu->sm_busy_list, sm) == -1)
		list_add(frm_gpu->sm_busy_list, sm);

	/* Set up warps */
	for (warp_id = 0; warp_id < thread_block->warp_count; warp_id++)
	{
		/* Assign ID */
		warp = thread_block->warps[warp_id];
		warp->id_in_sm = thread_block->id_in_sm *
			thread_block->warp_count + warp_id;

		/* Assign warp instruction queue */
		wiq_id = warp->id_in_sm % frm_gpu_num_warp_inst_queues;
		warp->warp_inst_queue = sm->warp_inst_queues[wiq_id];

		/* Assign warp instruction queue entry */
		entry_index = warp->id_in_sm / frm_gpu_num_warp_inst_queues;
		warp->warp_inst_queue_entry =
			warp->warp_inst_queue->entries[entry_index];

		/* Initialize warp instruction queue entry */
		warp->warp_inst_queue_entry->valid = 1;
		warp->warp_inst_queue_entry->ready = 1;
		warp->warp_inst_queue_entry->warp = warp;
	}

	/* Change thread block status to running */
	grid = thread_block->grid;
	list_remove(grid->pending_thread_blocks, thread_block);
	list_add(grid->running_thread_blocks, thread_block);

	/* Trace */
	frm_trace("frm.map_tb sm=%d tb=%d t_first=%d t_count=%d w_first=%d "
			"w_count=%d\n", sm->id, thread_block->id,
			thread_block->threads[0]->id, thread_block->thread_count, 
			thread_block->warps[0]->id, thread_block->warp_count);

	/* Stats */
	sm->mapped_thread_blocks++;
	if (frm_spatial_report_active)
		frm_report_mapped_thread_block(sm);
}


void frm_sm_unmap_thread_block(struct frm_sm_t *sm, struct frm_thread_block_t *thread_block)
{
	/* Reset mapped work-group */
	assert(sm->thread_block_count > 0);
	assert(sm->thread_blocks[thread_block->id_in_sm]);
	sm->thread_blocks[thread_block->id_in_sm] = NULL;
	sm->thread_block_count--;

	/* Unmap warps from warp instruction queue */
	//frm_warp_inst_queue_unmap_warps(thread_block->warp_inst_queue, 
	//		thread_block);

	/* If compute unit accepts work-groups again, insert into 
	 * 'sm_ready' list */
	if (list_index_of(frm_gpu->sm_ready_list, sm) == -1)
		list_add(frm_gpu->sm_ready_list, sm);

	/* If SM is not sm_busy anymore, remove it from 
	 * 'sm_busy' list */
	if (!sm->thread_block_count && 
			list_index_of(frm_gpu->sm_busy_list, sm) != -1)
		list_remove(frm_gpu->sm_busy_list, sm);

	/* Trace */
	frm_trace("si.unmap_wg cu=%d wg=%d\n", sm->id, thread_block->id);
}

void frm_sm_fetch(struct frm_sm_t *sm, int wiq_id)
{
	int j;
	int instructions_processed = 0;
	int thread_id;
	struct frm_warp_t *warp;
	struct frm_thread_t *thread;
	struct frm_uop_t *uop;
	struct frm_thread_uop_t *thread_uop;
	struct frm_warp_inst_queue_entry_t *warp_inst_queue_entry;
	char inst_str[1024];
	char inst_str_trimmed[1024];

	warp = sm->warp_inst_queues[wiq_id]->entries[0]->warp;

	/* No warp */
	if (!warp) 
		return;

	/* Sanity check warp */
	assert(warp->warp_inst_queue_entry);

	/* If instruction is ready the next cycle */
	if (warp->warp_inst_queue_entry->ready_next_cycle)
	{
		warp->warp_inst_queue_entry->ready = 1;
		warp->warp_inst_queue_entry->ready_next_cycle = 0;
		return;
	}

	/* Only fetch a fixed number of instructions per cycle */
	if (instructions_processed == frm_gpu_fe_fetch_width)
		return;

	/* WIQ entry not ready */
	if (!warp->warp_inst_queue_entry->ready)
		return;

	/* If the warp finishes, there still may be outstanding 
	 * memory operations, so if the entry is marked finished 
	 * the warp must also be finished, but not vice-versa */
	if (warp->warp_inst_queue_entry->warp_finished)
	{
		assert(warp->finished);
		return;
	}

	/* Warp is finished but other warps from thread block
	 * remain.  There may still be outstanding memory operations, 
	 * but no more instructions should be fetched. */
	if (warp->finished)
		return;

	/* Warp is ready but waiting on outstanding 
	 * memory instructions */
	if (warp->warp_inst_queue_entry->wait_for_mem)
	{
		if (!warp->warp_inst_queue_entry->lgkm_cnt &&
				!warp->warp_inst_queue_entry->vm_cnt)
		{
			warp->warp_inst_queue_entry->wait_for_mem =
				0;	
		}
		else
		{
			/* TODO Show a waiting state in visualization 
			 * tool */
			/* XXX uop is already freed */
			return;
		}
	}

	/* Warp is ready but waiting at barrier */
	if (warp->warp_inst_queue_entry->wait_for_barrier)
	{
		/* TODO Show a waiting state in visualization tool */
		/* XXX uop is already freed */
		return;
	}

	/* If fetch buffer full */
	if (list_count(sm->fetch_buffers[wiq_id]) == 
			frm_gpu_fe_fetch_buffer_size)
		return;

	/* Emulate instruction */
	frm_warp_execute(warp);

	warp_inst_queue_entry = warp->warp_inst_queue_entry;
	warp_inst_queue_entry->ready = 0;

	/* Create uop */
	uop = frm_uop_create();
	uop->warp = warp;
	uop->thread_block = warp->thread_block;
	uop->sm = sm;
	uop->id_in_sm = sm->uop_id_counter++;
	uop->id_in_warp = warp->uop_id_counter++;
	uop->warp_inst_queue_id = wiq_id;
	uop->vector_mem_read = warp->vector_mem_read;
	uop->vector_mem_write = warp->vector_mem_write;
	uop->lds_read = warp->lds_read;
	uop->lds_write = warp->lds_write;
	uop->warp_inst_queue_entry = warp->warp_inst_queue_entry;
	uop->warp_last_inst = warp->finished;
	uop->mem_wait_inst = warp->mem_wait;
	uop->barrier_wait_inst = warp->barrier;
	uop->inst = warp->inst;
	uop->cycle_created = asTiming(frm_gpu)->cycle;
	assert(warp->thread_block && uop->thread_block);

	/* Debug */
	//frm_inst_dump(inst_str, sizeof inst_str, 
	//	warp->grid->inst_buffer, warp->pc / 8);

	/* Trace */
	if (frm_tracing())
	{
		//frm_inst_dump(&warp->inst, warp->inst_size, 
		//	warp->pc, 
		//	warp->grid->inst_buffer + warp->pc,
		//	inst_str, sizeof inst_str);
		str_single_spaces(inst_str_trimmed, 
				sizeof inst_str_trimmed, 
				inst_str);
		frm_trace("si.new_inst id=%lld cu=%d ib=%d wg=%d "
				"wf=%d uop_id=%lld stg=\"f\" asm=\"%s\"\n", 
				uop->id_in_sm, sm->id, 
				uop->warp_inst_queue_id, uop->thread_block->id, 
				warp->id, uop->id_in_warp, 
				inst_str_trimmed);
	}

	/* Update last memory accesses */
	for (thread_id = uop->warp->threads[0]->id_in_warp; 
			thread_id < uop->warp->thread_count; 
			thread_id++)
	{
		thread = uop->warp->threads[thread_id];
		thread_uop = 
			&uop->thread_uop[thread->id_in_warp];

		/* Global memory */
		thread_uop->global_mem_access_addr = 
			thread->global_mem_access_addr;
		thread_uop->global_mem_access_size = 
			thread->global_mem_access_size;

		/* LDS */
		thread_uop->lds_access_count = 
			thread->lds_access_count;
		for (j = 0; j < thread->lds_access_count; j++)
		{
			thread_uop->lds_access_kind[j] = 
				thread->lds_access_type[j];
			thread_uop->lds_access_addr[j] = 
				thread->lds_access_addr[j];
			thread_uop->lds_access_size[j] = 
				thread->lds_access_size[j];
		}
	}

	/* Access instruction cache. Record the time when the 
	 * instruction will have been fetched, as per the latency 
	 * of the instruction memory. */
	uop->fetch_ready = asTiming(frm_gpu)->cycle + frm_gpu_fe_fetch_latency;

	/* Insert into fetch buffer */
	list_enqueue(sm->fetch_buffers[wiq_id], uop);

	instructions_processed++;
	sm->inst_count++;
}

/* Decode the instruction type */
void frm_sm_issue_oldest(struct frm_sm_t *sm, 
		int active_fb)
{
	struct frm_uop_t *uop;
	struct frm_uop_t *oldest_uop;
	int list_index;
	int list_entries;
	int i;
	int issued_insts;

	/* Branch unit */
	oldest_uop = NULL;
	list_index = 0;

	list_entries = list_count(sm->fetch_buffers[active_fb]);
	for (issued_insts = 0; 
			issued_insts < frm_gpu_fe_max_inst_issued_per_type;
			issued_insts++)
	{
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(sm->fetch_buffers[active_fb], 
					list_index);
			assert(uop);

			/* Only evaluate branch instructions */
			if (uop->inst.info->fmt) 
			{
				list_index++;
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(frm_gpu)->cycle < uop->fetch_ready)
			{
				list_index++;
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
					uop->fetch_ready < oldest_uop->fetch_ready)
			{
				oldest_uop = uop;
			}

			/* Issue the oldest branch instruction */
			if (oldest_uop &&
					list_count(sm->branch_unit.
						issue_buffer) < 
					frm_gpu_branch_unit_issue_buffer_size)
			{
				oldest_uop->issue_ready = asTiming(frm_gpu)->cycle + 
					frm_gpu_fe_issue_latency;
				list_remove(sm->
						fetch_buffers[active_fb], oldest_uop);
				list_enqueue(sm->branch_unit.
						issue_buffer, oldest_uop);

				/* Trace */
				frm_trace("si.inst id=%lld cu=%d wf=%d "
						"uop_id=%lld stg=\"i\"\n", 
						uop->id_in_sm, 
						sm->id, 
						uop->warp->id, 
						uop->id_in_warp);

				sm->branch_inst_count++;
			}
		}
	}

	/* SIMD unit */
	oldest_uop = NULL;
	list_index = 0;

	list_entries = list_count(sm->fetch_buffers[active_fb]);
	for (issued_insts = 0; 
			issued_insts < frm_gpu_fe_max_inst_issued_per_type;
			issued_insts++)
	{
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(sm->fetch_buffers[active_fb], 
					list_index);
			assert(uop);

			/* Only evaluate SIMD instructions */
			if (uop->inst.info->fmt != FRM_FMT_FP_FADD && 
					uop->inst.info->fmt != FRM_FMT_INT_IMAD && 
					uop->inst.info->fmt != FRM_FMT_INT_ISCADD && 
					uop->inst.info->fmt != FRM_FMT_MISC_S2R &&
					uop->inst.info->fmt != FRM_FMT_CTRL_EXIT)
			{	
				list_index++;
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(frm_gpu)->cycle < uop->fetch_ready)
			{
				list_index++;
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
					uop->fetch_ready < oldest_uop->fetch_ready)
			{
				oldest_uop = uop;
			}

			/* Issue the oldest SIMD instruction */
			if (oldest_uop &&
					list_count(sm->simd_units[active_fb]->
						issue_buffer) < 
					frm_gpu_simd_issue_buffer_size)
			{
				oldest_uop->issue_ready = asTiming(frm_gpu)->cycle + 
					frm_gpu_fe_issue_latency;
				list_remove(sm->
						fetch_buffers[active_fb], oldest_uop);
				list_enqueue(sm->
						simd_units[active_fb]->issue_buffer, 
						oldest_uop);

				/* Trace */
				frm_trace("si.inst id=%lld cu=%d wf=%d "
						"uop_id=%lld stg=\"i\"\n", 
						uop->id_in_sm, 
						sm->id, 
						uop->warp->id, 
						uop->id_in_warp);

				uop->warp_inst_queue_entry->ready_next_cycle = 1;

				sm->simd_inst_count++;
			}
		}
	}

	/* Vector memory */
	oldest_uop = NULL;
	list_index = 0;

	list_entries = list_count(sm->fetch_buffers[active_fb]);
	for (issued_insts = 0; 
			issued_insts < frm_gpu_fe_max_inst_issued_per_type;
			issued_insts++)
	{
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(sm->fetch_buffers[active_fb], 
					list_index);
			assert(uop);

			/* Only evaluate memory instructions */
			if (uop->inst.info->fmt != FRM_FMT_LDST_LD &&
					uop->inst.info->fmt != FRM_FMT_LDST_ST
					&&
					uop->inst.info->fmt != FRM_FMT_MOV_MOV)
			{	
				list_index++;
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(frm_gpu)->cycle < uop->fetch_ready)
			{
				list_index++;
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || uop->fetch_ready < 
					oldest_uop->fetch_ready)
			{
				oldest_uop = uop;
			}

			/* Issue the oldest memory instruction */
			if (oldest_uop &&
					list_count(sm->
						vector_mem_unit.issue_buffer) < 
					frm_gpu_vector_mem_issue_buffer_size)
			{
				oldest_uop->issue_ready = asTiming(frm_gpu)->cycle + 
					frm_gpu_fe_issue_latency;
				list_remove(sm->
						fetch_buffers[active_fb], oldest_uop);
				list_enqueue(sm->
						vector_mem_unit.issue_buffer, 
						oldest_uop);

				/* Trace */
				frm_trace("si.inst id=%lld cu=%d wf=%d "
						"uop_id=%lld stg=\"i\"\n", 
						uop->id_in_sm, 
						sm->id, 
						uop->warp->id, 
						uop->id_in_warp);

				uop->warp_inst_queue_entry->ready_next_cycle = 1;

				sm->vector_mem_inst_count++;
			}
		}
	}

	/* LDS */
	oldest_uop = NULL;
	list_index = 0;

	list_entries = list_count(sm->fetch_buffers[active_fb]);
	for (issued_insts = 0; 
			issued_insts < frm_gpu_fe_max_inst_issued_per_type;
			issued_insts++)
	{
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(sm->fetch_buffers[active_fb], 
					list_index);
			assert(uop);

			/* Only evaluate LDS instructions */
			//if (uop->inst.info->fmt != FRM_FMT_DS)
			//{	
			//	list_index++;
			//	continue;
			//}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(frm_gpu)->cycle < uop->fetch_ready)
			{
				list_index++;
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || uop->fetch_ready < 
					oldest_uop->fetch_ready)
			{
				oldest_uop = uop;
			}

			/* Issue the oldest LDS instruction */
			if (oldest_uop &&
					list_count(
						sm->lds_unit.issue_buffer) < 
					frm_gpu_lds_issue_buffer_size)
			{
				oldest_uop->issue_ready = asTiming(frm_gpu)->cycle + 
					frm_gpu_fe_issue_latency;
				list_remove(sm->
						fetch_buffers[active_fb], oldest_uop);
				list_enqueue(
						sm->lds_unit.issue_buffer, 
						oldest_uop);

				/* Trace */
				frm_trace("si.inst id=%lld cu=%d wf=%d "
						"uop_id=%lld stg=\"i\"\n", 
						uop->id_in_sm, 
						sm->id, 
						uop->warp->id, 
						uop->id_in_warp);

				uop->warp_inst_queue_entry->ready_next_cycle = 1;

				sm->lds_inst_count++;
			}
		}
	}

	/* Update visualization states for all instructions not issued */
	list_entries = list_count(sm->fetch_buffers[active_fb]);
	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(sm->fetch_buffers[active_fb], i);
		assert(uop);

		/* Skip all uops that have not yet completed the fetch */
		if (asTiming(frm_gpu)->cycle < uop->fetch_ready)
		{
			continue;
		}

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
				uop->id_in_sm, sm->id, 
				uop->warp->id, uop->id_in_warp);
	}
}

void frm_sm_update_fetch_visualization(
		struct frm_sm_t *sm, int non_active_fb)
{
	struct frm_uop_t *uop;
	int list_entries;
	int i;

	/* Update visualization states for all instructions not issued */
	list_entries = list_count(sm->fetch_buffers[non_active_fb]);
	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(sm->fetch_buffers[non_active_fb], i);
		assert(uop);

		/* Skip all uops that have not yet completed the fetch */
		if (asTiming(frm_gpu)->cycle < uop->fetch_ready)
		{
			continue;
		}

		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
				uop->id_in_sm, sm->id, 
				uop->warp->id, uop->id_in_warp);
	}
}

/* Decode the instruction type */
void frm_sm_issue_first(struct frm_sm_t *sm, 
		int active_fb)
{
	struct frm_uop_t *uop;
	int list_index = 0;
	int list_entries;
	int i;

	int total_insts_issued = 0;
	//int scalar_insts_issued = 0;
	//int branch_insts_issued = 0;
	//int lds_insts_issued = 0;
	int simd_insts_issued = 0;
	int mem_insts_issued = 0;

	list_entries = list_count(sm->fetch_buffers[active_fb]);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(sm->fetch_buffers[active_fb], 
				list_index);
		assert(uop);

		/* Skip all uops that have not yet completed the fetch */
		if (asTiming(frm_gpu)->cycle < uop->fetch_ready)
		{
			list_index++;
			continue;
		}

		/* Only issue a fixed number of instructions per cycle */
		if (total_insts_issued == frm_gpu_fe_issue_width)
		{
			frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
					"stg=\"s\"\n", uop->id_in_sm, 
					sm->id, uop->warp->id,
					uop->id_in_warp);
			list_index++;
			continue;
		}

		/* Determine instruction type.  This simply decodes the 
		 * instruction type, so that it can be issued to the proper 
		 * hardware unit.  It is not the full decode stage */
		switch (uop->inst.info->fmt)
		{

			///* Scalar ALU or Branch */
			//case FRM_FMT_SOPP:
			//{
			//	/* Branch Unit */
			//	if (uop->inst.micro_inst.sopp.op > 1 &&
			//		uop->inst.micro_inst.sopp.op < 10)
			//	{
			//		/* Stall if max branch instructions 
			//		 * already issued */
			//		assert(branch_insts_issued <= 
			//			frm_gpu_fe_max_inst_issued_per_type);
			//		if (branch_insts_issued == 
			//			frm_gpu_fe_max_inst_issued_per_type)
			//		{
			//			frm_trace("si.inst id=%lld cu=%d wf=%d "
			//				"uop_id=%lld stg=\"s\"\n", 
			//				uop->id_in_sm, 
			//				sm->id, 
			//				uop->warp->id, 
			//				uop->id_in_warp);
			//			list_index++;
			//			continue;
			//		}

			//		/* Stall if branch unit issue buffer is full */
			//		if (list_count(sm->
			//			branch_unit.issue_buffer) ==
			//			frm_gpu_branch_unit_issue_buffer_size)
			//		{
			//			frm_trace("si.inst id=%lld cu=%d wf=%d "
			//				"uop_id=%lld stg=\"s\"\n", 
			//				uop->id_in_sm, 
			//				sm->id, 
			//				uop->warp->id, 
			//				uop->id_in_warp);
			//			list_index++;
			//			continue;
			//		}

			//		uop->issue_ready = asTiming(frm_gpu)->cycle + 
			//			frm_gpu_fe_issue_latency;
			//		list_remove(sm->
			//			fetch_buffers[active_fb], uop);
			//		list_enqueue(sm->
			//			branch_unit.issue_buffer, uop);

			//		branch_insts_issued++;
			//		sm->branch_inst_count++;
			//	}
			//	/* Scalar Unit */
			//	else
			//	{
			//		/* Stall if max scalar instructions 
			//		 * already issued */
			//		assert(scalar_insts_issued <= 
			//			frm_gpu_fe_max_inst_issued_per_type);
			//		if (scalar_insts_issued == 
			//			frm_gpu_fe_max_inst_issued_per_type)
			//		{
			//			frm_trace("si.inst id=%lld cu=%d wf=%d "
			//				"uop_id=%lld stg=\"s\"\n", 
			//				uop->id_in_sm, 
			//				sm->id, 
			//				uop->warp->id, 
			//				uop->id_in_warp);
			//			list_index++;
			//			continue;
			//		}

			//		/* Stall if scalar unit issue buffer is full */
			//		if (list_count(sm->
			//			scalar_unit.issue_buffer) ==
			//			frm_gpu_scalar_unit_issue_buffer_size)
			//		{
			//			frm_trace("si.inst id=%lld cu=%d wf=%d "
			//				"uop_id=%lld stg=\"s\"\n", 
			//				uop->id_in_sm, 
			//				sm->id, 
			//				uop->warp->id, 
			//				uop->id_in_warp);
			//			list_index++;
			//			continue;
			//		}

			//		uop->issue_ready = asTiming(frm_gpu)->cycle + 
			//			frm_gpu_fe_issue_latency;
			//		list_remove(
			//			sm->
			//			fetch_buffers[active_fb], uop);
			//		list_enqueue(sm->
			//			scalar_unit.issue_buffer, uop);

			//		scalar_insts_issued++;
			//		sm->scalar_alu_inst_count++;
			//	}

			//	break;
			//}
			//case FRM_FMT_SOP1:
			//case FRM_FMT_SOP2:
			//case FRM_FMT_SOPC:
			//case FRM_FMT_SOPK:
			//{
			//	/* Stall if max scalar instructions already issued */
			//	assert(scalar_insts_issued <= 
			//		frm_gpu_fe_max_inst_issued_per_type);
			//	if (scalar_insts_issued == 
			//		frm_gpu_fe_max_inst_issued_per_type)
			//	{
			//		frm_trace("si.inst id=%lld cu=%d wf=%d "
			//			"uop_id=%lld stg=\"s\"\n",
			//			uop->id_in_sm, 
			//			sm->id, 
			//			uop->warp->id, 
			//			uop->id_in_warp);
			//		list_index++;
			//		continue;
			//	}

			//	/* Stall if scalar unit issue buffer is full */
			//	if (list_count(sm->scalar_unit.issue_buffer) 
			//		== frm_gpu_scalar_unit_issue_buffer_size)
			//	{
			//		frm_trace("si.inst id=%lld cu=%d wf=%d "
			//			"uop_id=%lld stg=\"s\"\n",
			//			uop->id_in_sm, 
			//			sm->id, 
			//			uop->warp->id, 
			//			uop->id_in_warp);
			//		list_index++;
			//		continue;
			//	}

			//	uop->issue_ready = asTiming(frm_gpu)->cycle + 
			//		frm_gpu_fe_issue_latency;
			//	list_remove(sm->fetch_buffers[active_fb], 
			//		uop);
			//	list_enqueue(sm->scalar_unit.issue_buffer, 
			//		uop);

			//	scalar_insts_issued++;
			//	sm->scalar_alu_inst_count++;

			//	break;
			//}

			///* Scalar memory */
			//case FRM_FMT_SMRD:
			//{
			//	/* Stall if max scalar instructions already issued */
			//	assert(scalar_insts_issued <= 
			//		frm_gpu_fe_max_inst_issued_per_type);
			//	if (scalar_insts_issued == 
			//		frm_gpu_fe_max_inst_issued_per_type)
			//	{
			//		frm_trace("si.inst id=%lld cu=%d wf=%d "
			//			"uop_id=%lld stg=\"s\"\n",
			//			uop->id_in_sm, 
			//			sm->id, 
			//			uop->warp->id, 
			//			uop->id_in_warp);
			//		list_index++;
			//		continue;
			//	}

			//	/* Stall if scalar unit issue buffer is full */
			//	if (list_count(sm->scalar_unit.issue_buffer) 
			//		== frm_gpu_scalar_unit_issue_buffer_size)
			//	{
			//		frm_trace("si.inst id=%lld cu=%d wf=%d "
			//			"uop_id=%lld stg=\"s\"\n",
			//			uop->id_in_sm, 
			//			sm->id, 
			//			uop->warp->id, 
			//			uop->id_in_warp);
			//		list_index++;
			//		continue;
			//	}

			//	uop->issue_ready = asTiming(frm_gpu)->cycle + 
			//		frm_gpu_fe_issue_latency;
			//	list_remove(sm->fetch_buffers[active_fb], 
			//		uop);
			//	list_enqueue(sm->scalar_unit.issue_buffer, 
			//		uop);

			//	scalar_insts_issued++;
			//	sm->scalar_mem_inst_count++;

			//	break;
			//}

			/* Vector ALU */
			case FRM_FMT_FP_FADD:
			case FRM_FMT_INT_IMAD:
			case FRM_FMT_MOV_MOV:
			case FRM_FMT_MISC_S2R:
			case FRM_FMT_CTRL_EXIT:
				{
					/* Stall if max SIMD instructions already issued */
					assert(simd_insts_issued <= 
							frm_gpu_fe_max_inst_issued_per_type);
					if (simd_insts_issued == 
							frm_gpu_fe_max_inst_issued_per_type)
					{
						frm_trace("si.inst id=%lld cu=%d wf=%d "
								"uop_id=%lld stg=\"s\"\n",
								uop->id_in_sm, 
								sm->id, 
								uop->warp->id, 
								uop->id_in_warp);
						list_index++;
						continue;
					}

					/* Stall if SIMD issue buffer is full */
					if (list_count(sm->simd_units[active_fb]->
								issue_buffer) == frm_gpu_simd_issue_buffer_size)
					{
						frm_trace("si.inst id=%lld cu=%d wf=%d "
								"uop_id=%lld stg=\"s\"\n",
								uop->id_in_sm, 
								sm->id, 
								uop->warp->id, 
								uop->id_in_warp);
						list_index++;
						continue;
					}

					uop->issue_ready = asTiming(frm_gpu)->cycle + 
						frm_gpu_fe_issue_latency;
					list_remove(sm->fetch_buffers[active_fb], 
							uop);
					list_enqueue(sm->simd_units[active_fb]->
							issue_buffer, uop);

					uop->warp_inst_queue_entry->ready_next_cycle = 1;

					simd_insts_issued++;
					sm->simd_inst_count++;

					break;
				}

				/* Memory instruction */
			case FRM_FMT_LDST_LD:
			case FRM_FMT_LDST_ST:
				{
					/* Stall if max vector memory instructions already 
					 * issued */
					assert(mem_insts_issued <= 
							frm_gpu_fe_max_inst_issued_per_type);
					if (mem_insts_issued == 
							frm_gpu_fe_max_inst_issued_per_type)
					{
						frm_trace("si.inst id=%lld cu=%d wf=%d "
								"uop_id=%lld stg=\"s\"\n",
								uop->id_in_sm, 
								sm->id, 
								uop->warp->id, 
								uop->id_in_warp);
						list_index++;
						continue;
					}

					/* Stall if vector memory issue buffer is full */
					if (list_count(
								sm->vector_mem_unit.issue_buffer) ==
							frm_gpu_vector_mem_issue_buffer_size)
					{
						frm_trace("si.inst id=%lld cu=%d wf=%d "
								"uop_id=%lld stg=\"s\"\n",
								uop->id_in_sm, 
								sm->id, 
								uop->warp->id, 
								uop->id_in_warp);
						list_index++;
						continue;
					}

					uop->issue_ready = asTiming(frm_gpu)->cycle + 
						frm_gpu_fe_issue_latency;
					list_remove(sm->fetch_buffers[active_fb], 
							uop);
					list_enqueue(sm->vector_mem_unit.issue_buffer,
							uop);

					uop->warp_inst_queue_entry->ready_next_cycle = 1;

					mem_insts_issued++;
					sm->vector_mem_inst_count++;

					break;
				}

				///* Local Data Share */ 
				//case FRM_FMT_DS:
				//{
				//	/* Stall if max LDS instructions already issued */
				//	assert(lds_insts_issued <= 
				//		frm_gpu_fe_max_inst_issued_per_type);
				//	if (lds_insts_issued == 
				//		frm_gpu_fe_max_inst_issued_per_type)
				//	{
				//		frm_trace("si.inst id=%lld cu=%d wf=%d "
				//			"uop_id=%lld stg=\"s\"\n",
				//			uop->id_in_sm, 
				//			sm->id, 
				//			uop->warp->id, 
				//			uop->id_in_warp);
				//		list_index++;
				//		continue;
				//	}

				//	/* Stall if the LDS issue buffer is full */
				//	if (list_count(sm->lds_unit.issue_buffer) ==
				//		frm_gpu_lds_issue_buffer_size)
				//	{
				//		frm_trace("si.inst id=%lld cu=%d wf=%d "
				//			"uop_id=%lld stg=\"s\"\n", 
				//			uop->id_in_sm, 
				//			sm->id, 
				//			uop->warp->id, 
				//			uop->id_in_warp);
				//		list_index++;
				//		continue;
				//	}

				//	uop->issue_ready = asTiming(frm_gpu)->cycle + 
				//		frm_gpu_fe_issue_latency;
				//	list_remove(sm->fetch_buffers[active_fb], 
				//		uop);
				//	list_enqueue(sm->lds_unit.issue_buffer, uop);

				//	uop->warp_inst_queue_entry->ready_next_cycle = 1;

				//	lds_insts_issued++;
				//	sm->lds_inst_count++;

				//	break;
				//}

			default:
				fatal("%s: instruction type not implemented",
						__FUNCTION__);

		}

		total_insts_issued++;

		/* Trace */
		frm_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"i\"\n", 
				uop->id_in_sm, sm->id, 
				uop->warp->id, uop->id_in_warp);
	}
}

/* Advance one cycle in the SM by running every stage from last to first */
void frm_sm_run(struct frm_sm_t *sm)
{
	//struct arch_t *arch = frm_emu->arch;
	int i;
	//int active_fetch_buffer;  /* Fetch buffer chosen to issue this cycle */

	/* SIMD */
	for (i = 0; i < sm->num_simd_units; i++)
		frm_simd_run(sm->simd_units[i]);

	/* Vector memory */
	frm_vector_mem_run(&sm->vector_mem_unit);

	/* Shared memory */
	frm_lds_run(&sm->lds_unit);

	/* collector unit */
	//frm_collector_unit_run(&sm->scalar_unit);

	/* Branch unit */
	frm_branch_unit_run(&sm->branch_unit);

	/* Issue from the active fetch buffer */
	//frm_sm_issue_first(sm, active_fetch_buffer);
	frm_sm_issue_oldest(sm, 0);
	frm_sm_issue_oldest(sm, 1);

	/* Update visualization in non-active fetch buffers */
	//for (i = 0; i < sm->num_simd_units; i++)
	//{
	//	if (i != 0)
	//	{
	//		frm_sm_update_fetch_visualization(
	//			sm, i);
	//	}
	//}

	/* Fetch */
	for (i = 0; i < sm->num_warp_inst_queues; i++)
		frm_sm_fetch(sm, i);

	/* Stats */
	sm->cycle++;

	if(frm_spatial_report_active)
		frm_sm_interval_update(sm);
}

