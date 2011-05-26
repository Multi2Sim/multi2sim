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

#include <gpukernel.h>
#include <gpuarch.h>
#include <debug.h>



/*
 * Public Variables
 */

int gpu_pipeline_debug_category;




/*
 * Schedule Stage
 */

static void gpu_compute_unit_schedule_next_subwavefront(struct gpu_compute_unit_t *compute_unit)
{
	struct gpu_device_t *device = compute_unit->device;
	struct gpu_ndrange_t *ndrange = device->ndrange;
	struct gpu_work_group_t *work_group;
	struct gpu_wavefront_t *wavefront;

	/* Go to next subwavefront */
	INIT_SCHEDULE.subwavefront_id++;
	if (INIT_SCHEDULE.subwavefront_id < gpu_compute_unit_time_slots)
		return;
	
	/* Scheduling of current wavefront finished */
	work_group = ndrange->work_groups[INIT_SCHEDULE.work_group_id];
	assert(work_group->running_list_head);
	INIT_SCHEDULE.subwavefront_id = 0;

	/* Schedule new wavefront */
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

	/* Schedule next subwavefront/wavefront */
	gpu_compute_unit_schedule_next_subwavefront(compute_unit);
}


void gpu_compute_unit_fetch(struct gpu_compute_unit_t *compute_unit)
{
	/* Check if fetch stage is active */
	if (!SCHEDULE_FETCH.do_fetch)
		return;

	/* By default, do not fetch next cycle */
	SCHEDULE_FETCH.do_fetch = 0;
}


void gpu_compute_unit_decode(struct gpu_compute_unit_t *compute_unit)
{
	/* Check if decode stage is active */
	if (!FETCH_DECODE.do_decode)
		return;
	
	/* By default, do not decode next cycle */
	FETCH_DECODE.do_decode = 0;
}


void gpu_compute_unit_read(struct gpu_compute_unit_t *compute_unit)
{
	/* Check if read stage is active */
	if (!DECODE_READ.do_read)
		return;
	
	/* By default, do not read next cycle */
	DECODE_READ.do_read = 0;
}


void gpu_compute_unit_execute(struct gpu_compute_unit_t *compute_unit)
{
	/* Check if execute stage is active */
	if (!READ_EXECUTE.do_execute)
		return;
	
	/* By default, do not execute next cycle */
	READ_EXECUTE.do_execute = 0;
}


void gpu_compute_unit_write(struct gpu_compute_unit_t *compute_unit)
{
	/* Check if write stage is active */
	if (!EXECUTE_WRITE.do_write)
		return;
	
	/* By default, do not write next cycle */
	EXECUTE_WRITE.do_write = 0;
}


void gpu_compute_unit_next_cycle(struct gpu_compute_unit_t *compute_unit)
{
	gpu_compute_unit_write(compute_unit);
	gpu_compute_unit_execute(compute_unit);
	gpu_compute_unit_read(compute_unit);
	gpu_compute_unit_decode(compute_unit);
	gpu_compute_unit_fetch(compute_unit);
	gpu_compute_unit_schedule(compute_unit);
}

