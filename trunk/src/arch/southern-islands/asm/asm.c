/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <misc.h>
#include <assert.h>
#include <string.h>
#include <debug.h>
#include <ctype.h>
#include <elf-format.h>
#include <mhandle.h>

#include "southern-islands-asm.h"



/* Table containing information of all instructions */
static struct si_inst_info_t si_inst_info[SI_INST_COUNT];

/* Defnine the number of valid opcodes. */
#define SI_INST_INFO_SOPP_OPCODE_SIZE 23
#define SI_INST_INFO_SOP1_OPCODE_SIZE 54
#define SI_INST_INFO_SOP2_OPCODE_SIZE 45
#define SI_INST_INFO_SMRD_OPCODE_SIZE 32
#define SI_INST_INFO_VOP3_OPCODE_SIZE 373
#define SI_INST_INFO_VOPC_OPCODE_SIZE 184
#define SI_INST_INFO_VOP1_OPCODE_SIZE 69
#define SI_INST_INFO_VOP2_OPCODE_SIZE 50
#define SI_INST_INFO_MTBUF_OPCODE_SIZE 8

/* String lengths for printing assembly */
#define MAX_INST_STR_SIZE 150
#define MAX_OPERAND_STR_SIZE 10
#define MAX_DAT_STR_SIZE 31

/* Pointers to 'si_inst_info' table indexed by instruction opcode */
static struct si_inst_info_t *si_inst_info_sopp[SI_INST_INFO_SOPP_OPCODE_SIZE];
static struct si_inst_info_t *si_inst_info_sop1[SI_INST_INFO_SOP1_OPCODE_SIZE];
static struct si_inst_info_t *si_inst_info_sop2[SI_INST_INFO_SOP2_OPCODE_SIZE];
static struct si_inst_info_t *si_inst_info_smrd[SI_INST_INFO_SMRD_OPCODE_SIZE];
static struct si_inst_info_t *si_inst_info_vop3[SI_INST_INFO_VOP3_OPCODE_SIZE];
static struct si_inst_info_t *si_inst_info_vopc[SI_INST_INFO_VOPC_OPCODE_SIZE];
static struct si_inst_info_t *si_inst_info_vop1[SI_INST_INFO_VOP1_OPCODE_SIZE];
static struct si_inst_info_t *si_inst_info_vop2[SI_INST_INFO_VOP2_OPCODE_SIZE];
static struct si_inst_info_t *si_inst_info_mtbuf[SI_INST_INFO_MTBUF_OPCODE_SIZE];

/* Helper functions to dump assembly code to a file */
void si_inst_dump_sopp(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f, int* new_label);
void si_inst_dump_sop1(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f);
void si_inst_dump_sop2(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f);
void si_inst_dump_smrd(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f);
void si_inst_dump_vop3(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f);
void si_inst_dump_vopc(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f);
void si_inst_dump_vop1(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f);
void si_inst_dump_vop2(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f);
void si_inst_dump_mtbuf(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f);

/* String maps for assembly dump. */
struct string_map_t sdst_map = {
	24, {
		{ "reserved", 0 },
		{ "reserved", 1 },
		{ "vcc_lo", 2 },
		{ "vcc_hi", 3 },
		{ "tba_lo", 4 },
		{ "tba_hi", 5 },
		{ "tma_lo", 6 },
		{ "tma_hi", 7 },
		{ "ttmp0", 8 },
		{ "ttmp1", 9 },
		{ "ttmp2", 10 },
		{ "ttmp3", 11 },
		{ "ttmp4", 12 },
		{ "ttmp5", 13 },
		{ "ttmp6", 14 },
		{ "ttmp7", 15 },
		{ "ttmp8", 16 },
		{ "ttmp9", 17 },
		{ "ttmp10", 18 },
		{ "ttmp11", 19 },
		{ "m0", 20 },
		{ "reserved", 21 },
		{ "exec_lo", 22 },
		{ "exec_hi", 23 }
	}
};

struct string_map_t ssrc_map = {
	16, {
		{ "0.5", 0 },
		{ "-0.5", 1 },
		{ "1.0", 2 },
		{ "-1.0", 3 },
		{ "2.0", 4 },
		{ "-2.0", 5 },
		{ "4.0", 6 },
		{ "-4.0", 7 },
		{ "reserved", 8 },
		{ "reserved", 9 },
		{ "reserved", 10 },
		{ "vccz", 11 },
		{ "execz", 12 },
		{ "scc", 13 },
		{ "reserved", 14 },
		{ "literal constant", 15 }
	}
};

