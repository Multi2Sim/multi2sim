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


#include <arch/southern-islands/emu/ndrange.h>
#include <arch/southern-islands/emu/wavefront.h>
#include <arch/southern-islands/emu/work-group.h>
#include <arch/x86/emu/emu.h>
#include <driver/opencl/opencl.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "compute-unit.h"
#include "gpu.h"
#include "simd-unit.h"
#include "uop.h"
#include "wavefront-pool.h"

#include "cycle-interval-report.h"

/*
 * Compute Unit
 */

void SIComputeUnitCreate(SIComputeUnit *self, SIGpu *gpu, int id)
{
	char buf[MAX_STRING_SIZE];
	int i;

	/* Initialize */
	self->id = id;
	self->gpu = gpu;

	/* Local memory */
	snprintf(buf, sizeof buf, "LDS[%d]", self->id);
	self->lds_module = mod_create(buf, mod_kind_local_memory,
		si_gpu_lds_num_ports, si_gpu_lds_block_size, 
		si_gpu_lds_latency);

	/* Hardware structures */
	self->num_wavefront_pools = si_gpu_num_wavefront_pools;
	self->wavefront_pools = xcalloc(
		self->num_wavefront_pools, 
		sizeof(struct si_wavefront_pool_t*));
	self->fetch_buffers = xcalloc(self->num_wavefront_pools,
		sizeof(struct list_t*));
	self->simd_units = xcalloc(self->num_wavefront_pools, 
		sizeof(struct si_simd_t*));

	self->scalar_unit.issue_buffer = list_create();
	self->scalar_unit.decode_buffer = list_create();
	self->scalar_unit.read_buffer = list_create();
	self->scalar_unit.exec_buffer = list_create();
	self->scalar_unit.write_buffer = list_create();
	self->scalar_unit.inflight_buffer = list_create();
	self->scalar_unit.compute_unit = self;

	self->branch_unit.issue_buffer = list_create();
	self->branch_unit.decode_buffer = list_create();
	self->branch_unit.read_buffer = list_create();
	self->branch_unit.exec_buffer = list_create();
	self->branch_unit.write_buffer = list_create();
	self->branch_unit.compute_unit = self;

	self->vector_mem_unit.issue_buffer = list_create();
	self->vector_mem_unit.decode_buffer = list_create();
	self->vector_mem_unit.read_buffer = list_create();
	self->vector_mem_unit.mem_buffer = list_create();
	self->vector_mem_unit.write_buffer = list_create();
	self->vector_mem_unit.compute_unit = self;

	self->lds_unit.issue_buffer = list_create();
	self->lds_unit.decode_buffer = list_create();
	self->lds_unit.read_buffer = list_create();
	self->lds_unit.mem_buffer = list_create();
	self->lds_unit.write_buffer = list_create();
	self->lds_unit.compute_unit = self;

	for (i = 0; i < self->num_wavefront_pools; i++) 
	{
		/* Allocate and initialize instruction buffers */
		self->wavefront_pools[i] = si_wavefront_pool_create();
		self->wavefront_pools[i]->id = i;
		self->wavefront_pools[i]->compute_unit = self;
		self->fetch_buffers[i] = list_create();

		/* Allocate SIMD structures */
		self->simd_units[i] = xcalloc(1, 
			sizeof(struct si_simd_t));
		self->simd_units[i]->id_in_compute_unit = i;
		self->simd_units[i]->compute_unit = self;
		self->simd_units[i]->wavefront_pool = 
			self->wavefront_pools[i];
		self->simd_units[i]->issue_buffer = list_create();
		self->simd_units[i]->decode_buffer = list_create();
		self->simd_units[i]->exec_buffer = list_create();
		self->simd_units[i]->subwavefront_pool =
			xcalloc(1, sizeof(struct si_subwavefront_pool_t));

		self->simd_units[i]->compute_unit = self;
		self->simd_units[i]->wkg_util = xcalloc(1, 
			sizeof(struct si_util_t));
		self->simd_units[i]->wvf_util = xcalloc(1, 
			sizeof(struct si_util_t));
		self->simd_units[i]->rdy_util = xcalloc(1, 
			sizeof(struct si_util_t));
		self->simd_units[i]->occ_util = xcalloc(1, 
			sizeof(struct si_util_t));
		self->simd_units[i]->wki_util = xcalloc(1, 
			sizeof(struct si_util_t));
		self->simd_units[i]->act_util = xcalloc(1, 
			sizeof(struct si_util_t));
		self->simd_units[i]->tot_util = xcalloc(1, 
			sizeof(struct si_util_t));
	}

	self->work_groups = 
		xcalloc(si_gpu_max_work_groups_per_wavefront_pool * 
		si_gpu_num_wavefront_pools, sizeof(void *));
}


