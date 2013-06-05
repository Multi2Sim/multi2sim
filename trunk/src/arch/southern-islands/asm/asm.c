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


struct si_inst_info_t si_inst_info[SI_INST_COUNT];


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
#define SI_INST_INFO_EXP_MAX_VALUE  0

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


struct str_map_t si_inst_fmt_map =
{
	18,
	{
		{ "<invalid>", SI_FMT_NONE },
		{ "sop2", SI_FMT_SOP2 },
		{ "sopk", SI_FMT_SOPK },
		{ "sop1", SI_FMT_SOP1 },
		{ "sopc", SI_FMT_SOPC },
		{ "sopp", SI_FMT_SOPP },
		{ "smrd", SI_FMT_SMRD },
		{ "vop2", SI_FMT_VOP2 },
		{ "vop1", SI_FMT_VOP1 },
		{ "vopc", SI_FMT_VOPC },
		{ "vop3a", SI_FMT_VOP3a },
		{ "vop3b", SI_FMT_VOP3b },
		{ "vintrp", SI_FMT_VINTRP },
		{ "ds", SI_FMT_DS },
		{ "mubuf", SI_FMT_MUBUF },
		{ "mtbuf", SI_FMT_MTBUF },
		{ "mimg", SI_FMT_MIMG },
		{ "exp", SI_FMT_EXP }
	}
};

