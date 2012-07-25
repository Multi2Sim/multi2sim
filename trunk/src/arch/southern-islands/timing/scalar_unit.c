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
	list_count = linked_list_count(scalar_unit->mem_queue);
	if (list_count)
	{
		linked_list_head(scalar_unit->mem_queue);
		for (i = 0; i < list_count; i++)
		{
			uop = linked_list_get(scalar_unit->mem_queue);
			assert(uop);

			if (!uop->global_mem_witness)
			{
				/* Access complete, remove the uop from the queue */
				linked_list_remove(scalar_unit->mem_queue);

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
				linked_list_next(scalar_unit->mem_queue);
			}
		}
	}

	/* Process completed ALU instructions */
	list_count = linked_list_count(scalar_unit->alu_queue);
	if (list_count)
	{
		linked_list_head(scalar_unit->alu_queue);
		for (i = 0; i < list_count; i++)
		{
			uop = linked_list_get(scalar_unit->alu_queue);
			assert(uop);

			if (uop->execute_ready <= si_gpu->cycle)
			{
				/* Access complete, remove the uop from the queue */
				linked_list_remove(scalar_unit->alu_queue);

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
				linked_list_next(scalar_unit->alu_queue);
			}
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

	list_count = linked_list_count(scalar_unit->exec_inst_buffer);

	/* No instruction to process */
	if (!list_count)
		return;

	linked_list_head(scalar_unit->exec_inst_buffer);
	for (i = 0; i < list_count; i++) 
	{
		if (instructions_issued == si_gpu_scalar_unit_issue_width)
			break;

		/* Process uop */
		uop = linked_list_get(scalar_unit->exec_inst_buffer); 
		assert(uop);

		/* Decode has not completed.  Safe to assume that no other
		 * decoded instructions are ready */
		if (si_gpu->cycle < uop->read_ready)
			break;
		
		/* Check if type is memory or ALU */
		if (uop->wavefront->inst.info->fmt == SI_FMT_SMRD)
		{	
			/* Scalar memory read */
			assert(linked_list_count(scalar_unit->mem_queue) <= 
				si_gpu_scalar_unit_inflight_mem_accesses);

			if (linked_list_count(scalar_unit->mem_queue) <
				si_gpu_scalar_unit_inflight_mem_accesses)
			{
				/* Access global memory */
				uop->global_mem_witness--;
				uop->global_mem_access_addr = 
					uop->wavefront->scalar_work_item->global_mem_access_addr;
				mod_access(scalar_unit->compute_unit->global_memory, 
					mod_access_load, uop->global_mem_access_addr, 
					&uop->global_mem_witness, NULL, NULL);

				/* Add the uop to the inflight memory access queue */
				linked_list_remove(scalar_unit->exec_inst_buffer); 
				linked_list_add(scalar_unit->mem_queue, uop);

				instructions_issued++;
				scalar_unit->inst_count++;
				scalar_unit->wavefront_count++;
			}
			else 
			{
				/* Memory unit is busy, try the next instruction */
				linked_list_next(scalar_unit->exec_inst_buffer);
			}
			continue;
		}
		else
		{
			/* Scalar ALU */
			if (linked_list_count(scalar_unit->alu_queue) <= 
				si_gpu_scalar_unit_issue_width)
			{
				/* Execution unit is available */
				uop->execute_ready = si_gpu->cycle + 
					si_gpu_scalar_unit_alu_latency;
				linked_list_remove(scalar_unit->exec_inst_buffer); 
				linked_list_add(scalar_unit->alu_queue, uop);

				instructions_issued++;
				scalar_unit->inst_count++;
				scalar_unit->wavefront_count++;
			}
			else 
			{
				/* Execution unit is busy, try the next instruction */
				linked_list_next(scalar_unit->exec_inst_buffer);
			}
			continue;
		}
	}
}

void si_scalar_unit_read(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;
	int instructions_issued = 0;
	int list_count;
	int i;

	list_count = linked_list_count(scalar_unit->read_inst_buffer);

	/* No instruction to process */
	if (!list_count)
		return;

	linked_list_head(scalar_unit->read_inst_buffer);
	for (i = 0; i < list_count; i++) 
	{
		if (instructions_issued == si_gpu_scalar_unit_issue_width)
			break;

		/* Process uop */
		uop = linked_list_get(scalar_unit->read_inst_buffer); 
		assert(uop);

		/* Read has not completed.  Safe to assume that no other
		 * decoded instructions are ready */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		uop->read_ready = si_gpu->cycle + si_gpu_scalar_unit_reg_latency;
		linked_list_remove(scalar_unit->read_inst_buffer);
		linked_list_add(scalar_unit->exec_inst_buffer, uop);

		instructions_issued++;
	}
}

void si_scalar_unit_run(struct si_scalar_unit_t *scalar_unit)
{
	si_scalar_unit_writeback(scalar_unit);
	si_scalar_unit_execute(scalar_unit);
	si_scalar_unit_read(scalar_unit);
}

