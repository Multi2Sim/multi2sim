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

#ifndef GPUDISASM_H
#define GPUDISASM_H

#include <stdint.h>
#include <stdio.h>


/* Microcode Formats */
enum fmt_enum {

	FMT_NONE = 0,

	/* Control flow instructions */
	FMT_CF_WORD0,
	FMT_CF_GWS_WORD0,
	FMT_CF_WORD1,

	FMT_CF_ALU_WORD0,
	FMT_CF_ALU_WORD1,
	
	FMT_CF_ALU_WORD0_EXT,
	FMT_CF_ALU_WORD1_EXT,

	FMT_CF_ALLOC_EXPORT_WORD0,
	FMT_CF_ALLOC_EXPORT_WORD0_RAT,
	FMT_CF_ALLOC_EXPORT_WORD1_BUF,
	FMT_CF_ALLOC_EXPORT_WORD1_SWIZ,

	/* ALU Clause Instructions */
	FMT_ALU_WORD0,
	FMT_ALU_WORD1_OP2,
	FMT_ALU_WORD1_OP3,

	/* LDS Clause Instructions */
	FMT_ALU_WORD0_LDS_IDX_OP,
	FMT_ALU_WORD1_LDS_IDX_OP,
	FMT_ALU_WORD1_LDS_DIRECT_LITERAL_LO,
	FMT_ALU_WORD1_LDS_DIRECT_LITERAL_HI,

	/* Instructions for a Fetch Through a Vertex Cache Clause */
	FMT_VTX_WORD0,
	FMT_VTX_WORD1_GPR,
	FMT_VTX_WORD1_SEM,
	FMT_VTX_WORD2,

	/* Instructions for a Fetch Through a Texture Cache Clause */
	FMT_TEX_WORD0,
	FMT_TEX_WORD1,
	FMT_TEX_WORD2,

	/* Memory Read Instructions */
	FMT_MEM_RD_WORD0,
	FMT_MEM_RD_WORD1,
	FMT_MEM_RD_WORD2,

	/* Global Data-Share Read/Write Instructions */
	FMT_MEM_GDS_WORD0,
	FMT_MEM_GDS_WORD1,
	FMT_MEM_GDS_WORD2,

	/* Max */
	FMT_COUNT
};


/* ALUs */
enum amd_alu_enum {

	AMD_ALU_X = 0,
	AMD_ALU_Y,
	AMD_ALU_Z,
	AMD_ALU_W,
	AMD_ALU_TRANS,

	AMD_ALU_COUNT
};




/*
 * String maps
 */

extern struct string_map_t amd_pv_map;
extern struct string_map_t amd_alu_map;
extern struct string_map_t bank_swizzle_map;
extern struct string_map_t rat_inst_map;
extern struct string_map_t cf_cond_map;
extern struct string_map_t src_sel_map;
extern struct string_map_t dst_sel_map;

extern struct string_map_t fmt_vtx_fetch_type_map;
extern struct string_map_t fmt_vtx_data_format_map;
extern struct string_map_t fmt_vtx_num_format_map;
extern struct string_map_t fmt_vtx_format_comp_map;
extern struct string_map_t fmt_vtx_srf_mode_map;
extern struct string_map_t fmt_vtx_endian_swap_map;
extern struct string_map_t fmt_lds_op_map;




/*
 * Structure of Microcode Format
 */

struct fmt_cf_word0_t
{
	unsigned int addr : 24;  /* [23:0] */
	unsigned int jump_table_sel : 3;  /* [26:24] */
	unsigned int __reserved0 : 5;  /* [31:27] */
};


struct fmt_cf_gws_word0_t
{
	unsigned int value : 10;  /* [9:0] */
	unsigned int __reserved0 : 6;  /* [15:10] */
	unsigned int resource : 5;  /* [20:16] */
	unsigned int __reserved1 : 4;  /* [24:21] */
	unsigned int s : 1;  /* [25] */
	unsigned int val_index_mode : 2;  /* [27:26] */
	unsigned int rsrc_index_mode : 2;  /* [29:28] */
	unsigned int gws_opcode : 2;  /* [31:30] */
};


struct fmt_cf_word1_t
{
	unsigned int pop_count : 3;  /* [2:0] */
	unsigned int cf_const : 5;  /* [7:3] */
	unsigned int cond : 2;  /* [9:8] */
	unsigned int count : 6;  /* [15:10] */
	unsigned int __reserved0 : 4;  /* [19:16] */
	unsigned int valid_pixel_mode : 1;  /* [20] */
	unsigned int end_of_program : 1;  /* [21] */
	unsigned int cf_inst : 8;  /* [29:22] */
	unsigned int whole_quad_mode : 1;  /* [30] */
	unsigned int barrier : 1;  /* [31] */
};


