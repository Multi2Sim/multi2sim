#include <southern-islands-timing.h>

int si_gpu_lds_inflight_mem_accesses = 32;
int si_gpu_lds_issue_width = 1;
int si_gpu_lds_reg_latency = 1;

void si_lds_writeback(struct si_lds_t *lds)
{
	struct si_uop_t *uop = NULL;
	struct si_wavefront_t *wavefront;
	int list_count;

	/* Process completed memory instructions */
	list_count = linked_list_count(lds->mem_out_buffer);
	linked_list_head(lds->mem_out_buffer);
	for (int i = 0; i < list_count; i++)
	{
		uop = linked_list_get(lds->mem_out_buffer);
		assert(uop);

		if (!uop->local_mem_witness)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(lds->mem_out_buffer);

			//printf("CYCLE[%lld]\t\tLDS\t\t\tWRITEBACK: UOP.ID[%lld]\n", si_gpu->cycle, uop->id);
			si_trace("si.inst id=%lld cu=%d stg=\"lds-w\"\n", uop->id_in_compute_unit,
							lds->compute_unit->id);

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
			linked_list_next(lds->mem_out_buffer);
		}
	}

	/* Statistics */
	si_gpu->last_complete_cycle = esim_cycle;
}

void si_lds_execute(struct si_lds_t *lds)
{
	struct si_uop_t *uop;
	struct si_work_item_uop_t *work_item_uop;
	struct si_work_item_t *work_item;
	int work_item_id;
	int instructions_issued = 0;
	int list_count;

	/* Look through the memory execution buffer looking for wavefronts ready to execute */
	list_count = linked_list_count(lds->mem_exec_buffer);
	linked_list_head(lds->mem_exec_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(lds->mem_exec_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_issued == si_gpu_branch_unit_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				lds->compute_unit->id);
			break;
		}

		/* Scalar memory read */
		assert(linked_list_count(lds->mem_out_buffer) <=
			si_gpu_lds_inflight_mem_accesses);

		/* If there is room in the outstanding memory buffer, issue the access */
		if (linked_list_count(lds->mem_out_buffer) <
			si_gpu_lds_inflight_mem_accesses)
		{
			/* Access local memory */
			SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
			{
				work_item = si_gpu->ndrange->work_items[work_item_id];
				work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
				for (i = 0; i < work_item_uop->local_mem_access_count; i++)
				{
					mod_access(lds->compute_unit->local_memory, mod_access_load,
						work_item_uop->local_mem_access_addr[i],
						&uop->local_mem_witness, NULL, NULL);
					uop->local_mem_witness--;
				}
			}

			/* Transfer the uop to the outstanding memory access buffer */
			linked_list_remove(lds->mem_exec_buffer);
			linked_list_add(lds->mem_out_buffer, uop);

			instructions_issued++;
			lds->inst_count++;
			lds->wavefront_count++;

			//printf("CYCLE[%lld]\t\tLDS\t\t\tEXECUTE: UOP.ID[%lld]\n", si_gpu->cycle, uop->id);
			si_trace("si.inst id=%lld cu=%d stg=\"lds-e\"\n", uop->id_in_compute_unit,
							lds->compute_unit->id);
		}
		else
		{
			/* Memory unit is busy, try later */
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				lds->compute_unit->id);
			break;
		}
	}
}

void si_lds_read(struct si_lds_t *lds)
{
	struct si_uop_t *uop;
	int instructions_issued = 0;
	int list_count;

	/* Look through the read buffer looking for wavefronts ready to issue */
	list_count = linked_list_count(lds->read_buffer);
	linked_list_head(lds->read_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(lds->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_issued == si_gpu_branch_unit_issue_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				lds->compute_unit->id);
			break;
		}

		/* Issue the uop if the exec_buffer is not full */
		if (linked_list_count(lds->mem_exec_buffer) <=
				si_gpu_simd_issue_width)
		{
			uop->read_ready = si_gpu->cycle + si_gpu_lds_reg_latency;
			linked_list_remove(lds->read_buffer);
			linked_list_add(lds->mem_exec_buffer, uop);

			instructions_issued++;

			//printf("CYCLE[%lld]\t\tLDS\t\t\tREAD: UOP.ID[%lld]\n", si_gpu->cycle, uop->id);
			si_trace("si.inst id=%lld cu=%d stg=\"lds-r\"\n", uop->id_in_compute_unit,
					lds->compute_unit->id);
		}
		else
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				lds->compute_unit->id);
			break;
		}
	}
}

void si_lds_run(struct si_lds_t *lds)
{
	/* Local Data Share stages */
	si_lds_writeback(lds);
	si_lds_execute(lds);
	si_lds_read(lds);
}
