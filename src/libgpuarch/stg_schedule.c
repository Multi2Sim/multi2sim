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
	struct gpu_device_t *device = compute_unit->device;
	struct gpu_ndrange_t *ndrange = device->ndrange;
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


static void gpu_compute_unit_emulate(struct gpu_compute_unit_t *compute_unit)
{
	int i;
	struct gpu_device_t *device = compute_unit->device;
	struct gpu_ndrange_t *ndrange = device->ndrange;
	struct gpu_work_group_t *work_group = ndrange->work_groups[INIT_SCHEDULE.work_group_id];
	struct gpu_wavefront_t *wavefront = ndrange->wavefronts[INIT_SCHEDULE.wavefront_id];

	assert(DOUBLE_LINKED_LIST_MEMBER(work_group, running, wavefront));
	gpu_pipeline_debug("emul");
	switch (wavefront->clause_kind) {

	case GPU_CLAUSE_CF:
		gpu_wavefront_execute(wavefront);
		gpu_pipeline_debug(" cat=CF");
		gpu_pipeline_debug(" inst=\"%s\"\n",
			wavefront->cf_inst.info->name);
		break;
	
	case GPU_CLAUSE_ALU:
		gpu_wavefront_execute(wavefront);
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
		gpu_pipeline_debug(" cat=TC");
		gpu_pipeline_debug(" inst=\"%s\"\n",
			wavefront->tc_inst.info->name);
		break;
	
	default:
		abort();

	}
	gpu_pipeline_debug("\n");
}


void gpu_compute_unit_schedule(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_device_t *device = compute_unit->device;
	struct gpu_ndrange_t *ndrange = device->ndrange;

	struct gpu_work_group_t *work_group;
	struct gpu_wavefront_t *wavefront;
	
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
	gpu_pipeline_debug("stg name=\"schedule\", work_group=\"%d\", wavefront=\"%d\", subwavefront=\"%d\"\n",
		INIT_SCHEDULE.work_group_id, INIT_SCHEDULE.wavefront_id, INIT_SCHEDULE.subwavefront_id);

	/* Emulate instruction if it's the first subwavefront */
	if (!INIT_SCHEDULE.subwavefront_id)
		gpu_compute_unit_emulate(compute_unit);

	/* Schedule next subwavefront/wavefront.
	 * If work-group finished, do not schedule anymore. */
	result = gpu_compute_unit_schedule_next_subwavefront(compute_unit);
	if (!result) {
		gpu_pipeline_debug("cu compute_unit=\"%d\", work_group=\"%d\", action=\"finish\"\n",
			compute_unit->id, work_group->id);
		INIT_SCHEDULE.do_schedule = 0;

		/* Set compute unit as idle.
		 * FIXME: this should be done at the last stage of the pipeline */
		DOUBLE_LINKED_LIST_REMOVE(device, busy, compute_unit);
		DOUBLE_LINKED_LIST_INSERT_TAIL(device, idle, compute_unit);
	}
}

