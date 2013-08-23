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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>

#include "alu-group.h"
#include "asm.h"
#include "inst.h"


/*
 * Class 'EvgInst'
 */

struct str_map_t evg_inst_alu_map = {
	5, {
		{ "x", EvgInstAluX },
		{ "y", EvgInstAluY },
		{ "z", EvgInstAluZ },
		{ "w", EvgInstAluW },
		{ "t", EvgInstAluTrans }
	}
};


struct str_map_t evg_inst_alu_pv_map = {
	5, {
		{ "PV.x", EvgInstAluX },
		{ "PV.y", EvgInstAluY },
		{ "PV.z", EvgInstAluZ },
		{ "PV.w", EvgInstAluW },
		{ "PS", EvgInstAluTrans }
	}
};


void EvgInstCreate(EvgInst *self, EvgAsm *as)
{
	/* Initialize */
	self->as = as;
}


void EvgInstDestroy(EvgInst *self)
{
}


void EvgInstClear(EvgInst *self)
{
	self->alu = 0;
	self->alu_group = NULL;
	self->info = NULL;
	self->words[0].word = 0;
	self->words[1].word = 0;
	self->words[2].word = 0;
}


void *EvgInstDecodeCF(EvgInst *self, void *buf)
{
	EvgAsm *as = self->as;

	unsigned int cf_inst_short;
	unsigned int cf_inst_long;

	int end_of_program;

	/* Read instruction words (64-bit) */
	EvgInstClear(self);
	self->words[0].word = * (unsigned int *) buf;
	self->words[1].word = * (unsigned int *) (buf + 4);

	/* Decode instruction */
	cf_inst_short = self->words[1].cf_alu_word1.cf_inst;
	self->info = IN_RANGE(cf_inst_short, 0, EVG_INST_INFO_CF_SHORT_SIZE - 1) ?
		as->inst_info_cf_short[cf_inst_short] : NULL;
	if (!self->info)
	{
		cf_inst_long = self->words[1].cf_word1.cf_inst;
		self->info = IN_RANGE(cf_inst_long, 0, EVG_INST_INFO_CF_LONG_SIZE - 1) ?
			as->inst_info_cf_long[cf_inst_long] : NULL;
		if (!self->info)
			fatal("unknown CF instruction; cf_inst_short=%d, cf_inst_long=%d",
				cf_inst_short, cf_inst_long);
	}

	/* If 'end_of_program' bit is set, return NULL */
	end_of_program = 0;
	if (self->info->fmt[1] == EvgInstFormatCfWord1
		|| self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Buf
		|| self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Swiz)
		end_of_program = self->words[1].cf_word1.end_of_program;
	if (end_of_program)
		return NULL;

	/* Return pointer to next instruction */
	return buf + 8;
}


void *EvgInstDecodeALU(EvgInst *self, void *buf)
{
	EvgAsm *as = self->as;

	unsigned int alu_inst_short;
	unsigned int alu_inst_long;

	/* Read instruction words (64-bit) */
	EvgInstClear(self);
	self->words[0].word = * (unsigned int *) buf;
	self->words[1].word = * (unsigned int *) (buf + 4);

	/* Decode instruction */
	alu_inst_short = self->words[1].alu_word1_op3.alu_inst;
	self->info = IN_RANGE(alu_inst_short, 0, EVG_INST_INFO_ALU_SHORT_SIZE - 1) ?
		as->inst_info_alu_short[alu_inst_short] : NULL;
	if (!self->info)
	{
		alu_inst_long = self->words[1].alu_word1_op2.alu_inst;
		self->info = IN_RANGE(alu_inst_long, 0, EVG_INST_INFO_ALU_LONG_SIZE - 1) ?
			as->inst_info_alu_long[alu_inst_long] : NULL;
		if (!self->info)
			fatal("unknown ALU instruction; alu_inst_short=%d, alu_inst_long=%d",
				alu_inst_short, alu_inst_long);
	}

	/* Return pointer to next instruction */
	return buf + 8;
}


void *EvgInstDecodeTC(EvgInst *self, void *buf)
{
	EvgAsm *as = self->as;

	unsigned int tex_inst;

	/* Read instruction words (96-bit, 128-bit padded) */
	EvgInstClear(self);
	self->words[0].word = * (unsigned int *) buf;
	self->words[1].word = * (unsigned int *) (buf + 4);
	self->words[2].word = * (unsigned int *) (buf + 8);

	/* Decode instruction */
	tex_inst = self->words[0].tex_word0.tex_inst;
	self->info = IN_RANGE(tex_inst, 0, EVG_INST_INFO_TEX_SIZE - 1) ?
		as->inst_info_tex[tex_inst] : NULL;
	if (!self->info)
		fatal("unknown TEX instruction; tex_inst=%d", tex_inst);

	/* Return pointer to next instruction */
	return buf + 16;
}


static struct str_map_t evg_bank_swizzle_map = {
	6, {
		{ "", 0 },
		{ "VEC_021", 1 },
		{ "VEC_120", 2 },
		{ "VEC_102", 3 },
		{ "VEC_201", 4 },
		{ "VEC_210", 5 }
	}
};

static struct str_map_t evg_rat_inst_map = {
	39, {
		{ "NOP", 0 },
		{ "STORE_TYPED", 1 },
		{ "STORE_RAW", 2 },
		{ "STORE_RAW_FDENORM", 3 },
		{ "CMPXCHG_INT", 4 },
		{ "CMPXCHG_FLT", 5 },
		{ "CMPXCHG_FDENORM", 6 },
		{ "ADD", 7 },
		{ "SUB", 8 },
		{ "RSUB", 9 },
		{ "MIN_INT", 10 },
		{ "MIN_UINT", 11 },
		{ "MAX_INT", 12 },
		{ "MAX_UINT", 13 },
		{ "AND", 14 },
		{ "OR", 15 },
		{ "XOR", 16 },
		{ "MSKOR", 17 },
		{ "INC_UINT", 18 },
		{ "DEC_UINT", 19 },
		{ "NOP_RTN", 32 },
		{ "XCHG_RTN", 34 },
		{ "XCHG_FDENORM_RTN", 35 },
		{ "CMPXCHG_INT_RTN", 36 },
		{ "EXPORT_RAT_INST_CMPXCHG_FLT_RTN", 37 },
		{ "EXPORT_RAT_INST_CMPXCHG_FDENORM_RTN", 38 },
		{ "EXPORT_RAT_INST_ADD_RTN", 39 },
		{ "EXPORT_RAT_INST_SUB_RTN", 40 },
		{ "EXPORT_RAT_INST_RSUB_RTN", 41 },
		{ "EXPORT_RAT_INST_MIN_INT_RTN", 42 },
		{ "XPORT_RAT_INST_MIN_UINT_RTN", 43 },
		{ "EXPORT_RAT_INST_MAX_INT_RTN", 44 },
		{ "EXPORT_RAT_INST_MAX_UINT_RTN", 45 },
		{ "EXPORT_RAT_INST_AND_RTN", 46 },
		{ "EXPORT_RAT_INST_OR_RTN", 47 },
		{ "EXPORT_RAT_INST_XOR_RTN", 48 },
		{ "EXPORT_RAT_INST_MSKOR_RTN", 49 },
		{ "EXPORT_RAT_INST_INC_UINT_RTN", 50 },
		{ "EXPORT_RAT_INST_DEC_UINT_RTN", 51 }
	}
};


static struct str_map_t evg_cf_cond_map = {
	4, {
		{ "ACTIVE", 0 },
		{ "FALSE", 1 },
		{ "BOOL", 2 },
		{ "NOT_BOOL", 3 }
	}
};