void SIComputeUnitDestroy(SIComputeUnit *self)
{
	int i;

	/* Scalar Unit */
	si_uop_list_free(self->scalar_unit.issue_buffer);
	si_uop_list_free(self->scalar_unit.decode_buffer);
	si_uop_list_free(self->scalar_unit.read_buffer);
	si_uop_list_free(self->scalar_unit.exec_buffer);
	si_uop_list_free(self->scalar_unit.write_buffer);
	si_uop_list_free(self->scalar_unit.inflight_buffer);
	list_free(self->scalar_unit.issue_buffer);
	list_free(self->scalar_unit.decode_buffer);
	list_free(self->scalar_unit.read_buffer);
	list_free(self->scalar_unit.exec_buffer);
	list_free(self->scalar_unit.write_buffer);
	list_free(self->scalar_unit.inflight_buffer);

	/* Branch Unit */
	si_uop_list_free(self->branch_unit.issue_buffer);
	si_uop_list_free(self->branch_unit.decode_buffer);
	si_uop_list_free(self->branch_unit.read_buffer);
	si_uop_list_free(self->branch_unit.exec_buffer);
	si_uop_list_free(self->branch_unit.write_buffer);
	list_free(self->branch_unit.issue_buffer);
	list_free(self->branch_unit.decode_buffer);
	list_free(self->branch_unit.read_buffer);
	list_free(self->branch_unit.exec_buffer);
	list_free(self->branch_unit.write_buffer);

	/* Vector Memory */
	si_uop_list_free(self->vector_mem_unit.issue_buffer);
	si_uop_list_free(self->vector_mem_unit.decode_buffer);
	si_uop_list_free(self->vector_mem_unit.read_buffer);
	si_uop_list_free(self->vector_mem_unit.mem_buffer);
	si_uop_list_free(self->vector_mem_unit.write_buffer);
	list_free(self->vector_mem_unit.issue_buffer);
	list_free(self->vector_mem_unit.decode_buffer);
	list_free(self->vector_mem_unit.read_buffer);
	list_free(self->vector_mem_unit.mem_buffer);
	list_free(self->vector_mem_unit.write_buffer);

	/* Local Data Share */
	si_uop_list_free(self->lds_unit.issue_buffer);
	si_uop_list_free(self->lds_unit.decode_buffer);
	si_uop_list_free(self->lds_unit.read_buffer);
	si_uop_list_free(self->lds_unit.mem_buffer);
	si_uop_list_free(self->lds_unit.write_buffer);
	list_free(self->lds_unit.issue_buffer);
	list_free(self->lds_unit.decode_buffer);
	list_free(self->lds_unit.read_buffer);
	list_free(self->lds_unit.mem_buffer);
	list_free(self->lds_unit.write_buffer);

	for (i = 0; i < self->num_wavefront_pools; i++)
	{
		/* SIMDs */
		si_uop_list_free(self->simd_units[i]->issue_buffer);
		si_uop_list_free(self->simd_units[i]->decode_buffer);
		si_uop_list_free(self->simd_units[i]->exec_buffer);
		list_free(self->simd_units[i]->issue_buffer);
		list_free(self->simd_units[i]->decode_buffer);
		list_free(self->simd_units[i]->exec_buffer);

		free(self->simd_units[i]->subwavefront_pool);
		free(self->simd_units[i]->wkg_util);
		free(self->simd_units[i]->wvf_util);
		free(self->simd_units[i]->rdy_util);
		free(self->simd_units[i]->occ_util);
		free(self->simd_units[i]->wki_util);
		free(self->simd_units[i]->act_util);
		free(self->simd_units[i]->tot_util);
		free(self->simd_units[i]);

		/* Common for compute unit */

		si_uop_list_free(self->fetch_buffers[i]);

		list_free(self->fetch_buffers[i]);

		si_wavefront_pool_free(self->wavefront_pools[i]);
	}
	free(self->simd_units);
	free(self->wavefront_pools);
	free(self->fetch_buffers);
	free(self->work_groups);  /* List of mapped work-groups */
	mod_free(self->lds_module);
}


void SIComputeUnitMapWorkGroup(SIComputeUnit *self, SIWorkGroup *work_group)
{
	SIGpu *gpu = self->gpu;
	SIWavefront *wavefront;

	int wavefront_id;
	int wfp_id;

	assert(self->work_group_count < gpu->work_groups_per_compute_unit);
	assert(!work_group->id_in_compute_unit);

	/* Find an available slot */
	while (work_group->id_in_compute_unit < gpu->work_groups_per_compute_unit
		&& self->work_groups[work_group->id_in_compute_unit])
	{
		work_group->id_in_compute_unit++;
	}
	assert(work_group->id_in_compute_unit < gpu->work_groups_per_compute_unit);
	self->work_groups[work_group->id_in_compute_unit] = work_group;
	self->work_group_count++;

	/* If compute unit is not full, add it back to the available list */
	assert(self->work_group_count <= gpu->work_groups_per_compute_unit);
	if (self->work_group_count < gpu->work_groups_per_compute_unit)
	{
		list_enqueue(gpu->available_compute_units, self);
	}

	/* Assign wavefronts identifiers in compute unit */
	SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
	{
		wavefront = work_group->wavefronts[wavefront_id];
		wavefront->id_in_compute_unit = work_group->id_in_compute_unit *
			work_group->wavefront_count + wavefront_id;
	}

	/* Set wavefront pool for work group */
	wfp_id = work_group->id_in_compute_unit % si_gpu_num_wavefront_pools;
	work_group->wavefront_pool = self->wavefront_pools[wfp_id];

	/* Insert wavefronts into an instruction buffer */
	si_wavefront_pool_map_wavefronts(work_group->wavefront_pool,
		work_group);

	si_trace("si.map_wg cu=%d wg=%d wi_first=%d wi_count=%d wf_first=%d "
		"wf_count=%d\n", self->id, work_group->id,
		work_group->work_items[0]->id, work_group->work_item_count,
		work_group->wavefronts[0]->id, work_group->wavefront_count);

	/* Stats */
	self->mapped_work_groups++;
	if (si_spatial_report_active)
		SIComputeUnitReportMapWorkGroup(self);
}


