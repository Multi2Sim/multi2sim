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

	/* Go to next subwavefront */
	INIT_SCHEDULE.subwavefront_id++;
	if (INIT_SCHEDULE.subwavefront_id < gpu_compute_unit_time_slots)
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

	/* Set fields */
	uop->clause_kind = wavefront->clause_kind;

	/* Emulate instruction */
	assert(DOUBLE_LINKED_LIST_MEMBER(work_group, running, wavefront));
	gpu_pipeline_debug("emul uop=\"%lld\",", (long long) uop->id);
	switch (wavefront->clause_kind) {

	case GPU_CLAUSE_CF:
		gpu_wavefront_execute(wavefront);
		amd_inst_copy(&uop->inst, &wavefront->cf_inst);

		gpu_pipeline_debug(" cat=CF");
		gpu_pipeline_debug(" inst=\"%s\"\n",
			wavefront->cf_inst.info->name);
		break;
	
	case GPU_CLAUSE_ALU:
		gpu_wavefront_execute(wavefront);
		amd_alu_group_copy(&uop->alu_group, &wavefront->alu_group);

		if (debug_status(gpu_pipeline_debug_category)) {
			for (i = 0; i < wavefront->alu_group.inst_count; i++) {
				struct amd_inst_t *inst = &wavefront->alu_group.inst[i];
				gpu_pipeline_debug(" inst.%s=\"%s\"",
					map_value(&amd_alu_map, inst->alu),
					inst->info->name);
			}
		}
		break;

	case GPU_CLAUSE_TC:
		gpu_wavefront_execute(wavefront);
		amd_inst_copy(&uop->inst, &wavefront->cf_inst);

		gpu_pipeline_debug(" cat=TC");
		gpu_pipeline_debug(" inst=\"%s\"\n",
			wavefront->tc_inst.info->name);
		break;
	
	default:
		abort();

	}
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
		uop = gpu_uop_create();
		uop->work_group = work_group;
		uop->wavefront = wavefront;
		uop->subwavefront_count = (wavefront->work_item_count + gpu_num_stream_cores - 1) / gpu_num_stream_cores;
		gpu_uop_emulate(uop);
		INIT_SCHEDULE.uop = uop;
	}
	gpu_pipeline_debug("stg name=\"schedule\", uop=\"%lld\", work_group=\"%d\", wavefront=\"%d\", subwavefront=\"%d\"\n",
		(long long) uop->id, INIT_SCHEDULE.work_group_id, INIT_SCHEDULE.wavefront_id, INIT_SCHEDULE.subwavefront_id);

	/* Transfer uop to next stage */
	SCHEDULE_FETCH.do_fetch = 1;
	SCHEDULE_FETCH.uop = uop;
	SCHEDULE_FETCH.subwavefront_id = INIT_SCHEDULE.subwavefront_id;

	/* Schedule next subwavefront/wavefront.
	 * If work-group finished, do not schedule anymore.
	 * A new work-group will be scheduled in the main simulation loop an set back the 'do_schedule' flag. */
	result = gpu_compute_unit_schedule_next_subwavefront(compute_unit);
	if (!result) {
		gpu_pipeline_debug("cu compute_unit=\"%d\", work_group=\"%d\", action=\"finish\"\n",
			compute_unit->id, work_group->id);
		INIT_SCHEDULE.do_schedule = 0;

		/* Set compute unit as idle.
		 * FIXME: this should be done at the last stage of the pipeline */
		DOUBLE_LINKED_LIST_REMOVE(gpu, busy, compute_unit);
		DOUBLE_LINKED_LIST_INSERT_TAIL(gpu, idle, compute_unit);
	}
}

