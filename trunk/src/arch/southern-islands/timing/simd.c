#include <southern-islands-timing.h>

/* Note that the SIMD ALU latency is the latency for one SUBwavefront
 * to execute, NOT an entire wavefront. */
int si_gpu_simd_alu_latency = 2;
int si_gpu_simd_reg_latency = 1;
int si_gpu_simd_width = 1;
int si_gpu_simd_num_subwavefronts = 4;

void si_simd_writeback(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	struct si_wavefront_t *wavefront;
	int list_count;

	/* Process all completed ALU instructions */
	list_count = linked_list_count(simd->alu_buffer);
	linked_list_head(simd->alu_buffer);
	for (int i = 0; i < list_count; i++)
	{
		uop = linked_list_get(simd->alu_buffer);
		assert(uop);

		if (uop->execute_ready <= si_gpu->cycle)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(simd->alu_buffer);

			si_trace("si.inst id=%lld cu=%d stg=\"simd-w\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id);

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
			linked_list_next(simd->alu_buffer);
		}
	}

	/* Statistics */
	si_gpu->last_complete_cycle = esim_cycle;
}

void si_simd_execute(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_count;
	int instructions_issued = 0;

	/* Look through the alu_entry_buffer to move entered wavefronts to the alu_buffer.
	 * This is not a part of issuing instructions, it is simply moving instructions
	 * that have fully entered the ALU pipeline so that others may now begin to enter.
	 * It takes (#subwavefronts) cycles for a wavefront to fully enter. */
	list_count = linked_list_count(simd->alu_entry_buffer);
	linked_list_head(simd->alu_entry_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(simd->alu_entry_buffer);
		assert(uop);

		/* Stop if the uop is not yet fully entered. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->alu_ready)
			break;

		/* Stall if the alu_buffer is full. */
		if (linked_list_count(simd->alu_buffer) >=
				si_gpu_simd_width*(si_gpu_simd_alu_latency))
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id);
			break;
		}

		/* Move the uop */
		uop->execute_ready = uop->alu_ready + si_gpu_simd_alu_latency - 1;

		linked_list_remove(simd->alu_entry_buffer);
		linked_list_add(simd->alu_buffer, uop);
	}

	/* Look through the read buffer looking for wavefronts ready to enter the ALU */
	list_count = linked_list_count(simd->read_buffer);
	linked_list_head(simd->read_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(simd->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully read yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->read_ready)
			break;

		/* Stall if the alu_entry_buffer is full. */
		if (linked_list_count(simd->alu_entry_buffer) >=
				si_gpu_simd_width*si_gpu_simd_num_subwavefronts)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id);
			break;
		}

		/* Stop if the issue width has been reached, stall */
		if (instructions_issued == si_gpu_simd_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id);
			break;
		}

		/* Enter the uop into the ALU */
		uop->alu_ready = si_gpu->cycle + si_gpu_simd_num_subwavefronts;

		linked_list_remove(simd->read_buffer);
		linked_list_add(simd->alu_entry_buffer, uop);

		instructions_issued++;

		si_trace("si.inst id=%lld cu=%d stg=\"simd-e\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id);
	}
}

void si_simd_read(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_count;
	int instructions_issued = 0;

	/* Look through the decode buffer looking for wavefronts ready to read */
	list_count = linked_list_count(simd->decode_buffer);
	linked_list_head(simd->decode_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Peek at the first uop */
		uop = linked_list_get(simd->decode_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Stall if the read_buffer is full. */
		if (linked_list_count(simd->read_buffer) >=
				si_gpu_simd_width*si_gpu_simd_reg_latency)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id);
			break;
		}

		/* Stop if the issue width has been reached, stall */
		if (instructions_issued == si_gpu_lds_width)
		{
			si_trace("si.inst id=%lld cu=%d stg=\"s\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id);
			break;
		}

		/* Issue the uop */
		uop->read_ready = si_gpu->cycle + si_gpu_simd_reg_latency;
		linked_list_remove(simd->decode_buffer);
		linked_list_add(simd->read_buffer, uop);

		instructions_issued++;

		si_trace("si.inst id=%lld cu=%d stg=\"simd-r\"\n", uop->id_in_compute_unit,
			simd->compute_unit->id);
	}
}

void si_simd_run(struct si_simd_t *simd)
{
	/* SIMD stages */
	si_simd_writeback(simd);
	si_simd_execute(simd);
	si_simd_read(simd);
}
