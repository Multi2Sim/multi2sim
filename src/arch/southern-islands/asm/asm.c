/* 
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <assert.h>
#include <ctype.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"
#include "bin-file.h"


/* Table containing information of all instructions */
static struct si_inst_info_t si_inst_info[SI_INST_COUNT];

/* Define the number of valid opcodes. */
#define SI_INST_INFO_SOPP_MAX_VALUE 22
#define SI_INST_INFO_SOPC_MAX_VALUE 16
#define SI_INST_INFO_SOP1_MAX_VALUE 53
#define SI_INST_INFO_SOPK_MAX_VALUE 21
#define SI_INST_INFO_SOP2_MAX_VALUE 44
#define SI_INST_INFO_SMRD_MAX_VALUE 31
#define SI_INST_INFO_VOP3_MAX_VALUE 452
#define SI_INST_INFO_VOPC_MAX_VALUE 247
#define SI_INST_INFO_VOP1_MAX_VALUE 68
#define SI_INST_INFO_VOP2_MAX_VALUE 49
#define SI_INST_INFO_VINTRP_MAX_VALUE 3
#define SI_INST_INFO_DS_MAX_VALUE 211
#define SI_INST_INFO_MTBUF_MAX_VALUE 7
#define SI_INST_INFO_MUBUF_MAX_VALUE 113
#define SI_INST_INFO_MIMG_MAX_VALUE 96
#define SI_INST_INFO_EXP_MAX_VALUE  1

/* String lengths for printing assembly */
#define MAX_OPERAND_STR_SIZE 11
#define MAX_DAT_STR_SIZE 31

/* Pointers to 'si_inst_info' table indexed by instruction opcode */
static struct si_inst_info_t *si_inst_info_sopp[SI_INST_INFO_SOPP_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_sopc[SI_INST_INFO_SOPC_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_sop1[SI_INST_INFO_SOP1_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_sopk[SI_INST_INFO_SOPK_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_sop2[SI_INST_INFO_SOP2_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_smrd[SI_INST_INFO_SMRD_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_vop3[SI_INST_INFO_VOP3_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_vopc[SI_INST_INFO_VOPC_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_vop1[SI_INST_INFO_VOP1_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_vop2[SI_INST_INFO_VOP2_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_vintrp[SI_INST_INFO_VINTRP_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_ds[SI_INST_INFO_DS_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_mtbuf[SI_INST_INFO_MTBUF_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_mubuf[SI_INST_INFO_MUBUF_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_mimg[SI_INST_INFO_MIMG_MAX_VALUE + 1];
static struct si_inst_info_t *si_inst_info_exp[SI_INST_INFO_EXP_MAX_VALUE + 1];

/* String maps for assembly dump. */
struct str_map_t sdst_map = {
	24, {
			{"reserved", 0},
			{"reserved", 1},
			{"vcc_lo", 2},
			{"vcc_hi", 3},
			{"tba_lo", 4},
			{"tba_hi", 5},
			{"tma_lo", 6},
			{"tma_hi", 7},
			{"ttmp0", 8},
			{"ttmp1", 9},
			{"ttmp2", 10},
			{"ttmp3", 11},
			{"ttmp4", 12},
			{"ttmp5", 13},
			{"ttmp6", 14},
			{"ttmp7", 15},
			{"ttmp8", 16},
			{"ttmp9", 17},
			{"ttmp10", 18},
			{"ttmp11", 19},
			{"m0", 20},
			{"reserved", 21},
			{"exec_lo", 22},
			{"exec_hi", 23}
		}
};

struct str_map_t ssrc_map = {
	16, {
			{"0.5", 0},
			{"-0.5", 1},
			{"1.0", 2},
			{"-1.0", 3},
			{"2.0", 4},
			{"-2.0", 5},
			{"4.0", 6},
			{"-4.0", 7},
			{"reserved", 8},
			{"reserved", 9},
			{"reserved", 10},
			{"vccz", 11},
			{"execz", 12},
			{"scc", 13},
			{"reserved", 14},
			{"literal constant", 15}
		}
};

struct str_map_t dfmt_map = {
	16, {
			{"invalid", 0},
			{"BUF_DATA_FORMAT_8", 1},
			{"BUF_DATA_FORMAT_16", 2},
			{"BUF_DATA_FORMAT_8_8", 3},
			{"BUF_DATA_FORMAT_32", 4},
			{"BUF_DATA_FORMAT_16_16", 5},
			{"BUF_DATA_FORMAT_10_11_11", 6},
			{"BUF_DATA_FORMAT_11_10_10", 7},
			{"BUF_DATA_FORMAT_10_10_10_2", 8},
			{"BUF_DATA_FORMAT_2_10_10_10", 9},
			{"BUF_DATA_FORMAT_8_8_8_8", 10},
			{"BUF_DATA_FORMAT_32_32", 11},
			{"BUF_DATA_FORMAT_16_16_16_16", 12},
			{"BUF_DATA_FORMAT_32_32_32", 13},
			{"BUF_DATA_FORMAT_32_32_32_32", 14},
			{"reserved", 15}
		}
};

struct str_map_t nfmt_map = {
	14, {
			{"BUF_NUM_FORMAT_UNORM", 0},
			{"BUF_NUM_FORMAT_SNORM", 1},
			{"BUF_NUM_FORMAT_UNSCALED", 2},
			{"BUF_NUM_FORMAT_SSCALED", 3},
			{"BUF_NUM_FORMAT_UINT", 4},
			{"BUF_NUM_FORMAT_SINT", 5},
			{"BUF_NUM_FORMAT_SNORM_NZ", 6},
			{"BUF_NUM_FORMAT_FLOAT", 7},
			{"reserved", 8},
			{"BUF_NUM_FORMAT_SRGB", 9},
			{"BUF_NUM_FORMAT_UBNORM", 10},
			{"BUF_NUM_FORMAT_UBNORM_NZ", 11},
			{"BUF_NUM_FORMAT_UBINT", 12},
			{"BUF_NUM_FORMAT_UBSCALED", 13}
		}
};

struct str_map_t OP16_map = {
	16, {
			{"f", 0},
			{"lt", 1},
			{"eq", 2},
			{"le", 3},
			{"gt", 4},
			{"lg", 5},
			{"ge", 6},
			{"o", 7},
			{"u", 8},
			{"nge", 9},
			{"nlg", 10},
			{"ngt", 11},
			{"nle", 12},
			{"neq", 13},
			{"nlt", 14},
			{"tru", 15},
		}
};

struct str_map_t OP8_map = {
	8, {
			{"f", 0},
			{"lt", 1},
			{"eq", 2},
			{"le", 3},
			{"gt", 4},
			{"lg", 5},
			{"ge", 6},
			{"tru", 7},
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
#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size, _flags) \
	info = &si_inst_info[SI_INST_##_name]; \
	info->inst = SI_INST_##_name; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->fmt = SI_FMT_##_fmt; \
	info->opcode = _opcode; \
	info->size = _size; \
	info->flags = _flags;
#include "asm.dat"
#undef DEFINST

	/* Tables of pointers to 'si_inst_info' */
	for (i = 1; i < SI_INST_COUNT; i++)
	{
		info = &si_inst_info[i];

		if (info->fmt == SI_FMT_SOPP)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_SOPP_MAX_VALUE));
			si_inst_info_sopp[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOPC)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_SOPC_MAX_VALUE));
			si_inst_info_sopc[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOP1)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_SOP1_MAX_VALUE));
			si_inst_info_sop1[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOPK)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_SOPK_MAX_VALUE));
			si_inst_info_sopk[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOP2)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_SOP2_MAX_VALUE));
			si_inst_info_sop2[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SMRD) 
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_SMRD_MAX_VALUE));
			si_inst_info_smrd[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOP3a || info->fmt == SI_FMT_VOP3b)
		{
			int i;

			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_VOP3_MAX_VALUE));
			si_inst_info_vop3[info->opcode] = info;
			if (info->flags & SI_INST_FLAG_OP8)
				for (i = 1; i < 8; i++)
					si_inst_info_vop3[info->opcode + i] = info;
			if (info->flags & SI_INST_FLAG_OP16)
				for (i = 1; i < 16; i++)
					si_inst_info_vop3[info->opcode + i] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOPC)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_VOPC_MAX_VALUE));
			si_inst_info_vopc[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOP1)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_VOP1_MAX_VALUE));
			si_inst_info_vop1[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOP2)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_VOP2_MAX_VALUE));
			si_inst_info_vop2[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VINTRP)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_VINTRP_MAX_VALUE));
			si_inst_info_vintrp[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_DS)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_DS_MAX_VALUE));
			si_inst_info_ds[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_MTBUF)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_MTBUF_MAX_VALUE));
			si_inst_info_mtbuf[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_MUBUF)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_MUBUF_MAX_VALUE));
			si_inst_info_mubuf[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_MIMG)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_MIMG_MAX_VALUE));
			si_inst_info_mimg[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_EXP)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_EXP_MAX_VALUE));
			si_inst_info_exp[info->opcode] = info;
			continue;
		}
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

