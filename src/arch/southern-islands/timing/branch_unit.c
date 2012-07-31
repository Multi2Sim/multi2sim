#include <southern-islands-timing.h>

int si_gpu_branch_unit_reg_latency = 1;
int si_gpu_branch_unit_latency = 1;
int si_gpu_branch_unit_issue_width = 1;

void si_branch_unit_writeback(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	struct si_wavefront_t *wavefront;
	int list_count;

	/* Process completed instructions */
	list_count = linked_list_count(branch_unit->out_buffer);
	linked_list_head(branch_unit->out_buffer);
	for (int i = 0; i < list_count; i++)
	{
		uop = linked_list_get(branch_unit->out_buffer);
		assert(uop);

		if (uop->execute_ready <= si_gpu->cycle)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(branch_unit->out_buffer);

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
			linked_list_next(branch_unit->out_buffer);
		}
	}

	/* Statistics */
	si_gpu->last_complete_cycle = esim_cycle;
}

void si_branch_unit_execute(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	int list_count;
	int instructions_issued = 0;

	/* Look through the execution buffer looking for wavefronts ready to execute */
	list_count = linked_list_count(branch_unit->exec_buffer);
	linked_list_head(branch_unit->exec_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_branch_unit_issue_width)
			break;

		/* Peek at the first uop */
		uop = linked_list_get(branch_unit->exec_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Branch */
		uop->execute_ready = si_gpu->cycle +
			si_gpu_branch_unit_latency;

		/* Transfer the uop to the outstanding execution buffer */
		linked_list_remove(branch_unit->exec_buffer);
		linked_list_add(branch_unit->out_buffer, uop);

		instructions_issued++;
		branch_unit->inst_count++;
		branch_unit->wavefront_count++;
	}
}

void si_branch_unit_read(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	int instructions_issued = 0;
	int list_count;

	/* Look through the read buffer looking for wavefronts ready to issue */
	list_count = linked_list_count(branch_unit->read_buffer);
	linked_list_head(branch_unit->read_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_branch_unit_issue_width)
			break;

		/* Peek at the first uop */
		uop = linked_list_get(branch_unit->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Issue the uop, no need to check if the exec_buffers are full */
		uop->read_ready = si_gpu->cycle + si_gpu_branch_unit_reg_latency;
		linked_list_remove(branch_unit->read_buffer);
		linked_list_add(branch_unit->exec_buffer, uop);

		instructions_issued++;
	}
}

void si_branch_unit_run(struct si_branch_unit_t *branch_unit)
{
	si_branch_unit_writeback(branch_unit);
	si_branch_unit_execute(branch_unit);
	si_branch_unit_read(branch_unit);
}
