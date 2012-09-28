#include <southern-islands-timing.h>

int si_gpu_scalar_unit_inflight_mem_accesses = 32;
int si_gpu_scalar_unit_alu_latency = 4;
int si_gpu_scalar_unit_reg_latency = 1;
int si_gpu_scalar_unit_width = 2;


void si_scalar_unit_writeback(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop = NULL;
	struct si_wavefront_t *wavefront;
	struct si_work_group_t *work_group;
	int i;
	int list_count;

	/* Process all completed memory instructions */
	list_count = linked_list_count(scalar_unit->mem_buffer);
	linked_list_head(scalar_unit->mem_buffer);
	for (i = 0; i < list_count; i++)
	{
		uop = linked_list_get(scalar_unit->mem_buffer);
		assert(uop);

		if (!uop->global_mem_witness)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(scalar_unit->mem_buffer);

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
			linked_list_next(scalar_unit->mem_buffer);
		}
	}

	/* Process all completed ALU instructions */
	list_count = linked_list_count(scalar_unit->alu_buffer);
	linked_list_head(scalar_unit->alu_buffer);
	for (i = 0; i < list_count; i++)
	{
		uop = linked_list_get(scalar_unit->alu_buffer);
		assert(uop);

		if (uop->execute_ready <= si_gpu->cycle)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(scalar_unit->alu_buffer);

			si_trace("si.inst id=%lld cu=%d stg=\"su-w\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id);

			/* Make the wavefront active again */
			wavefront = uop->wavefront;
			work_group = wavefront->work_group;
			if (!wavefront->finished)
				wavefront->ready = 1;
			else
				work_group->compute_unit_finished_count++;

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
			linked_list_next(scalar_unit->alu_buffer);
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

	/* Look through the memory read buffer looking for wavefronts ready to execute */
	list_count = linked_list_count(scalar_unit->mem_read_buffer);
	linked_list_head(scalar_unit->mem_read_buffer);
	for (i = 0; i < list_count; i++) 
	{
		/* Peek at the first uop */
		uop = linked_list_get(scalar_unit->mem_read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;
		
		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_scalar_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				scalar_unit->compute_unit->id);
			break;
		}

		/* Stall if the outstanding memory buffer is full. */
		if (linked_list_count(scalar_unit->mem_buffer) >=
			si_gpu_scalar_unit_inflight_mem_accesses)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
					scalar_unit->compute_unit->id);
			break;
		}

		/* Access global memory */
		uop->global_mem_witness--;
		uop->global_mem_access_addr =
			uop->wavefront->scalar_work_item->global_mem_access_addr;
		mod_access(scalar_unit->compute_unit->global_memory,
			mod_access_load, uop->global_mem_access_addr,
			&uop->global_mem_witness, NULL, NULL);

		/* Transfer the uop to the outstanding memory access buffer */
		linked_list_remove(scalar_unit->mem_read_buffer);
		linked_list_add(scalar_unit->mem_buffer, uop);

		instructions_issued++;
		scalar_unit->inst_count++;
		scalar_unit->wavefront_count++;

		si_trace("si.inst id=%lld cu=%d stg=\"su-m\"\n", uop->id_in_compute_unit,
			scalar_unit->compute_unit->id);
	}

	/* Look through the ALU read buffer looking for wavefronts ready to execute */
	list_count = linked_list_count(scalar_unit->alu_read_buffer);
	linked_list_head(scalar_unit->alu_read_buffer);
	for (i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(scalar_unit->alu_read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_scalar_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				scalar_unit->compute_unit->id);
			break;
		}

		/* Stall if the outstanding memory buffer is full. */
		if (linked_list_count(scalar_unit->alu_buffer) >=
			si_gpu_scalar_unit_width*si_gpu_scalar_unit_alu_latency)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
					scalar_unit->compute_unit->id);
			break;
		}

		/* Scalar ALU */
		uop->execute_ready = si_gpu->cycle +
			si_gpu_scalar_unit_alu_latency;

		/* Transfer the uop to the outstanding ALU execution buffer */
		linked_list_remove(scalar_unit->alu_read_buffer);
		linked_list_add(scalar_unit->alu_buffer, uop);

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

	/* Look through the decode buffer looking for wavefronts ready to read */
	list_count = linked_list_count(scalar_unit->decode_buffer);
	linked_list_head(scalar_unit->decode_buffer);
	for (i = 0; i < list_count; i++) 
	{
		/* Peek at the first uop */
		uop = linked_list_get(scalar_unit->decode_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stall if the read_buffer is full. */
		if((uop->wavefront->inst.info->fmt == SI_FMT_SMRD) ?
				(linked_list_count(scalar_unit->mem_read_buffer) >=
							si_gpu_scalar_unit_width*si_gpu_scalar_unit_reg_latency):
				(linked_list_count(scalar_unit->alu_read_buffer) >=
							si_gpu_scalar_unit_width*si_gpu_scalar_unit_reg_latency))
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				scalar_unit->compute_unit->id);
			/* Continue to try and read next instruction, other exec_buffer may not be full. */
			continue;
		}

		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_scalar_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				scalar_unit->compute_unit->id);
			break;
		}

		/* Issue the uop, */
		if (uop->wavefront->inst.info->fmt == SI_FMT_SMRD)
		{
			uop->read_ready = si_gpu->cycle + si_gpu_scalar_unit_reg_latency;
			linked_list_remove(scalar_unit->decode_buffer);
			linked_list_add(scalar_unit->mem_read_buffer, uop);
		}
		else
		{
			uop->read_ready = si_gpu->cycle + si_gpu_scalar_unit_reg_latency;
			linked_list_remove(scalar_unit->decode_buffer);
			linked_list_add(scalar_unit->alu_read_buffer, uop);
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

