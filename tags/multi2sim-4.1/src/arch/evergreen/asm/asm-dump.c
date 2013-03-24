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

#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"


/*
 * Common
 */

static struct str_map_t evg_fmt_rel_map =
{
	2, {
		{ "Absolute", 0 },
		{ "Relative", 1 }
	}
};


static struct str_map_t evg_fmt_chan_map =
{
	4, {
		{ "CHAN_X", 0 },
		{ "CHAN_Y", 1 },
		{ "CHAN_Z", 2 },
		{ "CHAN_W", 3 }
	}
};


static struct str_map_t evg_fmt_sel_map =
{
	7, {
		{ "SEL_X", 0 },
		{ "SEL_Y", 1 },
		{ "SEL_Z", 2 },
		{ "SEL_W", 3 },
		{ "SEL_0", 4 },
		{ "SEL_1", 5 },
		{ "SEL_MASK", 7 }
	}
};


static struct str_map_t evg_fmt_cf_index_mode_map =
{
	4, {
		{ "CF_INDEX_NONE", 0 },
		{ "CF_INDEX_0", 1 },
		{ "CF_INDEX_1", 2 },
		{ "CF_INVALID", 3 }
	}
};


static struct str_map_t evg_fmt_kcache_mode_map =
{
	4, {
		{ "CF_KCACHE_NOP", 0 },
		{ "CF_KCACHE_LOCK_1", 1 },
		{ "CF_KCACHE_LOCK_2", 2 },
		{ "CF_KCACHE_LOCK_LOOP_INDEX", 3 }
	}
};


struct str_map_t evg_inst_category_map =
{
	7, {
		{ "CF", EVG_INST_CAT_CF },
		{ "ALU", EVG_INST_CAT_ALU },
		{ "LDS", EVG_INST_CAT_LDS },
		{ "VTX", EVG_INST_CAT_VTX },
		{ "TEX", EVG_INST_CAT_TEX },
		{ "MEM_RD", EVG_INST_CAT_MEM_RD },
		{ "MEM_GDS", EVG_INST_CAT_MEM_GDS }
	}
};


struct str_map_t evg_fmt_cf_inst_map =
{
	55, {
		{ "CF_INST_NOP", 0 },
		{ "CF_INST_TC", 1 },
		{ "CF_INST_VC", 2 },
		{ "CF_INST_GDS", 3 },
		{ "CF_INST_LOOP_START", 4 },
		{ "CF_INST_LOOP_END", 5 },
		{ "CF_INST_LOOP_START_DX10", 6 },
		{ "CF_INST_LOOP_START_NO_AL", 7 },
		{ "CF_INST_LOOP_CONTINUE", 8 },
		{ "CF_INST_LOOP_BREAK", 9 },
		{ "CF_INST_JUMP", 10 },
		{ "CF_INST_PUSH", 11 },
		{ "CF_INST_ELSE", 13 },
		{ "CF_INST_POP", 14 },
		{ "CF_INST_CALL", 18 },
		{ "CF_INST_CALL_FS", 19 },
		{ "CF_INST_RETURN", 20 },
		{ "CF_INST_EMIT_VERTEX", 21 },
		{ "CF_INST_EMIT_CUT_VERTEX", 22 },
		{ "CF_INST_CUT_VERTEX", 23 },
		{ "CF_INST_KILL", 24 },
		{ "CF_INST_WAIT_ACK", 26 },
		{ "CF_INST_TC_ACK", 27 },
		{ "CF_INST_VC_ACK", 28 },
		{ "CF_INST_JUMPTABLE", 29 },
		{ "CF_INST_GLOBAL_WAVE_SYNC", 30 },
		{ "CF_INST_HALT", 31 },
		{ "CF_INST_MEM_STREAM0_BUF0", 64 },
		{ "CF_INST_MEM_STREAM0_BUF1", 65 },
		{ "CF_INST_MEM_STREAM0_BUF2", 66 },
		{ "CF_INST_MEM_STREAM0_BUF3", 67 },
		{ "CF_INST_MEM_STREAM1_BUF0", 68 },
		{ "CF_INST_MEM_STREAM1_BUF1", 69 },
		{ "CF_INST_MEM_STREAM1_BUF2", 70 },
		{ "CF_INST_MEM_STREAM1_BUF3", 71 },
		{ "CF_INST_MEM_STREAM2_BUF0", 72 },
		{ "CF_INST_MEM_STREAM2_BUF1", 73 },
		{ "CF_INST_MEM_STREAM2_BUF2", 74 },
		{ "CF_INST_MEM_STREAM2_BUF3", 75 },
		{ "CF_INST_MEM_STREAM3_BUF0", 76 },
		{ "CF_INST_MEM_STREAM3_BUF1", 77 },
		{ "CF_INST_MEM_STREAM3_BUF2", 78 },
		{ "CF_INST_MEM_STREAM3_BUF3", 79 },
		{ "CF_INST_MEM_WR_SCRATCH", 80 },
		{ "CF_INST_MEM_RING", 82 },
		{ "CF_INST_EXPORT", 83 },
		{ "CF_INST_EXPORT_DONE", 84 },
		{ "CF_INST_MEM_EXPORT", 85 },
		{ "CF_INST_MEM_RAT", 86 },
		{ "CF_INST_MEM_RAT_CACHELESS", 87 },
		{ "CF_INST_MEM_RING1", 88 },
		{ "CF_INST_MEM_RING2", 89 },
		{ "CF_INST_MEM_RING3", 90 },
		{ "CF_INST_MEM_EXPORT_COMBINED", 91 },
		{ "CF_INST_MEM_RAT_COMBINED_CACHELESS", 92 }
	}
};




/*
 * Invalid/unimplemented Microcode Format
 */

static void evg_fmt_invalid_dump(void *buf, FILE *f)
{
	fprintf(stderr, "dump: unsupported microcode format\n");
}




/*
 * EVG_CF_WORD0
 */

struct str_map_t evg_fmt_cf_word0_jts_map = {
	6, {
		{ "CF_JUMPTABLE_SEL_CONST_A", 0 },
		{ "CF_JUMPTABLE_SEL_CONST_B", 1 },
		{ "CF_JUMPTABLE_SEL_CONST_C", 2 },
		{ "CF_JUMPTABLE_SEL_CONST_D", 3 },
		{ "CF_JUMPTABLE_SEL_INDEX_0", 4 },
		{ "CF_JUMPTABLE_SEL_INDEX_1", 5 }
	}
};


void evg_fmt_cf_word0_dump(void *buf, FILE *f)
{
	struct evg_fmt_cf_word0_t *fmt = (struct evg_fmt_cf_word0_t *) buf;
	fprintf(f, "CF_WORD0\n");
	fprintf(f, "  addr = %d\n", fmt->addr);
	fprintf(f, "  jump_table_sel = %d (%s)\n", fmt->jump_table_sel, str_map_value(&evg_fmt_cf_word0_jts_map, fmt->jump_table_sel));
}




/*
 * EVG_CF_GWS_WORD0
 */

struct str_map_t evg_fmt_cf_gws_word0_val_index_mode_map = {
	4, {
		{ "GWS_INDEX_NONE", 0 },
		{ "GWS_INDEX_0", 1 },
		{ "GWS_INDEX_1", 2 },
		{ "GWS_INDEX_MIX", 3 }
	}
};

struct str_map_t evg_fmt_cf_gws_word0_gws_opcode_map = {
	4, {
		{ "GWS_SEMA_V", 0 },
		{ "GWS_SEMA_P", 1 },
		{ "GWS_BARRIER", 2 },
		{ "GWS_INIT", 3 }
	}
};