struct string_map_t dfmt_map = {
	16, {
		{ "invalid", 0 },
		{ "BUF_DATA_FORMAT_8", 1 },
		{ "BUF_DATA_FORMAT_16", 2 },
		{ "BUF_DATA_FORMAT_8_8", 3 },
		{ "BUF_DATA_FORMAT_32", 4 },
		{ "BUF_DATA_FORMAT_16_16", 5 },
		{ "BUF_DATA_FORMAT_10_11_11", 6 },
		{ "BUF_DATA_FORMAT_11_10_10", 7 },
		{ "BUF_DATA_FORMAT_10_10_10_2", 8 },
		{ "BUF_DATA_FORMAT_2_10_10_10", 9 },
		{ "BUF_DATA_FORMAT_8_8_8_8", 10 },
		{ "BUF_DATA_FORMAT_32_32", 11 },
		{ "BUF_DATA_FORMAT_16_16_16_16", 12 },
		{ "BUF_DATA_FORMAT_32_32_32", 13 },
		{ "BUF_DATA_FORMAT_32_32_32_32", 14 },
		{ "reserved", 15 }
	}
};

struct string_map_t nfmt_map = {
	14, {
		{ "BUF_NUM_FMT_UNORM", 0 },
		{ "BUF_NUM_FMT_SNORM", 1 },
		{ "BUF_NUM_FMT_UNSCALED", 2 },
		{ "BUF_NUM_FMT_SSCALED", 3 },
		{ "BUF_NUM_FMT_UINT", 4 },
		{ "BUF_NUM_FMT_SINT", 5 },
		{ "BUF_NUM_FMT_SNORM_NZ", 6 },
		{ "BUF_NUM_FMT_FLOAT", 7 },
		{ "reserved", 8 },
		{ "BUF_NUM_FMT_SRGB", 9 },
		{ "BUF_NUM_FMT_UBNORM", 10 },
		{ "BUF_NUM_FMT_UBNORM_NZ", 11 },
		{ "BUF_NUM_FMT_UBINT", 12 },
		{ "BUF_NUM_FMT_UBSCALED", 13 }
	}
};

/*
 * Initialization/finalization of disassembler
 */

void si_disasm_init()
{
	struct si_inst_info_t *info;
	int i;

	/* Type size assertions */
	assert(sizeof(union si_reg_t) == 4);

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size) \
	info = &si_inst_info[SI_INST_##_name]; \
	info->inst = SI_INST_##_name; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->fmt = SI_FMT_##_fmt; \
	info->opcode = _opcode; \
	info->size = _size;
#include "southern-islands-asm.dat"
#undef DEFINST
	
	/* Tables of pointers to 'si_inst_info' */
	for (i = 1; i < SI_INST_COUNT; i++)
	{
		info = &si_inst_info[i];

		if (info->fmt == SI_FMT_SOPP)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_SOPP_OPCODE_SIZE - 1));
			si_inst_info_sopp[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOP1)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_SOP1_OPCODE_SIZE - 1));
			si_inst_info_sop1[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOP2)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_SOP2_OPCODE_SIZE - 1));
			si_inst_info_sop2[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SMRD) 
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_SMRD_OPCODE_SIZE - 1));
			si_inst_info_smrd[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOP3a || info->fmt == SI_FMT_VOP3b)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_VOP3_OPCODE_SIZE - 1));
			si_inst_info_vop3[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOPC)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_VOPC_OPCODE_SIZE - 1));
			si_inst_info_vopc[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOP1)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_VOP1_OPCODE_SIZE - 1));
			si_inst_info_vop1[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOP2)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_VOP2_OPCODE_SIZE - 1));
			si_inst_info_vop2[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_MTBUF)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_MTBUF_OPCODE_SIZE - 1));
			si_inst_info_mtbuf[info->opcode] = info;
			continue;
		}
		/* TODO FILL IN REMAINING FORMATS */
		else 
		{
			fprintf(stderr, "warning: '%s' not indexed\n", info->name);
		}
	}
}


void si_disasm_done()
{

}



/*
 * Functions for decoding instructions
 */