void SIComputeUnitUnmapWorkGroup(SIComputeUnit *self, SIWorkGroup *work_group)
{
	SIGpu *gpu = self->gpu;
	SIEmu *emu = gpu->emu;
	OpenclDriver *driver = emu->opencl_driver;

	long work_group_id;

	/* Add work group register access statistics to compute unit */
	self->sreg_read_count += work_group->sreg_read_count;
	self->sreg_write_count += work_group->sreg_write_count;
	self->vreg_read_count += work_group->vreg_read_count;
	self->vreg_write_count += work_group->vreg_write_count;

	/* Reset mapped work-group */
	assert(self->work_group_count > 0);
	assert(self->work_groups[work_group->id_in_compute_unit]);
	self->work_groups[work_group->id_in_compute_unit] = NULL;
	self->work_group_count--;

	/* Unmap wavefronts from instruction buffer */
	si_wavefront_pool_unmap_wavefronts(work_group->wavefront_pool,
		work_group);

	/* Remove the work group from the GPU running queue */
	assert(list_index_of(gpu->running_work_groups, work_group) >= 0);
	list_remove(gpu->running_work_groups, work_group);

	/* Move the work group from the ndrange running queue to 
	 * completed queue */
	work_group_id = work_group->id; 
	work_group_id = (long )list_remove(
		work_group->ndrange->running_work_groups, 
		(void *) work_group_id);
	/* Can't check for null, since 0 is a valid value */
	assert(!work_group->ndrange->running_work_groups->error_code);
	list_enqueue(work_group->ndrange->completed_work_groups, 
		(void *) work_group_id);

	/* Let the driver know if the GPU needs more work */
	if (driver && !list_count(work_group->ndrange->running_work_groups) &&
		!list_count(work_group->ndrange->waiting_work_groups))
	{
		OpenclDriverNDRangeComplete(driver, work_group->ndrange);
	}

	/* If compute unit is not already in the available list, place
	 * it there */
	assert(self->work_group_count < gpu->work_groups_per_compute_unit);
	if (list_index_of(gpu->available_compute_units, self) < 0)
	{
		list_enqueue(gpu->available_compute_units, self);
	}

	/* Trace */
	si_trace("si.unmap_wg cu=%d wg=%d\n", self->id,
		work_group->id);

	if(si_spatial_report_active)
		SIComputeUnitReportUnmapWorkGroup(self);

	delete(work_group);
}


