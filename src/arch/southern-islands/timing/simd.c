#include <southern-islands-timing.h>

int si_gpu_simd_alu_latency = 4;
int si_gpu_simd_reg_latency = 1;
int si_gpu_simd_issue_width = 1;
int si_gpu_simd_num_subwavefronts = 4;

void si_simd_writeback(struct si_simd_t *simd)
{

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

		/* If the uop is in the ALU pipeline, move it to the outstanding ALU buffer */
		if (si_gpu->cycle >= uop->read_ready && si_gpu->cycle == uop->alu_ready)
		{
			linked_list_remove(simd->alu_exec_buffer);
			linked_list_add(simd->alu_out_buffer, uop);
		}
		/* If the uop is ready to be executed, set the ALU ready and execution
		 * ready cycles */
		else if (si_gpu->cycle == uop->read_ready)
		{
			uop->alu_ready = si_gpu->cycle + si_gpu_simd_num_subwavefronts;
			uop->execute_ready = uop->alu_ready + si_gpu_simd_alu_latency - 1;

			instructions_issued++;
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