struct fmt_cf_alu_word0_t
{
	unsigned int addr : 22;  /* [21:0] */
	unsigned int kcache_bank0 : 4;  /* [25:22] */
	unsigned int kcache_bank1 : 4;  /* [29:26] */
	unsigned int kcache_mode0 : 2;  /* [31:30] */
};


struct fmt_cf_alu_word1_t
{
	unsigned int kcache_mode1 : 2;  /* [1:0] */
	unsigned int kcache_addr0 : 8;  /* [9:2] */
	unsigned int kcache_addr1 : 8;  /* [17:10] */
	unsigned int count : 7;  /* [24:18] */
	unsigned int alt_const : 1;  /* [25] */
	unsigned int cf_inst : 4;  /* [29:26] */
	unsigned int whole_quad_mode : 1;  /* [30] */
	unsigned int barrier : 1;  /* [31] */
};


struct fmt_cf_alu_word0_ext_t {
	unsigned int __reserved0 : 4;  /* [3:0] */
	unsigned int kcache_bank_index_mode0 : 2;  /* [5:4] */
	unsigned int kcache_bank_index_mode1 : 2;  /* [7:6] */
	unsigned int kcache_bank_index_mode2 : 2;  /* [9:8] */
	unsigned int kcache_bank_index_mode3 : 2;  /* [11:10] */
	unsigned int __reserved1 : 10;  /* [21:12] */
	unsigned int kcache_bank2 : 4;  /* [25:22] */
	unsigned int kcache_bank3 : 4;  /* [29:26] */
	unsigned int kcache_mode2 : 2;  /* [31:30] */
};


struct fmt_cf_alu_word1_ext_t {
	unsigned int kcache_mode3 : 2;  /* [1:0] */
	unsigned int kcache_addr2 : 8;  /* [9:2] */
	unsigned int kcache_addr3 : 8;  /* [17:10] */
	unsigned int __reserved0 : 8;  /* [25:18] */
	unsigned int cf_inst : 4;  /* [29:26] */
	unsigned int __reserved1 : 1;  /* [30] */
	unsigned int barrier : 1;  /* [31] */
};


struct fmt_cf_alloc_export_word0_t {
	unsigned int array_base : 13;  /* [12:0] */
	unsigned int type : 2;  /* [14:13] */
	unsigned int rw_gpr : 7;  /* [21:15] */
	unsigned int rr : 1;  /* [22] */
	unsigned int index_gpr : 7;  /* [29:23] */
	unsigned int elem_size : 2;  /* [31:30] */
};


struct fmt_cf_alloc_export_word0_rat_t {
	unsigned int rat_id : 4;  /* [3:0] */
	unsigned int rat_inst : 6;  /* [9:4] */
	unsigned int __reserved0 : 1;  /* [10] */
	unsigned int rat_index_mode : 2;  /* [12:11] */
	unsigned int type : 2;  /* [14:13] */
	unsigned int rw_gpr : 7;  /* [21:15] */
	unsigned int rr : 1;  /* rw_rel [22] */
	unsigned int index_gpr : 7;  /* [29:23] */
	unsigned int elem_size : 2;  /* [31:30] */
};


struct fmt_cf_alloc_export_word1_buf_t {
	unsigned int array_size : 12;  /* [11:0] */
	unsigned int comp_mask : 4;  /* [15:12] */
	unsigned int burst_count : 4;  /* [19:16] */
	unsigned int valid_pixel_mode : 1;  /* [20] */
	unsigned int end_of_program : 1;  /* [21] */
	unsigned int cf_inst : 8;  /* [29:22] */
	unsigned int mark : 1;  /* [30] */
	unsigned int barrier : 1;  /* [31] */
};


struct fmt_cf_alloc_export_word1_swiz_t {
	unsigned int sel_x : 3;  /* [2:0] */
	unsigned int sel_y : 3;  /* [5:3] */
	unsigned int sel_z : 3;  /* [8:6] */
	unsigned int sel_w : 3;  /* [11:9] */
	unsigned int __reserved0 : 4;  /* [15:12] */
	unsigned int burst_count : 4;  /* [19:16] */
	unsigned int valid_pixel_mode : 1;  /* [20] */
	unsigned int end_of_program : 1;  /* [21] */
	unsigned int cf_inst : 8;  /* [29:22] */
	unsigned int mark : 1;  /* [30] */
	unsigned int barrier : 1;  /* [31] */
};