void evg_fmt_cf_gws_word0_dump(void *buf, FILE *f)
{
	struct evg_fmt_cf_gws_word0_t *fmt = (struct evg_fmt_cf_gws_word0_t *) buf;
	fprintf(f, "CF_GWS_WORD0\n");
	fprintf(f, "  value = %d\n", fmt->value);
	fprintf(f, "  resource = %d\n", fmt->resource);
	fprintf(f, "  sign = %d\n", fmt->s);
	fprintf(f, "  val_index_mode = %d (%s)\n", fmt->val_index_mode,
		str_map_value(&evg_fmt_cf_gws_word0_val_index_mode_map, fmt->val_index_mode));
	fprintf(f, "  rsrc_index_mode = %d (%s)\n", fmt->rsrc_index_mode,
		str_map_value(&evg_fmt_cf_index_mode_map, fmt->rsrc_index_mode));
	fprintf(f, "  gws_opcode = %d (%s)\n", fmt->gws_opcode, str_map_value(&evg_fmt_cf_gws_word0_gws_opcode_map, fmt->gws_opcode));
}




/*
 * EVG_CF_WORD1
 */

struct str_map_t evg_fmt_cf_word1_cond_map = {
	4, {
		{ "CF_COND_ACTIVE", 0 },
		{ "CF_COND_FALSE", 1 },
		{ "CF_COND_BOOL", 2 },
		{ "CF_COND_NOT_BOOL", 3 }
	}
};


void evg_fmt_cf_word1_dump(void *buf, FILE *f)
{
	struct evg_fmt_cf_word1_t *fmt = (struct evg_fmt_cf_word1_t *) buf;

	fprintf(f, "CF_WORD1\n");
	fprintf(f, "  pop_count = %d\n", fmt->pop_count);
	fprintf(f, "  cf_const = %d\n", fmt->cf_const);
	fprintf(f, "  cond = %d (%s)\n", fmt->cond, str_map_value(&evg_fmt_cf_word1_cond_map, fmt->cond));
	fprintf(f, "  count = %d\n", fmt->count);
	fprintf(f, "  valix_pixel_mode = %d\n", fmt->valid_pixel_mode);
	fprintf(f, "  end_of_program = %d\n", fmt->end_of_program);
	fprintf(f, "  cf_inst = %d (%s)\n", fmt->cf_inst, str_map_value(&evg_fmt_cf_inst_map, fmt->cf_inst));
	fprintf(f, "  whole_quad_mode = %d\n", fmt->whole_quad_mode);
	fprintf(f, "  barrier = %d\n", fmt->barrier);
}




/*
 * EVG_CF_ALU_WORD0
 */


void evg_fmt_cf_alu_word0_dump(void *buf, FILE *f)
{
	struct evg_fmt_cf_alu_word0_t *fmt = (struct evg_fmt_cf_alu_word0_t *) buf;

	fprintf(f, "CF_ALU_WORD0\n");
	fprintf(f, "  addr = %d\n", fmt->addr);
	fprintf(f, "  kcache_bank0 = %d\n", fmt->kcache_bank0);
	fprintf(f, "  kcache_bank1 = %d\n", fmt->kcache_bank1);
	fprintf(f, "  kcache_mode0 = %d (%s)\n", fmt->kcache_mode0, str_map_value(&evg_fmt_kcache_mode_map, fmt->kcache_mode0));
}




/*
 * EVG_CF_ALU_WORD1
 */

struct str_map_t evg_fmt_cf_alu_inst_map = {
	8, {
		{ "CF_INST_ALU", 8 },
		{ "CF_INST_ALU_PUSH_BEFORE", 9 },
		{ "CF_INST_ALU_POP_AFTER", 10 },
		{ "CF_INST_ALU_POP2_AFTER", 11 },
		{ "CF_INST_ALU_EXTENDED", 12 },
		{ "CF_INST_ALU_CONTINUE", 13 },
		{ "CF_INST_ALU_BREAK", 14 },
		{ "CF_INST_ALU_ELSE_AFTER", 15 }
	}
};


void evg_fmt_cf_alu_word1_dump(void *buf, FILE *f)
{
	struct evg_fmt_cf_alu_word1_t *fmt = (struct evg_fmt_cf_alu_word1_t *) buf;

	fprintf(f, "CF_ALU_WORD1\n");
	fprintf(f, "  kcache_mode1 = %d (%s)\n", fmt->kcache_mode1, str_map_value(&evg_fmt_kcache_mode_map, fmt->kcache_mode1));
	fprintf(f, "  kcache_addr0 = 0x%x\n", fmt->kcache_addr0);
	fprintf(f, "  kcache_addr1 = 0x%x\n", fmt->kcache_addr1);
	fprintf(f, "  count=%d\n", fmt->count);
	fprintf(f, "  alt_const=%d\n", fmt->alt_const);
	fprintf(f, "  cf_inst=%d (%s)\n", fmt->cf_inst, str_map_value(&evg_fmt_cf_alu_inst_map, fmt->cf_inst));
	fprintf(f, "  whole_quad_mode=%d\n", fmt->whole_quad_mode);
	fprintf(f, "  barrier=%d\n", fmt->barrier);
}




/*
 * EVG_CF_ALU_WORD0_EXT
 */

void evg_fmt_cf_alu_word0_ext_dump(void *buf, FILE *f)
{
	struct evg_fmt_cf_alu_word0_ext_t *fmt = (struct evg_fmt_cf_alu_word0_ext_t *) buf;

	fprintf(f, "CF_ALU_WORD0_EXT\n");
	fprintf(f, "  kcache_bank_index_mode0 = %d (%s)\n", fmt->kcache_bank_index_mode0,
		str_map_value(&evg_fmt_cf_index_mode_map, fmt->kcache_bank_index_mode0));
	fprintf(f, "  kcache_bank_index_mode1 = %d (%s)\n", fmt->kcache_bank_index_mode1,
		str_map_value(&evg_fmt_cf_index_mode_map, fmt->kcache_bank_index_mode1));
	fprintf(f, "  kcache_bank_index_mode2 = %d (%s)\n", fmt->kcache_bank_index_mode2,
		str_map_value(&evg_fmt_cf_index_mode_map, fmt->kcache_bank_index_mode2));
	fprintf(f, "  kcache_bank_index_mode3 = %d (%s)\n", fmt->kcache_bank_index_mode3,
		str_map_value(&evg_fmt_cf_index_mode_map, fmt->kcache_bank_index_mode3));
	fprintf(f, "  kcache_bank2 = %d\n", fmt->kcache_bank2);
	fprintf(f, "  kcache_bank3 = %d\n", fmt->kcache_bank3);
	fprintf(f, "  kcache_mode2 = %d (%s)\n", fmt->kcache_mode2, str_map_value(&evg_fmt_kcache_mode_map, fmt->kcache_mode2));
}




/*
 * EVG_CF_ALU_WORD1_EXT
 */


