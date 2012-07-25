#include <southern-islands-timing.h>

int si_gpu_simd_latency = 4;
int si_gpu_simd_issue_width = 1;

void si_simd_writeback(struct si_simd_t *simd)
{


}

void si_simd_execute(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_count;

	list_count = linked_list_count(simd->exec_inst_buffer);

	/* No instruction to process */
	if (!list_count)
		return;

	linked_list_head(simd->exec_inst_buffer);
	uop = linked_list_get(simd->exec_inst_buffer);
	linked_list_remove(simd->exec_inst_buffer);

	/* Make the wavefront active again */
	uop->wavefront->ready = 1;

	si_uop_free(uop);
}

void si_simd_run(struct si_simd_t *simd)
{
	/* SIMD stages */
	si_simd_writeback(simd);
	si_simd_execute(simd);
}
