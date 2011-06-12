/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpuarch.h>



/* Schedule next subwavefront in the work-group, and return 1 if one was found.
 * Return 0 if the work-group finished. */
static int gpu_compute_unit_schedule_next_subwavefront(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_ndrange_t *ndrange = gpu->ndrange;
	struct gpu_work_group_t *work_group;
	struct gpu_wavefront_t *wavefront;
	int subwavefront_count;

	/* Go to next subwavefront */
	/* FIXME: limit should be number of subwavefronts */
	wavefront = ndrange->wavefronts[INIT_SCHEDULE.wavefront_id];
	subwavefront_count = (wavefront->work_item_count + gpu_num_stream_cores - 1) / gpu_num_stream_cores;
	INIT_SCHEDULE.subwavefront_id++;
	if (INIT_SCHEDULE.subwavefront_id < subwavefront_count)
		return 1;
	
	/* Scheduling of current wavefront finished */
	work_group = ndrange->work_groups[INIT_SCHEDULE.work_group_id];
	if (!gpu_work_group_get_status(work_group, gpu_work_group_running))
		return 0;

	/* Schedule new wavefront */
	assert(work_group->running_list_head);
	INIT_SCHEDULE.subwavefront_id = 0;
	if (INIT_SCHEDULE.wavefront_running_next &&
		DOUBLE_LINKED_LIST_MEMBER(work_group, running, INIT_SCHEDULE.wavefront_running_next))
	{
		assert(INIT_SCHEDULE.wavefront_running_next->id != INIT_SCHEDULE.wavefront_id);
		wavefront = INIT_SCHEDULE.wavefront_running_next;
	} else {
		wavefront = work_group->running_list_head;
	}
	INIT_SCHEDULE.wavefront_id = wavefront->id;
	INIT_SCHEDULE.wavefront_running_next = wavefront->running_next;
	return 1;
}


static void gpu_uop_emulate(struct gpu_uop_t *uop)
{
	int i;
	struct gpu_work_group_t *work_group = uop->work_group;
	struct gpu_wavefront_t *wavefront = uop->wavefront;
	struct gpu_ndrange_t *ndrange = work_group->ndrange;
	char buf[MAX_STRING_SIZE];
	int inst_num;
	
	/* Set fields */
	uop->clause_kind = wavefront->clause_kind;
	uop->subwavefront_count = (wavefront->work_item_count + gpu_num_stream_cores - 1) / gpu_num_stream_cores;

	/* Emulate instruction */
	assert(DOUBLE_LINKED_LIST_MEMBER(work_group, running, wavefront));
	gpu_pipeline_debug("uop "
		"action=\"emul\", "
		"id=%lld, ",
		(long long) uop->id);
	
	switch (wavefront->clause_kind) {

	case GPU_CLAUSE_CF:
	case GPU_CLAUSE_TC:
		
		inst_num = (wavefront->cf_buf - ndrange->kernel->cal_abi->text_buffer) / 8;
		gpu_wavefront_execute(wavefront);
		amd_inst_copy(&uop->inst, &wavefront->cf_inst);

		/* Record lobal memory access */
		if (uop->inst.info->flags & AMD_INST_FLAG_MEM) {
			
			struct gpu_work_item_uop_t *work_item_uop;
			struct gpu_work_item_t *work_item;
			int work_item_id;

			assert((uop->inst.info->flags & AMD_INST_FLAG_MEM_READ) ||
				(uop->inst.info->flags & AMD_INST_FLAG_MEM_WRITE));
			uop->global_mem_access = (uop->inst.info->flags & AMD_INST_FLAG_MEM_READ) ? 1 : 2;
			FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id) {
				work_item = ndrange->work_items[work_item_id];
				work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
				work_item_uop->global_mem_access_addr = work_item->global_mem_access_addr;
				work_item_uop->global_mem_access_size = work_item->global_mem_access_size;
			}
		}

		/* Debug */
		if (debug_status(gpu_pipeline_debug_category)) {
			amd_inst_dump_buf(&wavefront->cf_inst, inst_num, 0, buf, MAX_STRING_SIZE);
			gpu_pipeline_debug(
				"cat=\"%s\", "
				"inst=\"%s\", ",
				map_value(&fmt_inst_category_map, wavefront->cf_inst.info->category),
				buf);
		}
		break;
	
	case GPU_CLAUSE_ALU:
		gpu_wavefront_execute(wavefront);
		amd_alu_group_copy(&uop->alu_group, &wavefront->alu_group);

		if (debug_status(gpu_pipeline_debug_category)) {
			for (i = 0; i < wavefront->alu_group.inst_count; i++) {
				struct amd_inst_t *inst = &wavefront->alu_group.inst[i];
				amd_inst_dump_buf(inst, -1, 0, buf, MAX_STRING_SIZE);
				gpu_pipeline_debug(
					"inst.%s=\"%s\", ",
					map_value(&amd_alu_map, inst->alu),
					buf);
			}
		}
		break;

	default:
		abort();

	}

	/* Set more fields */
	uop->last = gpu_work_group_get_status(work_group, gpu_work_group_finished);

	/* Debug */
	gpu_pipeline_debug(
		"subwavefront_count=\"%d\", "
		"last=\"%d\"\n",
		uop->subwavefront_count,
		uop->last);
}