void SIComputeUnitFetch(SIComputeUnit *self, int active_fb)
{
	SIGpu *gpu = self->gpu;
	SIWavefront *wavefront;
	SIWorkItem *work_item;

	int i, j;
	int instructions_processed = 0;
	int work_item_id;

	struct si_uop_t *uop;
	struct si_work_item_uop_t *work_item_uop;
	struct si_wavefront_pool_entry_t *wavefront_pool_entry;

	char inst_str[MAX_STRING_SIZE];
	char inst_str_trimmed[MAX_STRING_SIZE];

	assert(active_fb < self->num_wavefront_pools);

	for (i = 0; i < si_gpu_max_wavefronts_per_wavefront_pool; i++)
	{
		wavefront = self->wavefront_pools[active_fb]->
			entries[i]->wavefront;

		/* No wavefront */
		if (!wavefront) 
			continue;

		/* Sanity check wavefront */
		assert(wavefront->wavefront_pool_entry);
		assert(wavefront->wavefront_pool_entry ==
			self->wavefront_pools[active_fb]->entries[i]);

		/* Regardless of how many instructions have been fetched 
		 * already, this should always be checked */
		if (wavefront->wavefront_pool_entry->ready_next_cycle)
		{
			/* Allow instruction to be fetched next cycle */
			wavefront->wavefront_pool_entry->ready = 1;
			wavefront->wavefront_pool_entry->ready_next_cycle = 0;
			continue;
		}

		/* Only fetch a fixed number of instructions per cycle */
		if (instructions_processed == si_gpu_fe_fetch_width)
			continue;

		/* Wavefront isn't ready (previous instruction is still 
		 * in flight) */
		if (!wavefront->wavefront_pool_entry->ready)
			continue;

		/* If the wavefront finishes, there still may be outstanding 
		 * memory operations, so if the entry is marked finished 
		 * the wavefront must also be finished, but not vice-versa */
		if (wavefront->wavefront_pool_entry->wavefront_finished)
		{
			assert(wavefront->finished);
			continue;
		}

		/* Wavefront is finished but other wavefronts from workgroup 
		 * remain.  There may still be outstanding memory operations, 
		 * but no more instructions should be fetched. */
		if (wavefront->finished)
			continue;

		/* Wavefront is ready but waiting on outstanding 
		 * memory instructions */
		if (wavefront->wavefront_pool_entry->wait_for_mem)
		{
			if (!wavefront->wavefront_pool_entry->lgkm_cnt &&
				!wavefront->wavefront_pool_entry->exp_cnt &&
				!wavefront->wavefront_pool_entry->vm_cnt)
			{
				wavefront->wavefront_pool_entry->wait_for_mem =
					0;	
			}
			else
			{
				/* TODO Show a waiting state in visualization 
				 * tool */
				/* XXX uop is already freed */
				continue;
			}
		}

		/* Wavefront is ready but waiting at barrier */
		if (wavefront->wavefront_pool_entry->wait_for_barrier)
		{
			/* TODO Show a waiting state in visualization tool */
			/* XXX uop is already freed */
			continue;
		}

		/* Stall if fetch buffer full */
		assert(list_count(self->fetch_buffers[active_fb]) <= 
					si_gpu_fe_fetch_buffer_size);
		if (list_count(self->fetch_buffers[active_fb]) == 
					si_gpu_fe_fetch_buffer_size)
		{
			continue;
		}

		/* Emulate instruction */
		SIWavefrontExecute(wavefront);

		wavefront_pool_entry = wavefront->wavefront_pool_entry;
		wavefront_pool_entry->ready = 0;

		/* Create uop */
		uop = si_uop_create();
		uop->wavefront = wavefront;
		uop->work_group = wavefront->work_group;
		uop->compute_unit = self;
		uop->id_in_compute_unit = self->uop_id_counter++;
		uop->id_in_wavefront = wavefront->uop_id_counter++;
		uop->wavefront_pool_id = active_fb;
		uop->vector_mem_read = wavefront->vector_mem_read;
		uop->vector_mem_write = wavefront->vector_mem_write;
		uop->vector_mem_atomic = wavefront->vector_mem_atomic;
		uop->scalar_mem_read = wavefront->scalar_mem_read;
		uop->lds_read = wavefront->lds_read;
		uop->lds_write = wavefront->lds_write;
		uop->wavefront_pool_entry = wavefront->wavefront_pool_entry;
		uop->wavefront_last_inst = wavefront->finished;
		uop->mem_wait_inst = wavefront->mem_wait;
		uop->barrier_wait_inst = wavefront->barrier_inst;
		uop->inst = wavefront->inst;
		uop->cycle_created = asTiming(gpu)->cycle;
		uop->glc = wavefront->vector_mem_glc;
		assert(wavefront->work_group && uop->work_group);
		
		/* Trace */
		if (si_tracing())
		{
			SIInstWrapDumpBuf(wavefront->inst, inst_str, sizeof inst_str);
			str_single_spaces(inst_str_trimmed, 
				sizeof inst_str_trimmed, 
				inst_str);
			si_trace("si.new_inst id=%lld cu=%d ib=%d wg=%d "
				"wf=%d uop_id=%lld stg=\"f\" asm=\"%s\"\n", 
				uop->id_in_compute_unit, self->id, 
				uop->wavefront_pool_id, uop->work_group->id, 
				wavefront->id, uop->id_in_wavefront, 
				inst_str_trimmed);
		}
		
		/* Update last memory accesses */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
		{
			work_item = wavefront->work_items[work_item_id];
			work_item_uop = 
				&uop->work_item_uop[work_item->id_in_wavefront];

			/* Global memory */
			work_item_uop->global_mem_access_addr = 
				work_item->global_mem_access_addr;
			work_item_uop->global_mem_access_size = 
				work_item->global_mem_access_size;

			/* LDS */
			work_item_uop->lds_access_count = 
				work_item->lds_access_count;
			for (j = 0; j < work_item->lds_access_count; j++)
			{
				work_item_uop->lds_access_kind[j] = 
					work_item->lds_access_type[j];
				work_item_uop->lds_access_addr[j] = 
					work_item->lds_access_addr[j];
				work_item_uop->lds_access_size[j] = 
					work_item->lds_access_size[j];
			}
		}

		/* Access instruction cache. Record the time when the 
		 * instruction will have been fetched, as per the latency 
		 * of the instruction memory. */
		uop->fetch_ready = asTiming(gpu)->cycle + si_gpu_fe_fetch_latency;

		/* Insert into fetch buffer */
		list_enqueue(self->fetch_buffers[active_fb], uop);

		instructions_processed++;
		self->inst_count++;
	}
}