void evg_fmt_cf_alu_word1_ext_dump(void *buf, FILE *f)
{
	struct evg_fmt_cf_alu_word1_ext_t *fmt = (struct evg_fmt_cf_alu_word1_ext_t *) buf;

	fprintf(f, "CF_ALU_WORD1_EXT\n");
	fprintf(f, "  kcache_mode3 = %d (%s)\n", fmt->kcache_mode3, str_map_value(&evg_fmt_kcache_mode_map, fmt->kcache_mode3));
	fprintf(f, "  kcache_addr2 = 0x%x\n", fmt->kcache_addr2);
	fprintf(f, "  kcache_addr3 = 0x%x\n", fmt->kcache_addr3);
	fprintf(f, "  cf_inst = %d (%s)\n", fmt->cf_inst, str_map_value(&evg_fmt_cf_alu_inst_map, fmt->cf_inst));
	fprintf(f, "  barrier = %d\n", fmt->barrier);
}




/*
 * EVG_CF_ALLOC_EXPORT_WORD0
 */

struct str_map_t evg_fmt_cf_alloc_export_type_map = {
	4, {
		{ "EXPORT_PIXEL/EXPORT_WRITE", 0 },
		{ "EXPORT_POS/EXPORT_WRITE_IND", 1 },
		{ "EXPORT_PARAM/EXPORT_WRITE_ACK", 2 },
		{ "EXPORT_WRITE_IND_ACK", 3 }
	}
};


void evg_fmt_cf_alloc_export_word0_dump(void *buf, FILE *f)
{
	struct evg_fmt_cf_alloc_export_word0_t *fmt = (struct evg_fmt_cf_alloc_export_word0_t *) buf;

	fprintf(f, "CF_ALLOC_EXPORT_WORD0\n");
	fprintf(f, "  array_base = %d\n", fmt->array_base);
	fprintf(f, "  type = %d (%s)\n", fmt->type, str_map_value(&evg_fmt_cf_alloc_export_type_map, fmt->type));
	fprintf(f, "  rw_gpr = %d\n", fmt->rw_gpr);
	fprintf(f, "  rw_rel = %d (%s)\n", fmt->rr, str_map_value(&evg_fmt_rel_map, fmt->rr));
	fprintf(f, "  index_grp = %d\n", fmt->index_gpr);
	fprintf(f, "  elem_size = %d\n", fmt->elem_size);
}




/*
 * EVG_CF_ALLOC_EXPORT_WORD0_RAT
 */

