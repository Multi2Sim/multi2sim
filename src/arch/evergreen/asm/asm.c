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
#include <string.h>
#include <ctype.h>

#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>

#include "asm.h"



/*
 * Initialization/finalization of disassembler
 */

/* Table containing information of all instructions */
struct evg_inst_info_t evg_inst_info[EVG_INST_COUNT];

/* Pointers to 'amd_inst_info' table indexed by instruction opcode */
#define EVG_INST_INFO_CF_LONG_SIZE  256
#define EVG_INST_INFO_CF_SHORT_SIZE 16
#define EVG_INST_INFO_ALU_LONG_SIZE 256
#define EVG_INST_INFO_ALU_SHORT_SIZE 32
#define EVG_INST_INFO_TEX_SIZE 32

static struct evg_inst_info_t *evg_inst_info_cf_long[EVG_INST_INFO_CF_LONG_SIZE];  /* for 8-bit cf_inst */
static struct evg_inst_info_t *evg_inst_info_cf_short[EVG_INST_INFO_CF_SHORT_SIZE];  /* for 4-bit cf_inst */
static struct evg_inst_info_t *evg_inst_info_alu_long[EVG_INST_INFO_ALU_LONG_SIZE];  /* for ALU_OP2 */
static struct evg_inst_info_t *evg_inst_info_alu_short[EVG_INST_INFO_ALU_SHORT_SIZE];  /* for ALU_OP3 */
static struct evg_inst_info_t *evg_inst_info_tex[EVG_INST_INFO_TEX_SIZE];  /* For tex instructions */