struct fmt_alu_word0_t
{
	unsigned int src0_sel : 9;  /* [8:0] */
	unsigned int src0_rel : 1;  /* [9] */
	unsigned int src0_chan : 2;  /* [11:10] */
	unsigned int src0_neg : 1;  /* [12] */
	unsigned int src1_sel : 9;  /* [21:13] */
	unsigned int src1_rel : 1;  /* [22] */
	unsigned int src1_chan : 2;  /* [24:23] */
	unsigned int src1_neg : 1;  /* [25] */
	unsigned int index_mode : 3;  /* [28:26] */
	unsigned int pred_sel : 2;  /* [30:29] */
	unsigned int last : 1;  /* [31] */
};


struct fmt_alu_word1_op2_t
{
	unsigned int src0_abs : 1;  /* [0] */
	unsigned int src1_abs : 1;  /* [1] */
	unsigned int update_exec_mask : 1;  /* [2] */
	unsigned int update_pred : 1;  /* [3] */
	unsigned int write_mask : 1;  /* [4] */
	unsigned int omod : 2;  /* [6:5] */
	unsigned int alu_inst : 11;  /* [17:7] */
	unsigned int bank_swizzle : 3;  /* [20:18] */
	unsigned int dst_gpr : 7;  /* [27:21] */
	unsigned int dst_rel : 1;  /* [28] */
	unsigned int dst_chan : 2;  /* [30:29] */
	unsigned int clamp : 1;  /* [31] */
};


struct fmt_alu_word1_op3_t
{
	unsigned int src2_sel : 9;  /* [8:0] */
	unsigned int src2_rel : 1;  /* [9] */
	unsigned int src2_chan : 2;  /* [11:10] */
	unsigned int src2_neg : 1;  /* [12] */
	unsigned int alu_inst : 5;  /* [17:13] */
	unsigned int bank_swizzle : 3;  /* [20:18] */
	unsigned int dst_gpr : 7;  /* [27:21] */
	unsigned int dst_rel : 1;  /* [28] */
	unsigned int dst_chan : 2;  /* [30:29] */
	unsigned int clamp : 1;  /* [31] */
};


struct fmt_alu_word0_lds_idx_op_t
{
	unsigned int src0_sel : 9;  /* [8:0] */
	unsigned int src0_rel : 1;  /* [9] */
	unsigned int src0_chan : 2;  /* [11:10] */
	unsigned int idx_offset_4 : 1;  /* [12] */
	unsigned int src1_sel : 9;  /* [21:13] */
	unsigned int src1_rel : 1;  /* [22] */
	unsigned int src1_chan : 2;  /* [24:23] */
	unsigned int idx_offset_5 : 1;  /* [25] */
	unsigned int index_mode : 3;  /* [28:26] */
	unsigned int pred_sel : 2;  /* [30:29] */
	unsigned int last : 1;  /* [31] */
};


struct fmt_alu_word1_lds_idx_op_t
{
	unsigned int src2_sel : 9;  /* [8:0] */
	unsigned int src2_rel : 1;  /* [9] */
	unsigned int src2_chan : 2;  /* [11:10] */
	unsigned int idx_offset_1 : 1;  /* [12] */
	unsigned int alu_inst : 5;  /* [17:13] */
	unsigned int bank_swizzle : 3;  /* [20:18] */
	unsigned int lds_op : 6;  /* [26:21] */
	unsigned int idx_offset_0 : 1;  /* [27] */
	unsigned int idx_offset_2 : 1;  /* [28] */
	unsigned int dst_chan : 2;  /* [30:29] */
	unsigned int idx_offset_3 : 1;  /* [31] */
};


struct fmt_vtx_word0_t {
	unsigned int vc_inst : 5;
	unsigned int fetch_type : 2;
	unsigned int fetch_whole_quad : 1;
	unsigned int buffer_id : 8;
	unsigned int src_gpr : 7;
	unsigned int src_rel : 1;
	unsigned int src_sel_x : 2;
	unsigned int mega_fetch_count : 6;
};