void SIComputeUnitIssueOldestInst(SIComputeUnit *self, int active_fb)
{
	SIGpu *gpu = self->gpu;

	struct si_uop_t *uop;
	struct si_uop_t *oldest_uop;

	int list_entries;
	int i;
	int issued_insts;

	/* Branch unit */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate branch instructions */
			if (SIInstWrapGetFormat(uop->inst) != SIInstFormatSOPP) 
			{
				continue;
			}
			if (SIInstWrapGetBytes(uop->inst)->sopp.op <= 1 || 
				SIInstWrapGetBytes(uop->inst)->sopp.op >= 10)
			{
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->fetch_ready < oldest_uop->fetch_ready)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest branch instruction */
		if (oldest_uop &&
			list_count(self->branch_unit.issue_buffer) < 
				si_gpu_branch_unit_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], 
				oldest_uop);
			list_enqueue(self->branch_unit.
				issue_buffer, oldest_uop);

			/* Trace */
			si_trace("si.inst id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			self->branch_inst_count++;
		}
	}

	/* Scalar unit */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate scalar instructions */
			if (SIInstWrapGetFormat(uop->inst) != SIInstFormatSOPP && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatSOP1 && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatSOP2 && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatSOPC && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatSOPK && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatSMRD)
			{	
				continue;
			}

			if (SIInstWrapGetFormat(uop->inst) == SIInstFormatSOPP && 
			    SIInstWrapGetBytes(uop->inst)->sopp.op > 1 && 
				SIInstWrapGetBytes(uop->inst)->sopp.op < 10)
			{
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->fetch_ready < oldest_uop->fetch_ready)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest scalar instruction */
		if (oldest_uop &&
			list_count(self->scalar_unit.issue_buffer) < 
				si_gpu_scalar_unit_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], oldest_uop);
			list_enqueue(self->scalar_unit.issue_buffer, 
				oldest_uop);

			/* Trace */
			si_trace("si.inst id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			if (SIInstWrapGetFormat(oldest_uop->inst) == SIInstFormatSMRD)
			{
				self->scalar_mem_inst_count++;
				oldest_uop->wavefront_pool_entry->lgkm_cnt++;
			}
			else
			{
				/* Scalar ALU instructions have to
				 * complete before the next 
				 * instruction can be fetched */
				self->scalar_alu_inst_count++;
			}
		}
	}

	/* SIMD unit */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate SIMD instructions */
			if (SIInstWrapGetFormat(uop->inst) != SIInstFormatVOP2 && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatVOP1 && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatVOPC && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatVOP3a && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatVOP3b)
			{	
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->fetch_ready < oldest_uop->fetch_ready)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest SIMD instruction */
		if (oldest_uop &&
			list_count(self->simd_units[active_fb]->issue_buffer) < 
				si_gpu_simd_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], oldest_uop);
			list_enqueue(self->simd_units[active_fb]->issue_buffer, 
				oldest_uop);

			/* Trace */
			si_trace("si.inst id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			self->simd_inst_count++;
		}
	}

	/* Vector memory */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate memory instructions */
			if (SIInstWrapGetFormat(uop->inst) != SIInstFormatMTBUF &&
				SIInstWrapGetFormat(uop->inst) != SIInstFormatMUBUF)
			{	
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->fetch_ready < oldest_uop->fetch_ready)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest memory instruction */
		if (oldest_uop &&
			list_count(self->vector_mem_unit.issue_buffer) < 
				si_gpu_vector_mem_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], oldest_uop);
			list_enqueue(self->vector_mem_unit.issue_buffer, 
				oldest_uop);

			/* Trace */
			si_trace("si.inst id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			self->vector_mem_inst_count++;
			oldest_uop->wavefront_pool_entry->lgkm_cnt++;
		}
	}

	/* LDS */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate LDS instructions */
			if (SIInstWrapGetFormat(uop->inst) != SIInstFormatDS)
			{	
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->fetch_ready < oldest_uop->fetch_ready)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest LDS instruction */
		if (oldest_uop &&
			list_count(self->lds_unit.issue_buffer) < 
				si_gpu_lds_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], oldest_uop);
			list_enqueue(self->lds_unit.issue_buffer, oldest_uop);

			/* Trace */
			si_trace("si.inst id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			self->lds_inst_count++;
			oldest_uop->wavefront_pool_entry->lgkm_cnt++;
		}
	}

	/* Update visualization states for all instructions not issued */
	list_entries = list_count(self->fetch_buffers[active_fb]);
	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(self->fetch_buffers[active_fb], i);
		assert(uop);

		/* Skip all uops that have not yet completed the fetch */
		if (asTiming(gpu)->cycle < uop->fetch_ready)
		{
			continue;
		}

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
			uop->id_in_compute_unit, self->id, 
			uop->wavefront->id, uop->id_in_wavefront);
	}
}

