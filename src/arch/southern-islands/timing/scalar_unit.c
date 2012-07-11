#include <southern-islands-timing.h>

int si_gpu_scalar_unit_issue_rate = 1;
int si_gpu_scalar_unit_latency = 1;

void si_scalar_unit_run(struct si_scalar_unit_t *scalar_unit)
{
	struct si_uop_t *uop;

	if (!scalar_unit->inst_buffer)
		return;

	/* Process uop */
	uop = scalar_unit->inst_buffer; 
	uop->execute_ready = si_gpu->cycle + si_gpu_scalar_unit_latency;

	scalar_unit->inst_buffer = NULL;
}