static struct str_map_t evg_src_sel_map = {
	31, {
		{ "QA", 219 },  /* ALU_SRC_LDS_OQ_A */
		{ "QB", 220 },  /* ALU_SRC_LDS_OQ_B */
		{ "QA.pop", 221 },  /* ALU_SRC_LDS_OQ_A_POP */
		{ "QB.pop", 222 },  /* ALU_SRC_LDS_OQ_B_POP */
		{ "ALU_SRC_LDS_DIRECT_A", 223 },
		{ "ALU_SRC_LDS_DIRECT_B", 224 },
		{ "ALU_SRC_TIME_HI", 227 },
		{ "ALU_SRC_TIME_LO", 228 },
		{ "ALU_SRC_MASK_HI", 229 },
		{ "ALU_SRC_MASK_LO", 230 },
		{ "ALU_SRC_HW_WAVE_ID", 231 },
		{ "ALU_SRC_SIMD_ID", 232 },
		{ "ALU_SRC_SE_ID", 233 },
		{ "ALU_SRC_HW_THREADGRP_ID", 234 },
		{ "ALU_SRC_WAVE_ID_IN_GRP", 235 },
		{ "ALU_SRC_NUM_THREADGRP_WAVES", 236 },
		{ "ALU_SRC_HW_ALU_ODD", 237 },
		{ "ALU_SRC_LOOP_IDX", 238 },
		{ "ALU_SRC_PARAM_BASE_ADDR", 240 },
		{ "ALU_SRC_NEW_PRIM_MASK", 241 },
		{ "ALU_SRC_PRIM_MASK_LO", 243 },
		{ "ALU_SRC_1_DBL_L", 244 },
		{ "ALU_SRC_1_DBL_M", 245 },
		{ "ALU_SRC_0_5_DBL_L", 246 },
		{ "ALU_SRC_0_5_DBL_M", 247 },
		{ "0.0f", 248 },  /* ALU_SRC_0 */
		{ "1.0f", 249 },  /* ALU_SRC_1 */
		{ "1", 250 },  /* ALU_SRC_1_INT */
		{ "-1", 251 },  /* ALU_SRC_M_1_INT */
		{ "0.5", 252 },  /* ALU_SRC_0_5 */
		{ "PS", 255 }  /* ALU_SRC_PS */
	}
};


static struct str_map_t evg_dst_sel_map = {
	7, {
		{ "x", 0 },
		{ "y", 1 },
		{ "z", 2 },
		{ "w", 3 },
		{ "0", 4 },
		{ "1", 5 },
		{ "_", 7 }
	}
};

static struct str_map_t export_type_map = {
	4, {
		{ "PIX", 0 },
		{ "POS", 1 },
		{ "PARAM", 2 },
		{ "IND_ACK", 3 }
	}
};


static struct str_map_t evg_fmt_vtx_fetch_type_map = {  /* VTX_FETCH prefix omitted */
	3, {
		{ "VERTEX_DATA", 0 },
		{ "INSTANCE_DATA", 1 },
		{ "NO_INDEX_OFFSET", 2 }
	}
};


static struct str_map_t evg_fmt_vtx_data_format_map = {
	64, {
		{ "UNKNOWN", 0 },
		{ "8", 1 },
		{ "4_4", 2 },
		{ "3_3_2", 3 },
		{ "RESERVED_4", 4 },
		{ "16", 5 },
		{ "16_FLOAT", 6 },
		{ "8_8", 7 },
		{ "5_6_5", 8 },
		{ "6_5_5", 9 },
		{ "1_5_5_5", 10 },
		{ "4_4_4_4", 11 },
		{ "5_5_5_1", 12 },
		{ "32", 13 },
		{ "32_FLOAT", 14 },
		{ "16_16", 15 },
		{ "16_16_FLOAT", 16 },
		{ "8_24", 17 },
		{ "8_24_FLOAT", 18 },
		{ "24_8", 19 },
		{ "24_8_FLOAT", 20 },
		{ "10_11_11", 21 },
		{ "10_11_11_FLOAT", 22 },
		{ "11_11_10", 23 },
		{ "11_11_10_FLOAT", 24 },
		{ "2_10_10_10", 25 },
		{ "8_8_8_8", 26 },
		{ "10_10_10_2", 27 },
		{ "X24_8_32_FLOAT", 28 },
		{ "32_32", 29 },
		{ "32_32_FLOAT", 30 },
		{ "16_16_16_16", 31 },
		{ "16_16_16_16_FLOAT", 32 },
		{ "RESERVED_33", 33 },
		{ "32_32_32_32", 34 },
		{ "32_32_32_32_FLOAT", 35 },
		{ "RESERVED_36", 36 },
		{ "1", 37 },
		{ "1_REVERSED", 38 },
		{ "GB_GR", 39 },
		{ "BG_RG", 40 },
		{ "32_AS_8", 41 },
		{ "32_AS_8_8", 42 },
		{ "5_9_9_9_SHAREDEXP", 43 },
		{ "8_8_8", 44 },
		{ "16_16_16", 45 },
		{ "16_16_16_FLOAT", 46 },
		{ "32_32_32", 47 },
		{ "32_32_32_FLOAT", 48 },
		{ "BC1", 49 },
		{ "BC2", 50 },
		{ "BC3", 51 },
		{ "BC4", 52 },
		{ "BC5", 53 },
		{ "APC0", 54 },
		{ "APC1", 55 },
		{ "APC2", 56 },
		{ "APC3", 57 },
		{ "APC4", 58 },
		{ "APC5", 59 },
		{ "APC6", 60 },
		{ "APC7", 61 },
		{ "CTX1", 62 },
		{ "UNKNOWN", 63 }
	}
};


static struct str_map_t evg_fmt_vtx_num_format_map = {  /* Prefix NUM_FORMAT omitted */
	3, {
		{ "NORM", 0 },
		{ "INT", 1 },
		{ "SCALED", 2 }
	}
};


static struct str_map_t evg_fmt_vtx_format_comp_map = {  /* Prefix FORMAT_COMP omitted */
	2, {
		{ "UNSIGNED", 0 },
		{ "SIGNED", 1 }
	}
};


static struct str_map_t evg_fmt_vtx_srf_mode_map = {  /* Prefix SRF_MODE omitted */
	2, {
		{ "ZERO_CLAMP_MINUS_ONE", 0 },
		{ "NO_ZERO", 1 }
	}
};


static struct str_map_t evg_fmt_vtx_endian_swap_map = {  /* Prefix ENDIAN omitted */
	3, {
		{ "NONE", 0 },
		{ "8IN16", 1 },
		{ "8IN32", 2 }
	}
};


static struct str_map_t evg_fmt_lds_op_map = {
	46, {
		{ "ADD", 0 },
		{ "SUB", 1 },
		{ "RSUB", 2 },
		{ "INC", 3 },
		{ "DEC", 4 },
		{ "MIN_INT", 5 },
		{ "MAX_INT", 6 },
		{ "MIN_UINT", 7 },
		{ "MAX_UINT", 8 },
		{ "AND", 9 },
		{ "OR", 10 },
		{ "XOR", 11 },
		{ "MSKOR", 12 },
		{ "WRITE", 13 },
		{ "WRITE_REL", 14 },
		{ "WRITE2", 15 },
		{ "CMP_STORE", 16 },
		{ "CMP_STORE_SPF", 17 },
		{ "BYTE_WRITE", 18 },
		{ "SHORT_WRITE", 19 },
		{ "ADD_RET", 32 },
		{ "RSUB_RET", 34 },
		{ "INC_RET", 35 },
		{ "DEC_RET", 36 },
		{ "MIN_INT_RET", 37 },
		{ "MAX_INT_RET", 38 },
		{ "MIN_UINT_RET", 39 },
		{ "MAX_UINT_RET", 40 },
		{ "AND_RET", 41 },
		{ "OR_RET", 42 },
		{ "XOR_RET", 43 },
		{ "MSKOR_RET", 44 },
		{ "XCHG_RET", 45 },
		{ "XCHG_REL_RET", 46 },
		{ "XCHG2_RET", 47 },
		{ "CMP_XCHG_RET", 48 },
		{ "CMP_XCHG_SPF_RET", 49 },
		{ "READ_RET", 50 },
		{ "READ_REL_RET", 51 },
		{ "READ2_RET", 52 },
		{ "READWRITE_RET", 53 },
		{ "BYTE_READ_RET", 54 },
		{ "UBYTE_READ_RET", 55 },
		{ "SHORT_READ_RET", 56 },
		{ "USHORT_READ_RET", 57 },
		{ "ATOMIC_ORDERED_ALLOC_RET", 63 }
	}
};