int si_inst_decode(void *buf, struct si_inst_t *inst, unsigned int offset)
{

	/* Zero-out instruction structure */
	memset(inst, 0, sizeof(struct si_inst_t));

	/* All instructions will be at least 32-bits */
	unsigned int inst_size = 4;

	memcpy(&inst->micro_inst, buf, inst_size);

	/* Use the encoding field to determine the instruction type */
	if (inst->micro_inst.sopp.enc == 0x17F)
	{
		if (!si_inst_info_sopp[inst->micro_inst.sopp.op])
			fatal("Unimplemented Instruction: SOPP:%d  // %08X: %08X\n",
			inst->micro_inst.sopp.op, offset, * (unsigned int *) buf);

		inst->info = si_inst_info_sopp[inst->micro_inst.sopp.op];
	}
	else if (inst->micro_inst.sopc.enc == 0x17E)
	{
		if (!si_inst_info_sopc[inst->micro_inst.sopc.op])
			fatal("Unimplemented Instruction: SOPC:%d  // %08X: %08X\n",
			inst->micro_inst.sopc.op, offset, * (unsigned int *) buf);

		inst->info = si_inst_info_sopc[inst->micro_inst.sopc.op];

		/* Only one source field may use a literal constant, which is indicated by 0xFF. */
		assert(!(inst->micro_inst.sopc.ssrc0 == 0xFF && inst->micro_inst.sopc.ssrc1 == 0xFF));
		if (inst->micro_inst.sopc.ssrc0 == 0xFF || inst->micro_inst.sopc.ssrc1 == 0xFF)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}
	}
	else if (inst->micro_inst.sop1.enc == 0x17D)
	{
		if (!si_inst_info_sop1[inst->micro_inst.sop1.op])
			fatal("Unimplemented Instruction: SOP1:%d  // %08X: %08X\n",
			inst->micro_inst.sop1.op, offset, * (unsigned int *) buf);

		inst->info = si_inst_info_sop1[inst->micro_inst.sop1.op];

		/* 0xFF indicates the use of a literal constant as a source operand. */
		if (inst->micro_inst.sop1.ssrc0 == 0xFF)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}	
	}
	else if (inst->micro_inst.sopk.enc == 0xB)
	{
		if (!si_inst_info_sopk[inst->micro_inst.sopk.op])
			fatal("Unimplemented Instruction: SOPK:%d  // %08X: %08X\n",
			inst->micro_inst.sopk.op, offset, * (unsigned int *) buf);

		inst->info = si_inst_info_sopk[inst->micro_inst.sopk.op];
	}
	else if (inst->micro_inst.sop2.enc == 0x2)
	{
		if (!si_inst_info_sop2[inst->micro_inst.sop2.op])
			fatal("Unimplemented Instruction: SOP2:%d  // %08X: %08X\n",
			inst->micro_inst.sop2.op, offset, * (unsigned int *) buf);

		inst->info = si_inst_info_sop2[inst->micro_inst.sop2.op];

		/* Only one source field may use a literal constant, which is indicated by 0xFF. */
		assert(!(inst->micro_inst.sop2.ssrc0 == 0xFF && inst->micro_inst.sop2.ssrc1 == 0xFF));
		if (inst->micro_inst.sop2.ssrc0 == 0xFF || inst->micro_inst.sop2.ssrc1 == 0xFF)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}
	}
	else if (inst->micro_inst.smrd.enc == 0x18)
	{
		if (!si_inst_info_smrd[inst->micro_inst.smrd.op])
			fatal("Unimplemented Instruction: SMRD:%d  // %08X: %08X\n",
			inst->micro_inst.smrd.op, offset, * (unsigned int *) buf);

		inst->info = si_inst_info_smrd[inst->micro_inst.smrd.op];
	}
	else if (inst->micro_inst.vop3a.enc == 0x34)
	{
		/* 64 bit instruction. */
		inst_size = 8;
		memcpy(&inst->micro_inst, buf, inst_size);

		if (!si_inst_info_vop3[inst->micro_inst.vop3a.op])
			fatal("Unimplemented Instruction: VOP3:%d  // %08X: %08X %08X\n",
				inst->micro_inst.vop3a.op, offset, * (unsigned int *) buf,
				* (unsigned int *) (buf + 4));

		inst->info = si_inst_info_vop3[inst->micro_inst.vop3a.op];
	}
	else if (inst->micro_inst.vopc.enc == 0x3E)
	{
		if (!si_inst_info_vopc[inst->micro_inst.vopc.op])
			fatal("Unimplemented Instruction: VOPC:%d  // %08X: %08X\n",
			inst->micro_inst.vopc.op, offset, * (unsigned int *) buf);

		inst->info = si_inst_info_vopc[inst->micro_inst.vopc.op];

		/* 0xFF indicates the use of a literal constant as a source operand. */
		if (inst->micro_inst.vopc.src0 == 0xFF)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}
	}
	else if (inst->micro_inst.vop1.enc == 0x3F)
	{
		if (!si_inst_info_vop1[inst->micro_inst.vop1.op])
			fatal("Unimplemented Instruction: VOP1:%d  // %08X: %08X\n",
			inst->micro_inst.vop1.op, offset, * (unsigned int *) buf);

		inst->info = si_inst_info_vop1[inst->micro_inst.vop1.op];

		/* 0xFF indicates the use of a literal constant as a source operand. */
		if (inst->micro_inst.vop1.src0 == 0xFF)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}
	}
	else if (inst->micro_inst.vop2.enc == 0x0)
	{
		if (!si_inst_info_vop2[inst->micro_inst.vop2.op])
			fatal("Unimplemented Instruction: VOP2:%d  // %08X: %08X\n",
			inst->micro_inst.vop2.op, offset, * (unsigned int *) buf);

		inst->info = si_inst_info_vop2[inst->micro_inst.vop2.op];

		/* 0xFF indicates the use of a literal constant as a source operand. */
		if (inst->micro_inst.vop2.src0 == 0xFF)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}
	}
	else if (inst->micro_inst.vintrp.enc == 0x32)
	{
		if (!si_inst_info_vintrp[inst->micro_inst.vintrp.op])
			fatal("Unimplemented Instruction: VINTRP:%d  // %08X: %08X\n",
			inst->micro_inst.vintrp.op, offset, * (unsigned int *) buf);

		inst->info = si_inst_info_vintrp[inst->micro_inst.vintrp.op];

	}
	else if (inst->micro_inst.ds.enc == 0x36)
	{
		/* 64 bit instruction. */
		inst_size = 8;
		memcpy(&inst->micro_inst, buf, inst_size);

		if (!si_inst_info_ds[inst->micro_inst.ds.op])
			fatal("Unimplemented Instruction: DS:%d  // %08X: %08X %08X\n",
			inst->micro_inst.ds.op, offset, * (unsigned int *) buf,
			* (unsigned int *) (buf + 4));

		inst->info = si_inst_info_ds[inst->micro_inst.ds.op];
	}
	else if (inst->micro_inst.mtbuf.enc == 0x3A)
	{
		/* 64 bit instruction. */
		inst_size = 8;
		memcpy(&inst->micro_inst, buf, inst_size);

		if (!si_inst_info_mtbuf[inst->micro_inst.mtbuf.op])
			fatal("Unimplemented Instruction: MTBUF:%d  // %08X: %08X %08X\n",
			inst->micro_inst.mtbuf.op, offset, * (unsigned int *) buf,
			* (unsigned int *) (buf+4));

		inst->info = si_inst_info_mtbuf[inst->micro_inst.mtbuf.op];
	}
	else if (inst->micro_inst.mubuf.enc == 0x38)
	{
		/* 64 bit instruction. */
		inst_size = 8;
		memcpy(&inst->micro_inst, buf, inst_size);

		if(!si_inst_info_mubuf[inst->micro_inst.mubuf.op])
			fatal("Unimplemented Instruction: MUBUF:%d  // %08X: %08X %08X\n",
			inst->micro_inst.mubuf.op, offset, * (unsigned int *) buf,
			* (unsigned int *) (buf+4));

		inst->info = si_inst_info_mubuf[inst->micro_inst.mubuf.op];
	}
	else if (inst->micro_inst.mimg.enc == 0x3C)
	{
		/* 64 bit instruction. */
		inst_size = 8;
		memcpy(&inst->micro_inst, buf, inst_size);

		if(!si_inst_info_mimg[inst->micro_inst.mimg.op])
			fatal("Unimplemented Instruction: MIMG:%d  // %08X: %08X %08X\n",
			inst->micro_inst.mimg.op, offset, * (unsigned int *) buf,
			* (unsigned int *) (buf+4));

		inst->info = si_inst_info_mimg[inst->micro_inst.mimg.op];
	}
	else if (inst->micro_inst.mimg.enc == 0x3C)
	{
		/* 64 bit instruction. */
		inst_size = 8;
		memcpy(&inst->micro_inst, buf, inst_size);

		if(!si_inst_info_mimg[inst->micro_inst.mimg.op])
			fatal("Unimplemented Instruction: MIMG:%d  // %08X: %08X %08X\n",
			inst->micro_inst.mimg.op, offset, * (unsigned int *) buf,
			* (unsigned int *) (buf+4));

		inst->info = si_inst_info_mimg[inst->micro_inst.mimg.op];
	}
	else if (inst->micro_inst.exp.enc == 0x3E)
	{
		/* 64 bit instruction. */
		inst_size = 8;
		memcpy(&inst->micro_inst, buf, inst_size);

		/* Export is the only instruction in its kind */
		if (!si_inst_info_exp[0])
			fatal("Unimplemented Instruction: EXP\n");

		inst->info = si_inst_info_exp[0];
	}
	else
	{
		fatal("Unimplemented format. Instruction is:  // %08X: %08X\n", offset,
		((unsigned int*)buf)[0]);
	}

	return inst_size;
}