void evg_disasm_init()
{
	struct evg_inst_info_t *info;
	int i;

	/* Type size assertions */
	assert(sizeof(union evg_reg_t) == 4);

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _fmt0, _fmt1, _fmt2, _category, _opcode, _flags) \
	info = &evg_inst_info[EVG_INST_##_name]; \
	info->inst = EVG_INST_##_name; \
	info->category = EVG_INST_CAT_##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->fmt[0] = EVG_FMT_##_fmt0; \
	info->fmt[1] = EVG_FMT_##_fmt1; \
	info->fmt[2] = EVG_FMT_##_fmt2; \
	info->opcode = _opcode; \
	info->flags = _flags; \
	info->size = (EVG_FMT_##_fmt0 ? 1 : 0) + (EVG_FMT_##_fmt1 ? 1 : 0) + (EVG_FMT_##_fmt2 ? 1 : 0);
#include "asm.dat"
#undef DEFINST
	
	/* Tables of pointers to 'evg_inst_info' */
	for (i = 1; i < EVG_INST_COUNT; i++)
	{
		info = &evg_inst_info[i];
		if (info->fmt[1] == EVG_FMT_CF_WORD1 ||
			info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_BUF ||
			info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_SWIZ)
		{
			assert(IN_RANGE(info->opcode, 0, EVG_INST_INFO_CF_LONG_SIZE - 1));
			evg_inst_info_cf_long[info->opcode] = info;
			continue;
		}
		if (info->fmt[1] == EVG_FMT_CF_ALU_WORD1 ||
			info->fmt[1] == EVG_FMT_CF_ALU_WORD1_EXT)
		{
			assert(IN_RANGE(info->opcode, 0, EVG_INST_INFO_CF_SHORT_SIZE - 1));
			evg_inst_info_cf_short[info->opcode] = info;
			continue;
		}
		if (info->fmt[1] == EVG_FMT_ALU_WORD1_OP2) {
			assert(IN_RANGE(info->opcode, 0, EVG_INST_INFO_ALU_LONG_SIZE - 1));
			evg_inst_info_alu_long[info->opcode] = info;
			continue;
		}
		if (info->fmt[1] == EVG_FMT_ALU_WORD1_OP3 ||
			info->fmt[1] == EVG_FMT_ALU_WORD1_LDS_IDX_OP)
		{
			assert(IN_RANGE(info->opcode, 0, EVG_INST_INFO_ALU_SHORT_SIZE - 1));
			evg_inst_info_alu_short[info->opcode] = info;
			continue;
		}
		if (info->fmt[0] == EVG_FMT_TEX_WORD0 || info->fmt[0] == EVG_FMT_VTX_WORD0 || info->fmt[0] == EVG_FMT_MEM_RD_WORD0) {
			assert(IN_RANGE(info->opcode, 0, EVG_INST_INFO_TEX_SIZE - 1));
			evg_inst_info_tex[info->opcode] = info;
			continue;
		}
		fprintf(stderr, "warning: '%s' not indexed\n", info->name);
	}
}


void evg_disasm_done()
{
}


/* Disassemble entire ELF buffer containing Evergreen ISA.
 * The ELF buffer representing an entire kernel '.text' section can be used here. */
void evg_disasm_buffer(struct elf_buffer_t *buffer, FILE *f)
{
	void *cf_buf;
	int inst_count;
	int cf_inst_count;
	int sec_inst_count;
	int loop_idx;

	/* Initialize */
	cf_buf = buffer->ptr;
	inst_count = 0;
	cf_inst_count = 0;
	sec_inst_count = 0;
	loop_idx = 0;

	/* Disassemble */
	while (cf_buf)
	{
		struct evg_inst_t cf_inst;

		/* CF Instruction */
		cf_buf = evg_inst_decode_cf(cf_buf, &cf_inst);
                if (cf_inst.info->flags & EVG_INST_FLAG_DEC_LOOP_IDX)
                {
                        assert(loop_idx > 0);
                        loop_idx--;
                }

		evg_inst_dump(&cf_inst, cf_inst_count, loop_idx, f);
		cf_inst_count++;
		inst_count++;

		/* ALU Clause */
		if (cf_inst.info->fmt[0] == EVG_FMT_CF_ALU_WORD0)
		{
			void *alu_buf, *alu_buf_end;
			struct evg_alu_group_t alu_group;

			alu_buf = buffer->ptr + cf_inst.words[0].cf_alu_word0.addr * 8;
			alu_buf_end = alu_buf + (cf_inst.words[1].cf_alu_word1.count + 1) * 8;
			while (alu_buf < alu_buf_end)
			{
				alu_buf = evg_inst_decode_alu_group(alu_buf, sec_inst_count, &alu_group);
				evg_alu_group_dump(&alu_group, loop_idx, f);
				sec_inst_count++;
				inst_count++;
			}
		}

		/* TEX Clause */
		if (cf_inst.info->inst == EVG_INST_TC)
		{
			char *tex_buf, *tex_buf_end;
			struct evg_inst_t inst;

			tex_buf = buffer->ptr + cf_inst.words[0].cf_word0.addr * 8;
			tex_buf_end = tex_buf + (cf_inst.words[1].cf_word1.count + 1) * 16;
			while (tex_buf < tex_buf_end)
			{
				tex_buf = evg_inst_decode_tc(tex_buf, &inst);
				evg_inst_dump(&inst, sec_inst_count, loop_idx, f);
				sec_inst_count++;
				inst_count++;
			}
		}

		/* Increase loop depth counter */
                if (cf_inst.info->flags & EVG_INST_FLAG_INC_LOOP_IDX)
                        loop_idx++;
	}
}




/*
 * Decoder
 */

void *evg_inst_decode_cf(void *buf, struct evg_inst_t *inst)
{
	uint32_t cf_inst_short, cf_inst_long;
	int end_of_program;

	/* Read instruction words (64-bit) */
	memset(inst, 0, sizeof(struct evg_inst_t));
	memcpy(inst->words, buf, 8);

	/* Decode instruction */
	cf_inst_short = inst->words[1].cf_alu_word1.cf_inst;
	inst->info = IN_RANGE(cf_inst_short, 0, EVG_INST_INFO_CF_SHORT_SIZE - 1) ?
		evg_inst_info_cf_short[cf_inst_short] : NULL;
	if (!inst->info) {
		cf_inst_long = inst->words[1].cf_word1.cf_inst;
		inst->info = IN_RANGE(cf_inst_long, 0, EVG_INST_INFO_CF_LONG_SIZE - 1) ?
			evg_inst_info_cf_long[cf_inst_long] : NULL;
		if (!inst->info)
			fatal("unknown CF instruction; cf_inst_short=%d, cf_inst_long=%d",
				cf_inst_short, cf_inst_long);
	}

	/* If 'end_of_program' bit is set, return NULL */
	end_of_program = 0;
	if (inst->info->fmt[1] == EVG_FMT_CF_WORD1
		|| inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_BUF
		|| inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_SWIZ)
		end_of_program = inst->words[1].cf_word1.end_of_program;
	if (end_of_program)
		return NULL;

	/* Return pointer to next instruction */
	return buf + 8;
}


void *evg_inst_decode_alu(void *buf, struct evg_inst_t *inst)
{
	uint32_t alu_inst_short, alu_inst_long;

	/* Read instruction words (64-bit) */
	memcpy(inst->words, buf, 8);

	/* Decode instruction */
	alu_inst_short = inst->words[1].alu_word1_op3.alu_inst;
	inst->info = IN_RANGE(alu_inst_short, 0, EVG_INST_INFO_ALU_SHORT_SIZE - 1) ?
		evg_inst_info_alu_short[alu_inst_short] : NULL;
	if (!inst->info) {
		alu_inst_long = inst->words[1].alu_word1_op2.alu_inst;
		inst->info = IN_RANGE(alu_inst_long, 0, EVG_INST_INFO_ALU_LONG_SIZE - 1) ?
			evg_inst_info_alu_long[alu_inst_long] : NULL;
		if (!inst->info)
			fatal("unknown ALU instruction; alu_inst_short=%d, alu_inst_long=%d",
				alu_inst_short, alu_inst_long);
	}

	/* Return pointer to next instruction */
	return buf + 8;
}


void *evg_inst_decode_alu_group(void *buf, int group_id, struct evg_alu_group_t *group)
{
	int dest_chan, chan, last;
	struct evg_inst_t *inst;
	enum evg_alu_enum alu;
	int alu_busy[EVG_ALU_COUNT];

	/* Reset group */
	memset(group, 0, sizeof(struct evg_alu_group_t));
	group->id = group_id;
	memset(alu_busy, 0, sizeof(alu_busy));

	/* Decode instructions */
	do {
		
		/* Decode instruction */
		assert(group->inst_count < 5);
		inst = &group->inst[group->inst_count];
		buf = evg_inst_decode_alu(buf, inst);
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
		if (inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP3 && inst->words[1].alu_word1_op3.src2_sel == 253) {
			chan = inst->words[1].alu_word1_op3.src2_chan;
			group->literal_count = MAX(group->literal_count, (chan + 2) / 2);
		}

		/* Allocate instruction to ALU
		 * Page 4-5 Evergreen Manual - FIXME */

		/* Initially, set ALU as indicated by the destination operand. For both OP2 and OP3 formats, field
		 * 'dest_chan' is located at the same bit position. */
		dest_chan = inst->words[1].alu_word1_op2.dst_chan;
		alu = inst->info->flags & EVG_INST_FLAG_TRANS_ONLY ? EVG_ALU_TRANS : dest_chan;
		if (alu_busy[alu])
			alu = EVG_ALU_TRANS;
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


void *evg_inst_decode_tc(void *buf, struct evg_inst_t *inst)
{
	uint32_t tex_inst;

	/* Read instruction words (96-bit, 128-bit padded) */
	memset(inst, 0, sizeof(struct evg_inst_t));
	memcpy(inst->words, buf, 12);

	/* Decode instruction */
	tex_inst = inst->words[0].tex_word0.tex_inst;
	inst->info = IN_RANGE(tex_inst, 0, EVG_INST_INFO_TEX_SIZE - 1) ?
		evg_inst_info_tex[tex_inst] : NULL;
	if (!inst->info)
		fatal("unknown TEX instruction; tex_inst=%d", tex_inst);

	/* Return pointer to next instruction */
	return buf + 16;
}



/*
 * Disassembler
 */

struct str_map_t evg_pv_map = {
	5, {
		{ "PV.x", EVG_ALU_X },
		{ "PV.y", EVG_ALU_Y },
		{ "PV.z", EVG_ALU_Z },
		{ "PV.w", EVG_ALU_W },
		{ "PS", EVG_ALU_TRANS }
	}
};

struct str_map_t evg_alu_map = {
	5, {
		{ "x", EVG_ALU_X },
		{ "y", EVG_ALU_Y },
		{ "z", EVG_ALU_Z },
		{ "w", EVG_ALU_W },
		{ "t", EVG_ALU_TRANS }
	}
};

struct str_map_t evg_bank_swizzle_map = {
	6, {
		{ "", 0 },
		{ "VEC_021", 1 },
		{ "VEC_120", 2 },
		{ "VEC_102", 3 },
		{ "VEC_201", 4 },
		{ "VEC_210", 5 }
	}
};

struct str_map_t evg_rat_inst_map = {
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


struct str_map_t evg_cf_cond_map = {
	4, {
		{ "ACTIVE", 0 },
		{ "FALSE", 1 },
		{ "BOOL", 2 },
		{ "NOT_BOOL", 3 }
	}
};


struct str_map_t evg_src_sel_map = {
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


struct str_map_t evg_dst_sel_map = {
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

struct str_map_t export_type_map = {
	4, {
		{ "PIX", 0 },
		{ "POS", 1 },
		{ "PARAM", 2 },
		{ "IND_ACK", 3 }
	}
};


static int evg_inst_is_token(char *fmt_str, char *token_str, int *token_str_len)
{
	*token_str_len = strlen(token_str);
	return !strncmp(fmt_str, token_str, *token_str_len) &&
		!isalnum(fmt_str[*token_str_len]);
}


static char *evg_inst_token_prefix(int loop_idx, int *nl)
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


static void evg_inst_dump_kcache_buf(int kcache_idx, int kcache_bank, int kcache_mode, int kcache_addr,
	char **buf_ptr, int *size_ptr)
{
	if (!kcache_mode)  /* KCACHE_MODE_NOP */
		return;
	str_printf(buf_ptr, size_ptr, "KCACHE%d(CB%d:", kcache_idx, kcache_bank);
	switch (kcache_mode) {
	case 1:  /* KCACHE_LOCK_1 */
		str_printf(buf_ptr, size_ptr, "%d-%d", kcache_addr * 16, kcache_addr * 16 + 15);  /* FIXME: correct? */
		break;
	case 2:  /* KCACHE_LOCK_2 */
	case 3:  /* KCACHE_LOCK_LOOP_INDEX */
	default:
		fatal("amd_inst_dump_kcache: kcache_mode=%d not supported", kcache_mode);
	}
	str_printf(buf_ptr, size_ptr, ") ");
}


static void evg_inst_dump_gpr_buf(int gpr, int rel, int chan, int im,
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
			str_printf(buf_ptr, size_ptr, "%s[A0.%s]", gpr_str, str_map_value(&evg_alu_map, EVG_ALU_X + im));
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
		str_printf(buf_ptr, size_ptr, ".%s", str_map_value(&evg_alu_map, EVG_ALU_X + chan));
}


static void evg_inst_dump_op_dest_buf(struct evg_inst_t *inst, char **buf_ptr, int *size_ptr)
{
	int gpr, rel, chan, index_mode;

	/* Fields 'dst_gpr', 'dst_rel', and 'dst_chan' are at the same bit positions in both
	 * EVG_ALU_WORD1_OP2 and EVG_ALU_WORD1_OP3 formats. */
	gpr = inst->words[1].alu_word1_op2.dst_gpr;
	rel = inst->words[1].alu_word1_op2.dst_rel;
	chan = inst->words[1].alu_word1_op2.dst_chan;
	index_mode = inst->words[0].alu_word0.index_mode;

	/* If 'write_mask' field is clear, print underscore */
	if (inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP2 && !inst->words[1].alu_word1_op2.write_mask) {
		str_printf(buf_ptr, size_ptr, "____");
		return;
	}

	/* Print register */
	evg_inst_dump_gpr_buf(gpr, rel, chan, index_mode, buf_ptr, size_ptr);
}


/* Copy an instruction. No special handling of fields is needed here, just copy the whole
 * structure. */
void evg_inst_copy(struct evg_inst_t *dest, struct evg_inst_t *src)
{
	memcpy(dest, src, sizeof(struct evg_inst_t));
}


/* Copy an ALU group. Fields 'alu_group' in comprising 'alu_inst' elements need to be updated
 * to point to the destination ALU group. */
void evg_alu_group_copy(struct evg_alu_group_t *dest, struct evg_alu_group_t *src)
{
	int i;
	memcpy(dest, src, sizeof(struct evg_alu_group_t));
	for (i = 0; i < src->inst_count; i++)
		dest->inst[i].alu_group = dest;
}


/* Get parameters for a source register in an ALU instruction */
void evg_inst_get_op_src(struct evg_inst_t *inst, int src_idx,
	int *sel, int *rel, int *chan, int *neg, int *abs)
{
	/* Valid formats */
	assert(inst->info->fmt[0] == EVG_FMT_ALU_WORD0
		|| inst->info->fmt[0] == EVG_FMT_ALU_WORD0_LDS_IDX_OP);
	assert(inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP2
		|| inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP3
		|| inst->info->fmt[1] == EVG_FMT_ALU_WORD1_LDS_IDX_OP);
	
	/* Get parameters */
	switch (src_idx)
	{

	case 0:
		
		/* Fields:	src0_sel, src0_rel, src_chan
		 * Present:	EVG_ALU_WORD0, EVG_ALU_WORD0_LDS_IDX_OP 
		 * Absent:	-
		 */
		*sel = inst->words[0].alu_word0.src0_sel;
		*rel = inst->words[0].alu_word0.src0_rel;
		*chan = inst->words[0].alu_word0.src0_chan;

		/* Fields:	src0_neg
		 * Present:	EVG_ALU_WORD0
		 * Absent:	EVG_ALU_WORD0_LDS_IDX_OP
		 */
		*neg = inst->info->fmt[0] == EVG_FMT_ALU_WORD0 ? inst->words[0].alu_word0.src0_neg : 0;

		/* Fields:	src0_abs
		 * Present:	EVG_ALU_WORD1_OP2
		 * Absent:	EVG_ALU_WORD1_OP3, EVG_ALU_WORD1_LDS_IDX_OP
		 */
		*abs = inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP2 ? inst->words[1].alu_word1_op2.src0_abs : 0;
		break;

	case 1:
		
		/* Fields:	src1_sel, src1_rel, src1_chan
		 * Present:	EVG_ALU_WORD0, EVG_ALU_WORD0_LDS_IDX_OP
		 * Absent:	-
		 */
		*sel = inst->words[0].alu_word0.src1_sel;
		*rel = inst->words[0].alu_word0.src1_rel;
		*chan = inst->words[0].alu_word0.src1_chan;

		/* Fields:	src1_neg
		 * Present:	EVG_ALU_WORD0
		 * Absent:	EVG_ALU_WORD0_LDS_IDX_OP
		 */
		*neg = inst->info->fmt[0] == EVG_FMT_ALU_WORD0 ? inst->words[0].alu_word0.src1_neg : 0;

		/* Fields:	src_abs
		 * Present:	EVG_ALU_WORD1_OP2
		 * Absent:	EVG_ALU_WORD1_OP3, EVG_ALU_WORD1_LDS_IDX_OP
		 */
		*abs = inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP2 ? inst->words[1].alu_word1_op2.src1_abs : 0;
		break;

	case 2:
		
		/* Fields:	src2_sel, src2_rel, src2_chan
		 * Present:	EVG_ALU_WORD1_OP3, EVG_ALU_WORD1_LDS_IDX_OP
		 * Absent:	EVG_ALU_WORD1_OP2
		 */
		assert(inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP3
			|| inst->info->fmt[1] == EVG_FMT_ALU_WORD1_LDS_IDX_OP);
		*sel = inst->words[1].alu_word1_op3.src2_sel;
		*rel = inst->words[1].alu_word1_op3.src2_rel;
		*chan = inst->words[1].alu_word1_op3.src2_chan;

		/* Fields:	src2_neg
		 * Present:	EVG_ALU_WORD1_OP3
		 * Absent:	EVG_ALU_WORD1_LDS_IDX_OP, ALU_WORD_OP2
		 */
		*neg = inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP3 ? inst->words[1].alu_word1_op3.src2_neg : 0;

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


void amd_inst_dump_op_src_buf(struct evg_inst_t *inst, int src_idx, char **buf_ptr, int *size_ptr)
{
	int sel, rel, chan, neg, abs;

	/* Get parameters */
	evg_inst_get_op_src(inst, src_idx,
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
		index_mode = inst->words[0].alu_word0.index_mode;
		evg_inst_dump_gpr_buf(sel, rel, chan, index_mode, buf_ptr, size_ptr);
		goto end;
	}

	/* 128..159: Kcache constants in bank 0 */
	if (IN_RANGE(sel, 128, 159))
	{
		str_printf(buf_ptr, size_ptr, "KC0[%d].%s", sel - 128, str_map_value(&evg_alu_map, EVG_ALU_X + chan));
		goto end;
	}

	/* 160..191: Kcache constants in bank 1 */
	if (IN_RANGE(sel, 160, 191))
	{
		str_printf(buf_ptr, size_ptr, "KC1[%d].%s", sel - 160, str_map_value(&evg_alu_map, EVG_ALU_X + chan));
		goto end;
	}

	/* 256..287: Kcache constants in bank 2 */
	if (IN_RANGE(sel, 256, 287))
	{
		str_printf(buf_ptr, size_ptr, "KC2[%d].%s", sel - 256, str_map_value(&evg_alu_map, EVG_ALU_X + chan));
		goto end;
	}

	/* 288..319: Kcache constant in bank 3 */
	if (IN_RANGE(sel, 288, 319))
	{
		str_printf(buf_ptr, size_ptr, "KC3[%d].%s", sel - 288, str_map_value(&evg_alu_map, EVG_ALU_X + chan));
		goto end;
	}

	/* ALU_SRC_LITERAL */
	if (sel == 253)
	{
		assert(inst->alu_group);
		str_printf(buf_ptr, size_ptr, "(0x%08x, %.9ef).%s", inst->alu_group->literal[chan].as_uint,
			inst->alu_group->literal[chan].as_float, str_map_value(&evg_alu_map, EVG_ALU_X + chan));
		goto end;
	}

	/* ALU_SRC_PV */
	if (sel == 254)
	{
		str_printf(buf_ptr, size_ptr, "PV.%s", str_map_value(&evg_alu_map, EVG_ALU_X + chan));
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
void evg_inst_slot_dump_buf(struct evg_inst_t *inst, int count, int loop_idx, int slot,
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
	if (inst->info->category == EVG_INST_CAT_CF)
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
		str_printf(buf_ptr, size_ptr, "%s: ", str_map_value(&evg_alu_map, slot));

	/* Format */
	fmt_str = inst->info->fmt_str;
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
		if (evg_inst_is_token(fmt_str, "name", &len))
		{
			if (inst->info->category == EVG_INST_CAT_ALU)
				str_printf(buf_ptr, size_ptr, "%-11s", inst->info->name);
			else
				str_printf(buf_ptr, size_ptr, "%s", inst->info->name);

		}
		else if (evg_inst_is_token(fmt_str, "alu_dst", &len))
		{
			evg_inst_dump_op_dest_buf(inst, buf_ptr, size_ptr);
		}
		else if (evg_inst_is_token(fmt_str, "alu_src0", &len))
		{
			amd_inst_dump_op_src_buf(inst, 0, buf_ptr, size_ptr);

		}
		else if (evg_inst_is_token(fmt_str, "alu_src1", &len))
		{
			amd_inst_dump_op_src_buf(inst, 1, buf_ptr, size_ptr);
		}
		else if (evg_inst_is_token(fmt_str, "alu_src2", &len))
		{
			amd_inst_dump_op_src_buf(inst, 2, buf_ptr, size_ptr);
		}

		/* ALU modifiers */
		else if (evg_inst_is_token(fmt_str, "alu_mod", &len))
		{
			/* Padding */
			assert(inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP2 || inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP3);
			str_printf(buf_ptr, size_ptr, "    ");

			/* EVG_ALU_WORD1_OP2 - 'bank_swizzle' field.
			 * Common for EVG_ALU_WORD1_OP2 and EVG_ALU_WORD1_OP3 */
			str_printf(buf_ptr, size_ptr, "%s", str_map_value(&evg_bank_swizzle_map, inst->words[1].alu_word1_op2.bank_swizzle));
			
			/* EVG_ALU_WORD0 - 'pred_sel' field */
			if (inst->words[0].alu_word0.pred_sel == 2)  /* PRED_SEL_ZERO */
				str_printf(buf_ptr, size_ptr, " (!p)");
			else if (inst->words[0].alu_word0.pred_sel == 3)  /* PRED_SEL_ONE */
				str_printf(buf_ptr, size_ptr, " (p)");

			/* EVG_ALU_WORD1_OP2 - 'update_exec_mask' field */
			if (inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP2 && inst->words[1].alu_word1_op2.update_exec_mask)
				str_printf(buf_ptr, size_ptr, " UPDATE_EXEC_MASK");

			/* EVG_ALU_WORD1_OP2 - 'update_pred' field */
			if (inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP2 && inst->words[1].alu_word1_op2.update_pred)
				str_printf(buf_ptr, size_ptr, " UPDATE_PRED");
		}
		else if (evg_inst_is_token(fmt_str, "omod", &len))
		{
			assert(inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP2);
			switch (inst->words[1].alu_word1_op2.omod)
			{
			case 0: str_printf(buf_ptr, size_ptr, "  "); break;
			case 1: str_printf(buf_ptr, size_ptr, "*2"); break;
			case 2: str_printf(buf_ptr, size_ptr, "*4"); break;
			case 3: str_printf(buf_ptr, size_ptr, "/2"); break;
			}
		}
		else if (evg_inst_is_token(fmt_str, "clamp", &len))
		{
			assert(inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP2 || inst->info->fmt[1] == EVG_FMT_ALU_WORD1_OP3);
			if (inst->words[1].alu_word1_op2.clamp || inst->words[1].alu_word1_op3.clamp)
				str_printf(buf_ptr, size_ptr, "CLAMP");
		}
		else if (evg_inst_is_token(fmt_str, "cf_addr", &len))
		{
			assert(inst->info->fmt[0] == EVG_FMT_CF_WORD0);
			str_printf(buf_ptr, size_ptr, "%d", inst->words[0].cf_word0.addr);
		}
		else if (evg_inst_is_token(fmt_str, "cf_cnt", &len))
		{
			assert(inst->info->fmt[1] == EVG_FMT_CF_WORD1);
			str_printf(buf_ptr, size_ptr, "%d", inst->words[1].cf_word1.count + 1);
		}
		else if (evg_inst_is_token(fmt_str, "cf_stream_id", &len))
		{
			assert(inst->info->fmt[1] == EVG_FMT_CF_WORD1);
			/* For EMIT, CUT, EMIT_CUT, bit[10] are the stream ID */
			int stream_id = inst->words[1].cf_word1.count % 2;
			str_printf(buf_ptr, size_ptr, "%d", stream_id);
		}
		else if (evg_inst_is_token(fmt_str, "pop_count", &len))
		{
			int pop_count;

			assert(inst->info->fmt[1] == EVG_FMT_CF_WORD1);
			pop_count = inst->words[1].cf_word1.pop_count;
			if (pop_count)
				str_printf(buf_ptr, size_ptr, "POP_CNT(%d)", pop_count);
		}
		else if (evg_inst_is_token(fmt_str, "cf_cond", &len))
		{
			int cf_cond;
			int valid_pixel_mode;
			
			assert(inst->info->fmt[1] == EVG_FMT_CF_WORD1);
			valid_pixel_mode = inst->words[1].cf_word1.valid_pixel_mode;
			cf_cond = inst->words[1].cf_word1.cond;
			if (valid_pixel_mode && cf_cond)
				str_printf(buf_ptr, size_ptr, "CND(%s)", str_map_value(&evg_cf_cond_map, cf_cond));
		}
		else if (evg_inst_is_token(fmt_str, "cf_const", &len))
		{
			int cf_const;
			int cf_cond;
			int valid_pixel_mode;
			
			assert(inst->info->fmt[1] == EVG_FMT_CF_WORD1);
			valid_pixel_mode = inst->words[1].cf_word1.valid_pixel_mode;
			cf_cond = inst->words[1].cf_word1.cond;
			cf_const = inst->words[1].cf_word1.cf_const;
			if (valid_pixel_mode && IN_RANGE(cf_cond, 2, 3))
				str_printf(buf_ptr, size_ptr, "CF_CONST(%d)", cf_const);
		}
		else if (evg_inst_is_token(fmt_str, "wqm", &len))
		{
			int whole_quad_mode;
			
			assert(inst->info->fmt[1] == EVG_FMT_CF_WORD1 || inst->info->fmt[1] == EVG_FMT_CF_ALU_WORD1);
			whole_quad_mode = inst->words[1].cf_word1.whole_quad_mode;
			if (whole_quad_mode)
				str_printf(buf_ptr, size_ptr, "WHOLE_QUAD");
		}
		else if (evg_inst_is_token(fmt_str, "vpm", &len))
		{
			int valid_pixel_mode;
			
			assert(inst->info->fmt[1] == EVG_FMT_CF_WORD1 ||
				inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_BUF ||
				inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_SWIZ);
			valid_pixel_mode = inst->words[1].cf_word1.valid_pixel_mode;
			if (valid_pixel_mode)
				str_printf(buf_ptr, size_ptr, "VPM");
		}
		else if (evg_inst_is_token(fmt_str, "cf_alu_addr", &len))
		{
			assert(inst->info->fmt[0] == EVG_FMT_CF_ALU_WORD0);
			str_printf(buf_ptr, size_ptr, "%d", inst->words[0].cf_alu_word0.addr);
		}
		else if (evg_inst_is_token(fmt_str, "cf_alu_cnt", &len))
		{
			assert(inst->info->fmt[1] == EVG_FMT_CF_ALU_WORD1);
			str_printf(buf_ptr, size_ptr, "%d", inst->words[1].cf_alu_word1.count + 1);
		}
		else if (evg_inst_is_token(fmt_str, "loop_idx", &len))
		{
			//str_printf(buf_ptr, size_ptr, "i%d", loop_idx);
			/* FIXME: what is this field? I think it is the CF_CONST value, but
			 * needs to be checked with AMD's ISA dump. */
			str_printf(buf_ptr, size_ptr, "i%d", inst->words[1].cf_word1.cf_const);
		}
		else if (evg_inst_is_token(fmt_str, "mark", &len))
		{
			int mark;
			
			assert(inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_BUF ||
				inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_SWIZ);
			mark = inst->words[1].cf_alloc_export_word1_buf.mark;
			if (mark)
				str_printf(buf_ptr, size_ptr, "MARK");
		}
		else if (evg_inst_is_token(fmt_str, "burst_count", &len))
		{
			int burst_count;

			assert(inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_BUF ||
				inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_SWIZ);
			burst_count = inst->words[1].cf_alloc_export_word1_buf.burst_count;
			if (burst_count)
				str_printf(buf_ptr, size_ptr, "BRSTCNT(%d)", burst_count);
		}
		else if (evg_inst_is_token(fmt_str, "no_barrier", &len))
		{
			assert(inst->info->fmt[1] == EVG_FMT_CF_WORD1 || inst->info->fmt[1] == EVG_FMT_CF_ALU_WORD1 ||
				inst->info->fmt[1] == EVG_FMT_CF_ALU_WORD1_EXT ||
				inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_BUF ||
				inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_SWIZ);
			if (!inst->words[1].cf_word1.barrier)
				str_printf(buf_ptr, size_ptr, "NO_BARRIER");
		}
		else if (evg_inst_is_token(fmt_str, "vpm", &len))
		{
			assert(inst->info->fmt[1] == EVG_FMT_CF_WORD1 ||
				inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_BUF ||
				inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_SWIZ);
			if (inst->words[1].cf_word1.valid_pixel_mode)
				str_printf(buf_ptr, size_ptr, "VPM");
		}
		else if (evg_inst_is_token(fmt_str, "kcache", &len))
		{
			evg_inst_dump_kcache_buf(0,
				inst->words[0].cf_alu_word0.kcache_bank0,
				inst->words[0].cf_alu_word0.kcache_mode0,
				inst->words[1].cf_alu_word1.kcache_addr0,
				buf_ptr, size_ptr);
			evg_inst_dump_kcache_buf(1,
				inst->words[0].cf_alu_word0.kcache_bank1,
				inst->words[1].cf_alu_word1.kcache_mode1,
				inst->words[1].cf_alu_word1.kcache_addr1,
				buf_ptr, size_ptr);
		}
		else if (evg_inst_is_token(fmt_str, "exp_type", &len))
		{
			assert(inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0);
			str_printf(buf_ptr, size_ptr, "%s", str_map_value(&export_type_map, inst->words[0].cf_alloc_export_word0.type));
		}
		else if (evg_inst_is_token(fmt_str, "exp_array_base", &len))
		{
			/* FIXME */
			assert(inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0);
			if (inst->words[0].cf_alloc_export_word0.type == 0)
			{	
				int array_base = inst->words[0].cf_alloc_export_word0.array_base % 8;
				str_printf(buf_ptr, size_ptr, "%d", array_base);
			}
			else if (inst->words[0].cf_alloc_export_word0.type == 1)
			{
				int array_base = inst->words[0].cf_alloc_export_word0.array_base % 4;
				str_printf(buf_ptr, size_ptr, "%d", array_base);
			}
			else if (inst->words[0].cf_alloc_export_word0.type == 2)
			{
				int array_base = inst->words[0].cf_alloc_export_word0.array_base % 32;
				str_printf(buf_ptr, size_ptr, "%d", array_base);
			}
			else
			{
				int array_base = inst->words[0].cf_alloc_export_word0.array_base;
				str_printf(buf_ptr, size_ptr, "%d", array_base);
			}
		}
		else if (evg_inst_is_token(fmt_str, "exp_rw_gpr", &len))
		{
			assert(inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0);
			assert(inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_SWIZ || inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_BUF);

			int rw_gpr = inst->words[0].cf_alloc_export_word0.rw_gpr;
			if (inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_SWIZ)
			{
				int src_sel[4];
				src_sel[0] = inst->words[1].cf_alloc_export_word1_swiz.sel_x;
				src_sel[1] = inst->words[1].cf_alloc_export_word1_swiz.sel_y;
				src_sel[2] = inst->words[1].cf_alloc_export_word1_swiz.sel_z;
				src_sel[3] = inst->words[1].cf_alloc_export_word1_swiz.sel_w;

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
						fatal("%s: src_sel value %d is unknown", fmt_str, src_sel[i]);
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
						str_printf(buf_ptr, size_ptr, "R%d.%c%c%c%c", rw_gpr, src_sel_chars[0], src_sel_chars[1],
								   src_sel_chars[2], src_sel_chars[3]);			
				}
				else
				{
						str_printf(buf_ptr, size_ptr, "R%d", rw_gpr);
				}				
			}
			else if (inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_BUF)
			{
				str_printf(buf_ptr, size_ptr, "R%d", rw_gpr);
			}

		}
		else if (evg_inst_is_token(fmt_str, "exp_index_gpr", &len))
		{
			assert(inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			str_printf(buf_ptr, size_ptr, "%s", str_map_value(&evg_rat_inst_map, inst->words[0].cf_alloc_export_word0_rat.rat_inst));
		}
		else if (evg_inst_is_token(fmt_str, "rat_inst", &len))
		{
			assert(inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			str_printf(buf_ptr, size_ptr, "%s", str_map_value(&evg_rat_inst_map, inst->words[0].cf_alloc_export_word0_rat.rat_inst));

		}
		else if (evg_inst_is_token(fmt_str, "rat_id", &len))
		{
			assert(inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			str_printf(buf_ptr, size_ptr, "%d", inst->words[0].cf_alloc_export_word0_rat.rat_id);
		}
		else if (evg_inst_is_token(fmt_str, "rat_index_mode", &len))
		{
			int rim;
			assert(inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			rim = inst->words[0].cf_alloc_export_word0_rat.rat_index_mode;
			if (rim)
				str_printf(buf_ptr, size_ptr, "+idx%d", rim - 1);
		}
		else if (evg_inst_is_token(fmt_str, "rat_index_gpr", &len))
		{
			assert(inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			evg_inst_dump_gpr_buf(inst->words[0].cf_alloc_export_word0_rat.index_gpr, 0, -1, 0, buf_ptr, size_ptr);
		}
		else if (evg_inst_is_token(fmt_str, "comp_mask", &len))
		{
			int comp_mask;
			
			assert(inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_BUF);
			comp_mask = inst->words[1].cf_alloc_export_word1_buf.comp_mask;
			if (comp_mask != 0xf)
				str_printf(buf_ptr, size_ptr, ".%s%s%s%s", comp_mask & 1 ? "x" : "_", comp_mask & 2 ? "y" : "_",
					comp_mask & 4 ? "z" : "_", comp_mask & 8 ? "w" : "_");
		}
		else if (evg_inst_is_token(fmt_str, "rat_rw_gpr", &len))
		{
			assert(inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			evg_inst_dump_gpr_buf(inst->words[0].cf_alloc_export_word0_rat.rw_gpr, 0, -1, 0, buf_ptr, size_ptr);
		}
		else if (evg_inst_is_token(fmt_str, "array_size", &len))
		{
			int elem_size;
			int array_size;
			
			assert(inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0 || inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			assert(inst->info->fmt[1] == EVG_FMT_CF_ALLOC_EXPORT_WORD1_BUF);
			elem_size = inst->words[0].cf_alloc_export_word0.elem_size;
			array_size = inst->words[1].cf_alloc_export_word1_buf.array_size;
			str_printf(buf_ptr, size_ptr, "ARRAY_SIZE(%d", array_size);
			if (elem_size)
				str_printf(buf_ptr, size_ptr, ",%d", elem_size + 1);
			str_printf(buf_ptr, size_ptr, ")");
		}
		else if (evg_inst_is_token(fmt_str, "elem_size", &len))
		{
			int elem_size;
			
			assert(inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0 || inst->info->fmt[0] == EVG_FMT_CF_ALLOC_EXPORT_WORD0_RAT);
			elem_size = inst->words[0].cf_alloc_export_word0.elem_size;
			if (elem_size)
				str_printf(buf_ptr, size_ptr, "ELEM_SIZE(%d)", elem_size);
		}
		else if (evg_inst_is_token(fmt_str, "vtx_dst_gpr", &len))
		{
			int dst_gpr, dst_rel;
			int dst_sel_w, dst_sel_z, dst_sel_y, dst_sel_x;

			/* Destination register */
			assert(inst->info->fmt[1] == EVG_FMT_VTX_WORD1_GPR);
			dst_gpr = inst->words[1].vtx_word1_gpr.dst_gpr;
			dst_rel = inst->words[1].vtx_word1_gpr.dst_rel;
			evg_inst_dump_gpr_buf(dst_gpr, dst_rel, -1, 0, buf_ptr, size_ptr);

			/* Destination mask */
			dst_sel_x = inst->words[1].vtx_word1_gpr.dst_sel_x;
			dst_sel_y = inst->words[1].vtx_word1_gpr.dst_sel_y;
			dst_sel_z = inst->words[1].vtx_word1_gpr.dst_sel_z;
			dst_sel_w = inst->words[1].vtx_word1_gpr.dst_sel_w;
			if (dst_sel_x != 0 || dst_sel_y != 1 || dst_sel_z != 2 || dst_sel_w != 3)
				str_printf(buf_ptr, size_ptr, ".%s%s%s%s", str_map_value(&evg_dst_sel_map, dst_sel_x), str_map_value(&evg_dst_sel_map, dst_sel_y),
					str_map_value(&evg_dst_sel_map, dst_sel_z), str_map_value(&evg_dst_sel_map, dst_sel_w));
		}
		else if (evg_inst_is_token(fmt_str, "vtx_fetch_type", &len))
		{
			int fetch_type;
			
			assert(inst->info->fmt[0] == EVG_FMT_VTX_WORD0);
			fetch_type = inst->words[0].vtx_word0.fetch_type;
			if (fetch_type)
				str_printf(buf_ptr, size_ptr, "%sFETCH_TYPE(%s)", evg_inst_token_prefix(loop_idx, &nl),
					str_map_value(&evg_fmt_vtx_fetch_type_map, fetch_type));
		}
		else if (evg_inst_is_token(fmt_str, "vtx_fetch_whole_quad", &len))
		{
			int fetch_whole_quad;
			assert(inst->info->fmt[0] == EVG_FMT_VTX_WORD0);
			fetch_whole_quad = inst->words[0].vtx_word0.fetch_whole_quad;
			if (fetch_whole_quad)
				str_printf(buf_ptr, size_ptr, "WHOLE_QUAD");
		}
		else if (evg_inst_is_token(fmt_str, "vtx_buffer_id", &len))
		{
			int buffer_id;
			
			assert(inst->info->fmt[0] == EVG_FMT_VTX_WORD0);
			buffer_id = inst->words[0].vtx_word0.buffer_id;
			str_printf(buf_ptr, size_ptr, "fc%d", buffer_id);

		}
		else if (evg_inst_is_token(fmt_str, "vtx_src_gpr", &len))
		{
			int src_gpr;
			int src_rel;
			int src_sel;
			
			assert(inst->info->fmt[0] == EVG_FMT_VTX_WORD0);
			src_gpr = inst->words[0].vtx_word0.src_gpr;
			src_rel = inst->words[0].vtx_word0.src_rel;
			src_sel = inst->words[0].vtx_word0.src_sel_x;
			evg_inst_dump_gpr_buf(src_gpr, src_rel, src_sel, 0, buf_ptr, size_ptr);
		}
		else if (evg_inst_is_token(fmt_str, "vtx_data_format", &len))
		{
			int data_format;
			int use_const_fields;

			assert(inst->info->fmt[1] == EVG_FMT_VTX_WORD1_GPR || inst->info->fmt[1] == EVG_FMT_VTX_WORD1_SEM);
			data_format = inst->words[1].vtx_word1_gpr.data_format;
			use_const_fields = inst->words[1].vtx_word1_gpr.use_const_fields;
			if (!use_const_fields)
				str_printf(buf_ptr, size_ptr, "%sFORMAT(%s)", evg_inst_token_prefix(loop_idx, &nl),
					str_map_value(&evg_fmt_vtx_data_format_map, data_format));
		}
		else if (evg_inst_is_token(fmt_str, "vtx_num_format", &len))
		{
			int num_format;
			int use_const_fields;

			assert(inst->info->fmt[1] == EVG_FMT_VTX_WORD1_GPR || inst->info->fmt[1] == EVG_FMT_VTX_WORD1_SEM);
			num_format = inst->words[1].vtx_word1_gpr.num_format_all;
			use_const_fields = inst->words[1].vtx_word1_gpr.use_const_fields;
			if (!use_const_fields && num_format)
				str_printf(buf_ptr, size_ptr, "%sNUM_FORMAT(%s)", evg_inst_token_prefix(loop_idx, &nl),
					str_map_value(&evg_fmt_vtx_num_format_map, num_format));
		}
		else if (evg_inst_is_token(fmt_str, "vtx_format_comp", &len))
		{
			int format_comp;
			int use_const_fields;

			assert(inst->info->fmt[1] == EVG_FMT_VTX_WORD1_GPR || inst->info->fmt[1] == EVG_FMT_VTX_WORD1_SEM);
			format_comp = inst->words[1].vtx_word1_gpr.format_comp_all;
			use_const_fields = inst->words[1].vtx_word1_gpr.use_const_fields;
			if (!use_const_fields && format_comp)
				str_printf(buf_ptr, size_ptr, "%sFORMAT_COMP(%s)", evg_inst_token_prefix(loop_idx, &nl),
					str_map_value(&evg_fmt_vtx_format_comp_map, format_comp));
		}
		else if (evg_inst_is_token(fmt_str, "vtx_srf_mode", &len))
		{
			int srf_mode;
			int use_const_fields;

			assert(inst->info->fmt[1] == EVG_FMT_VTX_WORD1_GPR || inst->info->fmt[1] == EVG_FMT_VTX_WORD1_SEM);
			srf_mode = inst->words[1].vtx_word1_gpr.srf_mode_all;
			use_const_fields = inst->words[1].vtx_word1_gpr.use_const_fields;
			if (!use_const_fields && srf_mode)
				str_printf(buf_ptr, size_ptr, "%sSRF_MODE(%s)", evg_inst_token_prefix(loop_idx, &nl),
					str_map_value(&evg_fmt_vtx_srf_mode_map, srf_mode));
		}
		else if (evg_inst_is_token(fmt_str, "vtx_offset", &len))
		{
			int offset;
			
			assert(inst->info->fmt[2] == EVG_FMT_VTX_WORD2);
			offset = inst->words[2].vtx_word2.offset;
			if (offset)
				str_printf(buf_ptr, size_ptr, "OFFSET(%d)", offset);
		}
		else if (evg_inst_is_token(fmt_str, "vtx_endian_swap", &len))
		{
			int endian_swap;
			int use_const_fields;

			assert(inst->info->fmt[1] == EVG_FMT_VTX_WORD1_GPR || inst->info->fmt[1] == EVG_FMT_VTX_WORD1_SEM);
			assert(inst->info->fmt[2] == EVG_FMT_VTX_WORD2);
			use_const_fields = inst->words[1].vtx_word1_gpr.use_const_fields;
			endian_swap = inst->words[2].vtx_word2.endian_swap;
			if (!use_const_fields && endian_swap)
				str_printf(buf_ptr, size_ptr, "%sENDIAN_SWAP(%s)", evg_inst_token_prefix(loop_idx, &nl),
					str_map_value(&evg_fmt_vtx_endian_swap_map, endian_swap));
		}
		else if (evg_inst_is_token(fmt_str, "vtx_cbns", &len))
		{
			int cbns;
			
			assert(inst->info->fmt[2] == EVG_FMT_VTX_WORD2);
			cbns = inst->words[2].vtx_word2.const_buf_no_stride;
			if (cbns)
				str_printf(buf_ptr, size_ptr, "%sCONST_BUF_NO_STRIDE", evg_inst_token_prefix(loop_idx, &nl));

		}
		else if (evg_inst_is_token(fmt_str, "vtx_mega_fetch", &len))
		{
			int mega_fetch_count;
			
			assert(inst->info->fmt[0] == EVG_FMT_VTX_WORD0);
			mega_fetch_count = inst->words[0].vtx_word0.mega_fetch_count;
			str_printf(buf_ptr, size_ptr, "MEGA(%d)", mega_fetch_count + 1);
		}
		else if (evg_inst_is_token(fmt_str, "lds_op", &len))
		{
			assert(inst->info->fmt[1] == EVG_FMT_ALU_WORD1_LDS_IDX_OP);
			str_printf(buf_ptr, size_ptr, "%s", str_map_value(&evg_fmt_lds_op_map,
				inst->words[1].alu_word1_lds_idx_op.lds_op));
		}
		else if (evg_inst_is_token(fmt_str, "nl", &len))
		{
			/* Mark line break before printing next token */
			nl = 1;
		}
		else if (evg_inst_is_token(fmt_str, "dump", &len))
		{
			int i;

			str_printf(buf_ptr, size_ptr, "\n\n");
			for (i = 0; i < EVG_INST_MAX_WORDS; i++)
				if (inst->info->fmt[i])
					evg_inst_word_dump(inst->words[i].bytes, inst->info->fmt[i], stdout);
		}
		else if (evg_inst_is_token(fmt_str, "tex_src_reg", &len))
		{
			assert(inst->info->fmt[0] == EVG_FMT_TEX_WORD0);
			assert(inst->info->fmt[2] == EVG_FMT_TEX_WORD2);

			int src_gpr = inst->words[0].tex_word0.src_gpr;

			int src_sel[4];
			src_sel[0] = inst->words[2].tex_word2.ssx;
			src_sel[1] = inst->words[2].tex_word2.ssy;
			src_sel[2] = inst->words[2].tex_word2.ssz;
			src_sel[3] = inst->words[2].tex_word2.ssw;

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
					fatal("%s: src_sel value %d is unknown", fmt_str, src_sel[i]);
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
					str_printf(buf_ptr, size_ptr, "R%d.%c%c%c%c", src_gpr, src_sel_chars[0], src_sel_chars[1],
							   src_sel_chars[2], src_sel_chars[3]);			
			}
			else
			{
					str_printf(buf_ptr, size_ptr, "R%d", src_gpr);
			}
		}
		else if (evg_inst_is_token(fmt_str, "tex_dst_reg", &len))
		{
			assert(inst->info->fmt[1] == EVG_FMT_TEX_WORD1);

			int dst_gpr = inst->words[1].tex_word1.dst_gpr;

			int dst_sel[4];
			dst_sel[0] = inst->words[1].tex_word1.dsx;
			dst_sel[1] = inst->words[1].tex_word1.dsy;
			dst_sel[2] = inst->words[1].tex_word1.dsz;
			dst_sel[3] = inst->words[1].tex_word1.dsw;

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
				str_printf(buf_ptr, size_ptr, "R%d.%c%c%c%c", dst_gpr, dst_sel_chars[0], dst_sel_chars[1],
						   dst_sel_chars[2], dst_sel_chars[3]);
			}
			else
			{
				str_printf(buf_ptr, size_ptr, "R%d", dst_gpr);
			}
		}
		else if (evg_inst_is_token(fmt_str, "tex_res_id", &len))
		{
			assert(inst->info->fmt[0] == EVG_FMT_TEX_WORD0);

			int resource_id = inst->words[0].tex_word0.resource_id;
			str_printf(buf_ptr, size_ptr, "t%d", resource_id);
		}
		else if (evg_inst_is_token(fmt_str, "tex_sampler_id", &len))
		{
			assert(inst->info->fmt[2] == EVG_FMT_TEX_WORD2);

			int sampler_id = inst->words[2].tex_word2.sampler_id;
			str_printf(buf_ptr, size_ptr, "s%d", sampler_id);

		}
		else if (evg_inst_is_token(fmt_str, "tex_props", &len))
		{
			assert(inst->info->fmt[1] == EVG_FMT_TEX_WORD1);

			if (inst->words[1].tex_word1.ctx || inst->words[1].tex_word1.cty ||
				inst->words[1].tex_word1.ctz || inst->words[1].tex_word1.ctw)
			{
				/* We can't tell if coordinates are normalized or if sampler
				 * is going to be a kernel argument */
			}
			else
			{
				str_printf(buf_ptr, size_ptr, "UNNORM(XYZW)");
			}
		}
		else if (evg_inst_is_token(fmt_str, "mem_op_name", &len))
		{
			assert(inst->info->fmt[0] == EVG_FMT_MEM_RD_WORD0 || inst->info->fmt[0] == EVG_FMT_MEM_GDS_WORD0);

			/* MEM_RD instruction has subopcode in MEM_OP field
			 * main opcode borrows from VTX */
			if (inst->words[0].mem_rd_word0.mem_op == 0)
			{
				str_printf(buf_ptr, size_ptr, "MEM_RD_SCRATCH:");
			}
			else if (inst->words[0].mem_rd_word0.mem_op == 2)
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


void evg_inst_dump_buf(struct evg_inst_t *inst, int count, int loop_idx,
	char *buf, int size)
{
	evg_inst_slot_dump_buf(inst, count, loop_idx,
		-1, buf, size);
}


void evg_inst_dump_gpr(int gpr, int rel, int chan, int im, FILE *f)
{
	char buf[MAX_STRING_SIZE];
	char *sbuf = buf;
	int size = MAX_STRING_SIZE;

	evg_inst_dump_gpr_buf(gpr, rel, chan, im, &sbuf, &size);
	fprintf(f, "%s", buf);
}


void evg_inst_slot_dump(struct evg_inst_t *inst, int count, int loop_idx, int slot, FILE *f)
{
	char buf[MAX_STRING_SIZE];

	evg_inst_slot_dump_buf(inst, count, loop_idx, slot, buf, MAX_STRING_SIZE);
	fprintf(f, "%s\n", buf);
}


void evg_inst_dump(struct evg_inst_t *inst, int count, int loop_idx, FILE *f)
{
	evg_inst_slot_dump(inst, count,
		loop_idx, -1, f);
}


void evg_inst_dump_debug(struct evg_inst_t *inst, int count, int loop_idx, FILE *f)
{
	char buf[MAX_LONG_STRING_SIZE];
	char buf_no_spc[MAX_LONG_STRING_SIZE];

	evg_inst_dump_buf(inst, -1, 0, buf, sizeof buf);
	str_single_spaces(buf_no_spc, sizeof buf_no_spc, buf);
	if (count >= 0)
		fprintf(f, "cnt=%d ", count);
	if (loop_idx >= 0)
		fprintf(f, "l=%d ", loop_idx);
	fprintf(f, "inst=\"%s\"", buf_no_spc);
}


void evg_inst_words_dump(struct evg_inst_t *inst, FILE *f)
{
	int i;
	fprintf(f, "%s\n", inst->info->name);
	for (i = 0; i < EVG_INST_MAX_WORDS; i++)
	{
		if (!inst->info->fmt[i])
			break;
		evg_inst_word_dump(&inst->words[i], inst->info->fmt[i], f);
	}
}


void evg_alu_group_dump(struct evg_alu_group_t *group, int shift, FILE *f)
{
	struct evg_inst_t *inst;
	int i;

	for (i = 0; i < group->inst_count; i++)
	{
		inst = &group->inst[i];
		evg_inst_slot_dump(inst, i ? -1 : group->id, shift, inst->alu, f);
	}
}


void evg_alu_group_dump_buf(struct evg_alu_group_t *alu_group, char *buf, int size)
{
	struct evg_inst_t *inst;

	char str[MAX_LONG_STRING_SIZE];
	char str_trimmed[MAX_LONG_STRING_SIZE];

	char *space;

	int i;

	/* Add individual VLIW bundles */
	space = "";
	for (i = 0; i < alu_group->inst_count; i++)
	{
		/* Get instruction dump */
		inst = &alu_group->inst[i];
		evg_inst_dump_buf(inst, -1, 0, str, sizeof str);
		str_single_spaces(str_trimmed, sizeof str_trimmed, str);

		/* Copy to output buffer */
		str_printf(&buf, &size, "%s%s=\"%s\"", space,
			str_map_value(&evg_alu_map, inst->alu), str_trimmed);
		space = " ";
	}
}


void evg_alu_group_dump_debug(struct evg_alu_group_t *alu_group, int count, int loop_idx, FILE *f)
{
	struct evg_inst_t *inst;

	char buf[MAX_LONG_STRING_SIZE];
	char no_spc_buf[MAX_LONG_STRING_SIZE];

	char *spc;
	int i;

	/* Count and loop index */
	if (count >= 0)
		fprintf(f, "cnt=%d ", count);
	if (loop_idx >= 0)
		fprintf(f, "l=%d ", loop_idx);

	/* VLIW slots */
	spc = "";
	for (i = 0; i < alu_group->inst_count; i++)
	{
		inst = &alu_group->inst[i];
		evg_inst_dump_buf(inst, -1, 0, buf, sizeof buf);
		str_single_spaces(no_spc_buf, sizeof no_spc_buf, buf);
		fprintf(f, "%sinst.%s=\"%s\"", spc, str_map_value(&evg_alu_map, inst->alu), no_spc_buf);
		spc = " ";
	}
}