int si_inst_decode(void *buf, struct si_inst_t *inst) 
{
	/* Zero-out instruction structure */
	memset(inst, 0, sizeof(struct si_inst_t));

	/* All instructions will be at least 32-bits */
	memcpy(&inst->micro_inst, buf, 4);

	/* Use the encoding field to determine the instruction type */
	if (inst->micro_inst.sopp.enc == 0x17F)
	{
		assert(si_inst_info_sopp[inst->micro_inst.sopp.op]);
		inst->info = si_inst_info_sopp[inst->micro_inst.sopp.op];
	}
	else if (inst->micro_inst.sop1.enc == 0x17D)
	{
		assert(si_inst_info_sop1[inst->micro_inst.sop1.op]);
		inst->info = si_inst_info_sop1[inst->micro_inst.sop1.op];

		/* 0xFF indicates the use of a literal constant as a source operand. */
		if (inst->micro_inst.sop1.ssrc0 == 0xFF)
		{
			memcpy(&inst->micro_inst, buf, 8);
			inst->info->size = 8;
		}	
	}
	else if (inst->micro_inst.sop2.enc == 0x2)
	{
		assert(si_inst_info_sop2[inst->micro_inst.sop2.op]);
		inst->info = si_inst_info_sop2[(inst->micro_inst).sop2.op];

		/* Only one source field may use a literal constant, which is indicated by 0xFF. */
		assert(!(inst->micro_inst.sop2.ssrc0 == 0xFF && inst->micro_inst.sop2.ssrc1 == 0xFF));
		if (inst->micro_inst.sop2.ssrc0 == 0xFF || inst->micro_inst.sop2.ssrc1 == 0xFF)
		{
			memcpy(&inst->micro_inst, buf, 8);
			inst->info->size = 8;
		}
	}
	else if (inst->micro_inst.smrd.enc == 0x18) 
	{
		assert(si_inst_info_smrd[inst->micro_inst.smrd.op]);
		inst->info = si_inst_info_smrd[inst->micro_inst.smrd.op];
	}
	else if (inst->micro_inst.vop3a.enc == 0x34)
	{
		/* 64 bit instruction. */
		memcpy(&inst->micro_inst, buf, 8);
		assert(si_inst_info_vop3[inst->micro_inst.vop3a.op]);
		inst->info = si_inst_info_vop3[inst->micro_inst.vop3a.op];
	}
	else if (inst->micro_inst.vopc.enc == 0x3E)
	{
		assert(si_inst_info_vopc[inst->micro_inst.vopc.op]);
		inst->info = si_inst_info_vopc[inst->micro_inst.vopc.op];

		/* 0xFF indicates the use of a literal constant as a source operand. */
		if (inst->micro_inst.vopc.src0 == 0xFF)
		{
			memcpy(&inst->micro_inst, buf, 8);
			inst->info->size = 8;
		}
	}
	else if (inst->micro_inst.vop1.enc == 0x3F)
	{
		assert(si_inst_info_vop1[inst->micro_inst.vop1.op]);
		inst->info = si_inst_info_vop1[inst->micro_inst.vop1.op];

		/* 0xFF indicates the use of a literal constant as a source operand. */
		if (inst->micro_inst.vop1.src0 == 0xFF)
		{
			memcpy(&inst->micro_inst, buf, 8);
			inst->info->size = 8;
		}
	}
	else if (inst->micro_inst.vop2.enc == 0x0)
	{
		assert(si_inst_info_vop2[inst->micro_inst.vop2.op]);
		inst->info = si_inst_info_vop2[inst->micro_inst.vop2.op];

		/* 0xFF indicates the use of a literal constant as a source operand. */
		if (inst->micro_inst.vop2.src0 == 0xFF)
		{
			memcpy(&inst->micro_inst, buf, 8);
			inst->info->size = 8;
		}
	}
	else if (inst->micro_inst.mtbuf.enc == 0x3A)
	{
		/* 64 bit instruction. */
		memcpy(&inst->micro_inst, buf, 8);
		assert(si_inst_info_mtbuf[inst->micro_inst.mtbuf.op]);
		inst->info = si_inst_info_mtbuf[inst->micro_inst.mtbuf.op];
	}
	/* TODO FILL IN REMAINING FORMATS */
	else 
	{
		fatal("Unimplemented format. Instruction is: %08X\n", ((unsigned int*)buf)[0]);
	}

	return inst->info->size;
}

