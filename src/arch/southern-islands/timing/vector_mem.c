#include <southern-islands-timing.h>

int si_gpu_vector_mem_inflight_mem_accesses = 32;
int si_gpu_vector_mem_issue_width = 1;
int si_gpu_vector_mem_reg_latency = 1;

void si_vector_mem_writeback(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop = NULL;
	struct si_wavefront_t *wavefront;
	int list_count;

	/* Process completed memory instructions */
	list_count = linked_list_count(vector_mem->mem_out_buffer);
	linked_list_head(vector_mem->mem_out_buffer);
	for (int i = 0; i < list_count; i++)
	{
		uop = linked_list_get(vector_mem->mem_out_buffer);
		assert(uop);

		if (!uop->global_mem_witness)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(vector_mem->mem_out_buffer);

			/* Make the wavefront active again */
			wavefront = uop->wavefront;
			wavefront->ready = 1;

			//printf("CYCLE[%lld]\t\tVector Memory Unit\t\tWRITEBACK: UOP.ID[%lld]\n", si_gpu->cycle, uop->id);

			/* Free uop */
			if (si_tracing())
				si_gpu_uop_trash_add(uop);
			else
				si_uop_free(uop);
		}
		else
		{
			linked_list_next(vector_mem->mem_out_buffer);
		}
	}

	/* Statistics */
	si_gpu->last_complete_cycle = esim_cycle;
}

void si_vector_mem_execute(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop;
	struct si_work_item_uop_t *work_item_uop;
	struct si_work_item_t *work_item;
	int work_item_id;
	int instructions_issued = 0;
	int list_count;

	/* Look through the memory execution buffer looking for wavefronts ready to execute */
	list_count = linked_list_count(vector_mem->mem_exec_buffer);
	linked_list_head(vector_mem->mem_exec_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_vector_mem_issue_width)
			break;

		/* Peek at the first uop */
		uop = linked_list_get(vector_mem->mem_exec_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Vector memory read */
		assert(linked_list_count(vector_mem->mem_out_buffer) <=
			si_gpu_vector_mem_inflight_mem_accesses);

		/* If there is room in the outstanding memory buffer, issue the access */
		if (linked_list_count(vector_mem->mem_out_buffer) <
			si_gpu_vector_mem_inflight_mem_accesses)
		{
			/* Access global memory */
			SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
			{
				work_item = si_gpu->ndrange->work_items[work_item_id];
				work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
				mod_access(vector_mem->compute_unit->global_memory, mod_access_load,
					work_item_uop->global_mem_access_addr,
					&uop->global_mem_witness, NULL, NULL);
				uop->global_mem_witness--;
			}

			/* Transfer the uop to the outstanding memory access buffer */
			linked_list_remove(vector_mem->mem_exec_buffer);
			linked_list_add(vector_mem->mem_out_buffer, uop);

			instructions_issued++;
			vector_mem->inst_count++;
			vector_mem->wavefront_count++;

			//printf("CYCLE[%lld]\t\tVector Memory Unit\t\tEXECUTE: UOP.ID[%lld]\n", si_gpu->cycle, uop->id);
		}
		else
		{
			/* Memory unit is busy, try later */
			break;
		}
	}
}

void si_vector_mem_read(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop;
	int instructions_issued = 0;
	int list_count;

	/* Look through the read buffer looking for wavefronts ready to issue */
	list_count = linked_list_count(vector_mem->read_buffer);
	linked_list_head(vector_mem->read_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_vector_mem_issue_width)
			break;

		/* Peek at the first uop */
		uop = linked_list_get(vector_mem->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Issue the uop, no need to check if the exec_buffer is full */
		uop->read_ready = si_gpu->cycle + si_gpu_vector_mem_reg_latency;
		linked_list_remove(vector_mem->read_buffer);
		linked_list_add(vector_mem->mem_exec_buffer, uop);

		instructions_issued++;

		//printf("CYCLE[%lld]\t\tVector Memory Unit\t\tREAD: UOP.ID[%lld]\n", si_gpu->cycle, uop->id);
	}
}

void si_vector_mem_run(struct si_vector_mem_unit_t *vector_mem)
{
	/* Vector Memory stages */
	si_vector_mem_writeback(vector_mem);
	si_vector_mem_execute(vector_mem);
	si_vector_mem_read(vector_mem);
}