/* String maps for assembly dump. */
struct str_map_t si_inst_sdst_map = {
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

struct str_map_t si_inst_ssrc_map = {
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

struct str_map_t si_inst_buf_data_format_map = {
	16, {
		{"invalid", si_inst_buf_data_format_invalid },
		{"BUF_DATA_FORMAT_8", si_inst_buf_data_format_8 },
		{"BUF_DATA_FORMAT_16", si_inst_buf_data_format_16 },
		{"BUF_DATA_FORMAT_8_8", si_inst_buf_data_format_8_8 },
		{"BUF_DATA_FORMAT_32", si_inst_buf_data_format_32 },
		{"BUF_DATA_FORMAT_16_16", si_inst_buf_data_format_16_16 },
		{"BUF_DATA_FORMAT_10_11_11", si_inst_buf_data_format_10_11_11 },
		{"BUF_DATA_FORMAT_11_10_10", si_inst_buf_data_format_11_10_10 },
		{"BUF_DATA_FORMAT_10_10_10_2", si_inst_buf_data_format_10_10_10_2 },
		{"BUF_DATA_FORMAT_2_10_10_10", si_inst_buf_data_format_2_10_10_10 },
		{"BUF_DATA_FORMAT_8_8_8_8", si_inst_buf_data_format_8_8_8_8 },
		{"BUF_DATA_FORMAT_32_32", si_inst_buf_data_format_32_32 },
		{"BUF_DATA_FORMAT_16_16_16_16", si_inst_buf_data_format_16_16_16_16 },
		{"BUF_DATA_FORMAT_32_32_32", si_inst_buf_data_format_32_32_32 },
		{"BUF_DATA_FORMAT_32_32_32_32", si_inst_buf_data_format_32_32_32_32 },
		{"reserved", si_inst_buf_data_format_reserved }
	}
};

struct str_map_t si_inst_buf_num_format_map = {
	14, {
		{"BUF_NUM_FORMAT_UNORM", si_inst_buf_num_format_unorm },
		{"BUF_NUM_FORMAT_SNORM", si_inst_buf_num_format_snorm },
		{"BUF_NUM_FORMAT_UNSCALED", si_inst_buf_num_format_unscaled },
		{"BUF_NUM_FORMAT_SSCALED", si_inst_buf_num_format_sscaled },
		{"BUF_NUM_FORMAT_UINT", si_inst_buf_num_format_uint },
		{"BUF_NUM_FORMAT_SINT", si_inst_buf_num_format_sint },
		{"BUF_NUM_FORMAT_SNORM_NZ", si_inst_buf_num_format_snorm_nz },
		{"BUF_NUM_FORMAT_FLOAT", si_inst_buf_num_format_float },
		{"reserved", si_inst_buf_num_format_reserved },
		{"BUF_NUM_FORMAT_SRGB", si_inst_buf_num_format_srgb },
		{"BUF_NUM_FORMAT_UBNORM", si_inst_buf_num_format_ubnorm },
		{"BUF_NUM_FORMAT_UBNORM_NZ", si_inst_buf_num_format_ubnorm_nz },
		{"BUF_NUM_FORMAT_UBINT", si_inst_buf_num_format_ubint },
		{"BUF_NUM_FORMAT_UBSCALED", si_inst_buf_num_format_ubscaled }
	}
};

struct str_map_t si_inst_OP16_map = {
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

struct str_map_t si_inst_OP8_map = {
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

struct str_map_t si_inst_special_reg_map = {
	4, {
		{ "vcc", si_inst_special_reg_vcc },
		{ "scc", si_inst_special_reg_scc },
		{ "exec", si_inst_special_reg_exec },
		{ "tma", si_inst_special_reg_tma }
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
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_SOPP_MAX_VALUE));
			si_inst_info_sopp[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOPC)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_SOPC_MAX_VALUE));
			si_inst_info_sopc[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOP1)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_SOP1_MAX_VALUE));
			si_inst_info_sop1[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOPK)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_SOPK_MAX_VALUE));
			si_inst_info_sopk[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOP2)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_SOP2_MAX_VALUE));
			si_inst_info_sop2[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SMRD) 
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_SMRD_MAX_VALUE));
			si_inst_info_smrd[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOP3a || info->fmt == SI_FMT_VOP3b)
		{
			int i;

			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_VOP3_MAX_VALUE));
			si_inst_info_vop3[info->opcode] = info;
			if (info->flags & SI_INST_FLAG_OP8)
			{
				for (i = 1; i < 8; i++)
				{
					si_inst_info_vop3[info->opcode + i] = 
						info;
				}
			}
			if (info->flags & SI_INST_FLAG_OP16)
			{
				for (i = 1; i < 16; i++)
				{
					si_inst_info_vop3[info->opcode + i] = 
						info;
				}
			}
			continue;
		}
		else if (info->fmt == SI_FMT_VOPC)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_VOPC_MAX_VALUE));
			si_inst_info_vopc[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOP1)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_VOP1_MAX_VALUE));
			si_inst_info_vop1[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOP2)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_VOP2_MAX_VALUE));
			si_inst_info_vop2[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VINTRP)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_VINTRP_MAX_VALUE));
			si_inst_info_vintrp[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_DS)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_DS_MAX_VALUE));
			si_inst_info_ds[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_MTBUF)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_MTBUF_MAX_VALUE));
			si_inst_info_mtbuf[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_MUBUF)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_MUBUF_MAX_VALUE));
			si_inst_info_mubuf[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_MIMG)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_MIMG_MAX_VALUE));
			si_inst_info_mimg[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_EXP)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_EXP_MAX_VALUE));
			si_inst_info_exp[info->opcode] = info;
			continue;
		}
		else 
		{
			fprintf(stderr, "warning: '%s' not indexed\n", 
				info->name);
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
		{
			fatal("Unimplemented Instruction: SOPP:%d  "
				"// %08X: %08X\n", inst->micro_inst.sopp.op, 
				offset, * (unsigned int *) buf);
		}

		inst->info = si_inst_info_sopp[inst->micro_inst.sopp.op];
	}
	else if (inst->micro_inst.sopc.enc == 0x17E)
	{
		if (!si_inst_info_sopc[inst->micro_inst.sopc.op])
		{
			fatal("Unimplemented Instruction: SOPC:%d  "
				"// %08X: %08X\n", inst->micro_inst.sopc.op, 
				offset, * (unsigned int *) buf);
		}

		inst->info = si_inst_info_sopc[inst->micro_inst.sopc.op];

		/* Only one source field may use a literal constant, 
		 * which is indicated by 0xFF. */
		assert(!(inst->micro_inst.sopc.ssrc0 == 0xFF && 
			inst->micro_inst.sopc.ssrc1 == 0xFF));
		if (inst->micro_inst.sopc.ssrc0 == 0xFF || 
			inst->micro_inst.sopc.ssrc1 == 0xFF)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}
	}
	else if (inst->micro_inst.sop1.enc == 0x17D)
	{
		if (!si_inst_info_sop1[inst->micro_inst.sop1.op])
		{
			fatal("Unimplemented Instruction: SOP1:%d  "
				"// %08X: %08X\n", inst->micro_inst.sop1.op, 
				offset, *(unsigned int*)buf);
		}

		inst->info = si_inst_info_sop1[inst->micro_inst.sop1.op];

		/* 0xFF indicates the use of a literal constant as a 
		 * source operand. */
		if (inst->micro_inst.sop1.ssrc0 == 0xFF)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}	
	}
	else if (inst->micro_inst.sopk.enc == 0xB)
	{
		if (!si_inst_info_sopk[inst->micro_inst.sopk.op])
		{
			fatal("Unimplemented Instruction: SOPK:%d  "
				"// %08X: %08X\n", inst->micro_inst.sopk.op, 
				offset, * (unsigned int *) buf);
		}

		inst->info = si_inst_info_sopk[inst->micro_inst.sopk.op];
	}
	else if (inst->micro_inst.sop2.enc == 0x2)
	{
		if (!si_inst_info_sop2[inst->micro_inst.sop2.op])
		{
			fatal("Unimplemented Instruction: SOP2:%d  "
				"// %08X: %08X\n", inst->micro_inst.sop2.op, 
				offset, *(unsigned int *)buf);
		}

		inst->info = si_inst_info_sop2[inst->micro_inst.sop2.op];

		/* Only one source field may use a literal constant, 
		 * which is indicated by 0xFF. */
		assert(!(inst->micro_inst.sop2.ssrc0 == 0xFF && 
			inst->micro_inst.sop2.ssrc1 == 0xFF));
		if (inst->micro_inst.sop2.ssrc0 == 0xFF || 
			inst->micro_inst.sop2.ssrc1 == 0xFF)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}
	}
	else if (inst->micro_inst.smrd.enc == 0x18)
	{
		if (!si_inst_info_smrd[inst->micro_inst.smrd.op])
		{
			fatal("Unimplemented Instruction: SMRD:%d  "
				"// %08X: %08X\n", inst->micro_inst.smrd.op, 
				offset, *(unsigned int *)buf);
		}

		inst->info = si_inst_info_smrd[inst->micro_inst.smrd.op];
	}
	else if (inst->micro_inst.vop3a.enc == 0x34)
	{
		/* 64 bit instruction. */
		inst_size = 8;
		memcpy(&inst->micro_inst, buf, inst_size);

		if (!si_inst_info_vop3[inst->micro_inst.vop3a.op])
		{
			fatal("Unimplemented Instruction: VOP3:%d  "
				"// %08X: %08X %08X\n",
				inst->micro_inst.vop3a.op, offset, 
				*(unsigned int *)buf,
				*(unsigned int *)(buf + 4));
		}

		inst->info = si_inst_info_vop3[inst->micro_inst.vop3a.op];
	}
	else if (inst->micro_inst.vopc.enc == 0x3E)
	{
		if (!si_inst_info_vopc[inst->micro_inst.vopc.op])
		{
			fatal("Unimplemented Instruction: VOPC:%d  "
				"// %08X: %08X\n",
				inst->micro_inst.vopc.op, offset, 
				*(unsigned int *)buf);
		}

		inst->info = si_inst_info_vopc[inst->micro_inst.vopc.op];

		/* 0xFF indicates the use of a literal constant as a 
		 * source operand. */
		if (inst->micro_inst.vopc.src0 == 0xFF)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}
	}
	else if (inst->micro_inst.vop1.enc == 0x3F)
	{
		if (!si_inst_info_vop1[inst->micro_inst.vop1.op])
		{
			fatal("Unimplemented Instruction: VOP1:%d  "
				"// %08X: %08X\n", inst->micro_inst.vop1.op, 
				offset, * (unsigned int *) buf);
		}

		inst->info = si_inst_info_vop1[inst->micro_inst.vop1.op];

		/* 0xFF indicates the use of a literal constant as a 
		 * source operand. */
		if (inst->micro_inst.vop1.src0 == 0xFF)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}
	}
	else if (inst->micro_inst.vop2.enc == 0x0)
	{
		if (!si_inst_info_vop2[inst->micro_inst.vop2.op])
		{
			fatal("Unimplemented Instruction: VOP2:%d  "
				"// %08X: %08X\n", inst->micro_inst.vop2.op, 
				offset, * (unsigned int *) buf);
		}

		inst->info = si_inst_info_vop2[inst->micro_inst.vop2.op];

		/* 0xFF indicates the use of a literal constant as a 
		 * source operand. */
		if (inst->micro_inst.vop2.src0 == 0xFF)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}

		/* Some opcodes define a 32-bit literal constant following
		 * the instruction */
		if (inst->micro_inst.vop2.op == 32)
		{
			inst_size = 8;
			memcpy(&inst->micro_inst, buf, inst_size);
		}
	}
	else if (inst->micro_inst.vintrp.enc == 0x32)
	{
		if (!si_inst_info_vintrp[inst->micro_inst.vintrp.op])
		{
			fatal("Unimplemented Instruction: VINTRP:%d  "
				"// %08X: %08X\n", inst->micro_inst.vintrp.op,
				offset, * (unsigned int *) buf);
		}

		inst->info = si_inst_info_vintrp[inst->micro_inst.vintrp.op];

	}
	else if (inst->micro_inst.ds.enc == 0x36)
	{
		/* 64 bit instruction. */
		inst_size = 8;
		memcpy(&inst->micro_inst, buf, inst_size);

		if (!si_inst_info_ds[inst->micro_inst.ds.op])
		{
			fatal("Unimplemented Instruction: DS:%d  "
				"// %08X: %08X %08X\n", inst->micro_inst.ds.op, 
				offset, *(unsigned int *)buf,
				*(unsigned int *)(buf + 4));
		}

		inst->info = si_inst_info_ds[inst->micro_inst.ds.op];
	}
	else if (inst->micro_inst.mtbuf.enc == 0x3A)
	{
		/* 64 bit instruction. */
		inst_size = 8;
		memcpy(&inst->micro_inst, buf, inst_size);

		if (!si_inst_info_mtbuf[inst->micro_inst.mtbuf.op])
		{
			fatal("Unimplemented Instruction: MTBUF:%d  "
				"// %08X: %08X %08X\n", 
				inst->micro_inst.mtbuf.op, offset, 
				*(unsigned int *)buf, *(unsigned int *)(buf+4));
		}

		inst->info = si_inst_info_mtbuf[inst->micro_inst.mtbuf.op];
	}
	else if (inst->micro_inst.mubuf.enc == 0x38)
	{
		/* 64 bit instruction. */
		inst_size = 8;
		memcpy(&inst->micro_inst, buf, inst_size);

		if(!si_inst_info_mubuf[inst->micro_inst.mubuf.op])
		{
			fatal("Unimplemented Instruction: MUBUF:%d  "
				"// %08X: %08X %08X\n", 
				inst->micro_inst.mubuf.op, offset, 
				*(unsigned int *)buf,
				*(unsigned int *)(buf+4));
		}

		inst->info = si_inst_info_mubuf[inst->micro_inst.mubuf.op];
	}
	else if (inst->micro_inst.mimg.enc == 0x3C)
	{
		/* 64 bit instruction. */
		inst_size = 8;
		memcpy(&inst->micro_inst, buf, inst_size);

		if(!si_inst_info_mimg[inst->micro_inst.mimg.op])
		{
			fatal("Unimplemented Instruction: MIMG:%d  "
				"// %08X: %08X %08X\n", 
				inst->micro_inst.mimg.op, offset, 
				*(unsigned int *)buf,
				*(unsigned int *)(buf+4));
		}

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
		fatal("Unimplemented format. Instruction is:  // %08X: %08X\n", 
				offset, ((unsigned int*)buf)[0]);
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
		if (inst.info->fmt == SI_FMT_SOPP && 
			inst.micro_inst.sopp.op == 1)
		{
			break;
		}
		/* If the instruction branches, insert the label into 
		 * the sorted list. */
		if (inst.info->fmt == SI_FMT_SOPP &&
			(inst.micro_inst.sopp.op >= 2 && 
			 inst.micro_inst.sopp.op <= 9))
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

		si_inst_dump(&inst, inst_size, rel_addr, inst_buf, line, 
			line_size);
		fprintf(f, " %s", line);


		/* Break at end of program. */
		if (inst.info->fmt == SI_FMT_SOPP && 
			inst.micro_inst.sopp.op == 1)
		{
			break;
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
		str_printf(&pstr, &str_size, "%s", 
			str_map_value(&si_inst_sdst_map, operand - 104));
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
		str_printf(&pstr, &str_size, "%s", 
			str_map_value(&si_inst_ssrc_map, operand - 240));
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
			str_printf(&pstr, &str_size, "ttmp[%d:%d]", 
				operand - 112, operand_end - 112);
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
					fatal("Unimplemented operand series: "
						"[%d:%d]", operand, 
						operand_end);
			}
		}
	}
	else if (operand <= 255)
	{
		fatal("Illegal operand series: [%d:%d]", operand, operand_end);
	}
	else if (operand <= 511)
	{
		str_printf(&pstr, &str_size, "v[%d:%d]", operand - 256, 
			operand_end - 256);
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
		str_printf(&pstr, &str_size, "exp_prm_%d", operand - 32);
	}
}