void gpu_compute_unit_schedule(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_ndrange_t *ndrange = gpu->ndrange;

	struct gpu_work_group_t *work_group;
	struct gpu_wavefront_t *wavefront;

	struct gpu_uop_t *uop;
	
	int result;

	/* Check if schedule stage is active */
	if (!INIT_SCHEDULE.do_schedule)
		return;
	
	/* By default, do not schedule next cycle */
	//INIT_SCHEDULE.do_schedule = 0;
	
	/* Check boundaries of wavefront and subwavefront */
	assert(INIT_SCHEDULE.work_group_id >= 0 && INIT_SCHEDULE.work_group_id < ndrange->work_group_count);
	work_group = ndrange->work_groups[INIT_SCHEDULE.work_group_id];
	assert(INIT_SCHEDULE.wavefront_id >= work_group->wavefront_id_first
		&& INIT_SCHEDULE.wavefront_id <= work_group->wavefront_id_last);
	wavefront = ndrange->wavefronts[INIT_SCHEDULE.wavefront_id];
	assert(INIT_SCHEDULE.subwavefront_id >= 0 && INIT_SCHEDULE.subwavefront_id < gpu_compute_unit_time_slots);
	
	/* Create uop and emulate if this is the beginning of a wavefront */
	uop = INIT_SCHEDULE.uop;
	if (!INIT_SCHEDULE.subwavefront_id) {
		
		/* Create */
		uop = gpu_uop_create();
		uop->work_group = work_group;
		uop->wavefront = wavefront;
		gpu_pipeline_debug("uop "
			"action=\"create\", "
			"id=%lld, "
			"cu=%d, "
			"work_group=%d, "
			"wavefront=%d\n",
			(long long) uop->id,
			compute_unit->id,
			work_group->id,
			wavefront->id);

		/* Emulate */
		gpu_uop_emulate(uop);
		INIT_SCHEDULE.uop = uop;
	}
	
	/* Debug */
	gpu_pipeline_debug("uop "
		"action=\"update\", "
		"id=%lld, "
		"subwf=%d, "
		"cu=%d, "
		"stg=\"schedule\""
		"\n",
		(long long) uop->id,
		INIT_SCHEDULE.subwavefront_id,
		compute_unit->id);
	

	/* Transfer uop to next stage */
	SCHEDULE_FETCH.do_fetch = 1;
	SCHEDULE_FETCH.uop = uop;
	SCHEDULE_FETCH.subwavefront_id = INIT_SCHEDULE.subwavefront_id;

	/* Schedule next subwavefront/wavefront.
	 * If work-group finished, do not schedule anymore.
	 * A new work-group will be scheduled in the main simulation loop and set back the 'do_schedule' flag. */
	result = gpu_compute_unit_schedule_next_subwavefront(compute_unit);
	if (!result)
		INIT_SCHEDULE.do_schedule = 0;
}