void SIComputeUnitIssueOldestWF(SIComputeUnit *self, int active_fb)
{
	SIGpu *gpu = self->gpu;

	struct si_uop_t *uop;
	struct si_uop_t *oldest_uop;

	int list_entries;
	int i;
	int issued_insts;

	/* Branch unit */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate branch instructions */
			if (SIInstWrapGetFormat(uop->inst) != SIInstFormatSOPP) 
			{
				continue;
			}
			if (SIInstWrapGetBytes(uop->inst)->sopp.op <= 1 || 
				SIInstWrapGetBytes(uop->inst)->sopp.op >= 10)
			{
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->wavefront->id < oldest_uop->wavefront->id)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest branch instruction */
		if (oldest_uop &&
			list_count(self->branch_unit.issue_buffer) < 
				si_gpu_branch_unit_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], oldest_uop);
			list_enqueue(self->branch_unit.issue_buffer, 
				oldest_uop);

			/* Trace */
			si_trace("si.inst id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			self->branch_inst_count++;
		}
	}

	/* Scalar unit */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate scalar instructions */
			if (SIInstWrapGetFormat(uop->inst) != SIInstFormatSOPP && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatSOP1 && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatSOP2 && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatSOPC && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatSOPK && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatSMRD)
			{	
				continue;
			}
			if (SIInstWrapGetFormat(uop->inst) == SIInstFormatSOPP && 
			    SIInstWrapGetBytes(uop->inst)->sopp.op > 1 && 
				SIInstWrapGetBytes(uop->inst)->sopp.op < 10)
			{
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->wavefront->id < oldest_uop->wavefront->id)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest scalar instruction */
		if (oldest_uop &&
			list_count(self->scalar_unit.issue_buffer) < 
				si_gpu_scalar_unit_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], oldest_uop);
			list_enqueue(self->scalar_unit.issue_buffer, 
				oldest_uop);

			/* Trace */
			si_trace("si.inst id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			if (SIInstWrapGetFormat(oldest_uop->inst) == SIInstFormatSMRD)
			{
				self->scalar_mem_inst_count++;
				oldest_uop->wavefront_pool_entry->lgkm_cnt++;
			}
			else
			{
				/* Scalar ALU instructions have to
				 * complete before the next 
				 * instruction can be fetched */
				self->scalar_alu_inst_count++;
			}
		}
	}

	/* SIMD unit */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate SIMD instructions */
			if (SIInstWrapGetFormat(uop->inst) != SIInstFormatVOP2 && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatVOP1 && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatVOPC && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatVOP3a && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatVOP3b)
			{	
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->wavefront->id < oldest_uop->wavefront->id)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest SIMD instruction */
		if (oldest_uop &&
			list_count(self->simd_units[active_fb]->issue_buffer) < 
				si_gpu_simd_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], oldest_uop);
			list_enqueue(self->simd_units[active_fb]->issue_buffer, 
				oldest_uop);

			/* Trace */
			si_trace("si.inst id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			self->simd_inst_count++;
		}
	}

	/* Vector memory */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate memory instructions */
			if (SIInstWrapGetFormat(uop->inst) != SIInstFormatMTBUF && 
				SIInstWrapGetFormat(uop->inst) != SIInstFormatMUBUF)
			{	
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->wavefront->id < oldest_uop->wavefront->id)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest memory instruction */
		if (oldest_uop &&
			list_count(self->vector_mem_unit.issue_buffer) < 
				si_gpu_vector_mem_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], oldest_uop);
			list_enqueue(self->vector_mem_unit.issue_buffer, 
				oldest_uop);

			/* Trace */
			si_trace("si.inst id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			self->vector_mem_inst_count++;
			oldest_uop->wavefront_pool_entry->lgkm_cnt++;
		}
	}

	/* LDS */
	for (issued_insts = 0; 
		issued_insts < si_gpu_fe_max_inst_issued_per_type;
		issued_insts++)
	{
		list_entries = list_count(self->fetch_buffers[active_fb]);
		oldest_uop = NULL;
		for (i = 0; i < list_entries; i++)
		{
			uop = list_get(self->fetch_buffers[active_fb], i);
			assert(uop);

			/* Only evaluate LDS instructions */
			if (SIInstWrapGetFormat(uop->inst) != SIInstFormatDS)
			{	
				continue;
			}

			/* Skip all uops that have not yet completed 
			 * the fetch */
			if (asTiming(gpu)->cycle < uop->fetch_ready)
			{
				continue;
			}

			/* Save the oldest uop */
			if (!oldest_uop || 
				uop->wavefront->id < oldest_uop->wavefront->id)
			{
				oldest_uop = uop;
			}
		}

		/* Issue the oldest LDS instruction */
		if (oldest_uop &&
			list_count(self->lds_unit.issue_buffer) < 
			si_gpu_lds_issue_buffer_size)
		{
			oldest_uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], oldest_uop);
			list_enqueue(self->lds_unit.issue_buffer, 
				oldest_uop);

			/* Trace */
			si_trace("si.inst id=%lld cu=%d wf=%d "
				"uop_id=%lld stg=\"i\"\n", 
				oldest_uop->id_in_compute_unit, 
				self->id, 
				oldest_uop->wavefront->id, 
				oldest_uop->id_in_wavefront);

			self->lds_inst_count++;
			oldest_uop->wavefront_pool_entry->lgkm_cnt++;
		}
	}

	/* Update visualization states for all instructions not issued */
	list_entries = list_count(self->fetch_buffers[active_fb]);
	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(self->fetch_buffers[active_fb], i);
		assert(uop);

		/* Skip all uops that have not yet completed the fetch */
		if (asTiming(gpu)->cycle < uop->fetch_ready)
		{
			continue;
		}

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
			uop->id_in_compute_unit, self->id, 
			uop->wavefront->id, uop->id_in_wavefront);
	}
}

void SIComputeUnitUpdateFetchVisualization(SIComputeUnit *self,
		int non_active_fb)
{
	SIGpu *gpu = self->gpu;

	struct si_uop_t *uop;

	int list_entries;
	int i;

	/* Update visualization states for all instructions not issued */
	list_entries = list_count(self->fetch_buffers[non_active_fb]);
	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(self->fetch_buffers[non_active_fb], i);
		assert(uop);

		/* Skip all uops that have not yet completed the fetch */
		if (asTiming(gpu)->cycle < uop->fetch_ready)
		{
			continue;
		}

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"s\"\n", 
			uop->id_in_compute_unit, self->id, 
			uop->wavefront->id, uop->id_in_wavefront);
	}
}