void line_dump(char *inst_str, unsigned int rel_addr, void *buf, char *line, int line_size, int inst_size)
{
	int dat_str_size = MAX_DAT_STR_SIZE;
	char inst_dat_str[MAX_DAT_STR_SIZE];
	char *dat_str = &inst_dat_str[0];

	if (inst_size == 4)
	{
		str_printf(&dat_str, &dat_str_size, "// %08X: %08X", rel_addr,
			((unsigned int*)buf)[0]);
	}
	else
	{
		str_printf(&dat_str, &dat_str_size, "// %08X: %08X %08X", 
			rel_addr, ((unsigned int*)buf)[0], 
			((unsigned int*)buf)[1]);
	}

	if (strlen(inst_str) < 59)
	{
		str_printf(&line, &line_size, "%-59s%s\n", inst_str, 
			inst_dat_str);
	}
	else
	{
		str_printf(&line, &line_size, "%s %s\n", inst_str, 
			inst_dat_str);
	}
}

void si_inst_SSRC_dump(struct si_inst_t *inst, unsigned int ssrc, 
	char *operand_str, char **inst_str, int str_size)
{
	if (ssrc == 0xFF)
	{
		str_printf(inst_str, &str_size, "0x%08x", 
			inst->micro_inst.sop2.lit_cnst);
	}
	else
	{
		operand_dump_scalar(operand_str, ssrc);
		str_printf(inst_str, &str_size, "%s", operand_str);
	}
}

