#include <southern-islands-timing.h>
#include <heap.h>

/* FIXME Allow issue rate of > 1 instruction */
//int si_gpu_scalar_unit_issue_rate = 1;
int si_gpu_scalar_unit_exec_latency = 1;
int si_gpu_scalar_unit_inflight_mem_accesses = 32;

void si_scalar_unit_writeback(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;
	unsigned long long cycle;

	//printf(" heap count = %d\n", heap_count(scalar_unit->event_queue));
	/* Check if there is a uop ready to commit */
	cycle = heap_peek(scalar_unit->event_queue, (void **) &uop);
	if (!uop || cycle > si_gpu->cycle)
		return;

	/* Extract the event from the heap */
	heap_extract(scalar_unit->event_queue, NULL);

	/* Make the wavefront active again */
	uop->wavefront->ready = 1;
}

void si_scalar_unit_execute(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;

	if (!scalar_unit->exec_buffer)
	{
		/* No instruction to process */
		return;
	}

	uop = scalar_unit->exec_buffer; 

	heap_insert(scalar_unit->event_queue, si_gpu->cycle + si_gpu_scalar_unit_exec_latency, uop);
	
	/* Remove the uop from the execute buffer */
	scalar_unit->exec_buffer = NULL;

}

void si_scalar_unit_memory(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;

	if (!scalar_unit->mem_buffer)
	{
		/* No instruction to process */
		return;
	}

	if (linked_list_count(scalar_unit->mem_queue) > si_gpu_scalar_unit_inflight_mem_accesses)
	{
		/* Max outstanding memory accesses is reached, stall */
		return;
	}

	uop = scalar_unit->mem_buffer; 

	/* FIXME Implement memory operations */
	heap_insert(scalar_unit->event_queue, si_gpu->cycle + 10, uop);

	/* Remove the uop from the memory buffer */
	scalar_unit->mem_buffer = NULL;
}

void si_scalar_unit_read(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;

	if (!scalar_unit->inst_buffer)
	{
		//printf("    nothing to do\n");
		/* No instruction to process */
		return;
	}

	/* Process uop */
	uop = scalar_unit->inst_buffer; 
	
	/* Check if type is memory or ALU */
	if (uop->wavefront->inst.info->fmt == SI_FMT_SMRD)
	{	
		/* Scalar memory read */
		if (!scalar_unit->mem_buffer)
		{
			//printf("    scalar read\n");
			scalar_unit->mem_buffer = uop;
		}
		else 
		{
			//printf("    scalar read stall\n");
			/* FIXME This stalls scalar ALU instructions */
			return;
		}
	}
	else
	{
		/* Scalar ALU */
		if (!scalar_unit->exec_buffer)
		{
			//printf("    scalar alu\n");
			scalar_unit->exec_buffer = uop;
		}
		else 
		{
			//printf("    scalar alu stall\n");
			/* FIXME This stalls scalar MEM instructions */
			return;
		}
	}

	/* Uop was processed, so remove it from instruction buffer */
	scalar_unit->inst_buffer = NULL;
}

void si_scalar_unit_run(struct si_scalar_unit_t *scalar_unit)
{
	//printf("scalar run (cycle %lld, cu %d)\n", si_gpu->cycle, scalar_unit->compute_unit->id);
	//printf(" wb\n");
	si_scalar_unit_writeback(scalar_unit);
	//printf(" exec\n");
	si_scalar_unit_execute(scalar_unit);
	//printf(" mem\n");
	si_scalar_unit_memory(scalar_unit);
	//printf(" read\n");
	si_scalar_unit_read(scalar_unit);
}

