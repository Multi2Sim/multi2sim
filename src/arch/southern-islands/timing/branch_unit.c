#include <southern-islands-timing.h>

int si_gpu_branch_unit_reg_latency = 1;
int si_gpu_branch_unit_latency = 1;
int si_gpu_branch_unit_width = 1;

void si_branch_unit_writeback(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	struct si_wavefront_t *wavefront;
	int list_count;

	/* Process all completed instructions */
	list_count = linked_list_count(branch_unit->exec_buffer);
	linked_list_head(branch_unit->exec_buffer);
	for (int i = 0; i < list_count; i++)
	{
		uop = linked_list_get(branch_unit->exec_buffer);
		assert(uop);

		if (uop->execute_ready <= si_gpu->cycle)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(branch_unit->exec_buffer);

			si_trace("si.inst id=%lld cu=%d stg=\"bu-w\"\n", uop->id_in_compute_unit,
				branch_unit->compute_unit->id);

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
			linked_list_next(branch_unit->exec_buffer);
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

	/* Look through the read buffer looking for wavefronts ready to execute */
	list_count = linked_list_count(branch_unit->read_buffer);
	linked_list_head(branch_unit->read_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(branch_unit->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Stop if the issue width has been reached, stall */
		if (instructions_issued == si_gpu_branch_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				branch_unit->compute_unit->id);
			break;
		}

		/* Stall if the read_buffer is full. */
		if (linked_list_count(branch_unit->exec_buffer) >=
				si_gpu_branch_unit_width*si_gpu_branch_unit_latency)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				branch_unit->compute_unit->id);
			break;
		}

		/* Branch */
		uop->execute_ready = si_gpu->cycle +
			si_gpu_branch_unit_latency;

		/* Transfer the uop to the outstanding execution buffer */
		linked_list_remove(branch_unit->read_buffer);
		linked_list_add(branch_unit->exec_buffer, uop);

		instructions_issued++;
		branch_unit->inst_count++;
		branch_unit->wavefront_count++;

		si_trace("si.inst id=%lld cu=%d stg=\"bu-e\"\n", uop->id_in_compute_unit,
						branch_unit->compute_unit->id);
	}
}

void si_branch_unit_read(struct si_branch_unit_t *branch_unit)
{
	struct si_uop_t *uop;
	int instructions_issued = 0;
	int list_count;

	/* Look through the decode buffer looking for wavefronts ready to read */
	list_count = linked_list_count(branch_unit->decode_buffer);
	linked_list_head(branch_unit->decode_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(branch_unit->decode_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stall if the read_buffer is full. */
		if (linked_list_count(branch_unit->read_buffer) >=
				si_gpu_branch_unit_width*si_gpu_branch_unit_reg_latency)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				branch_unit->compute_unit->id);
			break;
		}

		/* Stop if the issue width has been reached, stall */
		if (instructions_issued == si_gpu_branch_unit_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				branch_unit->compute_unit->id);
			break;
		}

		/* Issue the uop */
		uop->read_ready = si_gpu->cycle + si_gpu_branch_unit_reg_latency;
		linked_list_remove(branch_unit->decode_buffer);
		linked_list_add(branch_unit->read_buffer, uop);

		instructions_issued++;

		si_trace("si.inst id=%lld cu=%d stg=\"bu-r\"\n", uop->id_in_compute_unit,
				branch_unit->compute_unit->id);
	}
}

void si_branch_unit_run(struct si_branch_unit_t *branch_unit)
{
	si_branch_unit_writeback(branch_unit);
	si_branch_unit_execute(branch_unit);
	si_branch_unit_read(branch_unit);
}