void si_disasm_buffer(struct elf_buffer_t *buffer, FILE *f)
{
	void *inst_buf = buffer->ptr;
	int inst_count = 0;
	int rel_addr = 0;

	int label_addr[buffer->size / 4];	/* A list of created labels sorted by rel_addr. */

	int *next_label = &label_addr[0];	/* The next label to dump. */
	int *end_label = &label_addr[0];	/* The address after the last label. */


	/* Read through instructions to find labels. */
	while (inst_buf)
	{
		struct si_inst_t inst;
		int inst_size;

		/* Zero-out instruction structure */
		memset(&inst, 0, sizeof(struct si_inst_t));

		/* Decode instruction */
		inst_size = si_inst_decode(inst_buf, &inst, rel_addr);

		/* If ENDPGM, break. */
		if (inst.info->fmt == SI_FMT_SOPP && inst.micro_inst.sopp.op == 1)
		{
			break;
		}
		/* If the instruction branches, insert the label into the sorted list. */
		if (inst.info->fmt == SI_FMT_SOPP &&
			(inst.micro_inst.sopp.op >= 2 && inst.micro_inst.sopp.op <= 9))
		{
			short simm16 = inst.micro_inst.sopp.simm16;
			int se_simm = simm16;
			int label = rel_addr + (se_simm * 4) + 4;

			/* Find position to insert label. */
			int *t_label = &label_addr[0];

			while (t_label < end_label && *t_label < label)
				t_label++;

			if (label != *t_label || t_label == end_label)
			{

				/* Shift labels after position down. */
				int *t2_label = end_label;

				while (t2_label > t_label)
				{
					*t2_label = *(t2_label - 1);
					t2_label--;
				}
				end_label++;

				/* Insert the new label. */
				*t_label = label;
			}

		}

		inst_buf += inst_size;
		rel_addr += inst_size;
	}


	/* Reset to disassemble. */
	inst_buf = buffer->ptr;
	rel_addr = 0;

	/* Disassemble */
	while (inst_buf)
	{
		struct si_inst_t inst;
		int inst_size;

		/* Parse the instruction */
		inst_size = si_inst_decode(inst_buf, &inst, rel_addr);

		inst_count++;

		/* Dump a label if necessary. */
		if (*next_label == rel_addr && next_label != end_label)
		{
			fprintf(f, "label_%04X:\n", rel_addr / 4);
			next_label++;
		}


		/* Dump the instruction */
		int line_size = MAX_INST_STR_SIZE;
		char line[line_size];

		if (inst.info->fmt == SI_FMT_SOPP)
		{
			si_inst_dump_sopp(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_SOPC)
		{
			si_inst_dump_sopc(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_SOP1)
		{
			si_inst_dump_sop1(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_SOPK)
		{
			si_inst_dump_sopk(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_SOP2)
		{
			si_inst_dump_sop2(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_SMRD)
		{
			si_inst_dump_smrd(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_VOP3a || inst.info->fmt == SI_FMT_VOP3b)
		{
			si_inst_dump_vop3(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_VOPC)
		{
			si_inst_dump_vopc(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_VOP1)
		{
			si_inst_dump_vop1(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_VOP2)
		{
			si_inst_dump_vop2(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_VINTRP)
		{
			si_inst_dump_vintrp(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_DS)
		{
			si_inst_dump_ds(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_MTBUF)
		{
			si_inst_dump_mtbuf(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_MUBUF)
		{
			si_inst_dump_mubuf(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_MIMG)
		{
			si_inst_dump_mimg(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else if (inst.info->fmt == SI_FMT_EXP)
		{
			si_inst_dump_exp(&inst, inst_size, rel_addr, inst_buf, line, line_size);
		}
		else
		{
			fatal("Unknown instruction: %08X\n", ((unsigned int*)inst_buf)[0]);
		}
		fprintf(f, " %s", line);


		/* Break at end of program. */
		if (inst.info->fmt == SI_FMT_SOPP && inst.micro_inst.sopp.op == 1)
			break;

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


void operand_dump(char *str, int operand)
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
		str_printf(&pstr, &str_size, "%s", str_map_value(&sdst_map, operand - 104));
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
		str_printf(&pstr, &str_size, "%s", str_map_value(&ssrc_map, operand - 240));
	}
	else if (operand <= 511)
	{
		/* VGPR */
		str_printf(&pstr, &str_size, "v%d", operand - 256);
	}
}

void operand_dump_series(char *str, int operand, int operand_end)
{
	assert(operand <= operand_end);
	if (operand == operand_end)
	{
		operand_dump(str, operand);
		return;
	}

	int str_size = MAX_OPERAND_STR_SIZE;
	char *pstr = str;

	if (operand <= 103)
	{
		str_printf(&pstr, &str_size, "s[%d:%d]", operand, operand_end);
	}
	else if (operand <= 245)
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
				case 128:
					str_printf(&pstr, &str_size, "0");
					break;
				case 131:
					str_printf(&pstr, &str_size, "3");
					break;
				case 208: 
					str_printf(&pstr, &str_size, "-16");
					break;
				case 240:
					str_printf(&pstr, &str_size, "0.5");
					break;
				case 242:
					str_printf(&pstr, &str_size, "1.0");
					break;
				case 243:
					str_printf(&pstr, &str_size, "-1.0");
					break;
				case 244:
					str_printf(&pstr, &str_size, "2.0");
					break;
				case 245:
					str_printf(&pstr, &str_size, "-2.0");
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
		str_printf(&pstr, &str_size, "v[%d:%d]", operand - 256, operand_end - 256);
	}
}

void operand_dump_scalar(char *str, int operand)
{
	operand_dump(str, operand);
}

void operand_dump_series_scalar(char *str, int operand, int operand_end)
{
	operand_dump_series(str, operand, operand_end);
}

void operand_dump_vector(char *str, int operand)
{
	operand_dump(str, operand + 256);
}

void operand_dump_series_vector(char *str, int operand, int operand_end)
{
	operand_dump_series(str, operand + 256, operand_end + 256);
}

void operand_dump_exp(char *str, int operand)
{
	/* Assume operand in range. */
	assert(operand >= 0 && operand <= 63);

	int str_size = MAX_OPERAND_STR_SIZE;
	char *pstr = str;

	if (operand <= 7)
	{
		/* EXP_MRT */
		str_printf(&pstr, &str_size, "exp_mrt_%d", operand);
	}
	else if (operand == 8)
	{
		/* EXP_Z */
		str_printf(&pstr, &str_size, "exp_mrtz");
	}
	else if (operand == 9)
	{
		/* EXP_NULL */
		str_printf(&pstr, &str_size, "exp_null");
	}
	else if (operand < 12)
	{
		fatal("Operand code [%d] unused.", operand);
	}
	else if (operand <= 15)
	{
		/* EXP_POS */
		str_printf(&pstr, &str_size, "exp_pos_%d", operand - 12);
	}
	else if (operand < 32)
	{
		fatal("Operand code [%d] unused.", operand);
	}
	else if (operand <= 63)
	{
		/* EXP_PARAM */
		str_printf(&pstr, &str_size, "exp_param_%d", operand - 32);
	}
}

void line_dump(char *inst_str, unsigned int rel_addr, void *buf, char *line, int line_size, int inst_size)
{
	int dat_str_size = MAX_DAT_STR_SIZE;
	char inst_dat_str[MAX_DAT_STR_SIZE];
	char *dat_str = &inst_dat_str[0];

	if (inst_size == 4)
	{
		str_printf(&dat_str, &dat_str_size, "// %08X: %08X", rel_addr, ((unsigned int*)buf)[0]);
	}
	else
	{
		str_printf(&dat_str, &dat_str_size, "// %08X: %08X %08X", rel_addr, ((unsigned int*)buf)[0], ((unsigned int*)buf)[1]);
	}

	if (strlen(inst_str) < 59)
	{
		str_printf(&line, &line_size, "%-59s%s\n", inst_str, inst_dat_str);
	}
	else
	{
		str_printf(&line, &line_size, "%s %s\n", inst_str, inst_dat_str);
	}
}

void si_inst_dump(struct si_inst_t *inst, int inst_size, void *inst_buf, uint32_t rel_addr, 
	char *line, int line_size)
{
	switch (inst->info->fmt)
	{

	case SI_FMT_SOPC:

		si_inst_dump_sopc(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_SOPK:

		si_inst_dump_sopk(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_SOPP:

		si_inst_dump_sopp(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_SOP1:

		si_inst_dump_sop1(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_SOP2:

		si_inst_dump_sop2(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_SMRD:

		si_inst_dump_smrd(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_VOP1:

		si_inst_dump_vop1(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_VOP2:

		si_inst_dump_vop2(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_VOP3a:
	case SI_FMT_VOP3b:

		si_inst_dump_vop3(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_VOPC:

		si_inst_dump_vopc(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_DS:

		si_inst_dump_ds(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_MTBUF:

		si_inst_dump_mtbuf(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_MUBUF:

		si_inst_dump_mubuf(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	case SI_FMT_EXP:
		si_inst_dump_exp(inst, inst_size, rel_addr, inst_buf, line, line_size);
		break;

	default:

		fatal("Unknown instruction: %08X\n", ((unsigned int*)inst_buf)[0]);
	}
}

void si_inst_dump_sopp(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
{
	struct si_fmt_sopp_t *sopp = &inst->micro_inst.sopp;

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
				if (and)
				{
					str_printf(&inst_str, &str_size, " & ");
				}

				str_printf(&inst_str, &str_size, "lgkmcnt(%d)", lgkm_cnt);
				and = 1;
			}

			int exp_cnt = (sopp->simm16 & 0x70) >> 4;

			if (exp_cnt != 0x7)
			{
				if (and)
				{
					str_printf(&inst_str, &str_size, " & ");
				}

				str_printf(&inst_str, &str_size, "expcnt(%d)", exp_cnt);
				and = 1;
			}
		}
		else if (is_token(fmt_str, "LABEL", &token_len))
		{
			short simm16 = sopp->simm16;
			int se_simm = simm16;

			str_printf(&inst_str, &str_size, "label_%04X", (rel_addr + (se_simm * 4) + 4) / 4);
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_sopc(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
{
	struct si_fmt_sopc_t *sopc = &inst->micro_inst.sopc;

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
		if (is_token(fmt_str, "SSRC0", &token_len))
		{
			if (sopc->ssrc0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "0x%08x", sopc->lit_cnst);
			}
			else
			{
				operand_dump_scalar(operand_str, sopc->ssrc0);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "SSRC1", &token_len))
		{
			if (sopc->ssrc1 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "0x%08x", sopc->lit_cnst);
			}
			else
			{
				operand_dump_scalar(operand_str, sopc->ssrc1);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_sop1(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
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
		if (is_token(fmt_str, "SDST", &token_len))
		{
			operand_dump_scalar(operand_str, sop1->sdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SSRC0", &token_len))
		{
			if (sop1->ssrc0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "0x%08x", sop1->lit_cnst);
			}
			else
			{
				operand_dump_scalar(operand_str, sop1->ssrc0);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "64_SDST", &token_len))
		{
			operand_dump_series_scalar(operand_str, sop1->sdst, sop1->sdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_SSRC0", &token_len))
		{
			if (sop1->ssrc0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "0x%08x", sop1->lit_cnst);
			}
			else
			{
				operand_dump_series_scalar(operand_str, sop1->ssrc0, sop1->ssrc0 + 1);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_sopk(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
{
	struct si_fmt_sopk_t *sopk = &inst->micro_inst.sopk;

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
			operand_dump_scalar(operand_str, sopk->sdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SIMM16", &token_len))
		{
			str_printf(&inst_str, &str_size, "0x%04x", sopk->simm16);
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_sop2(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
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

		/* Token */
		fmt_str++;
		if (is_token(fmt_str, "SDST", &token_len))
		{
			operand_dump_scalar(operand_str, sop2->sdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_SDST", &token_len))
		{
			operand_dump_series_scalar(operand_str, sop2->sdst, sop2->sdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SSRC0", &token_len))
		{
			if (sop2->ssrc0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "0x%08x", sop2->lit_cnst);
			}
			else
			{
				operand_dump_scalar(operand_str, sop2->ssrc0);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "64_SSRC0", &token_len))
		{
			if (sop2->ssrc0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "0x%08x", sop2->lit_cnst);
			}
			else
			{
				operand_dump_series_scalar(operand_str, sop2->ssrc0, sop2->ssrc0 + 1);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "SSRC1", &token_len))
		{
			if (sop2->ssrc1 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "0x%08x", sop2->lit_cnst);
			}
			else
			{
				operand_dump_scalar(operand_str, sop2->ssrc1);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "64_SSRC1", &token_len))
		{
			if (sop2->ssrc1 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "0x%08x", sop2->lit_cnst);
			}
			else
			{
				operand_dump_series_scalar(operand_str, sop2->ssrc1, sop2->ssrc1 + 1);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_smrd(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
{
	struct si_fmt_smrd_t *smrd = &inst->micro_inst.smrd;

	/* The sbase field is missing the LSB, so multiply by 2 */
	int sdst = smrd->sdst;
	int sdst_end;
	int sbase = smrd->sbase * 2;
	int sbase_end;

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
				sdst_end = sdst + 1;
				break;
			case 2:
				sdst_end = sdst + 3;
				break;
			case 3:
				sdst_end = sdst + 7;
				break;
			case 4:
				sdst_end = sdst + 15;
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
				sdst_end = sdst + 1;
				break;
			case 10:
				sdst_end = sdst + 3;
				break;
			case 11:
				sdst_end = sdst + 7;
				break;
			case 12:
				sdst_end = sdst + 15;
				break;
			default:
				assert("Invalid smrd opcode");
			}
		}
	}
	/* S_MEMTIME */
	else if (smrd->op == 30)
	{

	}
	/* S_DCACHE_INV */
	else if (smrd->op == 31)
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
			operand_dump_scalar(operand_str, sdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SERIES_SDST", &token_len))
		{
			operand_dump_series_scalar(operand_str, sdst, sdst_end);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SERIES_SBASE", &token_len))
		{
			operand_dump_series_scalar(operand_str, sbase, sbase_end);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "OFFSET", &token_len))
		{
			if (smrd->imm)
			{
				str_printf(&inst_str, &str_size, "0x%02x", smrd->offset);
			}
			else
			{
				operand_dump_scalar(operand_str, smrd->offset);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_vop3(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
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
		if (is_token(fmt_str, "64_SVDST", &token_len))
		{
			/* VOP3a compare operations use the VDST field to indicate the address of the scalar destination.*/
			operand_dump_series_scalar(operand_str, inst->micro_inst.vop3a.vdst, inst->micro_inst.vop3a.vdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VDST", &token_len))
		{
			operand_dump_vector(operand_str, inst->micro_inst.vop3a.vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_VDST", &token_len))
		{
			operand_dump_series_vector(operand_str, inst->micro_inst.vop3a.vdst, inst->micro_inst.vop3a.vdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_SDST", &token_len))
		{
			operand_dump_series_scalar(operand_str, inst->micro_inst.vop3b.sdst, inst->micro_inst.vop3b.sdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SRC0", &token_len))
		{
			operand_dump(operand_str, inst->micro_inst.vop3a.src0);

			if (!(IN_RANGE(inst->micro_inst.vop3a.op, 293, 298)) && !(IN_RANGE(inst->micro_inst.vop3a.op, 365, 366)))
			{
				if ((inst->micro_inst.vop3a.neg & 1) && (inst->micro_inst.vop3a.abs & 1))
				{
					str_printf(&inst_str, &str_size, "-abs(%s)", operand_str);
				}
				else if ((inst->micro_inst.vop3a.neg & 1) && !(inst->micro_inst.vop3a.abs & 1))
				{
					str_printf(&inst_str, &str_size, "-%s", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 1) && (inst->micro_inst.vop3a.abs & 1))
				{
					str_printf(&inst_str, &str_size, "abs(%s)", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 1) && !(inst->micro_inst.vop3a.abs & 1))
				{
					str_printf(&inst_str, &str_size, "%s", operand_str);
				}
			}
			else
			{
				if (inst->micro_inst.vop3a.neg & 1)
				{
					str_printf(&inst_str, &str_size, "-%s", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 1))
				{
					str_printf(&inst_str, &str_size, "%s", operand_str);
				}
			}
		}
		else if (is_token(fmt_str, "64_SRC0", &token_len))
		{
			operand_dump_series(operand_str, inst->micro_inst.vop3a.src0, inst->micro_inst.vop3a.src0 + 1);
			
			if (!(IN_RANGE(inst->micro_inst.vop3a.op, 293, 298)) && !(IN_RANGE(inst->micro_inst.vop3a.op, 365, 366)))
			{
				if ((inst->micro_inst.vop3a.neg & 1) && (inst->micro_inst.vop3a.abs & 1))
				{
					str_printf(&inst_str, &str_size, "-abs(%s)", operand_str);
				}
				else if ((inst->micro_inst.vop3a.neg & 1) && !(inst->micro_inst.vop3a.abs & 1))
				{
					str_printf(&inst_str, &str_size, "-%s", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 1) && (inst->micro_inst.vop3a.abs & 1))
				{
					str_printf(&inst_str, &str_size, "abs(%s)", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 1) && !(inst->micro_inst.vop3a.abs & 1))
				{
					str_printf(&inst_str, &str_size, "%s", operand_str);
				}
			}
			else
			{
				if (inst->micro_inst.vop3a.neg & 1)
				{
					str_printf(&inst_str, &str_size, "-%s", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 1))
				{
					str_printf(&inst_str, &str_size, "%s", operand_str);
				}
			}
		}
		else if (is_token(fmt_str, "SRC1", &token_len))
		{
			operand_dump(operand_str, inst->micro_inst.vop3a.src1);

			if (!(IN_RANGE(inst->micro_inst.vop3a.op, 293, 298)) && !(IN_RANGE(inst->micro_inst.vop3a.op, 365, 366)))
			{
				if ((inst->micro_inst.vop3a.neg & 2) && (inst->micro_inst.vop3a.abs & 2))
				{
					str_printf(&inst_str, &str_size, "-abs(%s)", operand_str);
				}
				else if ((inst->micro_inst.vop3a.neg & 2) && !(inst->micro_inst.vop3a.abs & 2))
				{
					str_printf(&inst_str, &str_size, "-%s", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 2) && (inst->micro_inst.vop3a.abs & 2))
				{
					str_printf(&inst_str, &str_size, "abs(%s)", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 2) && !(inst->micro_inst.vop3a.abs & 2))
				{
					str_printf(&inst_str, &str_size, "%s", operand_str);
				}
			}
			else
			{
				if (inst->micro_inst.vop3a.neg & 2)
				{
					str_printf(&inst_str, &str_size, "-%s", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 2))
				{
					str_printf(&inst_str, &str_size, "%s", operand_str);
				}
			}				
		}
		else if (is_token(fmt_str, "64_SRC1", &token_len))
		{
			operand_dump_series(operand_str, inst->micro_inst.vop3a.src1, inst->micro_inst.vop3a.src1 + 1);
			
			if (!(IN_RANGE(inst->micro_inst.vop3a.op, 293, 298)) && !(IN_RANGE(inst->micro_inst.vop3a.op, 365, 366)))
			{
				if ((inst->micro_inst.vop3a.neg & 2) && (inst->micro_inst.vop3a.abs & 2))
				{
					str_printf(&inst_str, &str_size, "-abs(%s)", operand_str);
				}
				else if ((inst->micro_inst.vop3a.neg & 2) && !(inst->micro_inst.vop3a.abs & 2))
				{
					str_printf(&inst_str, &str_size, "-%s", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 2) && (inst->micro_inst.vop3a.abs & 2))
				{
					str_printf(&inst_str, &str_size, "abs(%s)", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 2) && !(inst->micro_inst.vop3a.abs & 2))
				{
					str_printf(&inst_str, &str_size, "%s", operand_str);
				}
			}
			else
			{
				if (inst->micro_inst.vop3a.neg & 2)
				{
					str_printf(&inst_str, &str_size, "-%s", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 2))
				{
					str_printf(&inst_str, &str_size, "%s", operand_str);
				}
			}
		}
		else if (is_token(fmt_str, "SRC2", &token_len))
		{
			operand_dump(operand_str, inst->micro_inst.vop3a.src2);
			
			if (!(IN_RANGE(inst->micro_inst.vop3a.op, 293, 298)) && !(IN_RANGE(inst->micro_inst.vop3a.op, 365, 366)))
			{
				if ((inst->micro_inst.vop3a.neg & 4) && (inst->micro_inst.vop3a.abs & 4))
				{
					str_printf(&inst_str, &str_size, "-abs(%s)", operand_str);
				}
				else if ((inst->micro_inst.vop3a.neg & 4) && !(inst->micro_inst.vop3a.abs & 4))
				{
					str_printf(&inst_str, &str_size, "-%s", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 4) && (inst->micro_inst.vop3a.abs & 4))
				{
					str_printf(&inst_str, &str_size, "abs(%s)", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 4) && !(inst->micro_inst.vop3a.abs & 4))
				{
					str_printf(&inst_str, &str_size, "%s", operand_str);
				}
			}
			else
			{
				if (inst->micro_inst.vop3a.neg & 4)
				{
					str_printf(&inst_str, &str_size, "-%s", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 4))
				{
					str_printf(&inst_str, &str_size, "%s", operand_str);
				}
			}
		}
		else if (is_token(fmt_str, "64_SRC2", &token_len))
		{
			operand_dump_series(operand_str, inst->micro_inst.vop3a.src2, inst->micro_inst.vop3a.src2 + 1);
			
			if (!(IN_RANGE(inst->micro_inst.vop3a.op, 293, 298)) && !(IN_RANGE(inst->micro_inst.vop3a.op, 365, 366)))
			{
				if ((inst->micro_inst.vop3a.neg & 4) && (inst->micro_inst.vop3a.abs & 4))
				{
					str_printf(&inst_str, &str_size, "-abs(%s)", operand_str);
				}
				else if ((inst->micro_inst.vop3a.neg & 4) && !(inst->micro_inst.vop3a.abs & 4))
				{
					str_printf(&inst_str, &str_size, "-%s", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 4) && (inst->micro_inst.vop3a.abs & 4))
				{
					str_printf(&inst_str, &str_size, "abs(%s)", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 4) && !(inst->micro_inst.vop3a.abs & 4))
				{
					str_printf(&inst_str, &str_size, "%s", operand_str);
				}
			}
			else
			{
				if (inst->micro_inst.vop3a.neg & 4)
				{
					str_printf(&inst_str, &str_size, "-%s", operand_str);
				}
				else if (!(inst->micro_inst.vop3a.neg & 4))
				{
					str_printf(&inst_str, &str_size, "%s", operand_str);
				}
			}
		}
		else if (is_token(fmt_str, "OP16", &token_len))
		{
			str_printf(&inst_str, &str_size, "%s", str_map_value(&OP16_map, (inst->micro_inst.vop3a.op & 15)));
		}
		else if (is_token(fmt_str, "OP8", &token_len))
		{
			str_printf(&inst_str, &str_size, "%s", str_map_value(&OP8_map, (inst->micro_inst.vop3a.op & 15)));
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_vopc(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
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
				str_printf(&inst_str, &str_size, "0x%08x", vopc->lit_cnst);
			}
			else
			{
				operand_dump(operand_str, vopc->src0);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "64_SRC0", &token_len))
		{
			assert(vopc->src0 != 0xFF);

			operand_dump_series(operand_str, vopc->src0, vopc->src0 + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VSRC1", &token_len))
		{
			operand_dump_vector(operand_str, vopc->vsrc1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_VSRC1", &token_len))
		{
			assert(vopc->vsrc1 != 0xFF);

			operand_dump_series_vector(operand_str, vopc->vsrc1, vopc->vsrc1 + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_vop1(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
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
			operand_dump_vector(operand_str, vop1->vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_VDST", &token_len))
		{
			operand_dump_series_vector(operand_str, vop1->vdst, vop1->vdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SVDST", &token_len))
		{
			operand_dump_scalar(operand_str, vop1->vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SRC0", &token_len))
		{
			if (vop1->src0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "0x%08x", vop1->lit_cnst);
			}
			else
			{
				operand_dump(operand_str, vop1->src0);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "64_SRC0", &token_len))
		{
			assert(vop1->src0 != 0xFF);

			operand_dump_series(operand_str, vop1->src0, vop1->src0 + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_vop2(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
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
			operand_dump_vector(operand_str, vop2->vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SRC0", &token_len))
		{
			if (vop2->src0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "0x%08x", vop2->lit_cnst);
			}
			else
			{
				operand_dump(operand_str, vop2->src0);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "VSRC1", &token_len))
		{
			operand_dump_vector(operand_str, vop2->vsrc1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_vintrp(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
{
	struct si_fmt_vintrp_t *vintrp = &inst->micro_inst.vintrp;

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
			operand_dump_vector(operand_str, vintrp->vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VSRC_I_J", &token_len))
		{
			operand_dump_vector(operand_str, vintrp->vsrc);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "ATTR", &token_len))
		{
			str_printf(&inst_str, &str_size, "attr_%d", vintrp->attr);
		}
		else if (is_token(fmt_str, "ATTRCHAN", &token_len))
		{
			switch (vintrp->attrchan)
			{
				case 0:
					str_printf(&inst_str, &str_size, "x");
					break;
				case 1:
					str_printf(&inst_str, &str_size, "y");
					break;
				case 2:
					str_printf(&inst_str, &str_size, "z");
					break;
				case 3:
					str_printf(&inst_str, &str_size, "w");
					break;
				default:
					break;
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_ds(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
{
	struct si_fmt_ds_t *ds = &inst->micro_inst.ds;

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
			operand_dump_vector(operand_str, ds->vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "ADDR", &token_len))
		{
			operand_dump_vector(operand_str, ds->addr);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "DATA0", &token_len))
		{
			operand_dump_vector(operand_str, ds->data0);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_mtbuf(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
{
	struct si_fmt_mtbuf_t *mtbuf = &inst->micro_inst.mtbuf;
	int vdata_end;

	switch (mtbuf->op)
	{
		case 0:
		case 4:
			vdata_end = mtbuf->vdata + 0;
			break;
		case 1:
		case 5:
			vdata_end = mtbuf->vdata + 1;
			break;
		case 2:
		case 6:
			vdata_end = mtbuf->vdata + 2;
			break;
		case 3:
		case 7:
			vdata_end = mtbuf->vdata + 3;
			break;
		default:
			fatal("mtbuf opcode not recognized.");
	}

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
		if (is_token(fmt_str, "SERIES_VDATA", &token_len))
		{
			operand_dump_series_vector(operand_str, mtbuf->vdata, vdata_end);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VADDR", &token_len))
		{
			if (mtbuf->offen && mtbuf->index)
			{
				operand_dump_series_vector(operand_str, mtbuf->vaddr, mtbuf->vaddr + 1);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
			else
			{
				operand_dump_vector(operand_str, mtbuf->vaddr);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "SERIES_SRSRC", &token_len))
		{
			assert((mtbuf->srsrc << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, mtbuf->srsrc << 2, (mtbuf->srsrc << 2) + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SOFFSET", &token_len))
		{
			assert(mtbuf->soffset <= 103 ||
				mtbuf->soffset == 124 || 
				(mtbuf->soffset >= 128 && mtbuf->soffset <= 208));
			operand_dump_scalar(operand_str, mtbuf->soffset);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "INDEX", &token_len))
		{
			if (mtbuf->index)
			{
				str_printf(&inst_str, &str_size, " idxen");
			}
		}
		else if (is_token(fmt_str, "OFFEN", &token_len))
		{
			if (mtbuf->offen)
			{
				str_printf(&inst_str, &str_size, " offen");
			}
		}
		else if (is_token(fmt_str, "OFFSET", &token_len))
		{
			if (mtbuf->offset)
			{
				str_printf(&inst_str, &str_size, " offset:%d", mtbuf->offset);
			}
		}
		else if (is_token(fmt_str, "DFMT", &token_len))
		{
			str_printf(&inst_str, &str_size, "%s", str_map_value(&dfmt_map, mtbuf->dfmt));
		}
		else if (is_token(fmt_str, "NFMT", &token_len))
		{
			str_printf(&inst_str, &str_size, "%s", str_map_value(&nfmt_map, mtbuf->nfmt));	
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_mubuf(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
{
	struct si_fmt_mubuf_t *mubuf = &inst->micro_inst.mubuf;
	int vdata_end;

	switch (mubuf->op)
	{
		case 0:
		case 4:
		case 9:
		case 50:
			vdata_end = mubuf->vdata + 0;
			break;
		case 1:
		case 5:
			vdata_end = mubuf->vdata + 1;
			break;
		case 2:
		case 6:
			vdata_end = mubuf->vdata + 2;
			break;
		case 3:
		case 7:
			vdata_end = mubuf->vdata + 3;
			break;
	}

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
		if (is_token(fmt_str, "SERIES_VDATA", &token_len))
		{
			operand_dump_series_vector(operand_str, mubuf->vdata, vdata_end);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VADDR", &token_len))
		{
			if (mubuf->offen && mubuf->index)
			{
				operand_dump_series_vector(operand_str, mubuf->vaddr, mubuf->vaddr + 1);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
			else
			{
				operand_dump_vector(operand_str, mubuf->vaddr);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "SERIES_SRSRC", &token_len))
		{
			assert((mubuf->srsrc << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, mubuf->srsrc << 2, (mubuf->srsrc << 2) + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SOFFSET", &token_len))
		{
			assert(mubuf->soffset <= 103 ||
				mubuf->soffset == 124 || 
				(mubuf->soffset >= 128 && mubuf->soffset <= 208));
			operand_dump_scalar(operand_str, mubuf->soffset);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "INDEX", &token_len))
		{
			if (mubuf->index)
			{
				str_printf(&inst_str, &str_size, " idxen");
			}
		}
		else if (is_token(fmt_str, "OFFEN", &token_len))
		{
			if (mubuf->offen)
			{
				str_printf(&inst_str, &str_size, " offen");
			}
		}
		else if (is_token(fmt_str, "OFFSET", &token_len))
		{
			if (mubuf->offset)
			{
				str_printf(&inst_str, &str_size, " offset:%d", mubuf->offset);
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
}

void si_inst_dump_mimg(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
{
	
	struct si_fmt_mimg_t *mimg = &inst->micro_inst.mimg;

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
		if (is_token(fmt_str, "SERIES_VDATA", &token_len))
		{
			operand_dump_series_vector(operand_str, mimg->vdata, mimg->vdata + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VADDR", &token_len))
		{
			operand_dump_series_vector(operand_str, mimg->vaddr, mimg->vaddr + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SERIES_SRSRC", &token_len))
		{
			assert((mimg->srsrc << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, mimg->srsrc << 2, (mimg->srsrc << 2) + 7);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SERIES_SSAMP", &token_len))
		{
			assert((mimg->ssamp << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, mimg->ssamp << 2, (mimg->ssamp << 2) + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "DMASK", &token_len))
		{
			str_printf(&inst_str, &str_size, " dmask:0x%01x", mimg->dmask);
		}
		else if (is_token(fmt_str, "UNORM", &token_len))
		{
			if (mimg->unorm)
			{
				str_printf(&inst_str, &str_size, " unorm");
			}
		}
		else if (is_token(fmt_str, "GLC", &token_len))
		{
			if (mimg->glc)
			{
				str_printf(&inst_str, &str_size, " glc");
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);

}

void si_inst_dump_exp(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size)
{
	struct si_fmt_exp_t *exp = &inst->micro_inst.exp;

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
		if (is_token(fmt_str, "TGT", &token_len))
		{
			operand_dump_exp(operand_str, exp->tgt);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VSRC0", &token_len))
		{
			if (exp->compr == 0 && (exp->en && 0x0) == 0x0)
			{
				operand_dump_vector(operand_str, exp->vsrc0);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
			/* FIXME: when exp->compr == 1, export float16 format */
		}
		else if (is_token(fmt_str, "VSRC1", &token_len))
		{
			if (exp->compr == 0 && (exp->en && 0x2) == 0x2)
			{
				operand_dump_vector(operand_str, exp->vsrc1);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "VSRC2", &token_len))
		{
			if (exp->compr == 0 && (exp->en && 0x4) == 0x4)
			{
				operand_dump_vector(operand_str, exp->vsrc2);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else if (is_token(fmt_str, "VSRC3", &token_len))
		{
			if (exp->compr == 0 && (exp->en && 0x8) == 0x8)
			{
				operand_dump_vector(operand_str, exp->vsrc3);
				str_printf(&inst_str, &str_size, "%s", operand_str);
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}

	line_dump(orig_inst_str, rel_addr, buf, line, line_size, inst_size);
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
