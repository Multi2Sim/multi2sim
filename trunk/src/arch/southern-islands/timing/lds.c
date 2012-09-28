#include <southern-islands-timing.h>

int si_gpu_lds_inflight_mem_accesses = 32;
int si_gpu_lds_width = 1;
int si_gpu_lds_reg_latency = 1;

void si_lds_writeback(struct si_lds_t *lds)
{
	struct si_uop_t *uop = NULL;
	struct si_wavefront_t *wavefront;
	int list_count;

	/* Process all completed memory instructions */
	list_count = linked_list_count(lds->mem_buffer);
	linked_list_head(lds->mem_buffer);
	for (int i = 0; i < list_count; i++)
	{
		uop = linked_list_get(lds->mem_buffer);
		assert(uop);

		if (!uop->local_mem_witness)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(lds->mem_buffer);

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
			linked_list_next(lds->mem_buffer);
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
	int i, j;
	enum mod_access_kind_t access_type;

	/* Look through the read buffer looking for wavefronts ready to execute */
	list_count = linked_list_count(lds->read_buffer);
	linked_list_head(lds->read_buffer);
	for (i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(lds->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_issued == si_gpu_lds_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				lds->compute_unit->id);
			break;
		}

		/* Stall if the outstanding memory buffer is full. */
		if (linked_list_count(lds->mem_buffer) >=
			si_gpu_lds_inflight_mem_accesses)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
					lds->compute_unit->id);
			break;
		}

		/* Access local memory */
		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
		{
			work_item = si_gpu->ndrange->work_items[work_item_id];
			work_item_uop = &uop->work_item_uop[work_item->id_in_wavefront];
			for (j = 0; j < work_item_uop->local_mem_access_count; j++)
			{
				if (work_item->local_mem_access_type[j] == 1)
					access_type = mod_access_load;
				else if (work_item->local_mem_access_type[j] == 2)
					access_type = mod_access_store;
				else
					fatal("%s: invalid lds access type", __FUNCTION__);

				mod_access(lds->compute_unit->local_memory, access_type,
					work_item_uop->local_mem_access_addr[j],
					&uop->local_mem_witness, NULL, NULL);
				uop->local_mem_witness--;
			}
		}

		/* Transfer the uop to the outstanding memory access buffer */
		linked_list_remove(lds->read_buffer);
		linked_list_add(lds->mem_buffer, uop);

		instructions_issued++;
		lds->inst_count++;
		lds->wavefront_count++;

		si_trace("si.inst id=%lld cu=%d stg=\"lds-e\"\n", uop->id_in_compute_unit,
						lds->compute_unit->id);
	}
}

void si_lds_read(struct si_lds_t *lds)
{
	struct si_uop_t *uop;
	int instructions_issued = 0;
	int list_count;

	/* Look through the decode buffer looking for wavefronts ready to read */
	list_count = linked_list_count(lds->decode_buffer);
	linked_list_head(lds->decode_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(lds->decode_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stall if the read_buffer is full. */
		if (linked_list_count(lds->read_buffer) >=
				si_gpu_lds_width*si_gpu_lds_reg_latency)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
					lds->compute_unit->id);
			break;
		}

		/* Stop if the issue width has been reached, stall */
		if (instructions_issued == si_gpu_lds_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				lds->compute_unit->id);
			break;
		}

		/* Issue the uop */
		uop->read_ready = si_gpu->cycle + si_gpu_lds_reg_latency;
		linked_list_remove(lds->decode_buffer);
		linked_list_add(lds->read_buffer, uop);

		instructions_issued++;

		si_trace("si.inst id=%lld cu=%d stg=\"lds-r\"\n", uop->id_in_compute_unit,
				lds->compute_unit->id);
	}
}

void si_lds_run(struct si_lds_t *lds)
{
	/* Local Data Share stages */
	si_lds_writeback(lds);
	si_lds_execute(lds);
	si_lds_read(lds);
}