void si_disasm_buffer(struct elf_buffer_t *buffer, FILE *f)
{
	void *inst_buf = buffer->ptr;
	int inst_count = 0;
	int rel_addr = 0;

	int label_addr[buffer->size / 4]; /* A list of created labels sorted by rel_addr. */
	int* next_label = &label_addr[0]; /* The next label to dump. */
	int* end_label = &label_addr[0]; /* The address after the last label. */
	*next_label = -1;
	*end_label = -1;

	/* Disassemble */
	while (inst_buf)
	{
		struct si_inst_t inst;
		int inst_size;

		/* Parse the instruction */
	       	inst_size = si_inst_decode(inst_buf, &inst);

		inst_count++;

		/* Dump a label if necessary. */
		if (*next_label == rel_addr)
		{
			fprintf(f, "label_%04X:\n", rel_addr / 4);
			next_label++;
		}
		
		/* Dump the instruction */
		if (inst.info->fmt == SI_FMT_SOPP)
		{
			int label;	
			si_inst_dump_sopp(&inst, rel_addr, inst_buf, f, &label);

			/* Remember new label if created (keeping label list sorted by rel_addr). */
			if (label >= 0)
			{
				/* Find position to insert label. */
				int* t_label = next_label;
				while(label > *t_label && t_label < end_label) t_label++;

				/* Shift labels after position down. */
				int* t2_label = t_label;
				while(t2_label < end_label)
				{
					*(t2_label + 1) = *t2_label;
					t2_label++;
				}
				end_label++;

				/* Insert the new label. */
				*t_label = label;
			}

			/* Break at end of program. */
			if (inst.micro_inst.sopp.op == 1)
				break;
		}
		else if (inst.info->fmt == SI_FMT_SOP1)
		{
			si_inst_dump_sop1(&inst, rel_addr, inst_buf, f);
		}
		else if (inst.info->fmt == SI_FMT_SOP2)
		{
			si_inst_dump_sop2(&inst, rel_addr, inst_buf, f);
		}
		else if (inst.info->fmt == SI_FMT_SMRD) 
		{
			si_inst_dump_smrd(&inst, rel_addr, inst_buf, f);
		}
		else if (inst.info->fmt == SI_FMT_VOP3a || inst.info->fmt == SI_FMT_VOP3b)
		{
			si_inst_dump_vop3(&inst, rel_addr, inst_buf, f);
		}
		else if (inst.info->fmt == SI_FMT_VOPC)
		{
			si_inst_dump_vopc(&inst, rel_addr, inst_buf, f);
		}
		else if (inst.info->fmt == SI_FMT_VOP1)
		{
			si_inst_dump_vop1(&inst, rel_addr, inst_buf, f);
		}
		else if (inst.info->fmt == SI_FMT_VOP2)
		{
			si_inst_dump_vop2(&inst, rel_addr, inst_buf, f);
		}
		else if (inst.info->fmt == SI_FMT_MTBUF)
		{
			si_inst_dump_mtbuf(&inst, rel_addr, inst_buf, f);
		}
		/* TODO FILL IN REMAINING FORMATS */
		else 
		{
			fatal("Unknown instruction: %08X\n", ((unsigned int*)inst_buf)[0]);
		}

		/* Increment instruction pointer */
		inst_buf += inst_size;
		rel_addr += inst_size;
	}

}




/* 
 * Functions to print assembly output to file
 */

static int is_token(char *fmt_str, char *token_str, int *token_len)
{
	*token_len = strlen(token_str);
	return !strncmp(fmt_str, token_str, *token_len) &&
		!isalnum(fmt_str[*token_len]);
}

void operand_dump(char* str, int operand)
{
	/* Assume operand in range. */
	assert(operand >= 0 && operand <= 511);

	int str_size = MAX_OPERAND_STR_SIZE;
	char *pstr = str;
	if (operand <= 103)
	{
		/* SGPR */
		str_printf(&pstr, &str_size, "s%d", operand);
	}
	else if (operand <= 127)
	{
		/* sdst special registers */
		str_printf(&pstr, &str_size, "%s", map_value(&sdst_map, operand - 104));
	}
	else if (operand <= 192)
	{
		/* Positive integer constant */
		str_printf(&pstr, &str_size, "%d", operand - 128);
	}
	else if (operand <= 208)
	{
		/* Negative integer constant */
		str_printf(&pstr, &str_size, "-%d", operand - 192);
	}
	else if (operand <= 239)
	{
		fatal("Operand code unused.");
	}
	else if (operand <= 255)
	{
		str_printf(&pstr, &str_size, "%s", map_value(&ssrc_map, operand - 240));
	}
	else if (operand <= 511)
	{
		/* VGPR */
		str_printf(&pstr, &str_size, "v%d", operand - 256);
	}
}

void operand_dump_series(char* str, int operand, int operand_end)
{
	assert(operand <= operand_end);
	if (operand == operand_end)
	{
		operand_dump(str, operand);
	}

	int str_size = MAX_OPERAND_STR_SIZE;
	char* pstr = str;
	if (operand <= 103)
	{
		str_printf(&pstr, &str_size, "s[%d:%d]", operand, operand_end);
	}
	else if (operand <= 127)
	{
		if (operand >= 112 && operand <= 123)
		{
			assert(operand_end <= 123);
			str_printf(&pstr, &str_size, "ttmp[%d:%d]", operand - 112, operand_end - 112);
		}
		else
		{
			assert(operand_end == operand + 1);
			switch (operand)
			{
				case 106:
					str_printf(&pstr, &str_size, "vcc");
					break;
				case 108:
					str_printf(&pstr, &str_size, "tba");
					break;
				case 110:
					str_printf(&pstr, &str_size, "tma");
					break;
				case 126:
					str_printf(&pstr, &str_size, "exec");
					break;
				default:
					fatal("Unimplemented operand series: [%d:%d]", operand, operand_end);
			}
		}
	}
	else if (operand <= 255)
	{
		fatal("Illegal operand series: [%d:%d]", operand, operand_end);
	}
	else if (operand <= 511)
	{
		str_printf(&pstr, &str_size, "v[%d:%d]", operand, operand_end);
	}
}