static char *EvgInstTokenPrefix(int loop_idx, int *nl)
{
	static char token_prefix[MAX_STRING_SIZE];

	/* Empty prefix by default */
	token_prefix[0] = '\0';

	/* Line break */
	if (*nl)
	{
		int shift_count = loop_idx * 4 + 9;
		token_prefix[0] = '\n';
		memset(token_prefix + 1, ' ', shift_count);
		token_prefix[shift_count + 2] = '\0';
		*nl = 0;
	}

	/* Return prefix */
	return token_prefix;
}


static void EvgInstKcacheDumpBuf(int kcache_idx, int kcache_bank,
		int kcache_mode, int kcache_addr,
		char **buf_ptr, int *size_ptr)
{
	if (!kcache_mode)  /* KCACHE_MODE_NOP */
		return;
	str_printf(buf_ptr, size_ptr, "KCACHE%d(CB%d:", kcache_idx, kcache_bank);
	switch (kcache_mode) {
	case 1:  /* KCACHE_LOCK_1 */
		str_printf(buf_ptr, size_ptr, "%d-%d", kcache_addr * 16,
				kcache_addr * 16 + 15);  /* FIXME: correct? */
		break;
	case 2:  /* KCACHE_LOCK_2 */
	case 3:  /* KCACHE_LOCK_LOOP_INDEX */
	default:
		fatal("amd_inst_dump_kcache: kcache_mode=%d not supported", kcache_mode);
	}
	str_printf(buf_ptr, size_ptr, ") ");
}


static void EvgInstGprDumpBuf(int gpr, int rel, int chan, int im,
	char **buf_ptr, int *size_ptr)
{
	char gpr_str[MAX_STRING_SIZE];

	/* Register  */
	if (gpr <= 123)
		sprintf(gpr_str, "R%d", gpr);
	else
		sprintf(gpr_str, "T%d", 127 - gpr);

	/* Relative addressing */
	if (rel)
	{
		if (rel && IN_RANGE(im, 0, 3))
			str_printf(buf_ptr, size_ptr, "%s[A0.%s]", gpr_str,
					str_map_value(&evg_inst_alu_map, EvgInstAluX + im));
		else if (im == 4)
			str_printf(buf_ptr, size_ptr, "%s[AL]", gpr_str);
		else if (im == 5)
			str_printf(buf_ptr, size_ptr, "SR%d", gpr);
		else if (im == 6)
			str_printf(buf_ptr, size_ptr, "SR%d[A0.x]", gpr);
	}
	else
		str_printf(buf_ptr, size_ptr, "%s", gpr_str);

	/* Vector element */
	if (chan >= 0)
		str_printf(buf_ptr, size_ptr, ".%s",
				str_map_value(&evg_inst_alu_map, EvgInstAluX + chan));
}


static void EvgInstOpDestDumpBuf(EvgInst *self, char **buf_ptr, int *size_ptr)
{
	int gpr, rel, chan, index_mode;

	/* Fields 'dst_gpr', 'dst_rel', and 'dst_chan' are at the same bit positions in both
	 * EVG_ALU_WORD1_OP2 and EVG_ALU_WORD1_OP3 formats. */
	gpr = self->words[1].alu_word1_op2.dst_gpr;
	rel = self->words[1].alu_word1_op2.dst_rel;
	chan = self->words[1].alu_word1_op2.dst_chan;
	index_mode = self->words[0].alu_word0.index_mode;

	/* If 'write_mask' field is clear, print underscore */
	if (self->info->fmt[1] == EvgInstFormatAluWord1Op2 && !self->words[1].alu_word1_op2.write_mask) {
		str_printf(buf_ptr, size_ptr, "____");
		return;
	}

	/* Print register */
	EvgInstGprDumpBuf(gpr, rel, chan, index_mode, buf_ptr, size_ptr);
}


/* Get parameters for a source register in an ALU instruction */
void EvgInstGetOpSrc(EvgInst *self, int src_idx,
	int *sel, int *rel, int *chan, int *neg, int *abs)
{
	/* Valid formats */
	assert(self->info->fmt[0] == EvgInstFormatAluWord0
		|| self->info->fmt[0] == EvgInstFormatAluWord0LdsIdxOp);
	assert(self->info->fmt[1] == EvgInstFormatAluWord1Op2
		|| self->info->fmt[1] == EvgInstFormatAluWord1Op3
		|| self->info->fmt[1] == EvgInstFormatAluWord1LdsIdxOp);

	/* Get parameters */
	switch (src_idx)
	{

	case 0:

		/* Fields:	src0_sel, src0_rel, src_chan
		 * Present:	EVG_ALU_WORD0, EVG_ALU_WORD0_LDS_IDX_OP
		 * Absent:	-
		 */
		*sel = self->words[0].alu_word0.src0_sel;
		*rel = self->words[0].alu_word0.src0_rel;
		*chan = self->words[0].alu_word0.src0_chan;

		/* Fields:	src0_neg
		 * Present:	EVG_ALU_WORD0
		 * Absent:	EVG_ALU_WORD0_LDS_IDX_OP
		 */
		*neg = self->info->fmt[0] == EvgInstFormatAluWord0 ?
				self->words[0].alu_word0.src0_neg : 0;

		/* Fields:	src0_abs
		 * Present:	EVG_ALU_WORD1_OP2
		 * Absent:	EVG_ALU_WORD1_OP3, EVG_ALU_WORD1_LDS_IDX_OP
		 */
		*abs = self->info->fmt[1] == EvgInstFormatAluWord1Op2 ?
				self->words[1].alu_word1_op2.src0_abs : 0;
		break;

	case 1:

		/* Fields:	src1_sel, src1_rel, src1_chan
		 * Present:	EVG_ALU_WORD0, EVG_ALU_WORD0_LDS_IDX_OP
		 * Absent:	-
		 */
		*sel = self->words[0].alu_word0.src1_sel;
		*rel = self->words[0].alu_word0.src1_rel;
		*chan = self->words[0].alu_word0.src1_chan;

		/* Fields:	src1_neg
		 * Present:	EVG_ALU_WORD0
		 * Absent:	EVG_ALU_WORD0_LDS_IDX_OP
		 */
		*neg = self->info->fmt[0] == EvgInstFormatAluWord0 ?
				self->words[0].alu_word0.src1_neg : 0;

		/* Fields:	src_abs
		 * Present:	EVG_ALU_WORD1_OP2
		 * Absent:	EVG_ALU_WORD1_OP3, EVG_ALU_WORD1_LDS_IDX_OP
		 */
		*abs = self->info->fmt[1] == EvgInstFormatAluWord1Op2 ?
				self->words[1].alu_word1_op2.src1_abs : 0;
		break;

	case 2:

		/* Fields:	src2_sel, src2_rel, src2_chan
		 * Present:	EVG_ALU_WORD1_OP3, EVG_ALU_WORD1_LDS_IDX_OP
		 * Absent:	EVG_ALU_WORD1_OP2
		 */
		assert(self->info->fmt[1] == EvgInstFormatAluWord1Op3
			|| self->info->fmt[1] == EvgInstFormatAluWord1LdsIdxOp);
		*sel = self->words[1].alu_word1_op3.src2_sel;
		*rel = self->words[1].alu_word1_op3.src2_rel;
		*chan = self->words[1].alu_word1_op3.src2_chan;

		/* Fields:	src2_neg
		 * Present:	EVG_ALU_WORD1_OP3
		 * Absent:	EVG_ALU_WORD1_LDS_IDX_OP, ALU_WORD_OP2
		 */
		*neg = self->info->fmt[1] == EvgInstFormatAluWord1Op3 ?
				self->words[1].alu_word1_op3.src2_neg : 0;

		/* Fields:	src2_abs
		 * Present:	-
		 * Absent:	EVG_ALU_WORD1_OP3, EVG_ALU_WORD1_LDS_IDX_OP, ALU_WORD_OP2
		 */
		*abs = 0;
		break;

	default:
		fatal("amd_inst_dump_op_src: wrong src_idx");
	}
}