void si_inst_64_SSRC_dump(struct si_inst_t *inst, unsigned int ssrc, 
	char *operand_str, char **inst_str, int str_size)
{		
	if (ssrc == 0xFF)
	{
		str_printf(inst_str, &str_size, "0x%08x", 
			inst->micro_inst.sop2.lit_cnst);
	}
	else
	{
		operand_dump_series_scalar(operand_str, ssrc, ssrc + 1);
		str_printf(inst_str, &str_size, "%s", operand_str);
	}
}

void si_inst_VOP3_SRC_dump(struct si_inst_t *inst, unsigned int src, int neg, 
	char *operand_str, char **inst_str, int str_size)
{
	operand_dump(operand_str, src);

	if (!(IN_RANGE(inst->micro_inst.vop3a.op, 293, 298)) && 
		!(IN_RANGE(inst->micro_inst.vop3a.op, 365, 366)))
	{
		if ((inst->micro_inst.vop3a.neg & neg) && 
			(inst->micro_inst.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "-abs(%s)", 
				operand_str);
		}
		else if ((inst->micro_inst.vop3a.neg & neg) && 
			!(inst->micro_inst.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "-%s", operand_str);
		}
		else if (!(inst->micro_inst.vop3a.neg & neg) && 
			(inst->micro_inst.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "abs(%s)", operand_str);
		}
		else if (!(inst->micro_inst.vop3a.neg & neg) && 
			!(inst->micro_inst.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "%s", operand_str);
		}
	}
	else
	{
		if (inst->micro_inst.vop3a.neg & neg)
		{
			str_printf(inst_str, &str_size, "-%s", operand_str);
		}
		else if (!(inst->micro_inst.vop3a.neg & neg))
		{
			str_printf(inst_str, &str_size, "%s", operand_str);
		}
	}
}

