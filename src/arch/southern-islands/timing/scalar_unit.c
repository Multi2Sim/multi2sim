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


int si_gpu_scalar_unit_inflight_mem_accesses = 32;
int si_gpu_scalar_unit_alu_latency = 4;
int si_gpu_scalar_unit_reg_latency = 1;
int si_gpu_scalar_unit_issue_width = 1;


void si_scalar_unit_writeback(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop = NULL;
	struct si_wavefront_t *wavefront;
	struct si_work_group_t *work_group;
	struct si_ndrange_t *ndrange;
	int i;
	int list_count;
	int wavefront_id;

	/* Process completed memory instructions */
	list_count = linked_list_count(scalar_unit->mem_out_buffer);
	linked_list_head(scalar_unit->mem_out_buffer);
	for (i = 0; i < list_count; i++)
	{
		uop = linked_list_get(scalar_unit->mem_out_buffer);
		assert(uop);

		if (!uop->global_mem_witness)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(scalar_unit->mem_out_buffer);

			si_trace("si.inst id=%lld cu=%d stg=\"su-w\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id);

			/* Make the wavefront active again */
			wavefront = uop->wavefront;
			wavefront->ready = 1;

			/* Free uop */
			if (si_tracing())
				si_gpu_uop_trash_add(uop);
			else
				si_uop_free(uop);
		}
		else
		{
			linked_list_next(scalar_unit->mem_out_buffer);
		}
	}

	/* Process completed ALU instructions */
	list_count = linked_list_count(scalar_unit->alu_out_buffer);
	linked_list_head(scalar_unit->alu_out_buffer);
	for (i = 0; i < list_count; i++)
	{
		uop = linked_list_get(scalar_unit->alu_out_buffer);
		assert(uop);

		if (uop->execute_ready <= si_gpu->cycle)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(scalar_unit->alu_out_buffer);

			si_trace("si.inst id=%lld cu=%d stg=\"su-w\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id);

			/* Make the wavefront active again */
			wavefront = uop->wavefront;
			work_group = wavefront->work_group;
			ndrange = work_group->ndrange;
			if (wavefront->finished)
			{
				work_group->compute_unit_finished_count++;
			}
			else if (wavefront->barrier)
			{
				if (wavefront->barrier_cleared) 
				{
					/* All wavefronts have hit barrier */

					wavefront->barrier_cleared = 0;

					SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, 
						wavefront_id)
					{
						wavefront = ndrange->wavefronts[wavefront_id];
						wavefront->barrier = 0;
						wavefront->ready = 1;
					}
	
				}
				else 
				{
					/* Wavefront is waiting at barrier */
				}
			}
			else 
			{
				wavefront->ready = 1;
			}

			/* Check if wavefront finishes a work-group */
			assert(work_group->compute_unit_finished_count <=
				work_group->wavefront_count);
			if (work_group->compute_unit_finished_count ==
				work_group->wavefront_count)
				si_compute_unit_unmap_work_group(scalar_unit->compute_unit,
					work_group);

			/* Free uop */
			if (si_tracing())
				si_gpu_uop_trash_add(uop);
			else
				si_uop_free(uop);
		}
		else
		{
			linked_list_next(scalar_unit->alu_out_buffer);
		}
	}

	/* Statistics */
	si_gpu->last_complete_cycle = esim_cycle;
}