struct fmt_vtx_word1_gpr_t {
	unsigned int dst_gpr : 7;
	unsigned int dst_rel : 1;
	unsigned int __reserved0 : 1;
	unsigned int dst_sel_x : 3;
	unsigned int dst_sel_y : 3;
	unsigned int dst_sel_z : 3;
	unsigned int dst_sel_w : 3;
	unsigned int use_const_fields : 1;
	unsigned int data_format : 6;
	unsigned int num_format_all : 2;
	unsigned int format_comp_all : 1;
	unsigned int srf_mode_all : 1;
};


struct fmt_vtx_word1_sem_t {
	unsigned int semantic_id : 8;
	unsigned int __reserved0 : 1;
	unsigned int dst_sel_x : 3;
	unsigned int dst_sel_y : 3;
	unsigned int dst_sel_z : 3;
	unsigned int dst_sel_w : 3;
	unsigned int use_const_fields : 1;
	unsigned int data_format : 6;
	unsigned int num_format_all : 2;
	unsigned int format_comp_all : 1;
	unsigned int srf_mode_all : 1;
};


struct fmt_vtx_word2_t {
	unsigned int offset : 16;
	unsigned int endian_swap : 2;
	unsigned int const_buf_no_stride : 1;
	unsigned int mega_fetch : 1;
	unsigned int __reserved0 : 12;
};


struct fmt_tex_word0_t
{
	unsigned int tex_inst : 5;
	unsigned int inst_mod : 2;
	unsigned int fwq : 1;
	unsigned int resource_id : 8;
	unsigned int src_gpr : 7;
	unsigned int sr : 1;
	unsigned int ac : 1;
	unsigned int rim : 2;
	unsigned int sim : 2;
	unsigned int __reserved0 : 3;
};


struct fmt_tex_word1_t
{
	unsigned int dst_gpr : 7;
	unsigned int dr : 1;
	unsigned int __reserved0 : 1;
	unsigned int dsx : 3;
	unsigned int dsy : 3;
	unsigned int dsz : 3;
	unsigned int dsw : 3;
	unsigned int lod_bias : 7;
	unsigned int ctx : 1;
	unsigned int cty : 1;
	unsigned int ctz : 1;
	unsigned int ctw : 1;
};


struct fmt_tex_word2_t
{
	unsigned int offset_x : 5;
	unsigned int offset_y : 5;
	unsigned int offset_z : 5;
	unsigned int sampler_id : 5;
	unsigned int ssx : 3;
	unsigned int ssy : 3;
	unsigned int ssz : 3;
	unsigned int ssw : 3;
};


enum amd_category_enum {
	AMD_CAT_NONE = 0,

	AMD_CAT_CF,  /* Control-flow instructions */
	AMD_CAT_ALU,  /* ALU clause instructions */
	AMD_CAT_LDS,  /* LDS clause instructions */
	AMD_CAT_VTX,  /* Instructions for a fetch through a vertex cache clause */
	AMD_CAT_TEX,  /* Instructions for a fetch through a texture cache clause */
	AMD_CAT_MEM_RD,  /* Memory read instructions */
	AMD_CAT_MEM_GDS, /* Global data-share read/write instructions */

	AMD_CAT_COUNT
};


enum amd_inst_flags_enum {
	AMD_INST_FLAG_NONE		= 0x0000,
	AMD_INST_FLAG_TRANS_ONLY	= 0x0001,  /* Only executable in transcendental unit */
	AMD_INST_FLAG_INC_LOOP_IDX      = 0x0002,  /* CF inst increasing loop depth index */
	AMD_INST_FLAG_DEC_LOOP_IDX      = 0x0004,  /* CF inst decreasing loop index */
	AMD_INST_FLAG_DST_INT           = 0x0008,  /* Inst with integer dest operand */
	AMD_INST_FLAG_DST_UINT          = 0x0010,  /* Inst with unsigned int dest op */
	AMD_INST_FLAG_DST_FLOAT         = 0x0020,  /* Inst with float dest op */
	AMD_INST_FLAG_ACT_MASK		= 0x0040,  /* Inst affects the active mask (control flow) */
	AMD_INST_FLAG_LDS		= 0x0080,  /* Access to local memory */
	AMD_INST_FLAG_MEM		= 0x0100   /* Access to global memory */
};


enum amd_inst_enum {
	AMD_INST_NONE = 0,

#define DEFINST(_name, _fmt_str, _fmt0, _fmt1, _fmt2, _category, _cf_inst, _flags) \
	AMD_INST_##_name,
#include "gpudisasm.dat"
#undef DEFINST
	
	/* Max */
	AMD_INST_COUNT
};