void line_dump(char *inst_str, unsigned int rel_addr, void* buf, FILE *f, int inst_size)
{
	int dat_str_size = MAX_DAT_STR_SIZE;
	char inst_dat_str[MAX_DAT_STR_SIZE];
	char* dat_str = &inst_dat_str[0];
	if (inst_size == 4)
	{
		str_printf(&dat_str, &dat_str_size, "// %08X: %08X", rel_addr, ((unsigned int*)buf)[0]);
	}
	else
	{
		str_printf(&dat_str, &dat_str_size, "// %08X: %08X %08X", rel_addr, ((unsigned int*)buf)[0], ((unsigned int*)buf)[1]);
	}

	if(strlen(inst_str) <= 60)
	{
		fprintf(f, "%-60s%s\n", inst_str, inst_dat_str);
	}
	else
	{
		fprintf(f, "%s %s\n", inst_str, inst_dat_str);
	}
}

void si_inst_dump_sopp(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f, int* new_label)
{
	struct si_fmt_sopp_t *sopp = &inst->micro_inst.sopp;

	*new_label = -1;

	int str_size = MAX_INST_STR_SIZE;
	char orig_inst_str[MAX_INST_STR_SIZE];
	char *inst_str = &orig_inst_str[0];
	char *fmt_str = inst->info->fmt_str;
	int token_len;
	while (*fmt_str)
	{
		/* Literal */
		if (*fmt_str != '%')
		{
			str_printf(&inst_str, &str_size, "%c", *fmt_str);
			fmt_str++;
			continue;
		}

		/* Token */
		fmt_str++;
		if (is_token(fmt_str, "WAIT_CNT", &token_len))
		{
			unsigned int and = 0;

			int vm_cnt = (sopp->simm16 & 0xF);
			if (vm_cnt != 0xF)
			{
				str_printf(&inst_str, &str_size, "vmcnt(%d)", vm_cnt);
				and = 1;
			}

			int lgkm_cnt = (sopp->simm16 & 0x1f00) >> 8;
			if (lgkm_cnt != 0x1f)
			{
				if(and)
				{
					str_printf(&inst_str, &str_size, " & ");
				}

				str_printf(&inst_str, &str_size, "lgkmcnt(%d)", lgkm_cnt);
				and = 1;
			}
				
			int excount = (sopp->simm16 & 0x70) >> 4;
			if (excount != 0x7)
			{
				/*TODO: Implement excount field in s_waitcnt. */
				fatal("Instruction implementation incomplete.");
			}
		}
		else if (is_token(fmt_str, "LABEL", &token_len))
		{
			*new_label = rel_addr + (sopp->simm16 * 4) + 4;
			str_printf(&inst_str, &str_size, "label_%04X", *new_label / 4);
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}
	
	line_dump(orig_inst_str, rel_addr, buf, f, inst->info->size);
}

void si_inst_dump_sop1(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f)
{
	struct si_fmt_sop1_t *sop1 = &inst->micro_inst.sop1;

	int str_size = MAX_INST_STR_SIZE;
	char orig_inst_str[MAX_INST_STR_SIZE];
	char *inst_str = &orig_inst_str[0];
	char *fmt_str = inst->info->fmt_str;
	int token_len;
	char operand_str[MAX_OPERAND_STR_SIZE];
	while (*fmt_str)
	{
		/* Literal */
		if (*fmt_str != '%')
		{
			str_printf(&inst_str, &str_size, "%c", *fmt_str);
			fmt_str++;
			continue;
		}

		/* Token */
		fmt_str++;
		if (is_token(fmt_str, "64_SDST", &token_len))
		{
			operand_dump_series(operand_str, sop1->sdst, sop1->sdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_SSRC0", &token_len))
		{
			if (sop1->ssrc0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "%08x", sop1->lit_cnst);
			}
			else
			{
				operand_dump_series(operand_str, sop1->ssrc0, sop1->ssrc0 + 1);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}
	
	line_dump(orig_inst_str, rel_addr, buf, f, inst->info->size);
}

void si_inst_dump_sop2(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f)
{
	struct si_fmt_sop2_t *sop2 = &inst->micro_inst.sop2;
	
	int str_size = MAX_INST_STR_SIZE;
	char orig_inst_str[MAX_INST_STR_SIZE];
	char *inst_str = &orig_inst_str[0];
	char *fmt_str = inst->info->fmt_str;
	int token_len;
	char operand_str[MAX_OPERAND_STR_SIZE];
	while (*fmt_str)
	{
		/* Literal */
		if (*fmt_str != '%')
		{
			str_printf(&inst_str, &str_size, "%c", *fmt_str);
			fmt_str++;
			continue;
		}

		/* TODO: add tokens for s_and_b64. */

		/* Token */
		fmt_str++;
		if (is_token(fmt_str, "SDST", &token_len))
		{
			operand_dump(operand_str, sop2->sdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_SDST", &token_len))
		{
			operand_dump_series(operand_str, sop2->sdst, sop2->sdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SSRC0", &token_len))
		{
			if (sop2->ssrc0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "%08x", sop2->lit_cnst);
			}
			else
			{
				operand_dump(operand_str, sop2->ssrc0);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "64_SSRC0", &token_len))
		{
			if (sop2->ssrc0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "%08x", sop2->lit_cnst);
			}
			else
			{
				operand_dump_series(operand_str, sop2->ssrc0, sop2->ssrc0 + 1);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "SSRC1", &token_len))
		{
			if (sop2->ssrc1 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "%08x", sop2->lit_cnst);
			}
			else
			{
				operand_dump(operand_str, sop2->ssrc1);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "64_SSRC1", &token_len))
		{
			if (sop2->ssrc1 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "%08x", sop2->lit_cnst);
			}
			else
			{
				operand_dump_series(operand_str, sop2->ssrc1, sop2->ssrc1 + 1);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, f, inst->info->size);
}

void si_inst_dump_smrd(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f)
{
	struct si_fmt_smrd_t *smrd = &inst->micro_inst.smrd;

	/* The sbase field is missing the LSB, so multiply by 2 */
	int sdst = smrd->sdst;
	int sdst_end;
	int sbase = smrd->sbase*2;
	int sbase_end;

	/* TODO Handle the case where 'imm' is 0 */
	assert(smrd->imm); 

	/* S_LOAD_DWORD */
	if (IN_RANGE(smrd->op, 0, 4))
	{
		/* SBASE specifies two consecutive SGPRs */
		sbase_end = sbase + 1;
		
		if (smrd->op != 0) 
		{
			/* Multi-dword */
			switch (smrd->op)
			{
			case 1:
				sdst_end = sdst+1;
				break;
			case 2:
				sdst_end = sdst+3;
				break;
			case 3:
				sdst_end = sdst+7;
				break;
			case 4:
				sdst_end = sdst+15;
				break;
			default:
				assert("Invalid smrd opcode");
			}
		}
	}
	/* S_BUFFER_LOAD_DWORD */
	else if (IN_RANGE(smrd->op, 8, 12))
	{
		/* SBASE specifies four consecutive SGPRs */
		sbase_end = sbase + 3;

		if (smrd->op != 8) 
		{
			/* Multi-dword */
			switch (smrd->op)
			{
			case 9:
				sdst_end = sdst+1;
				break;
			case 10:
				sdst_end = sdst+3;
				break;
			case 11:
				sdst_end = sdst+7;
				break;
			case 12:
				sdst_end = sdst+15;
				break;
			default:
				assert("Invalid smrd opcode");
			}
		}
	}
	/* S_MEMTIME */
	else if(smrd->op == 30) 
	{

	}
	/* S_DCACHE_INV */
	else if(smrd->op == 31) 
	{

	}

	/* Write the assembly to file */
	
	int str_size = MAX_INST_STR_SIZE;
	char orig_inst_str[MAX_INST_STR_SIZE];
	char *inst_str = &orig_inst_str[0];
	char *fmt_str = inst->info->fmt_str;
	int token_len;
	char operand_str[MAX_OPERAND_STR_SIZE];
	while (*fmt_str)
	{
		/* Literal */
		if (*fmt_str != '%')
		{
			str_printf(&inst_str, &str_size, "%c", *fmt_str);
			fmt_str++;
			continue;
		}

		/* Token */
		fmt_str++;
		if (is_token(fmt_str, "SDST", &token_len))
		{
			operand_dump(operand_str, sdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SERIES_SDST", &token_len))
		{
			operand_dump_series(operand_str, sdst, sdst_end);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SERIES_SBASE", &token_len))
		{
			operand_dump_series(operand_str, sbase, sbase_end);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "OFFSET", &token_len))
		{
			str_printf(&inst_str, &str_size, "%02x", smrd->offset);
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}
		
		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, f, inst->info->size);
}

void si_inst_dump_vop3(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f)
{
	int str_size = MAX_INST_STR_SIZE;
	char orig_inst_str[MAX_INST_STR_SIZE];
	char *inst_str = &orig_inst_str[0];
	char *fmt_str = inst->info->fmt_str;
	int token_len;
	char operand_str[MAX_OPERAND_STR_SIZE];
	while (*fmt_str)
	{
		/* Literal */
		if (*fmt_str != '%')
		{
			str_printf(&inst_str, &str_size, "%c", *fmt_str);
			fmt_str++;
			continue;
		}

		/* Token */
		fmt_str++;
		if (is_token(fmt_str, "64_VDST", &token_len))
		{
			operand_dump_series(operand_str, inst->micro_inst.vop3a.vdst, inst->micro_inst.vop3a.vdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VDST", &token_len))
		{
			operand_dump(operand_str, inst->micro_inst.vop3a.vdst + 256);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SRC0", &token_len))
		{
			operand_dump(operand_str, inst->micro_inst.vop3a.src0);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SRC1", &token_len))
		{
			operand_dump(operand_str, inst->micro_inst.vop3a.src1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SRC2", &token_len))
		{
			operand_dump(operand_str, inst->micro_inst.vop3a.src2);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "0NEG", &token_len))
		{
			if (inst->micro_inst.vop3a.neg & 1)
			{
				str_printf(&inst_str, &str_size, "-");
			}
		}
		else if (is_token(fmt_str, "1NEG", &token_len))
		{
			if (inst->micro_inst.vop3a.neg & 2)
			{
				str_printf(&inst_str, &str_size, "-");
			}	
		}
		else if (is_token(fmt_str, "2NEG", &token_len))
		{
			if (inst->micro_inst.vop3a.neg & 4)
			{
				str_printf(&inst_str, &str_size, "-");
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, f, inst->info->size);
}

void si_inst_dump_vopc(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f)
{
	struct si_fmt_vopc_t *vopc = &inst->micro_inst.vopc;
	
	int str_size = MAX_INST_STR_SIZE;
	char orig_inst_str[MAX_INST_STR_SIZE];
	char *inst_str = &orig_inst_str[0];
	char *fmt_str = inst->info->fmt_str;
	int token_len;
	char operand_str[MAX_OPERAND_STR_SIZE];
	while (*fmt_str)
	{
		/* Literal */
		if (*fmt_str != '%')
		{
			str_printf(&inst_str, &str_size, "%c", *fmt_str);
			fmt_str++;
			continue;
		}

		/* Token */
		fmt_str++;
		if (is_token(fmt_str, "SRC0", &token_len))
		{
			if (vopc->src0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "%08x", vopc->lit_cnst);
			}
			else
			{
				operand_dump(operand_str, vopc->src0);
				str_printf(&inst_str, &str_size, "%s", operand_str);	
			}
		}
		else if (is_token(fmt_str, "VSRC1", &token_len))
		{
			operand_dump(operand_str, vopc->vsrc1 + 256);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}
	
	line_dump(orig_inst_str, rel_addr, buf, f, inst->info->size);
}

void si_inst_dump_vop1(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f)
{
	struct si_fmt_vop1_t *vop1 = &inst->micro_inst.vop1;
	
	int str_size = MAX_INST_STR_SIZE;
	char orig_inst_str[MAX_INST_STR_SIZE];
	char *inst_str = &orig_inst_str[0];
	char *fmt_str = inst->info->fmt_str;
	int token_len;
	char operand_str[MAX_OPERAND_STR_SIZE];
	while (*fmt_str)
	{
		/* Literal */
		if (*fmt_str != '%')
		{
			str_printf(&inst_str, &str_size, "%c", *fmt_str);
			fmt_str++;
			continue;
		}

		/* Token */
		fmt_str++;
		if (is_token(fmt_str, "VDST", &token_len))
		{
			operand_dump(operand_str, vop1->vdst + 256);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SRC0", &token_len))
		{
			if (vop1->src0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "%08x", vop1->lit_cnst);
			}
			else
			{
				operand_dump(operand_str, vop1->src0);
				str_printf(&inst_str, &str_size, "%s", operand_str);	
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}
	
	line_dump(orig_inst_str, rel_addr, buf, f, inst->info->size);
}

void si_inst_dump_vop2(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f)
{
	struct si_fmt_vop2_t *vop2 = &inst->micro_inst.vop2;

	int str_size = MAX_INST_STR_SIZE;
	char orig_inst_str[MAX_INST_STR_SIZE];
	char *inst_str = &orig_inst_str[0];
	char *fmt_str = inst->info->fmt_str;
	int token_len;
	char operand_str[MAX_OPERAND_STR_SIZE];
	while (*fmt_str)
	{
		/* Literal */
		if (*fmt_str != '%')
		{
			str_printf(&inst_str, &str_size, "%c", *fmt_str);
			fmt_str++;
			continue;
		}

		/* Token */
		fmt_str++;
		if (is_token(fmt_str, "VDST", &token_len))
		{
			operand_dump(operand_str, vop2->vdst + 256);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SRC0", &token_len))
		{
			if (vop2->src0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "%08x", vop2->lit_cnst);
			}
			else
			{
				operand_dump(operand_str, vop2->src0);
				str_printf(&inst_str, &str_size, "%s", operand_str);	
			}
		}
		else if (is_token(fmt_str, "VSRC1", &token_len))
		{
			operand_dump(operand_str, vop2->vsrc1 + 256);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);	
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, f, inst->info->size);
}

void si_inst_dump_mtbuf(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f)
{
	
	struct si_fmt_mtbuf_t *mtbuf = &inst->micro_inst.mtbuf;
	
	int str_size = MAX_INST_STR_SIZE;
	char orig_inst_str[MAX_INST_STR_SIZE];
	char *inst_str = &orig_inst_str[0];
	char *fmt_str = inst->info->fmt_str;
	int token_len;
	char operand_str[MAX_OPERAND_STR_SIZE];
	while (*fmt_str)
	{
		/* Literal */
		if (*fmt_str != '%')
		{
			str_printf(&inst_str, &str_size, "%c", *fmt_str);
			fmt_str++;
			continue;
		}

		/* Token */
		fmt_str++;
		if (is_token(fmt_str, "VDATA", &token_len))
		{
			operand_dump(operand_str, mtbuf->vdata + 256);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VADDR", &token_len))
		{
			operand_dump(operand_str, mtbuf->vaddr + 256);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SERIES_SRSRC", &token_len))
		{
			assert((mtbuf->srsrc << 2) % 4 == 0);
			operand_dump_series(operand_str, mtbuf->srsrc << 2, (mtbuf->srsrc << 2) + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SOFFSET", &token_len))
		{
			assert(mtbuf->soffset <= 103 ||
				mtbuf->soffset == 124 || 
				(mtbuf->soffset >= 128 && mtbuf->soffset <= 208));
			operand_dump(operand_str, mtbuf->soffset);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "INDEX", &token_len))
		{
			if(mtbuf->index)
			{
				str_printf(&inst_str, &str_size, "index");
			}
		}
		else if (is_token(fmt_str, "OFFEN", &token_len))
		{
			if (mtbuf->offen)
			{
				str_printf(&inst_str, &str_size, "offen");
			}
		}
		else if (is_token(fmt_str, "DFMT", &token_len))
		{
			str_printf(&inst_str, &str_size, "%s", map_value(&dfmt_map, mtbuf->dfmt));
		}
		else if (is_token(fmt_str, "NFMT", &token_len))
		{
			str_printf(&inst_str, &str_size, "%s", map_value(&nfmt_map, mtbuf->nfmt));	
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, f, inst->info->size);
}

/* GPU disassembler tool */
void si_disasm(char *path)
{
        struct elf_file_t *elf_file;
        struct elf_symbol_t *symbol;
        struct elf_section_t *section;

        struct si_bin_file_t *amd_bin;

        char kernel_name[MAX_STRING_SIZE];

        int i;

        /* Initialize disassembler */
        si_disasm_init();

        /* Decode external ELF */
        elf_file = elf_file_create_from_path(path);
        for (i = 0; i < list_count(elf_file->symbol_table); i++)
        {
                /* Get symbol and section */
                symbol = list_get(elf_file->symbol_table, i);
                section = list_get(elf_file->section_list, symbol->section);
                if (!section)
                        continue;

                /* If symbol is '__OpenCL_XXX_kernel', it points to internal ELF */
                if (str_prefix(symbol->name, "__OpenCL_") && str_suffix(symbol->name, "_kernel"))
                {
                        /* Decode internal ELF */
                        str_substr(kernel_name, sizeof(kernel_name), symbol->name, 9, strlen(symbol->name) - 16);
                        amd_bin = si_bin_file_create(section->buffer.ptr + symbol->value, symbol->size, kernel_name);

                        /* Get kernel name */
                        printf("**\n** Disassembly for '__kernel %s'\n**\n\n", kernel_name);
                        si_disasm_buffer(&amd_bin->enc_dict_entry_southern_islands->sec_text_buffer, stdout);
                        printf("\n\n\n");

                        /* Free internal ELF */
                        si_bin_file_free(amd_bin);
                }
        }

        /* Free external ELF */
        elf_file_free(elf_file);
        si_disasm_done();

        /* End */
        mhandle_done();
        exit(0);
}

