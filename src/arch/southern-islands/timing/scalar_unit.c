#include <southern-islands-timing.h>

int si_gpu_scalar_unit_inflight_mem_accesses = 32;
int si_gpu_scalar_unit_alu_latency = 4;
int si_gpu_scalar_unit_reg_latency = 1;
int si_gpu_scalar_unit_issue_width = 2;


void si_scalar_unit_writeback(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop = NULL;
	struct si_wavefront_t *wavefront;
	struct si_work_group_t *work_group;
	int i;
	int list_count;

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

			//printf("CYCLE[%lld]\t\tScalar Unit\t\tWRITEBACK: UOP.ID[%lld]  [MEM]\n", si_gpu->cycle, uop->id);

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

			//printf("CYCLE[%lld]\t\tScalar Unit\t\tWRITEBACK: UOP.ID[%lld]  [ALU]\n", si_gpu->cycle, uop->id);

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
		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_scalar_unit_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id);
			break;
		}

		/* Peek at the first uop */
		uop = linked_list_get(scalar_unit->mem_exec_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;
		
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
			//printf("CYCLE[%lld]\t\tScalar Unit\t\tEXECUTE: UOP.ID[%lld]  [MEM]\n", si_gpu->cycle, uop->id);
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
		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_scalar_unit_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id);
			break;
		}

		/* Peek at the first uop */
		uop = linked_list_get(scalar_unit->alu_exec_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

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
		//printf("CYCLE[%lld]\t\tScalar Unit\t\tEXECUTE: UOP.ID[%lld]  [ALU]\n", si_gpu->cycle, uop->id);
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
		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_scalar_unit_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit, 
				scalar_unit->compute_unit->id);
			break;
		}

		/* Peek at the first uop */
		uop = linked_list_get(scalar_unit->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Issue the uop, no need to check if the exec_buffers are full */
		uop->read_ready = si_gpu->cycle + si_gpu_scalar_unit_reg_latency;
		linked_list_remove(scalar_unit->read_buffer);

		if (uop->wavefront->inst.info->fmt == SI_FMT_SMRD)
		{
			linked_list_add(scalar_unit->mem_exec_buffer, uop);
			//printf("CYCLE[%lld]\t\tScalar Unit\t\tREAD: UOP.ID[%lld]  [MEM]\n", si_gpu->cycle, uop->id);
		}
		else
		{
			linked_list_add(scalar_unit->alu_exec_buffer, uop);
			//printf("CYCLE[%lld]\t\tScalar Unit\t\tREAD: UOP.ID[%lld]  [ALU]\n", si_gpu->cycle, uop->id);
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

