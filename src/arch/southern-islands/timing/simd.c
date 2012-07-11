#include <southern-islands-timing.h>

int si_gpu_simd_issue_rate = 1;
int si_gpu_simd_latency = 4;

void si_simd_writeback(struct si_simd_t *simd)
{


}

void si_simd_execute(struct si_simd_t *simd)
{


}

void si_simd_read(struct si_simd_t *simd)
{


}

void si_simd_run(struct si_simd_t *simd)
{
	/* SIMD stages */
	si_simd_writeback(simd);
	si_simd_execute(simd);
	si_simd_read(simd);

	/* Stats */
	simd->cycle++;
}
