#include <southern-islands-timing.h>

int si_gpu_simd_alu_latency = 4;
int si_gpu_simd_reg_latency = 1;
int si_gpu_simd_issue_width = 1;
int si_gpu_simd_num_subwavefronts = 4;

void si_simd_writeback(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	struct si_wavefront_t *wavefront;
	int list_count;

	/* Process completed ALU instructions */
	list_count = linked_list_count(simd->alu_out_buffer);
	linked_list_head(simd->alu_out_buffer);
	for (int i = 0; i < list_count; i++)
	{
		uop = linked_list_get(simd->alu_out_buffer);
		assert(uop);

		if (uop->execute_ready <= si_gpu->cycle)
		{
			/* Access complete, remove the uop from the queue */
			linked_list_remove(simd->alu_out_buffer);

			/* Make the wavefront active again */
			wavefront = uop->wavefront;
			wavefront->ready = 1;

			//printf("CYCLE[%lld]\t\tSIMD\t\t\tWRITEBACK: UOP.ID[%lld]\n", si_gpu->cycle, uop->id);

			/* Free uop */
			if (si_tracing())
				si_gpu_uop_trash_add(uop);
			else
				si_uop_free(uop);
		}
		else
		{
			linked_list_next(simd->alu_out_buffer);
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

	/* Look through the ALU execution buffer looking for wavefronts ready to execute */
	list_count = linked_list_count(simd->alu_exec_buffer);
	linked_list_head(simd->alu_exec_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_simd_issue_width)
			break;

		/* Peek at the first uop */
		uop = linked_list_get(simd->alu_exec_buffer);
		assert(uop);

		/* If the uop is now in the ALU pipeline, move it to the outstanding ALU buffer */
		if (si_gpu->cycle >= uop->read_ready && si_gpu->cycle == uop->alu_ready)
		{
			linked_list_remove(simd->alu_exec_buffer);
			linked_list_add(simd->alu_out_buffer, uop);
		}
		/* If the uop is ready to begin execution, set the ALU ready and execution
		 * ready cycles */
		else if (si_gpu->cycle == uop->read_ready)
		{
			uop->alu_ready = si_gpu->cycle + si_gpu_simd_num_subwavefronts;
			uop->execute_ready = uop->alu_ready + si_gpu_simd_alu_latency - 1;

			instructions_issued++;

			//printf("CYCLE[%lld]\t\tSIMD\t\t\tEXECUTE: UOP.ID[%lld]\n", si_gpu->cycle, uop->id);
		}
	}
}

void si_simd_read(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_count;
	int instructions_issued = 0;

	/* Look through the read buffer looking for wavefronts ready to issue */
	list_count = linked_list_count(simd->read_buffer);
	linked_list_head(simd->read_buffer);
	for (int i = 0; i < list_count; i++)
	{
		/* Stop if the issue width has been reached */
		if (instructions_issued == si_gpu_simd_issue_width)
			break;

		/* Peek at the first uop */
		uop = linked_list_get(simd->read_buffer);
		assert(uop);

		/* Stop if the uop has not been fully decoded yet. It is safe
		 * to assume that no other uop is ready either */
		if (si_gpu->cycle < uop->decode_ready)
			break;

		/* Issue the uop if the exec_buffer is not full */
		if (linked_list_count(simd->alu_exec_buffer) <=
				si_gpu_simd_issue_width)
		{
			uop->read_ready = si_gpu->cycle + si_gpu_simd_reg_latency;
			linked_list_remove(simd->read_buffer);
			linked_list_add(simd->alu_exec_buffer, uop);

			instructions_issued++;

			//printf("CYCLE[%lld]\t\tSIMD\t\t\tREAD: UOP.ID[%lld]\n", si_gpu->cycle, uop->id);
		}
		else
			break;
	}
}

void si_simd_run(struct si_simd_t *simd)
{
	/* SIMD stages */
	si_simd_writeback(simd);
	si_simd_execute(simd);
	si_simd_read(simd);
}