static void EvgInstOpSrcDumpBuf(EvgInst *self, int src_idx, char **buf_ptr, int *size_ptr)
{
	int sel, rel, chan, neg, abs;

	/* Get parameters */
	EvgInstGetOpSrc(self, src_idx,
		&sel, &rel, &chan, &neg, &abs);

	/* Negation and first bracket for abs */
	if (neg)
		str_printf(buf_ptr, size_ptr, "-");
	if (abs)
		str_printf(buf_ptr, size_ptr, "|");

	/* 0..127: Value in GPR */
	if (IN_RANGE(sel, 0, 127))
	{
		int index_mode;
		index_mode = self->words[0].alu_word0.index_mode;
		EvgInstGprDumpBuf(sel, rel, chan, index_mode, buf_ptr, size_ptr);
		goto end;
	}

	/* 128..159: Kcache constants in bank 0 */
	if (IN_RANGE(sel, 128, 159))
	{
		str_printf(buf_ptr, size_ptr, "KC0[%d].%s", sel - 128,
				str_map_value(&evg_inst_alu_map, EvgInstAluX + chan));
		goto end;
	}

	/* 160..191: Kcache constants in bank 1 */
	if (IN_RANGE(sel, 160, 191))
	{
		str_printf(buf_ptr, size_ptr, "KC1[%d].%s", sel - 160,
				str_map_value(&evg_inst_alu_map, EvgInstAluX + chan));
		goto end;
	}

	/* 256..287: Kcache constants in bank 2 */
	if (IN_RANGE(sel, 256, 287))
	{
		str_printf(buf_ptr, size_ptr, "KC2[%d].%s", sel - 256,
				str_map_value(&evg_inst_alu_map, EvgInstAluX + chan));
		goto end;
	}

	/* 288..319: Kcache constant in bank 3 */
	if (IN_RANGE(sel, 288, 319))
	{
		str_printf(buf_ptr, size_ptr, "KC3[%d].%s", sel - 288,
				str_map_value(&evg_inst_alu_map, EvgInstAluX + chan));
		goto end;
	}

	/* ALU_SRC_LITERAL */
	if (sel == 253)
	{
		assert(self->alu_group);
		str_printf(buf_ptr, size_ptr, "(0x%08x, %.9ef).%s", self->alu_group->literal[chan].as_uint,
			self->alu_group->literal[chan].as_float,
			str_map_value(&evg_inst_alu_map, EvgInstAluX + chan));
		goto end;
	}

	/* ALU_SRC_PV */
	if (sel == 254)
	{
		str_printf(buf_ptr, size_ptr, "PV.%s",
				str_map_value(&evg_inst_alu_map, EvgInstAluX + chan));
		goto end;
	}

	/* Other */
	str_printf(buf_ptr, size_ptr, "%s", str_map_value(&evg_src_sel_map, sel));

end:
	/* Second bracket for abs */
	if (abs)
		str_printf(buf_ptr, size_ptr, "|");
}


/* Dump an instruction. Use -1 for 'count', 'loop_idx', or 'alu', if the corresponding field
 * is not relevant in the instruction dump. */
