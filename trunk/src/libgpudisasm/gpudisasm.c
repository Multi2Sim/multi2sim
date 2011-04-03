/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpudisasm.h>
#include <misc.h>
#include <assert.h>
#include <string.h>
#include <debug.h>
#include <ctype.h>



/*
 * Initialization/finalization of disassembler
 */

/* Table containing information of all instructions */
static struct amd_inst_info_t amd_inst_info[AMD_INST_COUNT];

/* Pointers to 'amd_inst_info' table indexed by instruction opcode */
#define AMD_INST_INFO_CF_LONG_SIZE  256
#define AMD_INST_INFO_CF_SHORT_SIZE 16
#define AMD_INST_INFO_ALU_LONG_SIZE 256
#define AMD_INST_INFO_ALU_SHORT_SIZE 32
#define AMD_INST_INFO_TEX_SIZE 32
static struct amd_inst_info_t *amd_inst_info_cf_long[AMD_INST_INFO_CF_LONG_SIZE];  /* for 8-bit cf_inst */
static struct amd_inst_info_t *amd_inst_info_cf_short[AMD_INST_INFO_CF_SHORT_SIZE];  /* for 4-bit cf_inst */
static struct amd_inst_info_t *amd_inst_info_alu_long[AMD_INST_INFO_ALU_LONG_SIZE];  /* for ALU_OP2 */
static struct amd_inst_info_t *amd_inst_info_alu_short[AMD_INST_INFO_ALU_SHORT_SIZE];  /* for ALU_OP3 */
static struct amd_inst_info_t *amd_inst_info_tex[AMD_INST_INFO_TEX_SIZE];  /* For tex instructions */


