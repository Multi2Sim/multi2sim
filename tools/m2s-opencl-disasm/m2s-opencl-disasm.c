#include <misc.h>
#include <debug.h>
#include <gpudisasm.h>
#include <assert.h>



/*
 * Disassembler
 */

void hex_dump(void *base, void *from_ptr, void *to_ptr)
{
	unsigned int val;
	printf("0x%04x: ", (int) (from_ptr - base));
	while (from_ptr < to_ptr) {
		val = * (unsigned int *) from_ptr;
		printf("%04x ", val & 0xffff);
		from_ptr += 2;
	}
	printf("\n");
}


int main(int argc, char **argv)
{
	char *kernel;
	int size;

	char *prim_buf, *prim_buf_old;
	int prim_count;
	struct amd_inst_t cf_inst;
	int group_count;
	int loop_idx;

	/* Syntax */
	if (argc != 2)
		fatal("syntax: %s <isa_file>", argv[0]);
	
	/* Read file */
	kernel = read_buffer(argv[1], &size);
	if (!kernel)
		fatal("%s: cannot read file", argv[1]);
	
	/* Init */
	amd_disasm_init();
	
	/* Disassemble */
	prim_buf = kernel;
	prim_count = 0;
	group_count = 0;
	loop_idx = 0;
	while (prim_buf) {
		
		/* Decode and dump CF inst */
		prim_buf_old = prim_buf;
		prim_buf = amd_inst_decode_cf(prim_buf, &cf_inst);
		if (cf_inst.info->flags & AMD_INST_FLAG_DEC_LOOP_IDX) {
			assert(loop_idx > 0);
			loop_idx--;
		}
		hex_dump(kernel, prim_buf_old, prim_buf);
		amd_inst_dump(&cf_inst, prim_count, loop_idx, stdout);

		/* ALU clause */
		if (cf_inst.info->fmt[0] == FMT_CF_ALU_WORD0) {

			char *sec_buf, *sec_buf_end, *sec_buf_old;
			struct amd_alu_group_t alu_group;

			sec_buf = kernel + cf_inst.words[0].cf_alu_word0.addr * 8;
			sec_buf_end = sec_buf + (cf_inst.words[1].cf_alu_word1.count + 1) * 8;
			if (!IN_RANGE(sec_buf, kernel, kernel + size) || !IN_RANGE(sec_buf_end, kernel, kernel + size))
				fatal("ALU clause out of program space");
			while (sec_buf < sec_buf_end) {
				sec_buf_old = sec_buf;
				sec_buf = amd_inst_decode_alu_group(sec_buf, group_count, &alu_group);
				hex_dump(kernel, sec_buf_old, sec_buf);
				amd_alu_group_dump(&alu_group, loop_idx, stdout);
				group_count++;
			}
		}

		/* Fetch through a Texture Cache Clause */
		if (cf_inst.info->inst == AMD_INST_TC) {
			
			char *sec_buf, *sec_buf_end, *sec_buf_old;
			struct amd_inst_t inst;

			sec_buf = kernel + cf_inst.words[0].cf_word0.addr * 8;
			sec_buf_end = sec_buf + (cf_inst.words[1].cf_word1.count + 1) * 16;
			while (sec_buf < sec_buf_end) {
				sec_buf_old = sec_buf;
				sec_buf = amd_inst_decode_tc(sec_buf, &inst);
				hex_dump(kernel, sec_buf_old, sec_buf);
				amd_inst_dump(&inst, group_count, loop_idx, stdout);
				group_count++;
			}
		}

		/* Increase loop depth counter */
		if (cf_inst.info->flags & AMD_INST_FLAG_INC_LOOP_IDX)
			loop_idx++;

		/* Next CF inst */
		prim_count++;
	}
	printf("END_OF_PROGRAM\n");

	/* Free */
	free(kernel);
	amd_disasm_done();
	return 0;
}

