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

#include "southern-islands-timing.h"


int si_gpu_simd_alu_latency = 8;
int si_gpu_simd_reg_latency = 1;
int si_gpu_simd_issue_width = 1;
int si_gpu_simd_num_subwavefronts = 4;

void si_simd_writeback(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	struct si_wavefront_t *wavefront;
	int list_count;

	/* Process completed ALU instructions */
	list_count = linked_list_count(simd->alu_out_buffer);
	linked_list_head(simd->alu_out_buffer);
	for (int i = 0; i < list_count; i++)
	{
		uop = linked_list_get(simd->alu_out_buffer);
		assert(uop);

		if (uop->execute_ready <= si_gpu->cycle)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(simd->alu_out_buffer);

			si_trace("si.inst id=%lld cu=%d stg=\"simd-w\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id);

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
			linked_list_next(simd->alu_out_buffer);
		}
	}

	/* Statistics */
	si_gpu->last_complete_cycle = esim_cycle;
}

void si_simd_execute(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_count;
	int instructions_issued = 0;

	/* Look through the ALU execution buffer looking for wavefronts ready to execute */
	list_count = linked_list_count(simd->alu_exec_buffer);
	linked_list_head(simd->alu_exec_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(simd->alu_exec_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_issued == si_gpu_branch_unit_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id);
			break;
		}

		/* If the uop is ready to begin execution, set the ALU ready and execution
		 * ready cycles */
		if (si_gpu->cycle == uop->read_ready)
		{
			uop->alu_ready = si_gpu->cycle + si_gpu_simd_num_subwavefronts;
			uop->execute_ready = uop->alu_ready + si_gpu_simd_alu_latency - 1;

			instructions_issued++;

			si_trace("si.inst id=%lld cu=%d stg=\"simd-e\"\n", uop->id_in_compute_unit,
					simd->compute_unit->id);
		}
		/* If the uop is now in the ALU pipeline, move it to the outstanding ALU buffer */
		else if (si_gpu->cycle >= uop->alu_ready)
		{
			linked_list_remove(simd->alu_exec_buffer);
			linked_list_add(simd->alu_out_buffer, uop);
		}
	}
}

void si_simd_read(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_count;
	int instructions_issued = 0;

	/* Look through the read buffer looking for wavefronts ready to issue */
	list_count = linked_list_count(simd->read_buffer);
	linked_list_head(simd->read_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(simd->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_issued == si_gpu_branch_unit_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id);
			break;
		}

		/* Issue the uop if the exec_buffer is not full */
		if (linked_list_count(simd->alu_exec_buffer) < si_gpu_simd_issue_width)
		{
			uop->read_ready = si_gpu->cycle + si_gpu_simd_reg_latency;
			linked_list_remove(simd->read_buffer);
			linked_list_add(simd->alu_exec_buffer, uop);

			instructions_issued++;

			si_trace("si.inst id=%lld cu=%d stg=\"simd-r\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id);
		}
		else
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id);
			break;
		}
	}
}

void si_simd_run(struct si_simd_t *simd)
{
	/* SIMD stages */
	si_simd_writeback(simd);
	si_simd_execute(simd);
	si_simd_read(simd);
}