void si_scalar_unit_execute(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;
	int instructions_issued = 0;
	int list_count;
	int i;

	/* Look through the memory execution buffer looking for wavefronts ready to execute */
	list_count = linked_list_count(scalar_unit->mem_exec_buffer);
	linked_list_head(scalar_unit->mem_exec_buffer);
	for (i = 0; i < list_count; i++) 
	{
		/* Peek at the first uop */
		uop = linked_list_get(scalar_unit->mem_exec_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;
		
		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_scalar_unit_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				scalar_unit->compute_unit->id);
			break;
		}

		/* Scalar memory read */
		assert(linked_list_count(scalar_unit->mem_out_buffer) <=
			si_gpu_scalar_unit_inflight_mem_accesses);

		/* If there is room in the outstanding memory buffer, issue the access */
		if (linked_list_count(scalar_unit->mem_out_buffer) <
			si_gpu_scalar_unit_inflight_mem_accesses)
		{
			/* Access global memory */
			uop->global_mem_witness--;
			uop->global_mem_access_addr =
				uop->wavefront->scalar_work_item->global_mem_access_addr;
			mod_access(scalar_unit->compute_unit->global_memory,
				mod_access_load, uop->global_mem_access_addr,
				&uop->global_mem_witness, NULL, NULL);

			/* Transfer the uop to the outstanding memory access buffer */
			linked_list_remove(scalar_unit->mem_exec_buffer);
			linked_list_add(scalar_unit->mem_out_buffer, uop);

			instructions_issued++;
			scalar_unit->inst_count++;
			scalar_unit->wavefront_count++;

			si_trace("si.inst id=%lld cu=%d stg=\"su-m\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id);
		}
		else
		{
			/* Memory unit is busy, try later */
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id);
			break;
		}
	}

	/* Look through the ALU execution buffer looking for wavefronts ready to execute */
	list_count = linked_list_count(scalar_unit->alu_exec_buffer);
	linked_list_head(scalar_unit->alu_exec_buffer);
	for (i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(scalar_unit->alu_exec_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_scalar_unit_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				scalar_unit->compute_unit->id);
			break;
		}

		/* Scalar ALU */
		uop->execute_ready = si_gpu->cycle +
			si_gpu_scalar_unit_alu_latency;

		/* Transfer the uop to the outstanding ALU execution buffer */
		linked_list_remove(scalar_unit->alu_exec_buffer);
		linked_list_add(scalar_unit->alu_out_buffer, uop);

		instructions_issued++;
		scalar_unit->inst_count++;
		scalar_unit->wavefront_count++;

		si_trace("si.inst id=%lld cu=%d stg=\"su-a\"\n", uop->id_in_compute_unit, 
			scalar_unit->compute_unit->id);
	}
}

void si_scalar_unit_read(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;
	int instructions_issued = 0;
	int list_count;
	int i;

	/* Look through the read buffer looking for wavefronts ready to issue */
	list_count = linked_list_count(scalar_unit->read_buffer);
	linked_list_head(scalar_unit->read_buffer);
	for (i = 0; i < list_count; i++) 
	{
		/* Peek at the first uop */
		uop = linked_list_get(scalar_unit->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_scalar_unit_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				scalar_unit->compute_unit->id);
			break;
		}

		/* Issue the uop, if the exec_buffer is not full */
		if (uop->wavefront->inst.info->fmt == SI_FMT_SMRD &&
				linked_list_count(scalar_unit->mem_exec_buffer) <
				si_gpu_scalar_unit_issue_width)
		{
			uop->read_ready = si_gpu->cycle + si_gpu_scalar_unit_reg_latency;
			linked_list_remove(scalar_unit->read_buffer);
			linked_list_add(scalar_unit->mem_exec_buffer, uop);
		}
		else if (linked_list_count(scalar_unit->alu_exec_buffer) <
				si_gpu_scalar_unit_issue_width)
		{
			uop->read_ready = si_gpu->cycle + si_gpu_scalar_unit_reg_latency;
			linked_list_remove(scalar_unit->read_buffer);
			linked_list_add(scalar_unit->alu_exec_buffer, uop);
		}
		else
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				scalar_unit->compute_unit->id);
			break;
		}

		instructions_issued++;

		si_trace("si.inst id=%lld cu=%d stg=\"su-r\"\n", uop->id_in_compute_unit, 
			scalar_unit->compute_unit->id);
	}
}

void si_scalar_unit_run(struct si_scalar_unit_t *scalar_unit)
{
	si_scalar_unit_writeback(scalar_unit);
	si_scalar_unit_execute(scalar_unit);
	si_scalar_unit_read(scalar_unit);
}