struct str_map_t evg_fmt_cf_alloc_export_rat_inst_map = {
	39, {
		{ "EXPORT_RAT_INST_NOP", 0 },
		{ "EXPORT_RAT_INST_STORE_TYPED", 1 },
		{ "EXPORT_RAT_INST_STORE_RAW", 2 },
		{ "EXPORT_RAT_INST_STORE_RAW_FDENORM", 3 },
		{ "EXPORT_RAT_INST_CMPXCHG_INT", 4 },
		{ "EXPORT_RAT_INST_CMPXCHG_FLT", 5 },
		{ "EXPORT_RAT_INST_CMPXCHG_FDENORM", 6 },
		{ "EXPORT_RAT_INST_ADD", 7 },
		{ "EXPORT_RAT_INST_SUB", 8 },
		{ "EXPORT_RAT_INST_RSUB", 9 },
		{ "EXPORT_RAT_INST_MIN_INT", 10 },
		{ "EXPORT_RAT_INST_MIN_UINT", 11 },
		{ "EXPORT_RAT_INST_MAX_INT", 12 },
		{ "EXPORT_RAT_INST_MAX_UINT", 13 },
		{ "EXPORT_RAT_INST_AND", 14 },
		{ "EXPORT_RAT_INST_OR", 15 },
		{ "EXPORT_RAT_INST_XOR", 16 },
		{ "EXPORT_RAT_INST_MSKOR", 17 },
		{ "EXPORT_RAT_INST_INC_UINT", 18 },
		{ "EXPORT_RAT_INST_DEC_UINT", 19 },
		{ "EXPORT_RAT_INST_NOP_RTN", 32 },
		{ "EXPORT_RAT_INST_XCHG_RTN", 34 },
		{ "EXPORT_RAT_INST_XCHG_FDENORM_RTN", 35 },
		{ "EXPORT_RAT_INST_CMPXCHG_INT_RTN", 36 },
		{ "EXPORT_RAT_INST_CMPXCHG_FLT_RTN", 37 },
		{ "EXPORT_RAT_INST_CMPXCHG_FDENORM_RTN", 38 },
		{ "EXPORT_RAT_INST_ADD_RTN", 39 },
		{ "EXPORT_RAT_INST_SUB_RTN", 40 },
		{ "EXPORT_RAT_INST_RSUB_RTN", 41 },
		{ "EXPORT_RAT_INST_MIN_INT_RTN", 42 },
		{ "EXPORT_RAT_INST_MIN_UINT_RTN", 43 },
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


void evg_fmt_cf_alloc_export_word0_rat_dump(void *buf, FILE *f)
{
	struct evg_fmt_cf_alloc_export_word0_rat_t *fmt = (struct evg_fmt_cf_alloc_export_word0_rat_t *) buf;

	fprintf(f, "CF_ALLOC_EXPORT_WORD0_RAT\n");
	fprintf(f, "  rat_id = %d\n", fmt->rat_id);
	fprintf(f, "  rat_inst = %d (%s)\n", fmt->rat_inst, str_map_value(&evg_fmt_cf_alloc_export_rat_inst_map, fmt->rat_inst));
	fprintf(f, "  rat_index_mode = %d (%s)\n", fmt->rat_index_mode, str_map_value(&evg_fmt_cf_index_mode_map, fmt->rat_index_mode));
	fprintf(f, "  type = %d (%s)\n", fmt->type, str_map_value(&evg_fmt_cf_alloc_export_type_map, fmt->type));
	fprintf(f, "  rw_gpr = %d\n", fmt->rw_gpr);
	fprintf(f, "  rw_rel = %d (%s)\n", fmt->rr, str_map_value(&evg_fmt_rel_map, fmt->rr));
	fprintf(f, "  index_gpr = %d\n", fmt->index_gpr);
	fprintf(f, "  elem_size = %d\n", fmt->elem_size);
}



/*
 * EVG_CF_ALLOC_EXPORT_WORD1_BUF
 */

void evg_fmt_cf_alloc_export_word1_buf_dump(void *buf, FILE *f)
{
	struct evg_fmt_cf_alloc_export_word1_buf_t *fmt = (struct evg_fmt_cf_alloc_export_word1_buf_t *) buf;

	fprintf(f, "CF_ALLOC_EXPORT_WORD1_BUF\n");
	fprintf(f, "  array_size = %d\n", fmt->array_size);
	fprintf(f, "  comp_mask = 0x%x\n", fmt->comp_mask);
	fprintf(f, "  burst_count = %d\n", fmt->burst_count);
	fprintf(f, "  valid_pixel_mode = %d\n", fmt->valid_pixel_mode);
	fprintf(f, "  end_of_program = %d\n", fmt->end_of_program);
	fprintf(f, "  cf_inst = %d (%s)\n", fmt->cf_inst, str_map_value(&evg_fmt_cf_inst_map, fmt->cf_inst));
	fprintf(f, "  mark = %d\n", fmt->mark);
	fprintf(f, "  barrier = %d\n", fmt->barrier);
}




/*
 * EVG_CF_ALLOC_EXPORT_WORD1_SWIZ 
 */

void evg_fmt_cf_alloc_export_word1_swiz_dump(void *buf, FILE *f)
{
	struct evg_fmt_cf_alloc_export_word1_swiz_t *fmt = (struct evg_fmt_cf_alloc_export_word1_swiz_t *) buf;

	fprintf(f, "CF_ALLOC_EXPORT_WORD1_SWIZ\n");
	fprintf(f, "  sel_x = %d (%s)\n", fmt->sel_x, str_map_value(&evg_fmt_sel_map, fmt->sel_x));
	fprintf(f, "  sel_y = %d (%s)\n", fmt->sel_y, str_map_value(&evg_fmt_sel_map, fmt->sel_y));
	fprintf(f, "  sel_z = %d (%s)\n", fmt->sel_z, str_map_value(&evg_fmt_sel_map, fmt->sel_z));
	fprintf(f, "  sel_w = %d (%s)\n", fmt->sel_w, str_map_value(&evg_fmt_sel_map, fmt->sel_w));
	fprintf(f, "  burst_count = %d\n", fmt->burst_count);
	fprintf(f, "  valid_pixel_mode = %d\n", fmt->valid_pixel_mode);
	fprintf(f, "  end_of_program = %d\n", fmt->end_of_program);
	fprintf(f, "  cf_inst = %d (%s)\n", fmt->cf_inst, str_map_value(&evg_fmt_cf_inst_map, fmt->cf_inst));
	fprintf(f, "  mark = %d\n", fmt->mark);
	fprintf(f, "  barrier = %d\n", fmt->barrier);
}




/*
 * EVG_ALU_WORD0
 */

struct str_map_t evg_fmt_alu_src_sel_map =
{
	34, {
		{ "ALU_SRC_LDS_OQ_A", 219 },
		{ "ALU_SRC_LDS_OQ_B", 220 },
		{ "ALU_SRC_LDS_OQ_A_POP", 221 },
		{ "ALU_SRC_LDS_OQ_B_POP", 222 },
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
		{ "ALU_SRC_PRIM_MASK_HI", 242 },
		{ "ALU_SRC_PRIM_MASK_LO", 243 },
		{ "ALU_SRC_1_DBL_L", 244 },
		{ "ALU_SRC_1_DBL_M", 245 },
		{ "ALU_SRC_0_5_DBL_L", 246 },
		{ "ALU_SRC_0_5_DBL_M", 247 },
		{ "ALU_SRC_0", 248 },
		{ "ALU_SRC_1", 249 },
		{ "ALU_SRC_1_INT", 250 },
		{ "ALU_SRC_M_1_INT", 251 },
		{ "ALU_SRC_0_5", 252 },
		{ "ALU_SRC_LITERAL", 253 },
		{ "ALU_SRC_PV", 254 },
		{ "ALU_SRC_PS", 255 }
	}
};

struct str_map_t evg_fmt_alu_word0_index_mode_map = {
	4, {
		{ "INDEX_AR_X", 0 },
		{ "INDEX_LOOP", 4 },
		{ "INDEX_GLOBAL", 5 },
		{ "INDEX_GLOBAL_AR_X", 6 }
	}
};

struct str_map_t evg_fmt_alu_word0_pred_sel_map = {
	3, {
		{ "PRED_SEL_OFF", 0 },
		{ "PRED_SEL_ZERO", 2 },
		{ "PRED_SEL_ONE", 3 }
	}
};

static void evg_fmt_alu_src_sel_dump_buf(uint32_t src_sel, char **pbuf, int *psize)
{
	if (IN_RANGE(src_sel, 0, 127))
		str_printf(pbuf, psize, "GPR[%d]", src_sel);
	else if (IN_RANGE(src_sel, 128, 159))
		str_printf(pbuf, psize, "Kcache contant %d in bank 0", src_sel - 128);
	else if (IN_RANGE(src_sel, 160, 191))
		str_printf(pbuf, psize, "Kcache constant %d in bank 1", src_sel - 160);
	else if (IN_RANGE(src_sel, 256, 287))
		str_printf(pbuf, psize, "Kcache constant %d in bank 2", src_sel - 256);
	else if (IN_RANGE(src_sel, 288, 319))
		str_printf(pbuf, psize, "Kcache constant %d in bank 3", src_sel - 288);
	else
		str_printf(pbuf, psize, "%s", str_map_value(&evg_fmt_alu_src_sel_map, src_sel));
}


void evg_fmt_alu_word0_dump(void *buf, FILE *f)
{
	struct evg_fmt_alu_word0_t *fmt = (struct evg_fmt_alu_word0_t *) buf;
	char src0_sel_str[MAX_STRING_SIZE];
	char src1_sel_str[MAX_STRING_SIZE];
	char *str;
	int size;

	str = src0_sel_str;
	size = MAX_STRING_SIZE;
	evg_fmt_alu_src_sel_dump_buf(fmt->src0_sel, &str, &size);

	str = src1_sel_str;
	size = MAX_STRING_SIZE;
	evg_fmt_alu_src_sel_dump_buf(fmt->src1_sel, &str, &size);

	fprintf(f, "ALU_WORD0\n");
	fprintf(f, "  src0_sel = %d (%s)\n", fmt->src0_sel, src0_sel_str);
	fprintf(f, "  src1_sel = %d (%s)\n", fmt->src1_sel, src1_sel_str);
	fprintf(f, "  src0_rel = %d (%s)\n", fmt->src0_rel, str_map_value(&evg_fmt_rel_map, fmt->src0_rel));
	fprintf(f, "  src1_rel = %d (%s)\n", fmt->src1_rel, str_map_value(&evg_fmt_rel_map, fmt->src1_rel));
	fprintf(f, "  src0_chan = %d (%s)\n", fmt->src0_chan, str_map_value(&evg_fmt_chan_map, fmt->src0_chan));
	fprintf(f, "  src1_chan = %d (%s)\n", fmt->src1_chan, str_map_value(&evg_fmt_chan_map, fmt->src1_chan));
	fprintf(f, "  src0_neg = %d\n", fmt->src0_neg);
	fprintf(f, "  src1_neg = %d\n", fmt->src1_neg);
	fprintf(f, "  index_mode = %d (%s)\n", fmt->index_mode, str_map_value(&evg_fmt_alu_word0_index_mode_map, fmt->index_mode));
	fprintf(f, "  pred_sel = %d (%s)\n", fmt->pred_sel, str_map_value(&evg_fmt_alu_word0_pred_sel_map, fmt->pred_sel));
	fprintf(f, "  last = %d\n", fmt->last);
}




/*
 * EVG_ALU_WORD1_OP2
 */

struct str_map_t evg_fmt_alu_word1_op2_omod_map = {
	4, {
		{ "ALU_OMOD_OFF", 0 },
		{ "ALU_OMOD_M2", 1 },
		{ "ALU_OMOD_M4", 2 },
		{ "ALU_OMOD_D2", 3 }
	}
};

struct str_map_t evg_fmt_alu_word1_op2_alu_inst_map = {
	176, {
		{ "OP2_INST_ADD", 0 },
		{ "OP2_INST_MUL", 1 },
		{ "OP2_INST_MUL_IEEE", 2 },
		{ "OP2_INST_MAX", 3 },
		{ "OP2_INST_MIN", 4 },
		{ "OP2_INST_MAX_DX10", 5 },
		{ "OP2_INST_MIN_DX10", 6 },
		{ "OP2_INST_SETE", 8 },
		{ "OP2_INST_SETGT", 9 },
		{ "OP2_INST_SETGE", 10 },
		{ "OP2_INST_SETNE", 11 },
		{ "OP2_INST_SETE_DX10", 12 },
		{ "OP2_INST_SETGT_DX10", 13 },
		{ "OP2_INST_SETGE_DX10", 14 },
		{ "OP2_INST_SETNE_DX10", 15 },
		{ "OP2_INST_FRACT", 16 },
		{ "OP2_INST_TRUNC", 17 },
		{ "OP2_INST_CEIL", 18 },
		{ "OP2_INST_RNDNE", 19 },
		{ "OP2_INST_FLOOR", 20 },
		{ "OP2_INST_ASHR_INT", 21 },
		{ "OP2_INST_LSHR_INT", 22 },
		{ "OP2_INST_LSHL_INT", 23 },
		{ "OP2_INST_MOV", 25 },
		{ "OP2_INST_NOP", 26 },
		{ "OP2_INST_MUL_64", 27 },
		{ "OP2_INST_FLT64_TO_FLT32", 28 },
		{ "OP2_INST_FLT32_TO_FLT64", 29 },
		{ "OP2_INST_PRED_SETGT_UINT", 30 },
		{ "OP2_INST_PRED_SETGE_UINT", 31 },
		{ "OP2_INST_PRED_SETE", 32 },
		{ "OP2_INST_PRED_SETGT", 33 },
		{ "OP2_INST_PRED_SETGE", 34 },
		{ "OP2_INST_PRED_SETNE", 35 },
		{ "OP2_INST_PRED_SET_INV", 36 },
		{ "OP2_INST_PRED_SET_POP", 37 },
		{ "OP2_INST_PRED_SET_CLR", 38 },
		{ "OP2_INST_PRED_SET_RESTORE", 39 },
		{ "OP2_INST_PRED_SETE_PUSH", 40 },
		{ "OP2_INST_PRED_SETGT_PUSH", 41 },
		{ "OP2_INST_PRED_SETGE_PUSH", 42 },
		{ "OP2_INST_PRED_SETNE_PUSH", 43 },
		{ "OP2_INST_KILLE", 44 },
		{ "OP2_INST_KILLGT", 45 },
		{ "OP2_INST_KILLGE", 46 },
		{ "OP2_INST_KILLNE", 47 },
		{ "OP2_INST_AND_INT", 48 },
		{ "OP2_INST_OR_INT", 49 },
		{ "OP2_INST_XOR_INT", 50 },
		{ "OP2_INST_NOT_INT", 51 },
		{ "OP2_INST_ADD_INT", 52 },
		{ "OP2_INST_SUB_INT", 53 },
		{ "OP2_INST_MAX_INT", 54 },
		{ "OP2_INST_MIN_INT", 55 },
		{ "OP2_INST_MAX_UINT", 56 },
		{ "OP2_INST_MIN_UINT", 57 },
		{ "OP2_INST_SETE_INT", 58 },
		{ "OP2_INST_SETGT_INT", 59 },
		{ "OP2_INST_SETGE_INT", 60 },
		{ "OP2_INST_SETNE_INT", 61 },
		{ "OP2_INST_SETGT_UINT", 62 },
		{ "OP2_INST_SETGE_UINT", 63 },
		{ "OP2_INST_KILLGT_UINT", 64 },
		{ "OP2_INST_KILLGE_UINT", 65 },
		{ "OP2_INST_PREDE_INT", 66 },
		{ "OP2_INST_PRED_SETGT_INT", 67 },
		{ "OP2_INST_PRED_SETGE_INT", 68 },
		{ "OP2_INST_PRED_SETNE_INT", 69 },
		{ "OP2_INST_KILLE_INT", 70 },
		{ "OP2_INST_KILLGT_INT", 71 },
		{ "OP2_INST_KILLGE_INT", 72 },
		{ "OP2_INST_KILLNE_INT", 73 },
		{ "OP2_INST_PRED_SETE_PUSH_INT", 74 },
		{ "OP2_INST_PRED_SETGT_PUSH_INT", 75 },
		{ "OP2_INST_PRED_SETGE_PUSH_INT", 76 },
		{ "OP2_INST_PRED_SETNE_PUSH_INT", 77 },
		{ "OP2_INST_PRED_SETLT_PUSH_INT", 78 },
		{ "OP2_INST_PRED_SETLE_PUSH_INT", 79 },
		{ "OP2_INST_FLT_TO_INT", 80 },
		{ "OP2_INST_BFREV_INT", 81 },
		{ "OP2_INST_ADDC_UINT", 82 },
		{ "OP2_INST_SUBB_UINT", 83 },
		{ "OP2_INST_GROUP_BARRIER", 84 },
		{ "OP2_INST_GROUP_SEQ_BEGIN", 85 },
		{ "OP2_INST_GROUP_SEQ_END", 86 },
		{ "OP2_INST_SET_MODE", 87 },
		{ "OP2_INST_SET_CF_IDX0", 88 },
		{ "OP2_INST_SET_CF_IDX1", 89 },
		{ "OP2_INST_SET_LDS_SIZE", 90 },
		{ "OP2_INST_EXP_IEEE", 129 },
		{ "OP2_INST_LOG_CLAMPED", 130 },
		{ "OP2_INST_LOG_IEEE", 131 },
		{ "OP2_INST_RECIP_CLAMPED", 132 },
		{ "OP2_INST_RECIP_FF", 133 },
		{ "OP2_INST_RECIP_IEEE", 134 },
		{ "OP2_INST_RECIPSQRT_CLAMPED", 135 },
		{ "OP2_INST_RECIPSQRT_FF", 136 },
		{ "OP2_INST_RECIPSQRT_IEEE", 137 },
		{ "OP2_INST_SQRT_IEEE", 138 },
		{ "OP2_INST_SIN", 141 },
		{ "OP2_INST_COS", 142 },
		{ "OP2_INST_MULLO_INT", 143 },
		{ "OP2_INST_MULHI_INT", 144 },
		{ "OP2_INST_MULLO_UINT", 145 },
		{ "OP2_INST_MULHI_UINT", 146 },
		{ "OP2_INST_RECIP_INT", 147 },
		{ "OP2_INST_RECIP_UINT", 148 },
		{ "OP2_INST_RECIP_64", 149 },
		{ "OP2_INST_RECIP_CLAMPED_64", 150 },
		{ "OP2_INST_RECIPSQRT_64", 151 },
		{ "OP2_INST_RECIPSQRT_CLAMPED_64", 152 },
		{ "OP2_INST_SQRT_64", 153 },
		{ "OP2_INST_FLT_TO_UINT", 154 },
		{ "OP2_INST_INT_TO_FLT", 155 },
		{ "OP2_INST_UINT_TO_FLT", 156 },
		{ "OP2_INST_BFM_INT", 160 },
		{ "OP2_INST_FLT32_TO_FLT16", 162 },
		{ "OP2_INST_FLT16_TO_FLT32", 163 },
		{ "OP2_INST_UBYTE0_FLT", 164 },
		{ "OP2_INST_UBYTE1_FLT", 165 },
		{ "OP2_INST_UBYTE2_FLT", 166 },
		{ "OP2_INST_UBYTE3_FLT", 167 },
		{ "OP2_INST_BCNT_INT", 170 },
		{ "OP2_INST_FFBH_UINT", 171 },
		{ "OP2_INST_FFBL_INT", 172 },
		{ "OP2_INST_FFBH_INT", 173 },
		{ "OP2_INST_FLT_TO_UINT4", 174 },
		{ "OP2_INST_DOT_IEEE", 175 },
		{ "OP2_INST_FLT_TO_INT_RPI", 176 },
		{ "OP2_INST_FLT_TO_INT_FLOOR", 177 },
		{ "OP2_INST_MULHI_UINT24", 178 },
		{ "OP2_INST_MBCNT_32HI_INT", 179 },
		{ "OP2_INST_OFFSET_TO_FLT", 180 },
		{ "OP2_INST_MUL_UINT24", 181 },
		{ "OP2_INST_BCNT_ACCUM_PREV_INT", 182 },
		{ "OP2_INST_MBCNT_32LO_ACCUM_PREV_INT", 183 },
		{ "OP2_INST_SETE_64", 184 },
		{ "OP2_INST_SETNE_64", 185 },
		{ "OP2_INST_SETGT_64", 186 },
		{ "OP2_INST_SETGE_64", 187 },
		{ "OP2_INST_MIN_64", 188 },
		{ "OP2_INST_MAX_64", 189 },
		{ "OP2_INST_DOT4", 190 },
		{ "OP2_INST_DOT4_IEEE", 191 },
		{ "OP2_INST_CUBE", 192 },
		{ "OP2_INST_MAX4", 193 },
		{ "OP2_INST_FREXP_64", 196 },
		{ "OP2_INST_LDEXP_64", 197 },
		{ "OP2_INST_FRACT_64", 198 },
		{ "OP2_INST_PRED_SETGT_64", 199 },
		{ "OP2_INST_PRED_SETE_64", 200 },
		{ "OP2_INST_PRED_SETGE_64", 201 },
		{ "OP2_INST_MUL_64_VEC", 202 },
		{ "OP2_INST_ADD_64", 203 },
		{ "OP2_INST_MOVA_INT", 204 },
		{ "OP2_INST_FLT64_TO_FLT32_VEC", 205 },
		{ "OP2_INST_FLT32_TO_FLT64_VEC", 206 },
		{ "OP2_INST_SAD_ACCUM_PREV_UINT", 207 },
		{ "OP2_INST_DOT", 208 },
		{ "OP2_INST_MUL_PREV", 209 },
		{ "OP2_INST_MUL_IEEE_PREV", 210 },
		{ "OP2_INST_ADD_PREV", 211 },
		{ "OP2_INST_MULADD_PREV", 212 },
		{ "OP2_INST_MULADD_IEEE_PREV", 213 },
		{ "OP2_INST_INTERP_XY", 214 },
		{ "OP2_INST_INTERP_ZW", 215 },
		{ "OP2_INST_INTERP_X", 216 },
		{ "OP2_INST_INTERP_Z", 217 },
		{ "OP2_INST_STORE_FLAGS", 218 },
		{ "OP2_INST_LOAD_STORE_FLAGS", 219 },
		{ "OP2_INST_LDS_1A", 220 },
		{ "OP2_INST_LDS_1A1D", 221 },
		{ "OP2_INST_LDS_2A", 223 },
		{ "OP2_INST_INTERP_LOAD_P0", 224 },
		{ "OP2_INST_INTERP_LOAD_P10", 225 },
		{ "OP2_INST_INTERP_LOAD_P20", 226 }
	}
};

struct str_map_t evg_fmt_alu_bank_swizzle_map = {
	6, {
		{ "ALU_VEC_012, SQ_ALU_SCL_210", 0 },
		{ "ALU_VEC_021, SQ_ALU_SCL_122", 1 },
		{ "ALU_VEC_120, SQ_ALU_SCL_212", 2 },
		{ "ALU_VEC_102, SQ_ALU_SCL_221", 3 },
		{ "ALU_VEC_201", 4 },
		{ "ALU_VEC_210", 5 }
	}
};


void evg_fmt_alu_word1_op2_dump(void *buf, FILE *f)
{
	struct evg_fmt_alu_word1_op2_t *fmt = (struct evg_fmt_alu_word1_op2_t *) buf;

	fprintf(f, "ALU_WORD1_OP2\n");
	fprintf(f, "  src0_abs = %d\n", fmt->src0_abs);
	fprintf(f, "  src1_abs = %d\n", fmt->src1_abs);
	fprintf(f, "  update_exec_mask = %d\n", fmt->update_exec_mask);
	fprintf(f, "  update_pred = %d\n", fmt->update_pred);
	fprintf(f, "  write_mask = %d\n", fmt->write_mask);
	fprintf(f, "  omod = %d (%s)\n", fmt->omod, str_map_value(&evg_fmt_alu_word1_op2_omod_map, fmt->omod));
	fprintf(f, "  alu_inst = %d (%s)\n", fmt->alu_inst, str_map_value(&evg_fmt_alu_word1_op2_alu_inst_map, fmt->alu_inst));
	fprintf(f, "  bank_swizzle = %d (%s)\n", fmt->bank_swizzle, str_map_value(&evg_fmt_alu_bank_swizzle_map, fmt->bank_swizzle));
	fprintf(f, "  dst_gpr = %d\n", fmt->dst_gpr);
	fprintf(f, "  dst_rel = %d (%s)\n", fmt->dst_rel, str_map_value(&evg_fmt_rel_map, fmt->dst_rel));
	fprintf(f, "  dst_chan = %d (%s)\n", fmt->dst_chan, str_map_value(&evg_fmt_chan_map, fmt->dst_chan));
	fprintf(f, "  clamp = %d\n", fmt->clamp);
}




/*
 * EVG_ALU_WORD1_OP3
 */

struct str_map_t evg_fmt_alu_word1_op3_alu_inst_map = {
	25, {
		{ "OP3_INST_BFE_UINT", 4 },
		{ "OP3_INST_BFE_INT", 5 },
		{ "OP3_INST_BFI_INT", 6 },
		{ "OP3_INST_FMA", 7 },
		{ "OP3_INST_CNDNE_64", 9 },
		{ "OP3_INST_FMA_64", 10 },
		{ "OP3_INST_LERP_UINT", 11 },
		{ "OP3_INST_BIT_ALIGN_INT", 12 },
		{ "OP3_INST_BYTE_ALIGN_INT", 13 },
		{ "OP3_INST_SAD_ACCUM_UINT", 14 },
		{ "OP3_INST_SAD_ACCUM_HI_UINT", 15 },
		{ "OP3_INST_MULADD_UINT24", 16 },
		{ "OP3_INST_LDS_IDX_OP", 17 },
		{ "OP3_INST_MULADD", 20 },
		{ "OP3_INST_MULADD_M2", 21 },
		{ "OP3_INST_MULADD_M4", 22 },
		{ "OP3_INST_MULADD_D2", 23 },
		{ "OP3_INST_MULADD_IEEE", 24 },
		{ "OP3_INST_CNDE", 25 },
		{ "OP3_INST_CNDGT", 26 },
		{ "OP3_INST_CNDGE", 27 },
		{ "OP3_INST_CNDE_INT", 28 },
		{ "OP3_INST_CMNDGT_INT", 29 },
		{ "OP3_INST_CMNDGE_INT", 30 },
		{ "OP3_INST_MUL_LIT", 31 }
	}
};


void evg_fmt_alu_word1_op3_dump(void *buf, FILE *f)
{
	struct evg_fmt_alu_word1_op3_t *fmt = (struct evg_fmt_alu_word1_op3_t *) buf;
	char src2_sel_str[MAX_STRING_SIZE];
	char *str;
	int size;

	str = src2_sel_str;
	size = MAX_STRING_SIZE;
	evg_fmt_alu_src_sel_dump_buf(fmt->src2_sel, &str, &size);

	fprintf(f, "ALU_WORD1_OP3\n");
	fprintf(f, "  src2_sel = %d (%s)\n", fmt->src2_sel, src2_sel_str);
	fprintf(f, "  src2_rel = %d (%s)\n", fmt->src2_rel, str_map_value(&evg_fmt_rel_map, fmt->src2_rel));
	fprintf(f, "  src2_chan = %d (%s)\n", fmt->src2_chan, str_map_value(&evg_fmt_chan_map, fmt->src2_chan));
	fprintf(f, "  src2_neg = %d\n", fmt->src2_neg);
	fprintf(f, "  alu_inst = %d (%s)\n", fmt->alu_inst, str_map_value(&evg_fmt_alu_word1_op3_alu_inst_map, fmt->alu_inst));
	fprintf(f, "  bank_swizzle = %d (%s)\n", fmt->bank_swizzle, str_map_value(&evg_fmt_alu_bank_swizzle_map, fmt->bank_swizzle));
	fprintf(f, "  dst_gpr = %d\n", fmt->dst_gpr);
	fprintf(f, "  dst_rel = %d (%s)\n", fmt->dst_rel, str_map_value(&evg_fmt_rel_map, fmt->dst_rel));
	fprintf(f, "  dst_chan = %d (%s)\n", fmt->dst_chan, str_map_value(&evg_fmt_chan_map, fmt->dst_chan));
	fprintf(f, "  clamp = %d\n", fmt->clamp);
}



/*
 * EVG_ALU_WORD0_LDS_IDX_OP
 */

void evg_fmt_alu_word0_lds_idx_op_dump(void *buf, FILE *f)
{
	struct evg_fmt_alu_word0_lds_idx_op_t *fmt = (struct evg_fmt_alu_word0_lds_idx_op_t *) buf;
	char src0_sel_str[MAX_STRING_SIZE];
	char src1_sel_str[MAX_STRING_SIZE];
	char *str;
	int size;

	str = src0_sel_str;
	size = MAX_STRING_SIZE;
	evg_fmt_alu_src_sel_dump_buf(fmt->src0_sel, &str, &size);

	str = src1_sel_str;
	size = MAX_STRING_SIZE;
	evg_fmt_alu_src_sel_dump_buf(fmt->src1_sel, &str, &size);

	fprintf(f, "ALU_WORD0_LDS_IDX_OP\n");
	fprintf(f, "  src0_sel = %d (%s)\n", fmt->src0_sel, src0_sel_str);
	fprintf(f, "  src0_rel = %d (%s)\n", fmt->src0_rel, str_map_value(&evg_fmt_rel_map, fmt->src0_rel));
	fprintf(f, "  src0_chan = %d (%s)\n", fmt->src0_chan, str_map_value(&evg_fmt_chan_map, fmt->src0_chan));
	fprintf(f, "  idx_offset_4 = %d\n", fmt->idx_offset_4);
	fprintf(f, "  src1_sel = %d (%s)\n", fmt->src1_sel, src1_sel_str);
	fprintf(f, "  src1_rel = %d (%s)\n", fmt->src1_rel, str_map_value(&evg_fmt_rel_map, fmt->src1_rel));
	fprintf(f, "  src1_chan = %d (%s)\n", fmt->src1_chan, str_map_value(&evg_fmt_chan_map, fmt->src1_chan));
	fprintf(f, "  idx_offset_5 = %d\n", fmt->idx_offset_5);
	fprintf(f, "  index_mode = %d (%s)\n", fmt->index_mode, str_map_value(&evg_fmt_alu_word0_index_mode_map, fmt->index_mode));
	fprintf(f, "  pred_sel = %d (%s)\n", fmt->pred_sel, str_map_value(&evg_fmt_alu_word0_pred_sel_map, fmt->pred_sel));
	fprintf(f, "  last = %d\n", fmt->last);
}




/*
 * EVG_ALU_WORD1_LDS_IDX_OP
 */

struct str_map_t evg_fmt_lds_op_map = {
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

void evg_fmt_alu_word1_lds_idx_op_dump(void *buf, FILE *f)
{
	struct evg_fmt_alu_word1_lds_idx_op_t *fmt = (struct evg_fmt_alu_word1_lds_idx_op_t *) buf;
	char src2_sel_str[MAX_STRING_SIZE];
	char *str;
	int size;

	str = src2_sel_str;
	size = MAX_STRING_SIZE;
	evg_fmt_alu_src_sel_dump_buf(fmt->src2_sel, &str, &size);
	
	fprintf(f, "ALU_WORD1_LDS_IDX_OP\n");
	fprintf(f, "  src2_sel = %d (%s)\n", fmt->src2_sel, src2_sel_str);
	fprintf(f, "  src2_rel = %d (%s)\n", fmt->src2_rel, str_map_value(&evg_fmt_rel_map, fmt->src2_rel));
	fprintf(f, "  src2_chan = %d (%s)\n", fmt->src2_chan, str_map_value(&evg_fmt_chan_map, fmt->src2_chan));
	fprintf(f, "  idx_offset_1 = %d\n", fmt->idx_offset_1);
	fprintf(f, "  alu_inst = %d (%s)\n", fmt->alu_inst, str_map_value(&evg_fmt_alu_word1_op3_alu_inst_map, fmt->alu_inst));
	fprintf(f, "  bank_swizzle = %d (%s)\n", fmt->bank_swizzle, str_map_value(&evg_fmt_alu_bank_swizzle_map, fmt->bank_swizzle));
	fprintf(f, "  lds_op = %d (DS_INST_%s)\n", fmt->lds_op, str_map_value(&evg_fmt_lds_op_map, fmt->lds_op));
	fprintf(f, "  idx_offset_0 = %d\n", fmt->idx_offset_0);
	fprintf(f, "  idx_offset_2 = %d\n", fmt->idx_offset_2);
	fprintf(f, "  dst_chan = %d (%s)\n", fmt->dst_chan, str_map_value(&evg_fmt_chan_map, fmt->dst_chan));
	fprintf(f, "  index_offset_3 = %d\n", fmt->idx_offset_3);
}



/*
 * EVG_VTX_WORD0
 */

struct str_map_t evg_fmt_vc_inst_map = {
	3, {
		{ "VC_INST_FETCH", 0 },
		{ "VC_INST_SEMANTIC", 1 },
		{ "VC_INST_GET_BUFFER_RESINFO", 14 }
	}
};


struct str_map_t evg_fmt_vtx_fetch_type_map = {  /* VTX_FETCH prefix omitted */
	3, {
		{ "VERTEX_DATA", 0 },
		{ "INSTANCE_DATA", 1 },
		{ "NO_INDEX_OFFSET", 2 }
	}
};


struct str_map_t evg_fmt_vtx_src_sel_map = {
	3, {
		{ "SEL_X", 0 },
		{ "SEL_Y", 1 },
		{ "SEL_Z", 2 },
		{ "SEL_W", 3 }
	}
};


void evg_fmt_vtx_word0_dump(void *buf, FILE *f)
{
	struct evg_fmt_vtx_word0_t *fmt = (struct evg_fmt_vtx_word0_t *) buf;

	fprintf(f, "VTX_WORD0\n");
	fprintf(f, "  vc_inst = %d (%s)\n", fmt->vc_inst, str_map_value(&evg_fmt_vc_inst_map, fmt->vc_inst));
	fprintf(f, "  fetch_type = %d (%s)\n", fmt->fetch_type, str_map_value(&evg_fmt_vtx_fetch_type_map, fmt->fetch_type));
	fprintf(f, "  fetch_whole_quad = %d\n", fmt->fetch_whole_quad);
	fprintf(f, "  buffer_id = %d\n", fmt->buffer_id);
	fprintf(f, "  src_gpr = %d\n", fmt->src_gpr);
	fprintf(f, "  src_rel = %d\n", fmt->src_rel);
	fprintf(f, "  src_sel_x = %d (%s)\n", fmt->src_sel_x, str_map_value(&evg_fmt_vtx_src_sel_map, fmt->src_sel_x));
	fprintf(f, "  mega_fetch_count = %d\n", fmt->mega_fetch_count);
}




/*
 * EVG_VTX_WORD1_GPR
 */

struct str_map_t evg_fmt_vtx_data_format_map = {
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


struct str_map_t evg_fmt_vtx_num_format_map = {  /* Prefix NUM_FORMAT omitted */
	3, {
		{ "NORM", 0 },
		{ "INT", 1 },
		{ "SCALED", 2 }
	}
};


struct str_map_t evg_fmt_vtx_format_comp_map = {  /* Prefix FORMAT_COMP omitted */
	2, {
		{ "UNSIGNED", 0 },
		{ "SIGNED", 1 }
	}
};


struct str_map_t evg_fmt_vtx_srf_mode_map = {  /* Prefix SRF_MODE omitted */
	2, {
		{ "ZERO_CLAMP_MINUS_ONE", 0 },
		{ "NO_ZERO", 1 }
	}
};


void evg_fmt_vtx_word1_gpr_dump(void *buf, FILE *f)
{
	struct evg_fmt_vtx_word1_gpr_t *fmt = (struct evg_fmt_vtx_word1_gpr_t *) buf;

	fprintf(f, "VTX_WORD1_GPR\n");
	fprintf(f, "  dst_gpr = %d\n", fmt->dst_gpr);
	fprintf(f, "  dst_rel = %d\n", fmt->dst_rel);
	fprintf(f, "  dst_sel_x = %d (%s)\n", fmt->dst_sel_x, str_map_value(&evg_fmt_sel_map, fmt->dst_sel_x));
	fprintf(f, "  dst_sel_y = %d (%s)\n", fmt->dst_sel_y, str_map_value(&evg_fmt_sel_map, fmt->dst_sel_y));
	fprintf(f, "  dst_sel_z = %d (%s)\n", fmt->dst_sel_z, str_map_value(&evg_fmt_sel_map, fmt->dst_sel_z));
	fprintf(f, "  dst_sel_w = %d (%s)\n", fmt->dst_sel_w, str_map_value(&evg_fmt_sel_map, fmt->dst_sel_w));
	fprintf(f, "  use_const_fields = %d\n", fmt->use_const_fields);
	fprintf(f, "  data_format = %d (%s)\n", fmt->data_format, str_map_value(&evg_fmt_vtx_data_format_map, fmt->data_format));
	fprintf(f, "  num_format_all = %d (%s)\n", fmt->num_format_all, str_map_value(&evg_fmt_vtx_num_format_map, fmt->num_format_all));
	fprintf(f, "  format_comp_all = %d (%s)\n", fmt->format_comp_all, str_map_value(&evg_fmt_vtx_format_comp_map, fmt->format_comp_all));
	fprintf(f, "  srf_mode_all = %d (%s)\n", fmt->srf_mode_all, str_map_value(&evg_fmt_vtx_srf_mode_map, fmt->srf_mode_all));
}




/*
 * EVG_VTX_WORD2
 */

struct str_map_t evg_fmt_vtx_endian_swap_map = {  /* Prefix ENDIAN omitted */
	3, {
		{ "NONE", 0 },
		{ "8IN16", 1 },
		{ "8IN32", 2 }
	}
};


void evg_fmt_vtx_word2_dump(void *buf, FILE *f)
{
	struct evg_fmt_vtx_word2_t *fmt = (struct evg_fmt_vtx_word2_t *) buf;

	fprintf(f, "VTX_WORD2\n");
	fprintf(f, "  offset = 0x%x\n", fmt->offset);
	fprintf(f, "  endian_swap = %d (%s)\n", fmt->endian_swap, str_map_value(&evg_fmt_vtx_endian_swap_map, fmt->endian_swap));
	fprintf(f, "  const_buf_no_stride = %d\n", fmt->const_buf_no_stride);
	fprintf(f, "  mega_fetch = %d\n", fmt->mega_fetch);
	/* FIXME: error in format specification */
	/* fprintf(f, "  alt_const = %d\n", fmt->alt_const);
	fprintf(f, "  buffer_index_mode = %d\n", fmt->bim); */
}




/*
 * Common
 */

/* List of 'dump' functions for each microcode format */
evg_fmt_dump_func_t evg_fmt_dump_func_list[EVG_FMT_COUNT] = {
	
	evg_fmt_invalid_dump,  /* FMT_INVALID */

	evg_fmt_cf_word0_dump,
	evg_fmt_cf_gws_word0_dump,
	evg_fmt_cf_word1_dump,

	evg_fmt_cf_alu_word0_dump,
	evg_fmt_cf_alu_word1_dump,

	evg_fmt_cf_alu_word0_ext_dump,
	evg_fmt_cf_alu_word1_ext_dump,

	evg_fmt_cf_alloc_export_word0_dump,
	evg_fmt_cf_alloc_export_word0_rat_dump,
	evg_fmt_cf_alloc_export_word1_buf_dump,
	evg_fmt_cf_alloc_export_word1_swiz_dump,

	evg_fmt_alu_word0_dump,
	evg_fmt_alu_word1_op2_dump,
	evg_fmt_alu_word1_op3_dump,

	evg_fmt_alu_word0_lds_idx_op_dump,
	evg_fmt_alu_word1_lds_idx_op_dump,
	evg_fmt_invalid_dump,  /* EVG_FMT_ALU_WORD1_LDS_DIRECT_LITERAL_LO */
	evg_fmt_invalid_dump,  /* EVG_FMT_ALU_WORD1_LDS_DIRECT_LITERAL_HI */
	
	evg_fmt_vtx_word0_dump,
	evg_fmt_vtx_word1_gpr_dump,
	evg_fmt_invalid_dump,  /* EVG_FMT_VTX_WORD1_SEM */
	evg_fmt_vtx_word2_dump,

	evg_fmt_invalid_dump,  /* EVG_FMT_TEX_WORD0 */
	evg_fmt_invalid_dump,  /* EVG_FMT_TEX_WORD1 */
	evg_fmt_invalid_dump,  /* EVG_FMT_TEX_WORD2 */

	evg_fmt_invalid_dump,  /* EVG_FMT_MEM_RD_WORD0 */
	evg_fmt_invalid_dump,  /* EVG_FMT_MEM_RD_WORD1 */
	evg_fmt_invalid_dump,  /* EVG_FMT_MEM_RD_WORD2 */

	evg_fmt_invalid_dump,  /* EVG_FMT_MEM_GDS_WORD0 */
	evg_fmt_invalid_dump,  /* EVG_FMT_MEM_GDS_WORD1 */
	evg_fmt_invalid_dump,  /* EVG_FMT_MEM_GDS_WORD2 */
};


void evg_inst_word_dump(void *buf, enum evg_fmt_enum fmt, FILE *f)
{
	evg_fmt_dump_func_t dump_func;
	int i;

	/* Hexadecimal dump */
	fprintf(f, "Hex dump: ");
	for (i = 0; i < 4; i++)
		fprintf(f, "%02x ", ((unsigned char *) buf)[i]);
	fprintf(f, "\n");
	
	dump_func = evg_fmt_dump_func_list[fmt];
	dump_func(buf, f);
}