/* Decode the instruction type */
void SIComputeUnitIssueFirst(SIComputeUnit *self, int active_fb)
{
	SIGpu *gpu = self->gpu;

	struct si_uop_t *uop;

	int list_index = 0;
	int list_entries;
	int i;

	int total_insts_issued = 0;
	int scalar_insts_issued = 0;
	int branch_insts_issued = 0;
	int lds_insts_issued = 0;
	int simd_insts_issued = 0;
	int mem_insts_issued = 0;

	list_entries = list_count(self->fetch_buffers[active_fb]);
	
	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(self->fetch_buffers[active_fb], 
			list_index);
		assert(uop);

		/* Skip all uops that have not yet completed the fetch */
		if (asTiming(gpu)->cycle < uop->fetch_ready)
		{
			list_index++;
			continue;
		}

		/* Only issue a fixed number of instructions per cycle */
		if (total_insts_issued == si_gpu_fe_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit, 
				self->id, uop->wavefront->id,
				uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Determine instruction type.  This simply decodes the 
		 * instruction type, so that it can be issued to the proper 
		 * hardware unit.  It is not the full decode stage */
		switch (SIInstWrapGetFormat(uop->inst))
		{

		/* Scalar ALU or Branch */
		case SIInstFormatSOPP:
		{
			/* Branch Unit */
			if (SIInstWrapGetBytes(uop->inst)->sopp.op > 1 &&
				SIInstWrapGetBytes(uop->inst)->sopp.op < 10)
			{
				/* Stall if max branch instructions 
				 * already issued */
				assert(branch_insts_issued <= 
					si_gpu_fe_max_inst_issued_per_type);
				if (branch_insts_issued == 
					si_gpu_fe_max_inst_issued_per_type)
				{
					si_trace("si.inst id=%lld cu=%d wf=%d "
						"uop_id=%lld stg=\"s\"\n", 
						uop->id_in_compute_unit, 
						self->id, 
						uop->wavefront->id, 
						uop->id_in_wavefront);
					list_index++;
					continue;
				}

				/* Stall if branch unit issue buffer is full */
				if (list_count(self->
					branch_unit.issue_buffer) ==
					si_gpu_branch_unit_issue_buffer_size)
				{
					si_trace("si.inst id=%lld cu=%d wf=%d "
						"uop_id=%lld stg=\"s\"\n", 
						uop->id_in_compute_unit, 
						self->id, 
						uop->wavefront->id, 
						uop->id_in_wavefront);
					list_index++;
					continue;
				}

				uop->issue_ready = asTiming(gpu)->cycle +
					si_gpu_fe_issue_latency;
				list_remove(self->fetch_buffers[active_fb], 
					uop);
				list_enqueue(self->branch_unit.issue_buffer, 
					uop);

				branch_insts_issued++;
				self->branch_inst_count++;
			}
			/* Scalar Unit */
			else
			{
				/* Stall if max scalar instructions 
				 * already issued */
				assert(scalar_insts_issued <= 
					si_gpu_fe_max_inst_issued_per_type);
				if (scalar_insts_issued == 
					si_gpu_fe_max_inst_issued_per_type)
				{
					si_trace("si.inst id=%lld cu=%d wf=%d "
						"uop_id=%lld stg=\"s\"\n", 
						uop->id_in_compute_unit, 
						self->id, 
						uop->wavefront->id, 
						uop->id_in_wavefront);
					list_index++;
					continue;
				}

				/* Stall if scalar unit issue buffer is full */
				if (list_count(self->
					scalar_unit.issue_buffer) ==
					si_gpu_scalar_unit_issue_buffer_size)
				{
					si_trace("si.inst id=%lld cu=%d wf=%d "
						"uop_id=%lld stg=\"s\"\n", 
						uop->id_in_compute_unit, 
						self->id, 
						uop->wavefront->id, 
						uop->id_in_wavefront);
					list_index++;
					continue;
				}

				uop->issue_ready = asTiming(gpu)->cycle +
					si_gpu_fe_issue_latency;
				list_remove(self->fetch_buffers[active_fb], 
					uop);
				list_enqueue(self->scalar_unit.issue_buffer, 
					uop);

				scalar_insts_issued++;
				self->scalar_alu_inst_count++;
			}

			break;
		}
		case SIInstFormatSOP1:
		case SIInstFormatSOP2:
		case SIInstFormatSOPC:
		case SIInstFormatSOPK:
		{
			/* Stall if max scalar instructions already issued */
			assert(scalar_insts_issued <= 
				si_gpu_fe_max_inst_issued_per_type);
			if (scalar_insts_issued == 
				si_gpu_fe_max_inst_issued_per_type)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n",
					uop->id_in_compute_unit, 
					self->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			/* Stall if scalar unit issue buffer is full */
			if (list_count(self->scalar_unit.issue_buffer) 
				== si_gpu_scalar_unit_issue_buffer_size)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n",
					uop->id_in_compute_unit, 
					self->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], uop);
			list_enqueue(self->scalar_unit.issue_buffer, uop);

			scalar_insts_issued++;
			self->scalar_alu_inst_count++;

			break;
		}

		/* Scalar memory */
		case SIInstFormatSMRD:
		{
			/* Stall if max scalar instructions already issued */
			assert(scalar_insts_issued <= 
				si_gpu_fe_max_inst_issued_per_type);
			if (scalar_insts_issued == 
				si_gpu_fe_max_inst_issued_per_type)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n",
					uop->id_in_compute_unit, 
					self->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			/* Stall if scalar unit issue buffer is full */
			if (list_count(self->scalar_unit.issue_buffer) 
				== si_gpu_scalar_unit_issue_buffer_size)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n",
					uop->id_in_compute_unit, 
					self->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], uop);
			list_enqueue(self->scalar_unit.issue_buffer, uop);

			scalar_insts_issued++;
			self->scalar_mem_inst_count++;
			uop->wavefront_pool_entry->lgkm_cnt++;

			break;
		}

		/* Vector ALU */
		case SIInstFormatVOP2:
		case SIInstFormatVOP1:
		case SIInstFormatVOPC:
		case SIInstFormatVOP3a:
		case SIInstFormatVOP3b:
		{
			/* Stall if max SIMD instructions already issued */
			assert(simd_insts_issued <= 
				si_gpu_fe_max_inst_issued_per_type);
			if (simd_insts_issued == 
				si_gpu_fe_max_inst_issued_per_type)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n",
					uop->id_in_compute_unit, 
					self->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			/* Stall if SIMD issue buffer is full */
			if (list_count(self->simd_units[active_fb]->
				issue_buffer) == si_gpu_simd_issue_buffer_size)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n",
					uop->id_in_compute_unit, 
					self->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], uop);
			list_enqueue(self->simd_units[active_fb]->issue_buffer, 
				uop);

			simd_insts_issued++;
			self->simd_inst_count++;

			break;
		}

		/* Vector memory */
		case SIInstFormatMTBUF:
		case SIInstFormatMUBUF:
		{
			/* Stall if max vector memory instructions already 
			 * issued */
			assert(mem_insts_issued <= 
				si_gpu_fe_max_inst_issued_per_type);
			if (mem_insts_issued == 
				si_gpu_fe_max_inst_issued_per_type)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n",
					uop->id_in_compute_unit, 
					self->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			/* Stall if vector memory issue buffer is full */
			if (list_count(
				self->vector_mem_unit.issue_buffer) ==
				si_gpu_vector_mem_issue_buffer_size)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n",
					uop->id_in_compute_unit, 
					self->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], uop);
			list_enqueue(self->vector_mem_unit.issue_buffer, uop);

			mem_insts_issued++;
			self->vector_mem_inst_count++;
			uop->wavefront_pool_entry->lgkm_cnt++;

			break;
		}

		/* Local Data Share */ 
		case SIInstFormatDS:
		{
			/* Stall if max LDS instructions already issued */
			assert(lds_insts_issued <= 
				si_gpu_fe_max_inst_issued_per_type);
			if (lds_insts_issued == 
				si_gpu_fe_max_inst_issued_per_type)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n",
					uop->id_in_compute_unit, 
					self->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			/* Stall if the LDS issue buffer is full */
			if (list_count(self->lds_unit.issue_buffer) ==
				si_gpu_lds_issue_buffer_size)
			{
				si_trace("si.inst id=%lld cu=%d wf=%d "
					"uop_id=%lld stg=\"s\"\n", 
					uop->id_in_compute_unit, 
					self->id, 
					uop->wavefront->id, 
					uop->id_in_wavefront);
				list_index++;
				continue;
			}

			uop->issue_ready = asTiming(gpu)->cycle +
				si_gpu_fe_issue_latency;
			list_remove(self->fetch_buffers[active_fb], uop);
			list_enqueue(self->lds_unit.issue_buffer, uop);

			lds_insts_issued++;
			self->lds_inst_count++;
			uop->wavefront_pool_entry->lgkm_cnt++;

			break;
		}

		default:
			fatal("%s: instruction type not implemented",
				__FUNCTION__);

		}

		total_insts_issued++;

		/* Trace */
		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld stg=\"i\"\n", 
			uop->id_in_compute_unit, self->id, 
			uop->wavefront->id, uop->id_in_wavefront);
	}
}