void amd_disasm_init()
{
	struct amd_inst_info_t *info;
	int i;

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _fmt0, _fmt1, _fmt2, _category, _opcode, _flags) \
	info = &amd_inst_info[AMD_INST_##_name]; \
	info->inst = AMD_INST_##_name; \
	info->category = AMD_CAT_##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->fmt[0] = FMT_##_fmt0; \
	info->fmt[1] = FMT_##_fmt1; \
	info->fmt[2] = FMT_##_fmt2; \
	info->opcode = _opcode; \
	info->flags = _flags; \
	info->size = (FMT_##_fmt0 ? 1 : 0) + (FMT_##_fmt1 ? 1 : 0) + (FMT_##_fmt2 ? 1 : 0 );
#include "gpudisasm.dat"
#undef DEFINST
	
	/* Tables of pointers to 'amd_inst_info' */
	for (i = 1; i < AMD_INST_COUNT; i++) {
		info = &amd_inst_info[i];
		if (info->fmt[1] == FMT_CF_WORD1 ||
			info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_BUF ||
			info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_SWIZ)
		{
			assert(IN_RANGE(info->opcode, 0, AMD_INST_INFO_CF_LONG_SIZE - 1));
			amd_inst_info_cf_long[info->opcode] = info;
			continue;
		}
		if (info->fmt[1] == FMT_CF_ALU_WORD1 ||
			info->fmt[1] == FMT_CF_ALU_WORD1_EXT)
		{
			assert(IN_RANGE(info->opcode, 0, AMD_INST_INFO_CF_SHORT_SIZE - 1));
			amd_inst_info_cf_short[info->opcode] = info;
			continue;
		}
		if (info->fmt[1] == FMT_ALU_WORD1_OP2) {
			assert(IN_RANGE(info->opcode, 0, AMD_INST_INFO_ALU_LONG_SIZE - 1));
			amd_inst_info_alu_long[info->opcode] = info;
			continue;
		}
		if (info->fmt[1] == FMT_ALU_WORD1_OP3 ||
			info->fmt[1] == FMT_ALU_WORD1_LDS_IDX_OP)
		{
			assert(IN_RANGE(info->opcode, 0, AMD_INST_INFO_ALU_SHORT_SIZE - 1));
			amd_inst_info_alu_short[info->opcode] = info;
			continue;
		}
		if (info->fmt[0] == FMT_TEX_WORD0 || info->fmt[0] == FMT_VTX_WORD0) {
			assert(IN_RANGE(info->opcode, 0, AMD_INST_INFO_TEX_SIZE - 1));
			amd_inst_info_tex[info->opcode] = info;
			continue;
		}
		fprintf(stderr, "warning: '%s' not indexed\n", info->name);
	}
}


void amd_disasm_done()
{
}




/*
 * Decoder
 */

void *amd_inst_decode_cf(void *buf, struct amd_inst_t *inst)
{
	uint32_t cf_inst_short, cf_inst_long;
	int end_of_program;

	/* Read instruction words (64-bit) */
	memset(inst, 0, sizeof(struct amd_inst_t));
	memcpy(inst->words, buf, 8);

	/* Decode instruction */
	cf_inst_short = inst->words[1].cf_alu_word1.cf_inst;
	inst->info = IN_RANGE(cf_inst_short, 0, AMD_INST_INFO_CF_SHORT_SIZE - 1) ?
		amd_inst_info_cf_short[cf_inst_short] : NULL;
	if (!inst->info) {
		cf_inst_long = inst->words[1].cf_word1.cf_inst;
		inst->info = IN_RANGE(cf_inst_long, 0, AMD_INST_INFO_CF_LONG_SIZE - 1) ?
			amd_inst_info_cf_long[cf_inst_long] : NULL;
		if (!inst->info)
			fatal("unknown CF instruction; cf_inst_short=%d, cf_inst_long=%d",
				cf_inst_short, cf_inst_long);
	}

	/* If 'end_of_program' bit is set, return NULL */
	end_of_program = 0;
	if (inst->info->fmt[1] == FMT_CF_WORD1
		|| inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_BUF
		|| inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_SWIZ)
		end_of_program = inst->words[1].cf_word1.end_of_program;
	if (end_of_program)
		return NULL;

	/* Return pointer to next instruction */
	return buf + 8;
}


void *amd_inst_decode_alu(void *buf, struct amd_inst_t *inst)
{
	uint32_t alu_inst_short, alu_inst_long;

	/* Read instruction words (64-bit) */
	memset(inst, 0, sizeof(struct amd_inst_t));
	memcpy(inst->words, buf, 8);

	/* Decode instruction */
	alu_inst_short = inst->words[1].alu_word1_op3.alu_inst;
	inst->info = IN_RANGE(alu_inst_short, 0, AMD_INST_INFO_ALU_SHORT_SIZE - 1) ?
		amd_inst_info_alu_short[alu_inst_short] : NULL;
	if (!inst->info) {
		alu_inst_long = inst->words[1].alu_word1_op2.alu_inst;
		inst->info = IN_RANGE(alu_inst_long, 0, AMD_INST_INFO_ALU_LONG_SIZE - 1) ?
			amd_inst_info_alu_long[alu_inst_long] : NULL;
		if (!inst->info)
			fatal("unknown ALU instruction; alu_inst_short=%d, alu_inst_long=%d",
				alu_inst_short, alu_inst_long);
	}

	/* Return pointer to next instruction */
	return buf + 8;
}


void *amd_inst_decode_alu_group(void *buf, int group_id, struct amd_alu_group_t *group)
{
	int dest_chan, chan, last;
	struct amd_inst_t *inst;
	enum amd_alu_enum alu;
	int alu_busy[AMD_ALU_COUNT];

	/* Reset group */
	memset(group, 0, sizeof(struct amd_alu_group_t));
	group->id = group_id;
	memset(alu_busy, 0, sizeof(alu_busy));

	/* Decode instructions */
	do {
		
		/* Decode instruction */
		assert(group->inst_count < 5);
		inst = &group->inst[group->inst_count];
		buf = amd_inst_decode_alu(buf, inst);
		inst->alu_group = group;
		last = inst->words[0].alu_word0.last;

		/* Count associated literals.
		 * There is at least one literal slot if there is any destination element CHAN_
		 * There are two literal slots if there is any destination element CHAN_Z or CHAN_W. */
		if (inst->words[0].alu_word0.src0_sel == 253) {  /* ALU_SRC_LITERAL */
			chan = inst->words[0].alu_word0.src0_chan;
			group->literal_count = MAX(group->literal_count, (chan + 2) / 2);
		}
		if (inst->words[0].alu_word0.src1_sel == 253) {
			chan = inst->words[0].alu_word0.src1_chan;
			group->literal_count = MAX(group->literal_count, (chan + 2) / 2);
		}
		if (inst->info->fmt[1] == FMT_ALU_WORD1_OP3 && inst->words[1].alu_word1_op3.src2_sel == 253) {
			chan = inst->words[1].alu_word1_op3.src2_chan;
			group->literal_count = MAX(group->literal_count, (chan + 2) / 2);
		}

		/* Allocate instruction to ALU
		 * Page 4-5 Evergreen Manual - FIXME */

		/* Initially, set ALU as indicated by the destination operand. For both OP2 and OP3 formats, field
		 * 'dest_chan' is located at the same bit position. */
		dest_chan = inst->words[1].alu_word1_op2.dst_chan;
		alu = inst->info->flags & AMD_INST_FLAG_TRANS_ONLY ? AMD_ALU_TRANS : dest_chan;
		if (alu_busy[alu])
			alu = AMD_ALU_TRANS;
		if (alu_busy[alu])
			fatal("group_id=%d, inst_id=%d: cannot allocate ALU", group_id, group->inst_count);
		alu_busy[alu] = 1;
		inst->alu = alu;

		/* One more instruction */
		group->inst_count++;

	} while (!last);


	/* Decode literals */
	memcpy(group->literal, buf, group->literal_count * 8);
	buf += group->literal_count * 8;

	/* Return new buffer pointer */
	return buf;
}


void *amd_inst_decode_tc(void *buf, struct amd_inst_t *inst)
{
	uint32_t tex_inst;

	/* Read instruction words (96-bit, 128-bit padded) */
	memset(inst, 0, sizeof(struct amd_inst_t));
	memcpy(inst->words, buf, 12);

	/* Decode instruction */
	tex_inst = inst->words[0].tex_word0.tex_inst;
	inst->info = IN_RANGE(tex_inst, 0, AMD_INST_INFO_TEX_SIZE - 1) ?
		amd_inst_info_tex[tex_inst] : NULL;
	if (!inst->info)
		fatal("unknown TEX instruction; tex_inst=%d", tex_inst);

	/* Return pointer to next instruction */
	return buf + 16;
}



/*
 * Disassembler
 */

struct string_map_t amd_pv_map = {
	5, {
		{ "PV.x", AMD_ALU_X },
		{ "PV.y", AMD_ALU_Y },
		{ "PV.z", AMD_ALU_Z },
		{ "PV.w", AMD_ALU_W },
		{ "PS", AMD_ALU_TRANS }
	}
};

struct string_map_t amd_alu_map = {
	5, {
		{ "x", AMD_ALU_X },
		{ "y", AMD_ALU_Y },
		{ "z", AMD_ALU_Z },
		{ "w", AMD_ALU_W },
		{ "t", AMD_ALU_TRANS }
	}
};

struct string_map_t bank_swizzle_map = {
	6, {
		{ "", 0 },
		{ "VEC_021", 1 },
		{ "VEC_120", 2 },
		{ "VEC_102", 3 },
		{ "VEC_201", 4 },
		{ "VEC_210", 5 }
	}
};

struct string_map_t rat_inst_map = {
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


struct string_map_t cf_cond_map = {
	4, {
		{ "ACTIVE", 0 },
		{ "FALSE", 1 },
		{ "BOOL", 2 },
		{ "NOT_BOOL", 3 }
	}
};


struct string_map_t src_sel_map = {
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


struct string_map_t dst_sel_map = {
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


int amd_inst_is_token(char *fmt_str, char *token_str, int *token_str_len)
{
	*token_str_len = strlen(token_str);
	return !strncmp(fmt_str, token_str, *token_str_len) &&
		!isalnum(fmt_str[*token_str_len]);
}


char *amd_inst_token_prefix(int loop_idx, int *nl)
{
	static char token_prefix[MAX_STRING_SIZE];

	/* Empty prefix by default */
	token_prefix[0] = '\0';

	/* Line break */
	if (*nl) {
		int shift_count = loop_idx * 4 + 9;
		token_prefix[0] = '\n';
		memset(token_prefix + 1, ' ', shift_count);
		token_prefix[shift_count + 2] = '\0';
		*nl = 0;
	}

	/* Return prefix */
	return token_prefix;
}


void amd_inst_dump_kcache(int kcache_idx, int kcache_bank, int kcache_mode, int kcache_addr, FILE *f)
{
	if (!kcache_mode)  /* KCACHE_MODE_NOP */
		return;
	fprintf(f, "KCACHE%d(CB%d:", kcache_idx, kcache_bank);
	switch (kcache_mode) {
	case 1:  /* KCACHE_LOCK_1 */
		fprintf(f, "%d-%d", kcache_addr * 16, kcache_addr * 16 + 15);  /* FIXME: correct? */
		break;
	case 2:  /* KCACHE_LOCK_2 */
	case 3:  /* KCACHE_LOCK_LOOP_INDEX */
	default:
		fatal("amd_inst_dump_kcache: kcache_mode=%d not supported", kcache_mode);
	}
	fprintf(f, ") ");
}


void amd_inst_dump_gpr(int gpr, int rel, int chan, int im, FILE *f)
{
	char gpr_str[MAX_STRING_SIZE];

	/* Register  */
	if (gpr <= 123)
		sprintf(gpr_str, "R%d", gpr);
	else
		sprintf(gpr_str, "T%d", 127 - gpr);
	
	/* Relative addressing */
	if (rel) {
		if (rel && IN_RANGE(im, 0, 3))
			fprintf(f, "%s[A0.%s]", gpr_str, map_value(&amd_alu_map, AMD_ALU_X + im));
		else if (im == 4)
			fprintf(f, "%s[AL]", gpr_str);
		else if (im == 5)
			fprintf(f, "SR%d", gpr);
		else if (im == 6)
			fprintf(f, "SR%d[A0.x]", gpr);
	} else
		fprintf(f, "%s", gpr_str);

	/* Vector element */
	if (chan >= 0)
		fprintf(f, ".%s", map_value(&amd_alu_map, AMD_ALU_X + chan));
}


void amd_inst_dump_op_dest(struct amd_inst_t *inst, FILE *f)
{
	int gpr, rel, chan, index_mode;

	/* Fields 'dst_gpr', 'dst_rel', and 'dst_chan' are at the same bit positions in both
	 * ALU_WORD1_OP2 and ALU_WORD1_OP3 formats. */
	gpr = inst->words[1].alu_word1_op2.dst_gpr;
	rel = inst->words[1].alu_word1_op2.dst_rel;
	chan = inst->words[1].alu_word1_op2.dst_chan;
	index_mode = inst->words[0].alu_word0.index_mode;

	/* If 'write_mask' field is clear, print underscore */
	if (inst->info->fmt[1] == FMT_ALU_WORD1_OP2 && !inst->words[1].alu_word1_op2.write_mask) {
		fprintf(f, "____");
		return;
	}

	/* Print register */
	amd_inst_dump_gpr(gpr, rel, chan, index_mode, f);
}


/* Get parameters for a source register in an ALU instruction */
void amd_inst_get_op_src(struct amd_inst_t *inst, int src_idx,
	int *sel, int *rel, int *chan, int *neg, int *abs)
{
	/* Valid formats */
	assert(inst->info->fmt[0] == FMT_ALU_WORD0
		|| inst->info->fmt[0] == FMT_ALU_WORD0_LDS_IDX_OP);
	assert(inst->info->fmt[1] == FMT_ALU_WORD1_OP2
		|| inst->info->fmt[1] == FMT_ALU_WORD1_OP3
		|| inst->info->fmt[1] == FMT_ALU_WORD1_LDS_IDX_OP);
	
	/* Get parameters */
	switch (src_idx) {

	case 0:
		
		/* Fields:	src0_sel, src0_rel, src_chan
		 * Present:	ALU_WORD0, ALU_WORD0_LDS_IDX_OP 
		 * Absent:	-
		 */
		*sel = inst->words[0].alu_word0.src0_sel;
		*rel = inst->words[0].alu_word0.src0_rel;
		*chan = inst->words[0].alu_word0.src0_chan;

		/* Fields:	src0_neg
		 * Present:	ALU_WORD0
		 * Absent:	ALU_WORD0_LDS_IDX_OP
		 */
		*neg = inst->info->fmt[0] == FMT_ALU_WORD0 ? inst->words[0].alu_word0.src0_neg : 0;

		/* Fields:	src0_abs
		 * Present:	ALU_WORD1_OP2
		 * Absent:	ALU_WORD1_OP3, ALU_WORD1_LDS_IDX_OP
		 */
		*abs = inst->info->fmt[1] == FMT_ALU_WORD1_OP2 ? inst->words[1].alu_word1_op2.src0_abs : 0;
		break;

	case 1:
		
		/* Fields:	src1_sel, src1_rel, src1_chan
		 * Present:	ALU_WORD0, ALU_WORD0_LDS_IDX_OP
		 * Absent:	-
		 */
		*sel = inst->words[0].alu_word0.src1_sel;
		*rel = inst->words[0].alu_word0.src1_rel;
		*chan = inst->words[0].alu_word0.src1_chan;

		/* Fields:	src1_neg
		 * Present:	ALU_WORD0
		 * Absent:	ALU_WORD0_LDS_IDX_OP
		 */
		*neg = inst->info->fmt[0] == FMT_ALU_WORD0 ? inst->words[0].alu_word0.src1_neg : 0;

		/* Fields:	src_abs
		 * Present:	ALU_WORD1_OP2
		 * Absent:	ALU_WORD1_OP3, ALU_WORD1_LDS_IDX_OP
		 */
		*abs = inst->info->fmt[1] == FMT_ALU_WORD1_OP2 ? inst->words[1].alu_word1_op2.src1_abs : 0;
		break;

	case 2:
		
		/* Fields:	src2_sel, src2_rel, src2_chan
		 * Present:	ALU_WORD1_OP3, ALU_WORD1_LDS_IDX_OP
		 * Absent:	ALU_WORD1_OP2
		 */
		assert(inst->info->fmt[1] == FMT_ALU_WORD1_OP3
			|| inst->info->fmt[1] == FMT_ALU_WORD1_LDS_IDX_OP);
		*sel = inst->words[1].alu_word1_op3.src2_sel;
		*rel = inst->words[1].alu_word1_op3.src2_rel;
		*chan = inst->words[1].alu_word1_op3.src2_chan;

		/* Fields:	src2_neg
		 * Present:	ALU_WORD1_OP3
		 * Absent:	ALU_WORD1_LDS_IDX_OP, ALU_WORD_OP2
		 */
		*neg = inst->info->fmt[1] == FMT_ALU_WORD1_OP3 ? inst->words[1].alu_word1_op3.src2_neg : 0;

		/* Fields:	src2_abs
		 * Present:	-
		 * Absent:	ALU_WORD1_OP3, ALU_WORD1_LDS_IDX_OP, ALU_WORD_OP2
		 */
		*abs = 0;
		break;

	default:
		fatal("amd_inst_dump_op_src: wrong src_idx");
	}
}


void amd_inst_dump_op_src(struct amd_inst_t *inst, int src_idx, FILE *f)
{
	int sel, rel, chan, neg, abs;

	/* Get parameters */
	amd_inst_get_op_src(inst, src_idx,
		&sel, &rel, &chan, &neg, &abs);

	/* Negation and first bracket for abs */
	if (neg)
		fprintf(f, "-");
	if (abs)
		fprintf(f, "|");

	/* 0..127: Value in GPR */
	if (IN_RANGE(sel, 0, 127)) {
		int index_mode;
		index_mode = inst->words[0].alu_word0.index_mode;
		amd_inst_dump_gpr(sel, rel, chan, index_mode, f);
		goto end;
	}

	/* 128..159: Kcache constants in bank 0 */
	if (IN_RANGE(sel, 128, 159)) {
		fprintf(f, "KC0[%d].%s", sel - 128, map_value(&amd_alu_map, AMD_ALU_X + chan));
		goto end;
	}

	/* 160..191: Kcache constants in bank 1 */
	if (IN_RANGE(sel, 160, 191)) {
		fprintf(f, "KC1[%d].%s", sel - 160, map_value(&amd_alu_map, AMD_ALU_X + chan));
		goto end;
	}

	/* 256..287: Kcache constants in bank 2 */
	if (IN_RANGE(sel, 256, 287)) {
		fprintf(f, "KC2[%d].%s", sel - 256, map_value(&amd_alu_map, AMD_ALU_X + chan));
		goto end;
	}

	/* 288..319: Kcache constant in bank 3 */
	if (IN_RANGE(sel, 288, 319)) {
		fprintf(f, "KC3[%d].%s", sel - 288, map_value(&amd_alu_map, AMD_ALU_X + chan));
		goto end;
	}

	/* ALU_SRC_LITERAL */
	if (sel == 253) {
		uint32_t literal_int;
		float literal_float;
			
		assert(inst->alu_group);
		literal_float = inst->alu_group->literal[chan];
		literal_int = * (uint32_t *) &literal_float;
		fprintf(f, "(0x%08x, %.9ef).%s", literal_int, literal_float, map_value(&amd_alu_map, AMD_ALU_X + chan));
		goto end;
	}

	/* ALU_SRC_PV */
	if (sel == 254) {
		fprintf(f, "PV.%s", map_value(&amd_alu_map, AMD_ALU_X + chan));
		goto end;
	}

	/* Other */
	fprintf(f, "%s", map_value(&src_sel_map, sel));

end:
	/* Second bracket for abs */
	if (abs)
		fprintf(f, "|");
}


void amd_inst_dump(struct amd_inst_t *inst, int count, int loop_idx, FILE *f)
{
	char shift_str[MAX_STRING_SIZE];
	char *fmt_str;
	int len;
	int nl = 0;  /* new line */

	/* Shift */
	memset(shift_str, ' ', MAX_STRING_SIZE);
	shift_str[loop_idx * 4] = '\0';
	fprintf(f, "%s", shift_str);

	/* Instruction counter */
	if (inst->info->category == AMD_CAT_CF) {
		assert(count >= 0);
		fprintf(f, "%02d ", count);
	} else {
		if (count >= 0)
			fprintf(f, "     %2d  ", count);
		else
			fprintf(f, "         ");
	}
	
	/* ALU assignment for ALU instructions */
	if (inst->info->category == AMD_CAT_ALU)
		fprintf(f, "%s: ", map_value(&amd_alu_map, inst->alu));

	/* Format */
	fmt_str = inst->info->fmt_str;
	while (*fmt_str) {
		
		/* Literal */
		if (*fmt_str != '%') {
			fprintf(f, "%c", *fmt_str);
			fmt_str++;
			continue;
		}

		/* Token */
		fmt_str++;
		if (amd_inst_is_token(fmt_str, "name", &len)) {

			if (inst->info->category == AMD_CAT_ALU)
				fprintf(f, "%-11s", inst->info->name);
			else
				fprintf(f, "%s", inst->info->name);

		} else if (amd_inst_is_token(fmt_str, "alu_dst", &len)) {
			
			amd_inst_dump_op_dest(inst, f);

		} else if (amd_inst_is_token(fmt_str, "alu_src0", &len)) {
			
			amd_inst_dump_op_src(inst, 0, f);

		} else if (amd_inst_is_token(fmt_str, "alu_src1", &len)) {
			
			amd_inst_dump_op_src(inst, 1, f);

		} else if (amd_inst_is_token(fmt_str, "alu_src2", &len)) {
			
			amd_inst_dump_op_src(inst, 2, f);

		} else if (amd_inst_is_token(fmt_str, "alu_mod", &len)) {  /* ALU modifiers */
			
			/* Padding */
			assert(inst->info->fmt[1] == FMT_ALU_WORD1_OP2 || inst->info->fmt[1] == FMT_ALU_WORD1_OP3);
			fprintf(f, "    ");

			/* ALU_WORD1_OP2 - 'bank_swizzle' field.
			 * Common for ALU_WORD1_OP2 and ALU_WORD1_OP3 */
			fprintf(f, "%s", map_value(&bank_swizzle_map, inst->words[1].alu_word1_op2.bank_swizzle));
			
			/* ALU_WORD0 - 'pred_sel' field */
			if (inst->words[0].alu_word0.pred_sel == 2)  /* PRED_SEL_ZERO */
				fprintf(f, " (!p)");
			else if (inst->words[0].alu_word0.pred_sel == 3)  /* PRED_SEL_ONE */
				fprintf(f, " (p)");

			/* ALU_WORD1_OP2 - 'update_exec_mask' field */
			if (inst->info->fmt[1] == FMT_ALU_WORD1_OP2 && inst->words[1].alu_word1_op2.update_exec_mask)
				fprintf(f, " UPDATE_EXEC_MASK");

			/* ALU_WORD1_OP2 - 'update_pred' field */
			if (inst->info->fmt[1] == FMT_ALU_WORD1_OP2 && inst->words[1].alu_word1_op2.update_pred)
				fprintf(f, " UPDATE_PRED");

		} else if (amd_inst_is_token(fmt_str, "omod", &len)) {
			
			assert(inst->info->fmt[1] == FMT_ALU_WORD1_OP2);
			switch (inst->words[1].alu_word1_op2.omod) {
				case 0: fprintf(f, "  "); break;
				case 1: fprintf(f, "*2"); break;
				case 2: fprintf(f, "*4"); break;
				case 3: fprintf(f, "/2"); break;
			}
			
		} else if (amd_inst_is_token(fmt_str, "cf_addr", &len)) {

			assert(inst->info->fmt[0] == FMT_CF_WORD0);
			fprintf(f, "%d", inst->words[0].cf_word0.addr);

		} else if (amd_inst_is_token(fmt_str, "cf_cnt", &len)) {
			
			assert(inst->info->fmt[1] == FMT_CF_WORD1);
			fprintf(f, "%d", inst->words[1].cf_word1.count + 1);
		
		} else if (amd_inst_is_token(fmt_str, "pop_count", &len)) {

			int pop_count;
			assert(inst->info->fmt[1] == FMT_CF_WORD1);
			pop_count = inst->words[1].cf_word1.pop_count;
			if (pop_count)
				fprintf(f, "POP_CNT(%d)", pop_count);

		} else if (amd_inst_is_token(fmt_str, "cf_cond", &len)) {
			
			int cf_cond, valid_pixel_mode;
			assert(inst->info->fmt[1] == FMT_CF_WORD1);
			valid_pixel_mode = inst->words[1].cf_word1.valid_pixel_mode;
			cf_cond = inst->words[1].cf_word1.cond;
			if (valid_pixel_mode && cf_cond)
				fprintf(f, "CND(%s)", map_value(&cf_cond_map, cf_cond));

		} else if (amd_inst_is_token(fmt_str, "cf_const", &len)) {
			
			int cf_const, cf_cond, valid_pixel_mode;
			assert(inst->info->fmt[1] == FMT_CF_WORD1);
			valid_pixel_mode = inst->words[1].cf_word1.valid_pixel_mode;
			cf_cond = inst->words[1].cf_word1.cond;
			cf_const = inst->words[1].cf_word1.cf_const;
			if (valid_pixel_mode && IN_RANGE(cf_cond, 2, 3))
				fprintf(f, "CF_CONST(%d)", cf_const);

		} else if (amd_inst_is_token(fmt_str, "wqm", &len)) {
			
			int whole_quad_mode;
			assert(inst->info->fmt[1] == FMT_CF_WORD1 || inst->info->fmt[1] == FMT_CF_ALU_WORD1);
			whole_quad_mode = inst->words[1].cf_word1.whole_quad_mode;
			if (whole_quad_mode)
				fprintf(f, "WHOLE_QUAD");

		} else if (amd_inst_is_token(fmt_str, "vpm", &len)) {
			
			int valid_pixel_mode;
			assert(inst->info->fmt[1] == FMT_CF_WORD1 ||
				inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_BUF ||
				inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_SWIZ);
			valid_pixel_mode = inst->words[1].cf_word1.valid_pixel_mode;
			if (valid_pixel_mode)
				fprintf(f, "VPM");

		} else if (amd_inst_is_token(fmt_str, "cf_alu_addr", &len)) {

			assert(inst->info->fmt[0] == FMT_CF_ALU_WORD0);
			fprintf(f, "%d", inst->words[0].cf_alu_word0.addr);

		} else if (amd_inst_is_token(fmt_str, "cf_alu_cnt", &len)) {

			assert(inst->info->fmt[1] == FMT_CF_ALU_WORD1);
			fprintf(f, "%d", inst->words[1].cf_alu_word1.count + 1);

		} else if (amd_inst_is_token(fmt_str, "loop_idx", &len)) {
			
			fprintf(f, "i%d", loop_idx);

		} else if (amd_inst_is_token(fmt_str, "mark", &len)) {
			
			int mark;
			assert(inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_BUF ||
				inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_SWIZ);
			mark = inst->words[1].cf_alloc_export_word1_buf.mark;
			if (mark)
				fprintf(f, "MARK");

		} else if (amd_inst_is_token(fmt_str, "burst_count", &len)) {

			int burst_count;
			assert(inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_BUF ||
				inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_SWIZ);
			burst_count = inst->words[1].cf_alloc_export_word1_buf.burst_count;
			if (burst_count)
				fprintf(f, "BRSTCNT(%d)", burst_count);

		} else if (amd_inst_is_token(fmt_str, "no_barrier", &len)) {
			
			assert(inst->info->fmt[1] == FMT_CF_WORD1 || inst->info->fmt[1] == FMT_CF_ALU_WORD1 ||
				inst->info->fmt[1] == FMT_CF_ALU_WORD1_EXT ||
				inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_BUF ||
				inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_SWIZ);
			if (!inst->words[1].cf_word1.barrier)
				fprintf(f, "NO_BARRIER");

		} else if (amd_inst_is_token(fmt_str, "vpm", &len)) {
			
			assert(inst->info->fmt[1] == FMT_CF_WORD1 ||
				inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_BUF ||
				inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_SWIZ);
			if (inst->words[1].cf_word1.valid_pixel_mode)
				fprintf(f, "VPM");

		} else if (amd_inst_is_token(fmt_str, "kcache", &len)) {
			
			amd_inst_dump_kcache(0,
				inst->words[0].cf_alu_word0.kcache_bank0,
				inst->words[0].cf_alu_word0.kcache_mode0,
				inst->words[1].cf_alu_word1.kcache_addr0,
				f);
			amd_inst_dump_kcache(1,
				inst->words[0].cf_alu_word0.kcache_bank1,
				inst->words[1].cf_alu_word1.kcache_mode1,
				inst->words[1].cf_alu_word1.kcache_addr1,
				f);

		} else if (amd_inst_is_token(fmt_str, "rat_inst", &len)) {
			
			assert(inst->info->fmt[0] == FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			fprintf(f, "%s", map_value(&rat_inst_map, inst->words[0].cf_alloc_export_word0_rat.rat_inst));

		} else if (amd_inst_is_token(fmt_str, "rat_id", &len)) {
			
			assert(inst->info->fmt[0] == FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			fprintf(f, "%d", inst->words[0].cf_alloc_export_word0_rat.rat_id);

		} else if (amd_inst_is_token(fmt_str, "rat_index_mode", &len)) {
			
			int rim;
			assert(inst->info->fmt[0] == FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			rim = inst->words[0].cf_alloc_export_word0_rat.rat_index_mode;
			if (rim)
				fprintf(f, "+idx%d", rim - 1);

		} else if (amd_inst_is_token(fmt_str, "rat_index_gpr", &len)) {
			
			assert(inst->info->fmt[0] == FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			amd_inst_dump_gpr(inst->words[0].cf_alloc_export_word0_rat.index_gpr, 0, -1, 0, f);

		} else if (amd_inst_is_token(fmt_str, "comp_mask", &len)) {
			
			int comp_mask;
			assert(inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_BUF);
			comp_mask = inst->words[1].cf_alloc_export_word1_buf.comp_mask;
			if (comp_mask != 0xf)
				fprintf(f, ".%s%s%s%s", comp_mask & 1 ? "x" : "_", comp_mask & 2 ? "y" : "_",
					comp_mask & 4 ? "z" : "_", comp_mask & 8 ? "w" : "_");

		} else if (amd_inst_is_token(fmt_str, "rat_rw_gpr", &len)) {
			
			assert(inst->info->fmt[0] == FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			amd_inst_dump_gpr(inst->words[0].cf_alloc_export_word0_rat.rw_gpr, 0, -1, 0, f);
		
		} else if (amd_inst_is_token(fmt_str, "array_size", &len)) {
			
			int elem_size, array_size;
			assert(inst->info->fmt[0] == FMT_CF_ALLOC_EXPORT_WORD0 || inst->info->fmt[0] == FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			assert(inst->info->fmt[1] == FMT_CF_ALLOC_EXPORT_WORD1_BUF);
			elem_size = inst->words[0].cf_alloc_export_word0.elem_size;
			array_size = inst->words[1].cf_alloc_export_word1_buf.array_size;
			fprintf(f, "ARRAY_SIZE(%d", array_size);
			if (elem_size)
				fprintf(f, ",%d", elem_size + 1);
			fprintf(f, ")");

		} else if (amd_inst_is_token(fmt_str, "elem_size", &len)) {
			
			int elem_size;
			assert(inst->info->fmt[0] == FMT_CF_ALLOC_EXPORT_WORD0 || inst->info->fmt[0] == FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			elem_size = inst->words[0].cf_alloc_export_word0.elem_size;
			if (elem_size)
				fprintf(f, "ELEM_SIZE(%d)", elem_size);

		} else if (amd_inst_is_token(fmt_str, "vtx_dst_gpr", &len)) {
			
			int dst_gpr, dst_rel;
			int dst_sel_w, dst_sel_z, dst_sel_y, dst_sel_x;

			/* Destination register */
			assert(inst->info->fmt[1] == FMT_VTX_WORD1_GPR);
			dst_gpr = inst->words[1].vtx_word1_gpr.dst_gpr;
			dst_rel = inst->words[1].vtx_word1_gpr.dst_rel;
			amd_inst_dump_gpr(dst_gpr, dst_rel, -1, 0, f);

			/* Destination mask */
			dst_sel_x = inst->words[1].vtx_word1_gpr.dst_sel_x;
			dst_sel_y = inst->words[1].vtx_word1_gpr.dst_sel_y;
			dst_sel_z = inst->words[1].vtx_word1_gpr.dst_sel_z;
			dst_sel_w = inst->words[1].vtx_word1_gpr.dst_sel_w;
			if (dst_sel_x != 0 || dst_sel_y != 1 || dst_sel_z != 2 || dst_sel_w != 3)
				fprintf(f, ".%s%s%s%s", map_value(&dst_sel_map, dst_sel_x), map_value(&dst_sel_map, dst_sel_y),
					map_value(&dst_sel_map, dst_sel_z), map_value(&dst_sel_map, dst_sel_w));

		} else if (amd_inst_is_token(fmt_str, "vtx_fetch_type", &len)) {
			
			int fetch_type;
			assert(inst->info->fmt[0] == FMT_VTX_WORD0);
			fetch_type = inst->words[0].vtx_word0.fetch_type;
			if (fetch_type)
				fprintf(f, "%sFETCH_TYPE(%s)", amd_inst_token_prefix(loop_idx, &nl),
					map_value(&fmt_vtx_fetch_type_map, fetch_type));

		} else if (amd_inst_is_token(fmt_str, "vtx_fetch_whole_quad", &len)) {
			
			int fetch_whole_quad;
			assert(inst->info->fmt[0] == FMT_VTX_WORD0);
			fetch_whole_quad = inst->words[0].vtx_word0.fetch_whole_quad;
			if (fetch_whole_quad)
				fprintf(f, "WHOLE_QUAD");

		} else if (amd_inst_is_token(fmt_str, "vtx_buffer_id", &len)) {
			
			int buffer_id;
			assert(inst->info->fmt[0] == FMT_VTX_WORD0);
			buffer_id = inst->words[0].vtx_word0.buffer_id;
			fprintf(f, "fc%d", buffer_id);

		} else if (amd_inst_is_token(fmt_str, "vtx_src_gpr", &len)) {
			
			int src_gpr, src_rel, src_sel;
			assert(inst->info->fmt[0] == FMT_VTX_WORD0);
			src_gpr = inst->words[0].vtx_word0.src_gpr;
			src_rel = inst->words[0].vtx_word0.src_rel;
			src_sel = inst->words[0].vtx_word0.src_sel_x;
			amd_inst_dump_gpr(src_gpr, src_rel, src_sel, 0, f);

		} else if (amd_inst_is_token(fmt_str, "vtx_data_format", &len)) {
			
			int data_format, use_const_fields;

			assert(inst->info->fmt[1] == FMT_VTX_WORD1_GPR || inst->info->fmt[1] == FMT_VTX_WORD1_SEM);
			data_format = inst->words[1].vtx_word1_gpr.data_format;
			use_const_fields = inst->words[1].vtx_word1_gpr.use_const_fields;
			if (!use_const_fields)
				fprintf(f, "%sFORMAT(%s)", amd_inst_token_prefix(loop_idx, &nl),
					map_value(&fmt_vtx_data_format_map, data_format));

		} else if (amd_inst_is_token(fmt_str, "vtx_num_format", &len)) {
			
			int num_format, use_const_fields;

			assert(inst->info->fmt[1] == FMT_VTX_WORD1_GPR || inst->info->fmt[1] == FMT_VTX_WORD1_SEM);
			num_format = inst->words[1].vtx_word1_gpr.num_format_all;
			use_const_fields = inst->words[1].vtx_word1_gpr.use_const_fields;
			if (!use_const_fields && num_format)
				fprintf(f, "%sNUM_FORMAT(%s)", amd_inst_token_prefix(loop_idx, &nl),
					map_value(&fmt_vtx_num_format_map, num_format));

		} else if (amd_inst_is_token(fmt_str, "vtx_format_comp", &len)) {
			
			int format_comp, use_const_fields;

			assert(inst->info->fmt[1] == FMT_VTX_WORD1_GPR || inst->info->fmt[1] == FMT_VTX_WORD1_SEM);
			format_comp = inst->words[1].vtx_word1_gpr.format_comp_all;
			use_const_fields = inst->words[1].vtx_word1_gpr.use_const_fields;
			if (!use_const_fields && format_comp)
				fprintf(f, "%sFORMAT_COMP(%s)", amd_inst_token_prefix(loop_idx, &nl),
					map_value(&fmt_vtx_format_comp_map, format_comp));

		} else if (amd_inst_is_token(fmt_str, "vtx_srf_mode", &len)) {
			
			int srf_mode, use_const_fields;

			assert(inst->info->fmt[1] == FMT_VTX_WORD1_GPR || inst->info->fmt[1] == FMT_VTX_WORD1_SEM);
			srf_mode = inst->words[1].vtx_word1_gpr.srf_mode_all;
			use_const_fields = inst->words[1].vtx_word1_gpr.use_const_fields;
			if (!use_const_fields && srf_mode)
				fprintf(f, "%sSRF_MODE(%s)", amd_inst_token_prefix(loop_idx, &nl),
					map_value(&fmt_vtx_srf_mode_map, srf_mode));

		} else if (amd_inst_is_token(fmt_str, "vtx_offset", &len)) {
			
			int offset;
			assert(inst->info->fmt[2] == FMT_VTX_WORD2);
			offset = inst->words[2].vtx_word2.offset;
			if (offset)
				fprintf(f, "OFFSET(%d)", offset);

		} else if (amd_inst_is_token(fmt_str, "vtx_endian_swap", &len)) {
			
			int endian_swap, use_const_fields;

			assert(inst->info->fmt[1] == FMT_VTX_WORD1_GPR || inst->info->fmt[1] == FMT_VTX_WORD1_SEM);
			assert(inst->info->fmt[2] == FMT_VTX_WORD2);
			use_const_fields = inst->words[1].vtx_word1_gpr.use_const_fields;
			endian_swap = inst->words[2].vtx_word2.endian_swap;
			if (!use_const_fields && endian_swap)
				fprintf(f, "%sENDIAN_SWAP(%s)", amd_inst_token_prefix(loop_idx, &nl),
					map_value(&fmt_vtx_endian_swap_map, endian_swap));

		} else if (amd_inst_is_token(fmt_str, "vtx_cbns", &len)) {
			
			int cbns;
			assert(inst->info->fmt[2] == FMT_VTX_WORD2);
			cbns = inst->words[2].vtx_word2.const_buf_no_stride;
			if (cbns)
				fprintf(f, "%sCONST_BUF_NO_STRIDE", amd_inst_token_prefix(loop_idx, &nl));

		} else if (amd_inst_is_token(fmt_str, "vtx_mega_fetch", &len)) {
			
			int mega_fetch_count;
			assert(inst->info->fmt[0] == FMT_VTX_WORD0);
			mega_fetch_count = inst->words[0].vtx_word0.mega_fetch_count;
			fprintf(f, "MEGA(%d)", mega_fetch_count + 1);

		} else if (amd_inst_is_token(fmt_str, "lds_op", &len)) {
			
			assert(inst->info->fmt[1] == FMT_ALU_WORD1_LDS_IDX_OP);
			fprintf(f, "%s", map_value(&fmt_lds_op_map,
				inst->words[1].alu_word1_lds_idx_op.lds_op));

		} else if (amd_inst_is_token(fmt_str, "nl", &len)) {
			
			/* Mark line break before printing next token */
			nl = 1;

		} else if (amd_inst_is_token(fmt_str, "dump", &len)) {

			int i;
			fprintf(f, "\n\n");
			for (i = 0; i < AMD_INST_MAX_WORDS; i++)
				if (inst->info->fmt[i])
					fmt_word_dump(inst->words[i].bytes, inst->info->fmt[i], f);

		} else
			fatal("%s: token not recognized", fmt_str);
		fmt_str += len;
	}
	fprintf(f, "\n");
}


void amd_inst_words_dump(struct amd_inst_t *inst, FILE *f)
{
	int i;
	fprintf(f, "%s\n", inst->info->name);
	for (i = 0; i < AMD_INST_MAX_WORDS; i++) {
		if (!inst->info->fmt[i])
			break;
		fmt_word_dump(&inst->words[i], inst->info->fmt[i], f);
	}
}


void amd_alu_group_dump(struct amd_alu_group_t *group, int shift, FILE *f)
{
	int i;

	for (i = 0; i < group->inst_count; i++)
		amd_inst_dump(&group->inst[i], i ? -1 : group->id, shift, f);
	/*for (i = 0; i < group->literal_count; i++)
		fprintf(f, "(%g, %g)\n", group->literal[0].u.f[0], group->literal[0].u.f[1]);*/
}




