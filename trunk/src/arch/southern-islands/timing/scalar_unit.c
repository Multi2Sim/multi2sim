#include <southern-islands-timing.h>
#include <heap.h>

int si_gpu_scalar_unit_inflight_mem_accesses = 32;
int si_gpu_scalar_unit_exec_latency = 1;

void si_scalar_unit_execute_writeback(struct si_scalar_unit_t *scalar_unit)
{
}

void si_scalar_unit_execute(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;
	struct si_wavefront_t *wavefront;
	struct si_work_group_t *work_group;

	uop = scalar_unit->exec_buffer;

	/* No instruction to process */
	if (!uop)
		return;

#if 0
	heap_insert(scalar_unit->event_queue, si_gpu->cycle + si_gpu_scalar_unit_exec_latency, uop);
#endif
	
	wavefront = uop->wavefront;
	work_group = wavefront->work_group;

	/* Make the wavefront active again */
	if (!wavefront->finished)
		wavefront->ready = 1;
	else 
		work_group->compute_unit_finished_count++;

	/* Remove the uop from the execute buffer */
	scalar_unit->exec_buffer = NULL;

	/* Wavefront finishes a work-group */
	assert(work_group->compute_unit_finished_count <= work_group->wavefront_count);
	if (work_group->compute_unit_finished_count == work_group->wavefront_count)
		si_compute_unit_unmap_work_group(scalar_unit->compute_unit, work_group);

	/* Free uop */
	if (si_tracing())
		si_gpu_uop_trash_add(uop);
	else
		si_uop_free(uop);

	/* Statistics */
	si_gpu->last_complete_cycle = esim_cycle;
}

void si_scalar_unit_memory_writeback(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop = NULL;
	struct si_wavefront_t *wavefront;
	int i;
	int list_count;

	list_count = linked_list_count(scalar_unit->mem_queue);
	if (!list_count)
		return;

	linked_list_head(scalar_unit->mem_queue);
	for (i = 0; i < list_count; i++)
	{
		uop = linked_list_get(scalar_unit->mem_queue);
		assert(uop);

		if (!uop->global_mem_witness)
		{
			/* Access complete, remove the uop from the list */
			linked_list_remove(scalar_unit->mem_queue);
			break;
		}

		linked_list_next(scalar_unit->mem_queue);
	}

	/* No completed accesses */
	if (i == list_count)
		return;

	/* Make the wavefront active again */
	wavefront = uop->wavefront;
	wavefront->ready = 1;

	/* Free uop */
	if (si_tracing())
		si_gpu_uop_trash_add(uop);
	else
		si_uop_free(uop);

	/* Statistics */
	si_gpu->last_complete_cycle = esim_cycle;
}

void si_scalar_unit_memory(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;

	uop = scalar_unit->mem_buffer; 

	/* No instruction to process */
	if (!uop)
		return;

	assert(linked_list_count(scalar_unit->mem_queue) < 
		si_gpu_scalar_unit_inflight_mem_accesses);

	/* Max outstanding memory accesses is reached, stall */
	if (linked_list_count(scalar_unit->mem_queue) == si_gpu_scalar_unit_inflight_mem_accesses-1)
		return;

	/* Access global memory */
	uop->global_mem_witness--;
	uop->global_mem_access_addr = uop->wavefront->scalar_work_item->global_mem_access_addr;
	mod_access(scalar_unit->compute_unit->global_memory, mod_access_load, 
		uop->global_mem_access_addr, &uop->global_mem_witness, NULL, NULL);

	/* Add the uop to the inflight memory access queue */
	linked_list_add(scalar_unit->mem_queue, uop);

	/* Remove the uop from the memory buffer */
	scalar_unit->mem_buffer = NULL;
}

void si_scalar_unit_read(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;

	/* Process uop */
	uop = scalar_unit->inst_buffer; 

	/* No instruction to process */
	if (!uop)
		return;

	/* Decode has not completed */
	if (si_gpu->cycle < uop->decode_ready)
		return;
	
	/* Check if type is memory or ALU */
	if (uop->wavefront->inst.info->fmt == SI_FMT_SMRD)
	{	
		/* Scalar memory read */
		if (!scalar_unit->mem_buffer)
		{
			scalar_unit->mem_buffer = uop;
		}
		else 
		{
			/* Memory unit is busy */
			return;
		}
	}
	else
	{
		/* Scalar ALU */
		if (!scalar_unit->exec_buffer)
		{
			scalar_unit->exec_buffer = uop;
		}
		else 
		{
			/* Execution unit is busy */
			return;
		}
	}

	/* Uop was processed, so remove it from instruction buffer */
	scalar_unit->inst_buffer = NULL;
}

void si_scalar_unit_run(struct si_scalar_unit_t *scalar_unit)
{
	si_scalar_unit_memory_writeback(scalar_unit);
	si_scalar_unit_execute(scalar_unit);
	si_scalar_unit_memory(scalar_unit);
	si_scalar_unit_read(scalar_unit);
}

