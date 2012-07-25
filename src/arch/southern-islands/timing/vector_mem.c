#include <southern-islands-timing.h>

int si_gpu_vector_mem_issue_width = 1;


void si_vector_mem_writeback(struct si_vector_mem_unit_t *vector_mem)
{


}

void si_vector_mem_execute(struct si_vector_mem_unit_t *vector_mem)
{
	struct si_uop_t *uop;
	int list_count;

	list_count = linked_list_count(vector_mem->exec_inst_buffer);

	/* No instruction to process */
	if (!list_count)
		return;

	linked_list_head(vector_mem->exec_inst_buffer);
	uop = linked_list_get(vector_mem->exec_inst_buffer);
	linked_list_remove(vector_mem->exec_inst_buffer);

	/* Make the wavefront active again */
	uop->wavefront->ready = 1;

	si_uop_free(uop);
}

void si_vector_mem_run(struct si_vector_mem_unit_t *vector_mem)
{
	/* SIMD stages */
	si_vector_mem_writeback(vector_mem);
	si_vector_mem_execute(vector_mem);
}