void si_inst_VOP3_64_SRC_dump(struct si_inst_t *inst, unsigned int src, int neg, char *operand_str, char **inst_str, int str_size)
{
	operand_dump_series(operand_str, src, src + 1);
	
	if (!(IN_RANGE(inst->micro_inst.vop3a.op, 293, 298)) && 
		!(IN_RANGE(inst->micro_inst.vop3a.op, 365, 366)))
	{
		if ((inst->micro_inst.vop3a.neg & neg) && 
			(inst->micro_inst.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "-abs(%s)", 
				operand_str);
		}
		else if ((inst->micro_inst.vop3a.neg & neg) && 
			!(inst->micro_inst.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "-%s", operand_str);
		}
		else if (!(inst->micro_inst.vop3a.neg & neg) && 
			(inst->micro_inst.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "abs(%s)", operand_str);
		}
		else if (!(inst->micro_inst.vop3a.neg & neg) && 
			!(inst->micro_inst.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "%s", operand_str);
		}
	}
	else
	{
		if (inst->micro_inst.vop3a.neg & neg)
		{
			str_printf(inst_str, &str_size, "-%s", operand_str);
		}
		else if (!(inst->micro_inst.vop3a.neg & neg))
		{
			str_printf(inst_str, &str_size, "%s", operand_str);
		}
	}
}
void si_inst_SERIES_VDATA_dump(unsigned int vdata, int op, char *operand_str, 
	char **inst_str, int str_size)
{
	int vdata_end;

	switch (op)
	{
		case 0:
		case 4:
		case 9:
		case 12:
		case 24:
		case 28:
		case 50:
			vdata_end = vdata + 0;
			break;
		case 1:
		case 5:
			vdata_end = vdata + 1;
			break;
		case 2:
		case 6:
			vdata_end = vdata + 2;
			break;
		case 3:
		case 7:
			vdata_end = vdata + 3;
			break;
		default:
			fatal("MUBUF/MTBUF opcode not recognized");
	}

	operand_dump_series_vector(operand_str, vdata, vdata_end);
	str_printf(inst_str, &str_size, "%s", operand_str);
}

void si_inst_MADDR_dump(struct si_inst_t *inst, char *operand_str, 
	char **inst_str, int str_size)
{
	/* soffset */
	assert(inst->micro_inst.mtbuf.soffset <= 103 ||
		inst->micro_inst.mtbuf.soffset == 124 ||
		(inst->micro_inst.mtbuf.soffset >= 128 && 
		inst->micro_inst.mtbuf.soffset <= 208));
	operand_dump_scalar(operand_str, inst->micro_inst.mtbuf.soffset);
	str_printf(inst_str, &str_size, "%s", operand_str);

	/* offen */
	if (inst->micro_inst.mtbuf.offen)
		str_printf(inst_str, &str_size, " offen");

	/* index */
	if (inst->micro_inst.mtbuf.idxen)
		str_printf(inst_str, &str_size, " idxen");

	/* offset */
	if (inst->micro_inst.mtbuf.offset)
		str_printf(inst_str, &str_size, " offset:%d", 
			inst->micro_inst.mtbuf.offset);
}

void si_inst_DUG_dump(struct si_inst_t *inst, char *operand_str, 
	char **inst_str, int str_size)
{
	/* DMASK */
	str_printf(inst_str, &str_size, " dmask:0x%01x", 
		inst->micro_inst.mimg.dmask);
	
	/* UNORM */
	if (inst->micro_inst.mimg.unorm)
		str_printf(inst_str, &str_size, " unorm");
	
	/* GLC */
	if (inst->micro_inst.mimg.glc)
		str_printf(inst_str, &str_size, " glc");
}

