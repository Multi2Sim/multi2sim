#include <southern-islands-timing.h>

int si_gpu_vector_mem_issue_rate = 1;
int si_gpu_vector_mem_latency = 4;

void si_vector_mem_writeback(struct si_vector_mem_unit_t *vector_mem)
{


}

void si_vector_mem_execute(struct si_vector_mem_unit_t *vector_mem)
{


}

void si_vector_mem_read(struct si_vector_mem_unit_t *vector_mem)
{
	/* FIXME */

	struct si_uop_t *uop;
	uop = vector_mem->inst_buffer; 

	/* No instruction to process */
	if (!uop)
		return;

	/* Make the wavefront active again */
	uop->wavefront->ready = 1;

	vector_mem->inst_buffer = NULL;

	si_uop_free(uop);
}

void si_vector_mem_run(struct si_vector_mem_unit_t *vector_mem)
{
	/* SIMD stages */
	si_vector_mem_writeback(vector_mem);
	si_vector_mem_execute(vector_mem);
	si_vector_mem_read(vector_mem);

	/* Stats */
	vector_mem->cycle++;
}