union amd_inst_word_t {

	char bytes[4];

	struct fmt_cf_word0_t cf_word0;
	struct fmt_cf_gws_word0_t cf_gws_word0;
	struct fmt_cf_word1_t cf_word1;

	struct fmt_cf_alu_word0_t cf_alu_word0;
	struct fmt_cf_alu_word1_t cf_alu_word1;
		
	struct fmt_cf_alu_word0_ext_t cf_alu_word0_ext;
	struct fmt_cf_alu_word1_ext_t cf_alu_word1_ext;
		
	struct fmt_cf_alloc_export_word0_t cf_alloc_export_word0;
	struct fmt_cf_alloc_export_word0_rat_t cf_alloc_export_word0_rat;
	struct fmt_cf_alloc_export_word1_buf_t cf_alloc_export_word1_buf;
	struct fmt_cf_alloc_export_word1_swiz_t cf_alloc_export_word1_swiz;

	struct fmt_alu_word0_t alu_word0;
	struct fmt_alu_word1_op2_t alu_word1_op2;
	struct fmt_alu_word1_op3_t alu_word1_op3;

	struct fmt_alu_word0_lds_idx_op_t alu_word0_lds_idx_op;
	struct fmt_alu_word1_lds_idx_op_t alu_word1_lds_idx_op;

	struct fmt_vtx_word0_t vtx_word0;
	struct fmt_vtx_word1_gpr_t vtx_word1_gpr;
	struct fmt_vtx_word1_sem_t vtx_word1_sem;
	struct fmt_vtx_word2_t vtx_word2;

	struct fmt_tex_word0_t tex_word0;
	struct fmt_tex_word1_t tex_word1;
	struct fmt_tex_word2_t tex_word2;
};


#define AMD_INST_MAX_WORDS  3
struct amd_inst_info_t {
	enum amd_inst_enum inst;
	enum amd_category_enum category;
	char *name;
	char *fmt_str;
	enum fmt_enum fmt[AMD_INST_MAX_WORDS];  /* Word formats */
	int opcode;  /* Operation code */
	enum amd_inst_flags_enum flags;  /* Flag bitmap */
	int size;  /* Number of words (32-bit) */
};


struct amd_inst_t {
	
	/* Basic instruction info */
	struct amd_inst_info_t *info;  /* Pointer to 'amd_inst_info' table */
	union amd_inst_word_t words[AMD_INST_MAX_WORDS];
	
	/* For ALU instructions, identifier of assigned ALU (ALU_[X,Y,Z,W] or ALU_TRANS) */
	enum amd_alu_enum alu;

	/* If 'category' is AMD_CAT_ALU, alu group where it belongs */
	struct amd_alu_group_t *alu_group;
};


#define ALU_GROUP_INST_COUNT_MAX  5
#define ALU_GROUP_LITERAL_COUNT_MAX  4
struct amd_alu_group_t
{
	int id;  /* Identifier of ALU group */
	int inst_count;  /* Number of instruction slots (max. 5) */
	int literal_count;  /* Number of literal constant slots (max. 2) */
	struct amd_inst_t inst[ALU_GROUP_INST_COUNT_MAX];
	float literal[ALU_GROUP_LITERAL_COUNT_MAX];  /* Literals for X,Y,Z,W elements */
};


typedef void (*fmt_dump_func_t)(void *buf, FILE *);

void amd_disasm_init(void);
void amd_disasm_done(void);

void fmt_word_dump(void *buf, enum fmt_enum fmt, FILE *f);
void amd_inst_dump_gpr(int gpr, int rel, int chan, int im, FILE *f);
void amd_inst_dump(struct amd_inst_t *inst, int count, int shift, FILE *f);
void amd_inst_words_dump(struct amd_inst_t *inst, FILE *f);
void amd_alu_group_dump(struct amd_alu_group_t *group, int shift, FILE *f);

/* Obtaining source operand fields for ALU instructions */
void amd_inst_get_op_src(struct amd_inst_t *inst, int src_idx,
	int *sel, int *rel, int *chan, int *neg, int *abs);

/* Decode */
void *amd_inst_decode_cf(void *buf, struct amd_inst_t *inst);
void *amd_inst_decode_alu(void *buf, struct amd_inst_t *inst);
void *amd_inst_decode_alu_group(void *buf, int group_id, struct amd_alu_group_t *group);
void *amd_inst_decode_tc(void *buf, struct amd_inst_t *inst);

#endif