void EvgInstSlotDumpBuf(EvgInst *self, int count, int loop_idx, int slot,
	char *buf, int size)
{
	char shift_str[MAX_STRING_SIZE];
	char *fmt_str;
	int len;
	int nl = 0;  /* new line */

	char **buf_ptr = &buf;
	int *size_ptr = &size;

	/* Shift */
	assert(loop_idx >= 0);
	memset(shift_str, ' ', MAX_STRING_SIZE);
	shift_str[loop_idx * 4] = '\0';
	str_printf(buf_ptr, size_ptr, "%s", shift_str);

	/* Instruction counter */
	if (self->info->category == EvgInstCategoryCF)
	{
		if (count >= 0)
			str_printf(buf_ptr, size_ptr, "%02d ", count);
		else
			str_printf(buf_ptr, size_ptr, "   ");
	}
	else
	{
		if (count >= 0)
			str_printf(buf_ptr, size_ptr, "   %4d  ", count);
		else
			str_printf(buf_ptr, size_ptr, "         ");
	}

	/* VLIW slot */
	if (slot >= 0)
		str_printf(buf_ptr, size_ptr, "%s: ", str_map_value(&evg_inst_alu_map, slot));

	/* Format */
	fmt_str = self->info->fmt_str;
	while (*fmt_str)
	{
		/* Literal */
		if (*fmt_str != '%')
		{
			str_printf(buf_ptr, size_ptr, "%c", *fmt_str);
			fmt_str++;
			continue;
		}

		/* Token */
		fmt_str++;
		if (asm_is_token(fmt_str, "name", &len))
		{
			if (self->info->category == EvgInstCategoryALU)
				str_printf(buf_ptr, size_ptr, "%-11s", self->info->name);
			else
				str_printf(buf_ptr, size_ptr, "%s", self->info->name);

		}
		else if (asm_is_token(fmt_str, "alu_dst", &len))
		{
			EvgInstOpDestDumpBuf(self, buf_ptr, size_ptr);
		}
		else if (asm_is_token(fmt_str, "alu_src0", &len))
		{
			EvgInstOpSrcDumpBuf(self, 0, buf_ptr, size_ptr);

		}
		else if (asm_is_token(fmt_str, "alu_src1", &len))
		{
			EvgInstOpSrcDumpBuf(self, 1, buf_ptr, size_ptr);
		}
		else if (asm_is_token(fmt_str, "alu_src2", &len))
		{
			EvgInstOpSrcDumpBuf(self, 2, buf_ptr, size_ptr);
		}

		/* ALU modifiers */
		else if (asm_is_token(fmt_str, "alu_mod", &len))
		{
			/* Padding */
			assert(self->info->fmt[1] == EvgInstFormatAluWord1Op2 ||
					self->info->fmt[1] == EvgInstFormatAluWord1Op3);
			str_printf(buf_ptr, size_ptr, "    ");

			/* EVG_ALU_WORD1_OP2 - 'bank_swizzle' field.
			 * Common for EVG_ALU_WORD1_OP2 and EVG_ALU_WORD1_OP3 */
			str_printf(buf_ptr, size_ptr, "%s", str_map_value(&evg_bank_swizzle_map,
					self->words[1].alu_word1_op2.bank_swizzle));

			/* EVG_ALU_WORD0 - 'pred_sel' field */
			if (self->words[0].alu_word0.pred_sel == 2)  /* PRED_SEL_ZERO */
				str_printf(buf_ptr, size_ptr, " (!p)");
			else if (self->words[0].alu_word0.pred_sel == 3)  /* PRED_SEL_ONE */
				str_printf(buf_ptr, size_ptr, " (p)");

			/* EVG_ALU_WORD1_OP2 - 'update_exec_mask' field */
			if (self->info->fmt[1] == EvgInstFormatAluWord1Op2 &&
					self->words[1].alu_word1_op2.update_exec_mask)
				str_printf(buf_ptr, size_ptr, " UPDATE_EXEC_MASK");

			/* EVG_ALU_WORD1_OP2 - 'update_pred' field */
			if (self->info->fmt[1] == EvgInstFormatAluWord1Op2 &&
					self->words[1].alu_word1_op2.update_pred)
				str_printf(buf_ptr, size_ptr, " UPDATE_PRED");
		}
		else if (asm_is_token(fmt_str, "omod", &len))
		{
			assert(self->info->fmt[1] == EvgInstFormatAluWord1Op2);
			switch (self->words[1].alu_word1_op2.omod)
			{
			case 0: str_printf(buf_ptr, size_ptr, "  "); break;
			case 1: str_printf(buf_ptr, size_ptr, "*2"); break;
			case 2: str_printf(buf_ptr, size_ptr, "*4"); break;
			case 3: str_printf(buf_ptr, size_ptr, "/2"); break;
			}
		}
		else if (asm_is_token(fmt_str, "clamp", &len))
		{
			assert(self->info->fmt[1] == EvgInstFormatAluWord1Op2 ||
					self->info->fmt[1] == EvgInstFormatAluWord1Op3);
			if (self->words[1].alu_word1_op2.clamp ||
					self->words[1].alu_word1_op3.clamp)
				str_printf(buf_ptr, size_ptr, "CLAMP");
		}
		else if (asm_is_token(fmt_str, "cf_addr", &len))
		{
			assert(self->info->fmt[0] == EvgInstFormatCfWord0);
			str_printf(buf_ptr, size_ptr, "%d", self->words[0].cf_word0.addr);
		}
		else if (asm_is_token(fmt_str, "cf_cnt", &len))
		{
			assert(self->info->fmt[1] == EvgInstFormatCfWord1);
			str_printf(buf_ptr, size_ptr, "%d", self->words[1].cf_word1.count + 1);
		}
		else if (asm_is_token(fmt_str, "cf_stream_id", &len))
		{
			assert(self->info->fmt[1] == EvgInstFormatCfWord1);
			/* For EMIT, CUT, EMIT_CUT, bit[10] are the stream ID */
			int stream_id = self->words[1].cf_word1.count % 2;
			str_printf(buf_ptr, size_ptr, "%d", stream_id);
		}
		else if (asm_is_token(fmt_str, "pop_count", &len))
		{
			int pop_count;

			assert(self->info->fmt[1] == EvgInstFormatCfWord1);
			pop_count = self->words[1].cf_word1.pop_count;
			if (pop_count)
				str_printf(buf_ptr, size_ptr, "POP_CNT(%d)", pop_count);
		}
		else if (asm_is_token(fmt_str, "cf_cond", &len))
		{
			int cf_cond;
			int valid_pixel_mode;

			assert(self->info->fmt[1] == EvgInstFormatCfWord1);
			valid_pixel_mode = self->words[1].cf_word1.valid_pixel_mode;
			cf_cond = self->words[1].cf_word1.cond;
			if (valid_pixel_mode && cf_cond)
				str_printf(buf_ptr, size_ptr, "CND(%s)",
						str_map_value(&evg_cf_cond_map, cf_cond));
		}
		else if (asm_is_token(fmt_str, "cf_const", &len))
		{
			int cf_const;
			int cf_cond;
			int valid_pixel_mode;

			assert(self->info->fmt[1] == EvgInstFormatCfWord1);
			valid_pixel_mode = self->words[1].cf_word1.valid_pixel_mode;
			cf_cond = self->words[1].cf_word1.cond;
			cf_const = self->words[1].cf_word1.cf_const;
			if (valid_pixel_mode && IN_RANGE(cf_cond, 2, 3))
				str_printf(buf_ptr, size_ptr, "CF_CONST(%d)", cf_const);
		}
		else if (asm_is_token(fmt_str, "wqm", &len))
		{
			int whole_quad_mode;

			assert(self->info->fmt[1] == EvgInstFormatCfWord1 ||
					self->info->fmt[1] == EvgInstFormatCfAluWord1);
			whole_quad_mode = self->words[1].cf_word1.whole_quad_mode;
			if (whole_quad_mode)
				str_printf(buf_ptr, size_ptr, "WHOLE_QUAD");
		}
		else if (asm_is_token(fmt_str, "vpm", &len))
		{
			int valid_pixel_mode;

			assert(self->info->fmt[1] == EvgInstFormatCfWord1 ||
				self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Buf ||
				self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Swiz);
			valid_pixel_mode = self->words[1].cf_word1.valid_pixel_mode;
			if (valid_pixel_mode)
				str_printf(buf_ptr, size_ptr, "VPM");
		}
		else if (asm_is_token(fmt_str, "cf_alu_addr", &len))
		{
			assert(self->info->fmt[0] == EvgInstFormatCfAluWord0);
			str_printf(buf_ptr, size_ptr, "%d", self->words[0].cf_alu_word0.addr);
		}
		else if (asm_is_token(fmt_str, "cf_alu_cnt", &len))
		{
			assert(self->info->fmt[1] == EvgInstFormatCfAluWord1);
			str_printf(buf_ptr, size_ptr, "%d", self->words[1].cf_alu_word1.count + 1);
		}
		else if (asm_is_token(fmt_str, "loop_idx", &len))
		{
			//str_printf(buf_ptr, size_ptr, "i%d", loop_idx);
			/* FIXME: what is this field? I think it is the CF_CONST value, but
			 * needs to be checked with AMD's ISA dump. */
			str_printf(buf_ptr, size_ptr, "i%d", self->words[1].cf_word1.cf_const);
		}
		else if (asm_is_token(fmt_str, "mark", &len))
		{
			int mark;

			assert(self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Buf ||
				self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Swiz);
			mark = self->words[1].cf_alloc_export_word1_buf.mark;
			if (mark)
				str_printf(buf_ptr, size_ptr, "MARK");
		}
		else if (asm_is_token(fmt_str, "burst_count", &len))
		{
			int burst_count;

			assert(self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Buf ||
				self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Swiz);
			burst_count = self->words[1].cf_alloc_export_word1_buf.burst_count;
			if (burst_count)
				str_printf(buf_ptr, size_ptr, "BRSTCNT(%d)", burst_count);
		}
		else if (asm_is_token(fmt_str, "no_barrier", &len))
		{
			assert(self->info->fmt[1] == EvgInstFormatCfWord1 ||
					self->info->fmt[1] == EvgInstFormatCfAluWord1 ||
					self->info->fmt[1] == EvgInstFormatCfAluWord1Ext ||
					self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Buf ||
					self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Swiz);
			if (!self->words[1].cf_word1.barrier)
				str_printf(buf_ptr, size_ptr, "NO_BARRIER");
		}
		else if (asm_is_token(fmt_str, "vpm", &len))
		{
			assert(self->info->fmt[1] == EvgInstFormatCfWord1 ||
				self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Buf ||
				self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Swiz);
			if (self->words[1].cf_word1.valid_pixel_mode)
				str_printf(buf_ptr, size_ptr, "VPM");
		}
		else if (asm_is_token(fmt_str, "kcache", &len))
		{
			EvgInstKcacheDumpBuf(0,
				self->words[0].cf_alu_word0.kcache_bank0,
				self->words[0].cf_alu_word0.kcache_mode0,
				self->words[1].cf_alu_word1.kcache_addr0,
				buf_ptr, size_ptr);
			EvgInstKcacheDumpBuf(1,
				self->words[0].cf_alu_word0.kcache_bank1,
				self->words[1].cf_alu_word1.kcache_mode1,
				self->words[1].cf_alu_word1.kcache_addr1,
				buf_ptr, size_ptr);
		}
		else if (asm_is_token(fmt_str, "exp_type", &len))
		{
			assert(self->info->fmt[0] == EvgInstFormatCfAllocExportWord0);
			str_printf(buf_ptr, size_ptr, "%s", str_map_value(&export_type_map,
					self->words[0].cf_alloc_export_word0.type));
		}
		else if (asm_is_token(fmt_str, "exp_array_base", &len))
		{
			/* FIXME */
			assert(self->info->fmt[0] == EvgInstFormatCfAllocExportWord0);
			if (self->words[0].cf_alloc_export_word0.type == 0)
			{
				int array_base = self->words[0].cf_alloc_export_word0.array_base % 8;
				str_printf(buf_ptr, size_ptr, "%d", array_base);
			}
			else if (self->words[0].cf_alloc_export_word0.type == 1)
			{
				int array_base = self->words[0].cf_alloc_export_word0.array_base % 4;
				str_printf(buf_ptr, size_ptr, "%d", array_base);
			}
			else if (self->words[0].cf_alloc_export_word0.type == 2)
			{
				int array_base = self->words[0].cf_alloc_export_word0.array_base % 32;
				str_printf(buf_ptr, size_ptr, "%d", array_base);
			}
			else
			{
				int array_base = self->words[0].cf_alloc_export_word0.array_base;
				str_printf(buf_ptr, size_ptr, "%d", array_base);
			}
		}
		else if (asm_is_token(fmt_str, "exp_rw_gpr", &len))
		{
			assert(self->info->fmt[0] == EvgInstFormatCfAllocExportWord0);
			assert(self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Swiz ||
					self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Buf);

			int rw_gpr = self->words[0].cf_alloc_export_word0.rw_gpr;
			if (self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Swiz)
			{
				int src_sel[4];
				src_sel[0] = self->words[1].cf_alloc_export_word1_swiz.sel_x;
				src_sel[1] = self->words[1].cf_alloc_export_word1_swiz.sel_y;
				src_sel[2] = self->words[1].cf_alloc_export_word1_swiz.sel_z;
				src_sel[3] = self->words[1].cf_alloc_export_word1_swiz.sel_w;

				char src_sel_chars[4];

				/* Add element selections */
				int i;
				for (i = 0; i < 4; i++)
				{
					if (src_sel[i] == 0)
					{
						src_sel_chars[i] = 'x';
					}
					else if (src_sel[i] == 1)
					{
						src_sel_chars[i] = 'y';
					}
					else if (src_sel[i] == 2)
					{
						src_sel_chars[i] = 'z';
					}
					else if (src_sel[i] == 3)
					{
						src_sel_chars[i] = 'w';
					}
					else if (src_sel[i] == 4)
					{
						src_sel_chars[i] = '0';
					}
					else if (src_sel[i] == 5)
					{
						src_sel_chars[i] = '1';
					}
					else if (src_sel[i] == 6)
					{
						fatal("%s: src_sel value 6 in reserved", fmt_str);
					}
					else if (src_sel[i] == 7)
					{
						/* FIXME: Mark this element */
						src_sel_chars[i] = '_';
					}
					else
					{
						fatal("%s: src_sel value %d is unknown",
								fmt_str, src_sel[i]);
					}
				}

				int count = 0;
				for (i = 0; i < 4; i++)
				{
					if (src_sel[i] == i)
					{
						count++;
					}
				}

				/* if count == 4, only print R/T instead of R.xyzw */
				if (count != 4)
				{
						str_printf(buf_ptr, size_ptr, "R%d.%c%c%c%c",
								rw_gpr, src_sel_chars[0],
								src_sel_chars[1],
								src_sel_chars[2],
								src_sel_chars[3]);
				}
				else
				{
						str_printf(buf_ptr, size_ptr, "R%d", rw_gpr);
				}
			}
			else if (self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Buf)
			{
				str_printf(buf_ptr, size_ptr, "R%d", rw_gpr);
			}

		}
		else if (asm_is_token(fmt_str, "exp_index_gpr", &len))
		{
			assert(self->info->fmt[0] == EvgInstFormatCfAllocExportWord0Rat);
			str_printf(buf_ptr, size_ptr, "%s", str_map_value(&evg_rat_inst_map,
					self->words[0].cf_alloc_export_word0_rat.rat_inst));
		}
		else if (asm_is_token(fmt_str, "rat_inst", &len))
		{
			assert(self->info->fmt[0] == EvgInstFormatCfAllocExportWord0Rat);
			str_printf(buf_ptr, size_ptr, "%s", str_map_value(&evg_rat_inst_map,
					self->words[0].cf_alloc_export_word0_rat.rat_inst));

		}
		else if (asm_is_token(fmt_str, "rat_id", &len))
		{
			assert(self->info->fmt[0] == EvgInstFormatCfAllocExportWord0Rat);
			str_printf(buf_ptr, size_ptr, "%d", self->words[0].cf_alloc_export_word0_rat.rat_id);
		}
		else if (asm_is_token(fmt_str, "rat_index_mode", &len))
		{
			int rim;
			assert(self->info->fmt[0] == EvgInstFormatCfAllocExportWord0Rat);
			rim = self->words[0].cf_alloc_export_word0_rat.rat_index_mode;
			if (rim)
				str_printf(buf_ptr, size_ptr, "+idx%d", rim - 1);
		}
		else if (asm_is_token(fmt_str, "rat_index_gpr", &len))
		{
			assert(self->info->fmt[0] == EvgInstFormatCfAllocExportWord0Rat);
			EvgInstGprDumpBuf(self->words[0].cf_alloc_export_word0_rat.index_gpr,
					0, -1, 0, buf_ptr, size_ptr);
		}
		else if (asm_is_token(fmt_str, "comp_mask", &len))
		{
			int comp_mask;

			assert(self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Buf);
			comp_mask = self->words[1].cf_alloc_export_word1_buf.comp_mask;
			if (comp_mask != 0xf)
				str_printf(buf_ptr, size_ptr, ".%s%s%s%s", comp_mask & 1 ? "x" :
						"_", comp_mask & 2 ? "y" : "_",
					comp_mask & 4 ? "z" : "_", comp_mask & 8 ? "w" : "_");
		}
		else if (asm_is_token(fmt_str, "rat_rw_gpr", &len))
		{
			assert(self->info->fmt[0] == EvgInstFormatCfAllocExportWord0Rat);
			EvgInstGprDumpBuf(self->words[0].cf_alloc_export_word0_rat.rw_gpr,
					0, -1, 0, buf_ptr, size_ptr);
		}
		else if (asm_is_token(fmt_str, "array_size", &len))
		{
			int elem_size;
			int array_size;

			assert(self->info->fmt[0] == EvgInstFormatCfAllocExportWord0 ||
					self->info->fmt[0] == EvgInstFormatCfAllocExportWord0Rat);
			assert(self->info->fmt[1] == EvgInstFormatCfAllocExportWord1Buf);
			elem_size = self->words[0].cf_alloc_export_word0.elem_size;
			array_size = self->words[1].cf_alloc_export_word1_buf.array_size;
			str_printf(buf_ptr, size_ptr, "ARRAY_SIZE(%d", array_size);
			if (elem_size)
				str_printf(buf_ptr, size_ptr, ",%d", elem_size + 1);
			str_printf(buf_ptr, size_ptr, ")");
		}
		else if (asm_is_token(fmt_str, "elem_size", &len))
		{
			int elem_size;

			assert(self->info->fmt[0] == EvgInstFormatCfAllocExportWord0 ||
					self->info->fmt[0] == EvgInstFormatCfAllocExportWord0Rat);
			elem_size = self->words[0].cf_alloc_export_word0.elem_size;
			if (elem_size)
				str_printf(buf_ptr, size_ptr, "ELEM_SIZE(%d)", elem_size);
		}
		else if (asm_is_token(fmt_str, "vtx_dst_gpr", &len))
		{
			int dst_gpr, dst_rel;
			int dst_sel_w, dst_sel_z, dst_sel_y, dst_sel_x;

			/* Destination register */
			assert(self->info->fmt[1] == EvgInstFormatVtxWord1Gpr);
			dst_gpr = self->words[1].vtx_word1_gpr.dst_gpr;
			dst_rel = self->words[1].vtx_word1_gpr.dst_rel;
			EvgInstGprDumpBuf(dst_gpr, dst_rel, -1, 0, buf_ptr, size_ptr);

			/* Destination mask */
			dst_sel_x = self->words[1].vtx_word1_gpr.dst_sel_x;
			dst_sel_y = self->words[1].vtx_word1_gpr.dst_sel_y;
			dst_sel_z = self->words[1].vtx_word1_gpr.dst_sel_z;
			dst_sel_w = self->words[1].vtx_word1_gpr.dst_sel_w;
			if (dst_sel_x != 0 || dst_sel_y != 1 || dst_sel_z != 2 || dst_sel_w != 3)
				str_printf(buf_ptr, size_ptr, ".%s%s%s%s",
						str_map_value(&evg_dst_sel_map, dst_sel_x),
						str_map_value(&evg_dst_sel_map, dst_sel_y),
						str_map_value(&evg_dst_sel_map, dst_sel_z),
						str_map_value(&evg_dst_sel_map, dst_sel_w));
		}
		else if (asm_is_token(fmt_str, "vtx_fetch_type", &len))
		{
			int fetch_type;

			assert(self->info->fmt[0] == EvgInstFormatVtxWord0);
			fetch_type = self->words[0].vtx_word0.fetch_type;
			if (fetch_type)
				str_printf(buf_ptr, size_ptr, "%sFETCH_TYPE(%s)",
						EvgInstTokenPrefix(loop_idx, &nl),
					str_map_value(&evg_fmt_vtx_fetch_type_map, fetch_type));
		}
		else if (asm_is_token(fmt_str, "vtx_fetch_whole_quad", &len))
		{
			int fetch_whole_quad;
			assert(self->info->fmt[0] == EvgInstFormatVtxWord0);
			fetch_whole_quad = self->words[0].vtx_word0.fetch_whole_quad;
			if (fetch_whole_quad)
				str_printf(buf_ptr, size_ptr, "WHOLE_QUAD");
		}
		else if (asm_is_token(fmt_str, "vtx_buffer_id", &len))
		{
			int buffer_id;

			assert(self->info->fmt[0] == EvgInstFormatVtxWord0);
			buffer_id = self->words[0].vtx_word0.buffer_id;
			str_printf(buf_ptr, size_ptr, "fc%d", buffer_id);

		}
		else if (asm_is_token(fmt_str, "vtx_src_gpr", &len))
		{
			int src_gpr;
			int src_rel;
			int src_sel;

			assert(self->info->fmt[0] == EvgInstFormatVtxWord0);
			src_gpr = self->words[0].vtx_word0.src_gpr;
			src_rel = self->words[0].vtx_word0.src_rel;
			src_sel = self->words[0].vtx_word0.src_sel_x;
			EvgInstGprDumpBuf(src_gpr, src_rel, src_sel, 0, buf_ptr, size_ptr);
		}
		else if (asm_is_token(fmt_str, "vtx_data_format", &len))
		{
			int data_format;
			int use_const_fields;

			assert(self->info->fmt[1] == EvgInstFormatVtxWord1Gpr ||
					self->info->fmt[1] == EvgInstFormatVtxWord1Sem);
			data_format = self->words[1].vtx_word1_gpr.data_format;
			use_const_fields = self->words[1].vtx_word1_gpr.use_const_fields;
			if (!use_const_fields)
				str_printf(buf_ptr, size_ptr, "%sFORMAT(%s)", EvgInstTokenPrefix(loop_idx, &nl),
					str_map_value(&evg_fmt_vtx_data_format_map, data_format));
		}
		else if (asm_is_token(fmt_str, "vtx_num_format", &len))
		{
			int num_format;
			int use_const_fields;

			assert(self->info->fmt[1] == EvgInstFormatVtxWord1Gpr
					|| self->info->fmt[1] == EvgInstFormatVtxWord1Sem);
			num_format = self->words[1].vtx_word1_gpr.num_format_all;
			use_const_fields = self->words[1].vtx_word1_gpr.use_const_fields;
			if (!use_const_fields && num_format)
				str_printf(buf_ptr, size_ptr, "%sNUM_FORMAT(%s)",
						EvgInstTokenPrefix(loop_idx, &nl),
						str_map_value(&evg_fmt_vtx_num_format_map, num_format));
		}
		else if (asm_is_token(fmt_str, "vtx_format_comp", &len))
		{
			int format_comp;
			int use_const_fields;

			assert(self->info->fmt[1] == EvgInstFormatVtxWord1Gpr
					|| self->info->fmt[1] == EvgInstFormatVtxWord1Sem);
			format_comp = self->words[1].vtx_word1_gpr.format_comp_all;
			use_const_fields = self->words[1].vtx_word1_gpr.use_const_fields;
			if (!use_const_fields && format_comp)
				str_printf(buf_ptr, size_ptr, "%sFORMAT_COMP(%s)",
						EvgInstTokenPrefix(loop_idx, &nl),
						str_map_value(&evg_fmt_vtx_format_comp_map, format_comp));
		}
		else if (asm_is_token(fmt_str, "vtx_srf_mode", &len))
		{
			int srf_mode;
			int use_const_fields;

			assert(self->info->fmt[1] == EvgInstFormatVtxWord1Gpr
					|| self->info->fmt[1] == EvgInstFormatVtxWord1Sem);
			srf_mode = self->words[1].vtx_word1_gpr.srf_mode_all;
			use_const_fields = self->words[1].vtx_word1_gpr.use_const_fields;
			if (!use_const_fields && srf_mode)
				str_printf(buf_ptr, size_ptr, "%sSRF_MODE(%s)", EvgInstTokenPrefix(loop_idx, &nl),
					str_map_value(&evg_fmt_vtx_srf_mode_map, srf_mode));
		}
		else if (asm_is_token(fmt_str, "vtx_offset", &len))
		{
			int offset;

			assert(self->info->fmt[2] == EvgInstFormatVtxWord2);
			offset = self->words[2].vtx_word2.offset;
			if (offset)
				str_printf(buf_ptr, size_ptr, "OFFSET(%d)", offset);
		}
		else if (asm_is_token(fmt_str, "vtx_endian_swap", &len))
		{
			int endian_swap;
			int use_const_fields;

			assert(self->info->fmt[1] == EvgInstFormatVtxWord1Gpr
					|| self->info->fmt[1] == EvgInstFormatVtxWord1Sem);
			assert(self->info->fmt[2] == EvgInstFormatVtxWord2);
			use_const_fields = self->words[1].vtx_word1_gpr.use_const_fields;
			endian_swap = self->words[2].vtx_word2.endian_swap;
			if (!use_const_fields && endian_swap)
				str_printf(buf_ptr, size_ptr, "%sENDIAN_SWAP(%s)", EvgInstTokenPrefix(loop_idx, &nl),
					str_map_value(&evg_fmt_vtx_endian_swap_map, endian_swap));
		}
		else if (asm_is_token(fmt_str, "vtx_cbns", &len))
		{
			int cbns;

			assert(self->info->fmt[2] == EvgInstFormatVtxWord2);
			cbns = self->words[2].vtx_word2.const_buf_no_stride;
			if (cbns)
				str_printf(buf_ptr, size_ptr, "%sCONST_BUF_NO_STRIDE", EvgInstTokenPrefix(loop_idx, &nl));

		}
		else if (asm_is_token(fmt_str, "vtx_mega_fetch", &len))
		{
			int mega_fetch_count;

			assert(self->info->fmt[0] == EvgInstFormatVtxWord0);
			mega_fetch_count = self->words[0].vtx_word0.mega_fetch_count;
			str_printf(buf_ptr, size_ptr, "MEGA(%d)", mega_fetch_count + 1);
		}
		else if (asm_is_token(fmt_str, "lds_op", &len))
		{
			assert(self->info->fmt[1] == EvgInstFormatAluWord1LdsIdxOp);
			str_printf(buf_ptr, size_ptr, "%s", str_map_value(&evg_fmt_lds_op_map,
				self->words[1].alu_word1_lds_idx_op.lds_op));
		}
		else if (asm_is_token(fmt_str, "nl", &len))
		{
			/* Mark line break before printing next token */
			nl = 1;
		}
		else if (asm_is_token(fmt_str, "tex_src_reg", &len))
		{
			assert(self->info->fmt[0] == EvgInstFormatTexWord0);
			assert(self->info->fmt[2] == EvgInstFormatTexWord2);

			int src_gpr = self->words[0].tex_word0.src_gpr;

			int src_sel[4];
			src_sel[0] = self->words[2].tex_word2.ssx;
			src_sel[1] = self->words[2].tex_word2.ssy;
			src_sel[2] = self->words[2].tex_word2.ssz;
			src_sel[3] = self->words[2].tex_word2.ssw;

			char src_sel_chars[4];

			/* Add element selections */
			int i;
			for (i = 0; i < 4; i++)
			{
				if (src_sel[i] == 0)
				{
					src_sel_chars[i] = 'x';
				}
				else if (src_sel[i] == 1)
				{
					src_sel_chars[i] = 'y';
				}
				else if (src_sel[i] == 2)
				{
					src_sel_chars[i] = 'z';
				}
				else if (src_sel[i] == 3)
				{
					src_sel_chars[i] = 'w';
				}
				else if (src_sel[i] == 4)
				{
					src_sel_chars[i] = '0';
				}
				else if (src_sel[i] == 5)
				{
					src_sel_chars[i] = '1';
				}
				else if (src_sel[i] == 6)
				{
					fatal("%s: src_sel value 6 in reserved", fmt_str);
				}
				else if (src_sel[i] == 7)
				{
					fatal("%s: src_sel value 7 is not supported", fmt_str);
				}
				else
				{
					fatal("%s: src_sel value %d is unknown",
							fmt_str, src_sel[i]);
				}
			}

			int count = 0;
			for(i = 0; i < 4; i++)
			{
				if (src_sel[i] == i)
				{
					count++;
				}
			}
			/* if count == 4, only print R/T instead of R.xyzw */
			if (count != 4)
			{
					str_printf(buf_ptr, size_ptr, "R%d.%c%c%c%c",
							src_gpr, src_sel_chars[0], src_sel_chars[1],
							src_sel_chars[2], src_sel_chars[3]);
			}
			else
			{
					str_printf(buf_ptr, size_ptr, "R%d", src_gpr);
			}
		}
		else if (asm_is_token(fmt_str, "tex_dst_reg", &len))
		{
			assert(self->info->fmt[1] == EvgInstFormatTexWord1);

			int dst_gpr = self->words[1].tex_word1.dst_gpr;

			int dst_sel[4];
			dst_sel[0] = self->words[1].tex_word1.dsx;
			dst_sel[1] = self->words[1].tex_word1.dsy;
			dst_sel[2] = self->words[1].tex_word1.dsz;
			dst_sel[3] = self->words[1].tex_word1.dsw;

			char dst_sel_chars[4];

			/* Add element selections */
			int i;
			for(i = 0; i < 4; i++)
			{
				if (dst_sel[i] == 0)
				{
					dst_sel_chars[i] = 'x';
				}
				else if (dst_sel[i] == 1)
				{
					dst_sel_chars[i] = 'y';
				}
				else if (dst_sel[i] == 2)
				{
					dst_sel_chars[i] = 'z';
				}
				else if (dst_sel[i] == 3)
				{
					dst_sel_chars[i] = 'w';
				}
				else if (dst_sel[i] == 4)
				{
					dst_sel_chars[i] = '0';
				}
				else if (dst_sel[i] == 5)
				{
					dst_sel_chars[i] = '1';
				}
				else if (dst_sel[i] == 6)
				{
					fatal("%s: dst_sel value 6 in reserved", fmt_str);
				}
				else if (dst_sel[i] == 7)
				{
					dst_sel_chars[i] = '_';
				}
				else
				{
					fatal("%s: dst_sel value %d is unknown", fmt_str, dst_sel[i]);
				}
			}

			int count = 0;
			for(i = 0; i < 4; i++)
			{
				if (dst_sel[i] == i)
				{
					count++;
				}
			}
			/* if count == 4, only print R/T instead of R.xyzw */
			if (count != 4)
			{
				str_printf(buf_ptr, size_ptr, "R%d.%c%c%c%c",
						dst_gpr, dst_sel_chars[0], dst_sel_chars[1],
						dst_sel_chars[2], dst_sel_chars[3]);
			}
			else
			{
				str_printf(buf_ptr, size_ptr, "R%d", dst_gpr);
			}
		}
		else if (asm_is_token(fmt_str, "tex_res_id", &len))
		{
			assert(self->info->fmt[0] == EvgInstFormatTexWord0);

			int resource_id = self->words[0].tex_word0.resource_id;
			str_printf(buf_ptr, size_ptr, "t%d", resource_id);
		}
		else if (asm_is_token(fmt_str, "tex_sampler_id", &len))
		{
			assert(self->info->fmt[2] == EvgInstFormatTexWord2);

			int sampler_id = self->words[2].tex_word2.sampler_id;
			str_printf(buf_ptr, size_ptr, "s%d", sampler_id);

		}
		else if (asm_is_token(fmt_str, "tex_props", &len))
		{
			assert(self->info->fmt[1] == EvgInstFormatTexWord1);

			if (self->words[1].tex_word1.ctx || self->words[1].tex_word1.cty ||
				self->words[1].tex_word1.ctz || self->words[1].tex_word1.ctw)
			{
				/* We can't tell if coordinates are normalized or if sampler
				 * is going to be a kernel argument */
			}
			else
			{
				str_printf(buf_ptr, size_ptr, "UNNORM(XYZW)");
			}
		}
		else if (asm_is_token(fmt_str, "mem_op_name", &len))
		{
			assert(self->info->fmt[0] == EvgInstFormatMemRdWord0 ||
					self->info->fmt[0] == EvgInstFormatMemGdsWord0);

			/* MEM_RD instruction has subopcode in MEM_OP field
			 * main opcode borrows from VTX */
			if (self->words[0].mem_rd_word0.mem_op == 0)
			{
				str_printf(buf_ptr, size_ptr, "MEM_RD_SCRATCH:");
			}
			else if (self->words[0].mem_rd_word0.mem_op == 2)
			{
				str_printf(buf_ptr, size_ptr, "MEM_RD_SCATTER:");
			}
			else
				str_printf(buf_ptr, size_ptr, "UNKNOWN");

		}
		else
			fatal("%s: token not recognized", fmt_str);

		fmt_str += len;
	}
}


void EvgInstDumpBuf(EvgInst *self, int count, int loop_idx,
	char *buf, int size)
{
	EvgInstSlotDumpBuf(self, count, loop_idx,
		-1, buf, size);
}


void EvgInstDumpGpr(int gpr, int rel, int chan, int im, FILE *f)
{
	char buf[MAX_STRING_SIZE];
	char *sbuf = buf;
	int size = MAX_STRING_SIZE;

	EvgInstGprDumpBuf(gpr, rel, chan, im, &sbuf, &size);
	fprintf(f, "%s", buf);
}


void EvgInstSlotDump(EvgInst *self, int count, int loop_idx, int slot, FILE *f)
{
	char buf[MAX_STRING_SIZE];

	EvgInstSlotDumpBuf(self, count, loop_idx, slot, buf, sizeof buf);
	fprintf(f, "%s\n", buf);
}


void EvgInstDump(EvgInst *self, int count, int loop_idx, FILE *f)
{
	EvgInstSlotDump(self, count, loop_idx, -1, f);
}