/* Advance one cycle in the compute unit by running every stage from 
 * last to first */
void SIComputeUnitRun(SIComputeUnit *self)
{
	SIGpu *gpu = self->gpu;

	int i;
	int num_simd_units;
	int active_issue_buffer;  

	/* Return if no work groups are mapped to this compute unit */
	if (!self->work_group_count)
		return;

	/* Issue buffer chosen to issue this cycle */
	active_issue_buffer = asTiming(gpu)->cycle %
		self->num_wavefront_pools;

	assert(active_issue_buffer >= 0 && 
		active_issue_buffer < self->num_wavefront_pools);

	/* SIMDs */
	num_simd_units = self->num_wavefront_pools;
	for (i = 0; i < num_simd_units; i++)
		si_simd_run(self->simd_units[i]);

	/* Vector memory */
	si_vector_mem_run(&self->vector_mem_unit);

	/* LDS */
	si_lds_run(&self->lds_unit);

	/* Scalar unit */
	si_scalar_unit_run(&self->scalar_unit);

	/* Branch unit */
	si_branch_unit_run(&self->branch_unit);

	/* Issue from the active issue buffer */
	//SIComputeUnitIssueFirst(compute_unit, active_issue_buffer);
	//SIComputeUnitIssueOldestInst(self, active_issue_buffer);
	SIComputeUnitIssueOldestWF(self, active_issue_buffer);

	/* Update visualization in non-active issue buffers */
	for (i = 0; i < num_simd_units; i++)
	{
		if (i != active_issue_buffer)
		{
			SIComputeUnitUpdateFetchVisualization(self, i);
		}
	}

	/* Fetch */
	for (i = 0; i < num_simd_units; i++)
		SIComputeUnitFetch(self, i);

	/* Stats */
	self->cycle++;

	if(si_spatial_report_active)
		SIComputeUnitReportUpdate(self);
}