void si_inst_dump(struct si_inst_t *inst, unsigned int inst_size, 
	unsigned int rel_addr, void *buf, char *line, int line_size)
{
	int str_size = MAX_INST_STR_SIZE;
	int token_len;
	
	char orig_inst_str[MAX_INST_STR_SIZE];
	char orig_operand_str[MAX_OPERAND_STR_SIZE];
	
	char *operand_str = &orig_operand_str[0];
	char *inst_str = &orig_inst_str[0];
	char *fmt_str = inst->info->fmt_str;

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
			struct si_fmt_sopp_t *sopp = &inst->micro_inst.sopp;

			unsigned int and = 0;
			int vm_cnt = (sopp->simm16 & 0xF);

			if (vm_cnt != 0xF)
			{
				str_printf(&inst_str, &str_size, "vmcnt(%d)",
					vm_cnt);
				and = 1;
			}

			int lgkm_cnt = (sopp->simm16 & 0x1f00) >> 8;

			if (lgkm_cnt != 0x1f)
			{
				if (and)
				{
					str_printf(&inst_str, &str_size, " & ");
				}

				str_printf(&inst_str, &str_size, "lgkmcnt(%d)", 
					lgkm_cnt);
				and = 1;
			}

			int exp_cnt = (sopp->simm16 & 0x70) >> 4;

			if (exp_cnt != 0x7)
			{
				if (and)
				{
					str_printf(&inst_str, &str_size, " & ");
				}

				str_printf(&inst_str, &str_size, "expcnt(%d)",
					exp_cnt);
				and = 1;
			}
		}
		else if (is_token(fmt_str, "LABEL", &token_len))
		{		
			struct si_fmt_sopp_t *sopp = &inst->micro_inst.sopp;
	
			short simm16 = sopp->simm16;
			int se_simm = simm16;

			str_printf(&inst_str, &str_size, "label_%04X", 
				(rel_addr + (se_simm * 4) + 4) / 4);
		}
		else if (is_token(fmt_str, "SSRC0", &token_len))
		{	
			si_inst_SSRC_dump(inst, inst->micro_inst.sop2.ssrc0, 
				operand_str, &inst_str, str_size);
		}
		else if (is_token(fmt_str, "64_SSRC0", &token_len))
		{
			si_inst_64_SSRC_dump(inst, inst->micro_inst.sop2.ssrc0, 
				operand_str, &inst_str, str_size);
		}
		else if (is_token(fmt_str, "SSRC1", &token_len))
		{
			si_inst_SSRC_dump(inst, inst->micro_inst.sop2.ssrc1, 
				operand_str, &inst_str, str_size);
		}
		else if (is_token(fmt_str, "64_SSRC1", &token_len))
		{
			si_inst_64_SSRC_dump(inst, inst->micro_inst.sop2.ssrc1, 
				operand_str, &inst_str, str_size);
		}
		else if (is_token(fmt_str, "SDST", &token_len))
		{	
			operand_dump_scalar(operand_str, 
				inst->micro_inst.sop2.sdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_SDST", &token_len))
		{
			operand_dump_series_scalar(operand_str, 
				inst->micro_inst.sop2.sdst, 
				inst->micro_inst.sop2.sdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SIMM16", &token_len))
		{
			str_printf(&inst_str, &str_size, "0x%04x", 
				inst->micro_inst.sopk.simm16);
		}
		else if (is_token(fmt_str, "SRC0", &token_len))
		{
			if (inst->micro_inst.vopc.src0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "0x%08x", 
					inst->micro_inst.vopc.lit_cnst);
			}
			else
			{
				operand_dump(operand_str, 
					inst->micro_inst.vopc.src0);
				str_printf(&inst_str, &str_size, "%s", 
					operand_str);
			}
		}
		else if (is_token(fmt_str, "64_SRC0", &token_len))
		{
			assert(inst->micro_inst.vopc.src0 != 0xFF);

			operand_dump_series(operand_str, 
				inst->micro_inst.vopc.src0, 
				inst->micro_inst.vopc.src0 + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VSRC1", &token_len))
		{
			operand_dump_vector(operand_str, 
				inst->micro_inst.vopc.vsrc1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_VSRC1", &token_len))
		{
			assert(inst->micro_inst.vopc.vsrc1 != 0xFF);

			operand_dump_series_vector(operand_str, 
				inst->micro_inst.vopc.vsrc1, 
				inst->micro_inst.vopc.vsrc1 + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VDST", &token_len))
		{
			operand_dump_vector(operand_str, 
				inst->micro_inst.vop1.vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_VDST", &token_len))
		{
			operand_dump_series_vector(operand_str, 
				inst->micro_inst.vop1.vdst, 
				inst->micro_inst.vop1.vdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SVDST", &token_len))
		{
			operand_dump_scalar(operand_str, 
				inst->micro_inst.vop1.vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VOP3_64_SVDST", &token_len))
		{
			/* VOP3a compare operations use the VDST field to 
			 * indicate the address of the scalar destination.*/
			operand_dump_series_scalar(operand_str, 
				inst->micro_inst.vop3a.vdst, 
				inst->micro_inst.vop3a.vdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VOP3_VDST", &token_len))
		{
			operand_dump_vector(operand_str, 
				inst->micro_inst.vop3a.vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VOP3_64_VDST", &token_len))
		{
			operand_dump_series_vector(operand_str, 
				inst->micro_inst.vop3a.vdst, 
				inst->micro_inst.vop3a.vdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VOP3_64_SDST", &token_len))
		{
			operand_dump_series_scalar(operand_str, 
				inst->micro_inst.vop3b.sdst, 
				inst->micro_inst.vop3b.sdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VOP3_SRC0", &token_len))
		{
			si_inst_VOP3_SRC_dump(inst, 
				inst->micro_inst.vop3a.src0, 1, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "VOP3_64_SRC0", &token_len))
		{
			si_inst_VOP3_64_SRC_dump(inst, 
				inst->micro_inst.vop3a.src0, 1, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "VOP3_SRC1", &token_len))
		{
			si_inst_VOP3_SRC_dump(inst, 
				inst->micro_inst.vop3a.src1, 2, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "VOP3_64_SRC1", &token_len))
		{
			si_inst_VOP3_64_SRC_dump(inst, 
				inst->micro_inst.vop3a.src1, 2, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "VOP3_SRC2", &token_len))
		{
			si_inst_VOP3_SRC_dump(inst, inst->micro_inst.vop3a.src2, 
				4, operand_str, &inst_str, str_size);
		}
		else if (is_token(fmt_str, "VOP3_64_SRC2", &token_len))
		{
			si_inst_VOP3_64_SRC_dump(inst, 
				inst->micro_inst.vop3a.src2, 4, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "VOP3_OP16", &token_len))
		{
			str_printf(&inst_str, &str_size, "%s", 
				str_map_value(&si_inst_OP16_map, 
					(inst->micro_inst.vop3a.op & 15)));
		}
		else if (is_token(fmt_str, "VOP3_OP8", &token_len))
		{
			str_printf(&inst_str, &str_size, "%s", 
				str_map_value(&si_inst_OP8_map, 
					(inst->micro_inst.vop3a.op & 15)));
		}
		else if (is_token(fmt_str, "SMRD_SDST", &token_len))
		{
			operand_dump_scalar(operand_str, 
				inst->micro_inst.smrd.sdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SERIES_SDST", &token_len))
		{
			
			/* The sbase field is missing the LSB, 
			 * so multiply by 2 */
			int sdst = inst->micro_inst.smrd.sdst;
			int sdst_end;
			int op = inst->micro_inst.smrd.op;

			/* S_LOAD_DWORD */
			if (IN_RANGE(op, 0, 4))
			{
				if (op != 0)
				{
					/* Multi-dword */
					switch (op)
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
							fatal("Invalid smrd "
								"opcode");
					}
				}
			}
			/* S_BUFFER_LOAD_DWORD */
			else if (IN_RANGE(op, 8, 12))
			{	
				if (op != 8)
				{
					/* Multi-dword */
					switch (op)
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
							fatal("Invalid smrd "
								"opcode");
					}
				}
			}
			/* S_MEMTIME */
			else if (op == 30)
			{
				fatal("S_MEMTIME instruction not currently" 
					"supported");
			}
			/* S_DCACHE_INV */
			else if (op == 31)
			{
				fatal("S_DCACHE_INV instruction not" 
					"currently supported");
			}
			else
			{
				fatal("Invalid smrd opcode");
			}

			operand_dump_series_scalar(operand_str, sdst, sdst_end);
			str_printf(&inst_str, &str_size, "%s", operand_str);

		}
		else if (is_token(fmt_str, "SERIES_SBASE", &token_len))
		{
			
			/* The sbase field is missing the LSB, 
			 * so multiply by 2 */
			int sbase = inst->micro_inst.smrd.sbase * 2;
			int sbase_end;
			int op = inst->micro_inst.smrd.op;

			/* S_LOAD_DWORD */
			if (IN_RANGE(op, 0, 4))
			{
				/* SBASE specifies two consecutive SGPRs */
				sbase_end = sbase + 1;
			}
			/* S_BUFFER_LOAD_DWORD */
			else if (IN_RANGE(op, 8, 12))
			{
				/* SBASE specifies four consecutive SGPRs */
				sbase_end = sbase + 3;
			}
			/* S_MEMTIME */
			else if (op == 30)
			{
				fatal("S_MEMTIME instruction not currently"
					" supported");
			}
			/* S_DCACHE_INV */
			else if (op == 31)
			{
				fatal("S_DCACHE_INV instruction not currently"
					" supported");
			}
			else
			{
				fatal("Invalid smrd opcode");
			}

			operand_dump_series_scalar(operand_str, sbase, 
				sbase_end);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VOP2_LIT", &token_len))
		{
			str_printf(&inst_str, &str_size, "0x%08x", 
				inst->micro_inst.vop2.lit_cnst);
		}
		else if (is_token(fmt_str, "OFFSET", &token_len))
		{
			if (inst->micro_inst.smrd.imm)
			{
				str_printf(&inst_str, &str_size, "0x%02x", 
					inst->micro_inst.smrd.offset);
			}
			else
			{
				operand_dump_scalar(operand_str, 
					inst->micro_inst.smrd.offset);
				str_printf(&inst_str, &str_size, "%s", 
					operand_str);
			}
		}
		else if (is_token(fmt_str, "DS_VDST", &token_len))
		{
			operand_dump_vector(operand_str, 
				inst->micro_inst.ds.vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "ADDR", &token_len))
		{
			operand_dump_vector(operand_str, 
				inst->micro_inst.ds.addr);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "DATA0", &token_len))
		{
			operand_dump_vector(operand_str, 
				inst->micro_inst.ds.data0);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "DATA1", &token_len))
		{
			operand_dump_vector(operand_str, 
				inst->micro_inst.ds.data1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "OFFSET0", &token_len))
		{
			if(inst->micro_inst.ds.offset0)
			{
				str_printf(&inst_str, &str_size, "offset0:%u ", 
					inst->micro_inst.ds.offset0);
			}
		}
		else if (is_token(fmt_str, "DS_SERIES_VDST", &token_len))
		{
			operand_dump_series_vector(operand_str, 
				inst->micro_inst.ds.vdst, 
				inst->micro_inst.ds.vdst+ 1);
			str_printf(&inst_str, &str_size, "%s", 
				operand_str);
		}
		else if (is_token(fmt_str, "OFFSET1", &token_len))
		{
			if(inst->micro_inst.ds.offset1)
			{
				str_printf(&inst_str, &str_size, "offset1:%u ", 
					inst->micro_inst.ds.offset1);
			}
		}
		else if (is_token(fmt_str, "VINTRP_VDST", &token_len))
		{
			operand_dump_vector(operand_str, 
				inst->micro_inst.vintrp.vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VSRC_I_J", &token_len))
		{
			operand_dump_vector(operand_str, 
				inst->micro_inst.vintrp.vsrc);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "ATTR", &token_len))
		{
			str_printf(&inst_str, &str_size, "attr_%d", 
				inst->micro_inst.vintrp.attr);
		}
		else if (is_token(fmt_str, "ATTRCHAN", &token_len))
		{
			switch (inst->micro_inst.vintrp.attrchan)
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
		else if (is_token(fmt_str, "MU_SERIES_VDATA", &token_len))
		{
			si_inst_SERIES_VDATA_dump(inst->micro_inst.mubuf.vdata, 
				inst->micro_inst.mubuf.op, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "MU_GLC", &token_len))
		{
			if (inst->micro_inst.mubuf.glc)
				str_printf(&inst_str, &str_size, "glc");
		}
		else if (is_token(fmt_str, "VADDR", &token_len))
		{
			if (inst->micro_inst.mtbuf.offen && 
				inst->micro_inst.mtbuf.idxen)
			{
				operand_dump_series_vector(operand_str, 
					inst->micro_inst.mtbuf.vaddr, 
					inst->micro_inst.mtbuf.vaddr + 1);
				str_printf(&inst_str, &str_size, "%s", 
					operand_str);
			}
			else
			{
				operand_dump_vector(operand_str, 
					inst->micro_inst.mtbuf.vaddr);
				str_printf(&inst_str, &str_size, "%s", 
					operand_str);
			}
		}
		else if (is_token(fmt_str, "MU_MADDR", &token_len))
		{
			si_inst_MADDR_dump(inst, operand_str, &inst_str, 
				str_size);
		}
		else if (is_token(fmt_str, "MT_SERIES_VDATA", &token_len))
		{
			si_inst_SERIES_VDATA_dump(inst->micro_inst.mtbuf.vdata, 
				inst->micro_inst.mtbuf.op, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "SERIES_SRSRC", &token_len))
		{
			assert((inst->micro_inst.mtbuf.srsrc << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, 
				inst->micro_inst.mtbuf.srsrc << 2, 
				(inst->micro_inst.mtbuf.srsrc << 2) + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "MT_MADDR", &token_len))
		{
			si_inst_MADDR_dump(inst, operand_str, &inst_str, 
				str_size);
		
			/* Format */
			str_printf(&inst_str, &str_size, " format:[%s,%s]",
				str_map_value(&si_inst_buf_data_format_map, 
					inst->micro_inst.mtbuf.dfmt),
				str_map_value(&si_inst_buf_num_format_map, 
					inst->micro_inst.mtbuf.nfmt));
		}
		else if (is_token(fmt_str, "MIMG_SERIES_VDATA", &token_len))
		{
			operand_dump_series_vector(operand_str, 
				inst->micro_inst.mimg.vdata, 
				inst->micro_inst.mimg.vdata + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "MIMG_VADDR", &token_len))
		{
			operand_dump_series_vector(operand_str, 
				inst->micro_inst.mimg.vaddr, 
				inst->micro_inst.mimg.vaddr + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "MIMG_SERIES_SRSRC", &token_len))
		{
			assert((inst->micro_inst.mimg.srsrc << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, 
				inst->micro_inst.mimg.srsrc << 2, 
				(inst->micro_inst.mimg.srsrc << 2) + 7);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "MIMG_DUG_SERIES_SRSRC", &token_len))
		{
			assert((inst->micro_inst.mimg.srsrc << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, 
				inst->micro_inst.mimg.srsrc << 2, 
				(inst->micro_inst.mimg.srsrc << 2) + 7);
			str_printf(&inst_str, &str_size, "%s", operand_str);

			/* Call si_inst_DUG_dump to print 
			 * dmask, unorm, and glc */
			si_inst_DUG_dump(inst, operand_str, &inst_str, 
				str_size);
		}
		else if (is_token(fmt_str, "MIMG_SERIES_SSAMP", &token_len))
		{
			assert((inst->micro_inst.mimg.ssamp << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, 
				inst->micro_inst.mimg.ssamp << 2, 
				(inst->micro_inst.mimg.ssamp << 2) + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "MIMG_DUG_SERIES_SSAMP", 
			&token_len))
		{
			assert((inst->micro_inst.mimg.ssamp << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, 
				inst->micro_inst.mimg.ssamp << 2, 
				(inst->micro_inst.mimg.ssamp << 2) + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
			
			/* Call si_inst_DUG_dump to print 
			 * dmask, unorm, and glc */
			si_inst_DUG_dump(inst, operand_str, &inst_str, 
				str_size);
		}
		else if (is_token(fmt_str, "TGT", &token_len))
		{
			operand_dump_exp(operand_str, 
				inst->micro_inst.exp.tgt);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "EXP_VSRCs", &token_len))
		{
			if (inst->micro_inst.exp.compr == 0 && 
				(inst->micro_inst.exp.en && 0x0) == 0x0)
			{
				operand_dump_vector(operand_str, 
					inst->micro_inst.exp.vsrc0);
				str_printf(&inst_str, &str_size, 
					"[%s ", operand_str);
				operand_dump_vector(operand_str, 
					inst->micro_inst.exp.vsrc1);
				str_printf(&inst_str, &str_size, "%s ", 
					operand_str);
				operand_dump_vector(operand_str, 
					inst->micro_inst.exp.vsrc2);
				str_printf(&inst_str, &str_size, "%s ", 
					operand_str);
				operand_dump_vector(operand_str, 
					inst->micro_inst.exp.vsrc3);
				str_printf(&inst_str, &str_size, "%s]", 
					operand_str);
			}
			else if (inst->micro_inst.exp.compr == 1 && 
				(inst->micro_inst.exp.en && 0x0) == 0x0)
			{
				operand_dump_vector(operand_str, 
					inst->micro_inst.exp.vsrc0);
				str_printf(&inst_str, &str_size, "[%s ", 
					operand_str);
				operand_dump_vector(operand_str, 
					inst->micro_inst.exp.vsrc1);
				str_printf(&inst_str, &str_size, "%s]", 
					operand_str);
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

		/* If symbol is '__OpenCL_XXX_kernel', it points 
		 * to internal ELF */
		if (str_prefix(symbol->name, "__OpenCL_") && 
			str_suffix(symbol->name, "_kernel"))
		{
			/* Decode internal ELF */
			str_substr(kernel_name, sizeof(kernel_name), 
				symbol->name, 9, strlen(symbol->name) - 16);
			amd_bin = si_bin_file_create(
				section->buffer.ptr + symbol->value, 
				symbol->size, kernel_name);

			/* Get kernel name */
			printf("**\n** Disassembly for '__kernel %s'\n**\n\n",
				kernel_name);
			si_disasm_buffer( 
				&amd_bin->enc_dict_entry_southern_islands->
				sec_text_buffer, stdout);
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
