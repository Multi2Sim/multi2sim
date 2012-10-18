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

#include "timing.h"


struct si_inst_buffer_t *si_inst_buffer_create()
{
	struct si_inst_buffer_t *inst_buffer;
	int i;

	/* Create */
	inst_buffer = calloc(1, sizeof(struct si_inst_buffer_t));
	if (!inst_buffer)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	inst_buffer->entries = calloc(si_gpu_max_wavefronts_per_inst_buffer, 
		sizeof(struct si_inst_buffer_entry_t*));
	if (!inst_buffer->entries)
		fatal("%s: out of memory", __FUNCTION__);

	for (i = 0; i < si_gpu_max_wavefronts_per_inst_buffer; i++) 
	{
		inst_buffer->entries[i] = calloc(1, sizeof(struct si_inst_buffer_entry_t));
		if (!inst_buffer->entries[i])
			fatal("%s: out of memory", __FUNCTION__);

		inst_buffer->entries[i]->id_in_inst_buffer = i;
		inst_buffer->entries[i]->inst_buffer = inst_buffer;
	}

	inst_buffer->wavefront_count = 0;

	/* Return */
	return inst_buffer;
}

void si_inst_buffer_free(struct si_inst_buffer_t *inst_buffer)
{
	int i;

	for (i = 0; i < si_gpu_max_wavefronts_per_inst_buffer; i++) 
		free(inst_buffer->entries[i]);

	free(inst_buffer->entries);
	free(inst_buffer);
}

void si_inst_buffer_map_wavefronts(struct si_inst_buffer_t *inst_buffer, 
	struct si_work_group_t *work_group)
{
	struct si_ndrange_t *ndrange = work_group->ndrange;
	struct si_wavefront_t *wavefront;
	int wg_id_in_ib;
	int first_entry;
	int i;

	/* Determine starting ID for wavefronts in the instruction buffer */
	wg_id_in_ib = work_group->id_in_compute_unit/si_gpu_num_inst_buffers;
	first_entry = wg_id_in_ib * ndrange->wavefronts_per_work_group;

	/* Assign wavefronts a slot in the instruction buffer */
	for (i = 0; i < ndrange->wavefronts_per_work_group; i++) 
	{
		wavefront = work_group->wavefronts[i];
		wavefront->inst_buffer_entry = inst_buffer->entries[first_entry + i];
		assert(!wavefront->inst_buffer_entry->valid);

		/* Set initial state */
		wavefront->inst_buffer_entry->valid = 1;
		wavefront->inst_buffer_entry->ready = 1;
		wavefront->inst_buffer_entry->wavefront = wavefront;

		inst_buffer->wavefront_count++;
	}
}

void si_inst_buffer_unmap_wavefronts(struct si_inst_buffer_t *inst_buffer, 
	struct si_work_group_t *work_group)
{
	struct si_ndrange_t *ndrange = work_group->ndrange;
	struct si_wavefront_t *wavefront;
	int wf_id_in_ib;
	int i;

	/* Reset mapped wavefronts */
	assert(inst_buffer->wavefront_count >= ndrange->wavefronts_per_work_group);

	for (i = 0; i < ndrange->wavefronts_per_work_group; i++) 
	{
		wavefront = work_group->wavefronts[i];
		wf_id_in_ib = wavefront->inst_buffer_entry->id_in_inst_buffer;

		/* TODO Add complete flag to slots in instruction buffer */
		/* TODO Check that all slots are complete before setting to NULL */
		assert(inst_buffer->entries[wf_id_in_ib]->wavefront);
		assert(inst_buffer->entries[wf_id_in_ib]->wavefront->id == wavefront->id);
		inst_buffer->entries[wf_id_in_ib]->valid = 0;
		inst_buffer->entries[wf_id_in_ib]->wavefront_finished = 0;
		inst_buffer->entries[wf_id_in_ib]->wavefront = NULL;
	}
	inst_buffer->wavefront_count -= ndrange->wavefronts_per_work_group;
}
