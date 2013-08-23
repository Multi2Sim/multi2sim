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

#ifndef EVERGREEN_ASM_INST_H
#define EVERGREEN_ASM_INST_H

#include <lib/class/class.h>
#include <lib/util/string.h>


/*
 * Class 'EvgInst'
 */

#define EVG_INST_GPR_NUM_ELEM  5
typedef struct
{
	/* x, y, z, w, t */
	unsigned int elem[EVG_INST_GPR_NUM_ELEM];
} EvgInstGpr;


typedef enum
{
	EvgInstClauseInvalid = 0,
	EvgInstClauseCF,  /* Control-flow */
	EvgInstClauseALU,  /* ALU clause */
	EvgInstClauseTEX,  /* Fetch trough a Texture Cache Clause */
	EvgInstClauseVC  /* Fetch through a Vertex Cache Clause */
} EvgInstClause;


typedef union
{
	signed int as_int;
	unsigned int as_uint;

	signed short int as_short[2];
	unsigned short int as_ushort[2];

	float as_float;
} EvgInstReg;


extern struct str_map_t evg_inst_alu_map;
extern struct str_map_t evg_inst_alu_pv_map;
typedef enum
{
	EvgInstAluX = 0,
	EvgInstAluY,
	EvgInstAluZ,
	EvgInstAluW,
	EvgInstAluTrans,

	EvgInstAluCount
} EvgInstAlu;


typedef enum
{
	EvgInstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _fmt0, _fmt1, _fmt2, _category, _cf_inst, _flags) \
	EVG_INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
	EvgInstOpcodeCount
} EvgInstOpcode;


typedef enum
{
	EvgInstCategoryInvalid = 0,

	EvgInstCategoryCF,  /* Control-flow instructions */
	EvgInstCategoryALU,  /* ALU clause instructions */
	EvgInstCategoryLDS,  /* LDS clause instructions */
	EvgInstCategoryVTX,  /* Instructions for a fetch through a vertex cache clause */
	EvgInstCategoryTEX,  /* Instructions for a fetch through a texture cache clause */
	EvgInstCategoryMemRd,  /* Memory read instructions */
	EvgInstCategoryMemGDS, /* Global data-share read/write instructions */

	EvgInstCategoryCount
} EvgInstCategory;


/* Microcode Formats */
typedef enum
{
	EvgInstFormatInvalid = 0,

	/* Control flow instructions */
	EvgInstFormatCfWord0,
	EvgInstFormatCfGwsWord0,
	EvgInstFormatCfWord1,

	EvgInstFormatCfAluWord0,
	EvgInstFormatCfAluWord1,

	EvgInstFormatCfAluWord0Ext,
	EvgInstFormatCfAluWord1Ext,

	EvgInstFormatCfAllocExportWord0,
	EvgInstFormatCfAllocExportWord0Rat,
	EvgInstFormatCfAllocExportWord1Buf,
	EvgInstFormatCfAllocExportWord1Swiz,

	/* ALU Clause Instructions */
	EvgInstFormatAluWord0,
	EvgInstFormatAluWord1Op2,
	EvgInstFormatAluWord1Op3,

	/* LDS Clause Instructions */
	EvgInstFormatAluWord0LdsIdxOp,
	EvgInstFormatAluWord1LdsIdxOp,
	EvgInstFormatAluWord1LdsDirectLiteralLo,
	EvgInstFormatAluWord1LdsDirectLiteralHi,

	/* Instructions for a Fetch Through a Vertex Cache Clause */
	EvgInstFormatVtxWord0,
	EvgInstFormatVtxWord1Gpr,
	EvgInstFormatVtxWord1Sem,
	EvgInstFormatVtxWord2,

	/* Instructions for a Fetch Through a Texture Cache Clause */
	EvgInstFormatTexWord0,
	EvgInstFormatTexWord1,
	EvgInstFormatTexWord2,

	/* Memory Read Instructions */
	EvgInstFormatMemRdWord0,
	EvgInstFormatMemRdWord1,
	EvgInstFormatMemRdWord2,

	/* Global Data-Share Read/Write Instructions */
	EvgInstFormatMemGdsWord0,
	EvgInstFormatMemGdsWord1,
	EvgInstFormatMemGdsWord2,

	/* Max */
	EvgInstFormatCount
} EvgInstFormat;


typedef enum
{
	EvgInstFlagInvalid = 0x0000,
	EvgInstFlagTransOnly = 0x0001,  /* Only executable in transcendental unit */
	EvgInstFlagIncLoopIdx = 0x0002,  /* CF inst increasing loop depth index */
	EvgInstFlagDecLoopIdx = 0x0004,  /* CF inst decreasing loop index */
	EvgInstFlagDstInt = 0x0008,  /* Inst with integer dest operand */
	EvgInstFlagDstUint = 0x0010,  /* Inst with unsigned int dest op */
	EvgInstFlagDstFloat = 0x0020,  /* Inst with float dest op */
	EvgInstFlagActMask = 0x0040,  /* Inst affects the active mask (control flow) */
	EvgInstFlagLDS = 0x0080,  /* Access to local memory */
	EvgInstFlagMem = 0x0100,  /* Access to global memory */
	EvgInstFlagMemRead = 0x0200,  /* Read to global memory */
	EvgInstFlagMemWrite = 0x0400,  /* Write to global memory */
	EvgInstFlagPredMask = 0x0800   /* Inst affects the predicate mask */
} EvgInstFlag;


typedef struct
{
	unsigned int addr : 24;  /* [23:0] */
	unsigned int jump_table_sel : 3;  /* [26:24] */
	unsigned int __reserved0 : 5;  /* [31:27] */
} EvgInstBytesCfWord0;

typedef struct
{
	unsigned int value : 10;  /* [9:0] */
	unsigned int __reserved0 : 6;  /* [15:10] */
	unsigned int resource : 5;  /* [20:16] */
	unsigned int __reserved1 : 4;  /* [24:21] */
	unsigned int s : 1;  /* [25] */
	unsigned int val_index_mode : 2;  /* [27:26] */
	unsigned int rsrc_index_mode : 2;  /* [29:28] */
	unsigned int gws_opcode : 2;  /* [31:30] */
} EvgInstBytesCfGwsWord0;

typedef struct
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
} EvgInstBytesCfWord1;

typedef struct
{
	unsigned int addr : 22;  /* [21:0] */
	unsigned int kcache_bank0 : 4;  /* [25:22] */
	unsigned int kcache_bank1 : 4;  /* [29:26] */
	unsigned int kcache_mode0 : 2;  /* [31:30] */
} EvgInstBytesCfAluWord0;

typedef struct
{
	unsigned int kcache_mode1 : 2;  /* [1:0] */
	unsigned int kcache_addr0 : 8;  /* [9:2] */
	unsigned int kcache_addr1 : 8;  /* [17:10] */
	unsigned int count : 7;  /* [24:18] */
	unsigned int alt_const : 1;  /* [25] */
	unsigned int cf_inst : 4;  /* [29:26] */
	unsigned int whole_quad_mode : 1;  /* [30] */
	unsigned int barrier : 1;  /* [31] */
} EvgInstBytesCfAluWord1;

typedef struct
{
	unsigned int __reserved0 : 4;  /* [3:0] */
	unsigned int kcache_bank_index_mode0 : 2;  /* [5:4] */
	unsigned int kcache_bank_index_mode1 : 2;  /* [7:6] */
	unsigned int kcache_bank_index_mode2 : 2;  /* [9:8] */
	unsigned int kcache_bank_index_mode3 : 2;  /* [11:10] */
	unsigned int __reserved1 : 10;  /* [21:12] */
	unsigned int kcache_bank2 : 4;  /* [25:22] */
	unsigned int kcache_bank3 : 4;  /* [29:26] */
	unsigned int kcache_mode2 : 2;  /* [31:30] */
} EvgInstBytesCfAluWord0Ext;

typedef struct
{
	unsigned int kcache_mode3 : 2;  /* [1:0] */
	unsigned int kcache_addr2 : 8;  /* [9:2] */
	unsigned int kcache_addr3 : 8;  /* [17:10] */
	unsigned int __reserved0 : 8;  /* [25:18] */
	unsigned int cf_inst : 4;  /* [29:26] */
	unsigned int __reserved1 : 1;  /* [30] */
	unsigned int barrier : 1;  /* [31] */
} EvgInstBytesCfAluWord1Ext;

typedef struct
{
	unsigned int array_base : 13;  /* [12:0] */
	unsigned int type : 2;  /* [14:13] */
	unsigned int rw_gpr : 7;  /* [21:15] */
	unsigned int rr : 1;  /* [22] */
	unsigned int index_gpr : 7;  /* [29:23] */
	unsigned int elem_size : 2;  /* [31:30] */
} EvgInstBytesCfAllocExportWord0;

typedef struct
{
	unsigned int rat_id : 4;  /* [3:0] */
	unsigned int rat_inst : 6;  /* [9:4] */
	unsigned int __reserved0 : 1;  /* [10] */
	unsigned int rat_index_mode : 2;  /* [12:11] */
	unsigned int type : 2;  /* [14:13] */
	unsigned int rw_gpr : 7;  /* [21:15] */
	unsigned int rr : 1;  /* rw_rel [22] */
	unsigned int index_gpr : 7;  /* [29:23] */
	unsigned int elem_size : 2;  /* [31:30] */
} EvgInstBytesCfAllocExportWord0Rat;

typedef struct
{
	unsigned int array_size : 12;  /* [11:0] */
	unsigned int comp_mask : 4;  /* [15:12] */
	unsigned int burst_count : 4;  /* [19:16] */
	unsigned int valid_pixel_mode : 1;  /* [20] */
	unsigned int end_of_program : 1;  /* [21] */
	unsigned int cf_inst : 8;  /* [29:22] */
	unsigned int mark : 1;  /* [30] */
	unsigned int barrier : 1;  /* [31] */
} EvgInstBytesCfAllocExportWord1Buf;

typedef struct
{
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
} EvgInstBytesCfAllocExportWord1Swiz;

typedef struct
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
} EvgInstBytesAluWord0;

typedef struct
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
} EvgInstBytesAluWord1Op2;

typedef struct
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
} EvgInstBytesAluWord1Op3;

typedef struct
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
} EvgInstBytesAluWord0LdsIdxOp;

typedef struct
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
} EvgInstBytesAluWord1LdsIdxOp;

typedef struct
{
	unsigned int vc_inst : 5;  /* [4:0] */
	unsigned int fetch_type : 2;  /* [5:6] */
	unsigned int fetch_whole_quad : 1;  /* [7] */
	unsigned int buffer_id : 8;  /* [15:8] */
	unsigned int src_gpr : 7;  /* [22:16] */
	unsigned int src_rel : 1;  /* [23] */
	unsigned int src_sel_x : 2;  /* [25:24] */
	unsigned int mega_fetch_count : 6;  /* [31:26] */
} EvgInstBytesVtxWord0;

typedef struct
{
	unsigned int dst_gpr : 7;  /* [6:0] */
	unsigned int dst_rel : 1;  /* [7] */
	unsigned int __reserved0 : 1;  /* [8] */
	unsigned int dst_sel_x : 3;	 /* [11:9] */
	unsigned int dst_sel_y : 3;  /* [14:12] */
	unsigned int dst_sel_z : 3;  /* [17:15] */
	unsigned int dst_sel_w : 3;  /* [20:17] */
	unsigned int use_const_fields : 1; /* [21] */
	unsigned int data_format : 6;  /* [27:22] */
	unsigned int num_format_all : 2;  /* [29:28] */
	unsigned int format_comp_all : 1;  /* [30] */
	unsigned int srf_mode_all : 1;  /* [31] */
} EvgInstBytesVtxWord1Gpr;

typedef struct
{
	unsigned int semantic_id : 8;  /* [7:0] */
	unsigned int __reserved0 : 1;  /* [8] */
	unsigned int dst_sel_x : 3;  /* [11:9] */
	unsigned int dst_sel_y : 3;  /* [14:12] */
	unsigned int dst_sel_z : 3;  /* [17:15] */
	unsigned int dst_sel_w : 3;  /* [20:18] */
	unsigned int use_const_fields : 1;  /* [21] */
	unsigned int data_format : 6;  /* [27:22] */
	unsigned int num_format_all : 2;  /* [29:28] */
	unsigned int format_comp_all : 1; /* [30] */
	unsigned int srf_mode_all : 1;  /* [31] */
} EvgInstBytesVtxWord1Sem;

typedef struct
{
	unsigned int offset : 16;  /* [15:0] */
	unsigned int endian_swap : 2;  /* [17:16] */
	unsigned int const_buf_no_stride : 1;  /* [18] */
	unsigned int mega_fetch : 1;  /* [19] */
	/* FIXME : 3 bits are reserved or used ? */
	// unsigned int alt_const : 1;  /* [20] */
	// unsigned int bim : 2; /* [22:21] */
	// unsigned int __reserved0 : 9;  /* [31:23] */
	unsigned int __reserved0 : 12;  /* [31:21] */
} EvgInstBytesVtxWord2;

typedef struct
{
	unsigned int tex_inst : 5;	/* [4:0] */
	unsigned int inst_mod : 2;  /* [6:5] */
	unsigned int fwq : 1;  /* [7] */
	unsigned int resource_id : 8;  /* [15:8] */
	unsigned int src_gpr : 7;  /* [22:16] */
	unsigned int sr : 1;  /* [23] */
	unsigned int ac : 1;  /* [24] */
	unsigned int rim : 2;  /* [26:25] */
	unsigned int sim : 2;  /* [28:27] */
	unsigned int __reserved0 : 3;  /* [31:29] */
} EvgInstBytesTexWord0;

typedef struct
{
	unsigned int dst_gpr : 7;  /* [6:0] */
	unsigned int dr : 1;  /* [7] */
	unsigned int __reserved0 : 1;  /* [8] */
	unsigned int dsx : 3;  /* [11:9] */
	unsigned int dsy : 3;  /* [14:12] */
	unsigned int dsz : 3;  /* [17:15] */
	unsigned int dsw : 3;  /* [20:18] */
	unsigned int lod_bias : 7;  /* [27:21] */
	unsigned int ctx : 1;  /* [28] */
	unsigned int cty : 1;  /* [29] */
	unsigned int ctz : 1;  /* [30] */
	unsigned int ctw : 1;  /* [31] */
} EvgInstBytesTexWord1;

typedef struct
{
	unsigned int offset_x : 5;  /* [4:0] */
	unsigned int offset_y : 5;  /* [9:5] */
	unsigned int offset_z : 5;  /* [14:10] */
	unsigned int sampler_id : 5;  /* [19:15] */
	unsigned int ssx : 3;  /* [22:20] */
	unsigned int ssy : 3;  /* [25:23] */
	unsigned int ssz : 3;  /* [28:26] */
	unsigned int ssw : 3;  /* [31:29] */
} EvgInstBytesTexWord2;

typedef struct
{
	unsigned int mem_inst : 5;  /* [4:0] */
	unsigned int elem_size : 2;  /* [6:5] */
	unsigned int fwq : 1;  /* [7] */
	unsigned int mem_op : 3;  /* [10:8] */
	unsigned int uncached : 1;  /* [11] */
	unsigned int indexed : 1;  /* [12] */
	unsigned int __reserved0 : 3;  /* [15:13] */
	unsigned int src_gpr : 7;  /* [22:16] */
	unsigned int src_rel : 1;  /* [23] */
	unsigned int ssx : 2;  /* [25:24] */
	unsigned int burst_count : 4;  /* [29:26] */
	unsigned int _reserved1 : 2;  /* [31:30] */
} EvgInstBytesMemRdWord0;

typedef struct
{
	unsigned int dst_gpr : 7;  /* [6:0] */
	unsigned int dst_rel : 1;  /* [7] */
	unsigned int __reserved0 : 1;  /* [8] */
	unsigned int dsx : 3;  /* [11:9] */
	unsigned int dsy : 3;  /* [14:12] */
	unsigned int dsw : 3;  /* [17:15] */
	unsigned int dsz : 3;  /* [20:18] */
	unsigned int __reserved1 : 1;  /* [21] */
	unsigned int data_format : 6;  /* [27:22] */
	unsigned int num_format_all : 2;  /* [29:28] */
	unsigned int format_comp_all : 1;  /* [30] */
	unsigned int srf_mode_all : 1;  /* [31] */
} EvgInstBytesMemRdWord1;

typedef struct
{
	unsigned int array_base : 13;  /* [12:0] */
	unsigned int __reserved0 : 3;  /* [15:13] */
	unsigned int endian_swap : 2;  /* [17:16] */
	unsigned int __reserved1 : 2;  /* [19:18] */
	unsigned int array_size : 12;  /* [31:20] */
} EvgInstBytesMemRdWord2;

typedef struct
{
	unsigned int mem_inst : 5;  /* [4:0] */
	unsigned int __reserved0 : 3;  /* [7:5] */
	unsigned int mem_op : 3;  /* [10:8] */
	unsigned int src_gpr : 7;  /* [17:11] */
	unsigned int src_rel_mode : 2;  /* [19:18] */
	unsigned int ssx : 3;  /* [22:20] */
	unsigned int ssy : 3;  /* [25:23] */
	unsigned int ssz : 3;  /* [29:26] */
	unsigned int __reserved1 : 3;  /* [31:29] */
} EvgInstBytesMemGdsWord0;

typedef struct
{
	unsigned int dst_gpr : 7;  /* [6:0] */
	unsigned int dst_rel_mode : 2;  /* [8:7] */
	unsigned int gds_op : 6;  /* [14:9] */
	unsigned int __reserved0 : 1;  /* [15] */
	unsigned int src_gpr : 7;  /* [22:16] */
	unsigned int __reserved1 : 1;  /* [23] */
	unsigned int uim : 2;  /* [25:24] */
	unsigned int uav_id : 4;  /* [29:26] */
	unsigned int alloc_consume : 1;  /* [30] */
	unsigned int bcast_first_req : 1;  /* [31] */
} EvgInstBytesMemGdsWord1;

typedef struct
{
	unsigned int dsx : 3;  /* [2:0] */
	unsigned int dsy : 3;  /* [5:3] */
	unsigned int dsz : 3;  /* [8:6] */
	unsigned int dsw : 3;  /* [11:9] */
	unsigned int __reserved0 : 20;  /* [31:12] */
} EvgInstBytesMemGdsWord2;


typedef union
{
	char bytes[4];
	unsigned int word;

	EvgInstBytesCfWord0 cf_word0;
	EvgInstBytesCfGwsWord0 cf_gws_word0;
	EvgInstBytesCfWord1 cf_word1;

	EvgInstBytesCfAluWord0 cf_alu_word0;
	EvgInstBytesCfAluWord1 cf_alu_word1;

	EvgInstBytesCfAluWord0Ext cf_alu_word0_ext;
	EvgInstBytesCfAluWord1Ext cf_alu_word1_ext;

	EvgInstBytesCfAllocExportWord0 cf_alloc_export_word0;
	EvgInstBytesCfAllocExportWord0Rat cf_alloc_export_word0_rat;
	EvgInstBytesCfAllocExportWord1Buf cf_alloc_export_word1_buf;
	EvgInstBytesCfAllocExportWord1Swiz cf_alloc_export_word1_swiz;

	EvgInstBytesAluWord0 alu_word0;
	EvgInstBytesAluWord1Op2 alu_word1_op2;
	EvgInstBytesAluWord1Op3 alu_word1_op3;

	EvgInstBytesAluWord0LdsIdxOp alu_word0_lds_idx_op;
	EvgInstBytesAluWord1LdsIdxOp alu_word1_lds_idx_op;

	EvgInstBytesVtxWord0 vtx_word0;
	EvgInstBytesVtxWord1Gpr vtx_word1_gpr;
	EvgInstBytesVtxWord1Sem vtx_word1_sem;
	EvgInstBytesVtxWord2 vtx_word2;

	EvgInstBytesTexWord0 tex_word0;
	EvgInstBytesTexWord1 tex_word1;
	EvgInstBytesTexWord2 tex_word2;

	EvgInstBytesMemRdWord0 mem_rd_word0;
	EvgInstBytesMemRdWord1 mem_rd_word1;
	EvgInstBytesMemRdWord2 mem_rd_word2;

	EvgInstBytesMemGdsWord0 mem_gds_word0;
	EvgInstBytesMemGdsWord1 mem_gds_word1;
	EvgInstBytesMemGdsWord2 mem_gds_word2;

} EvgInstBytes;


#define EVG_INST_MAX_WORDS  3
typedef struct
{
	EvgInstOpcode opcode;
	EvgInstCategory category;
	char *name;
	char *fmt_str;
	EvgInstFormat fmt[EVG_INST_MAX_WORDS];  /* Word formats */

	/* Instruction bits identifying the instruction */
	int op;

	EvgInstFlag flags;  /* Flag bitmap */
	int size;  /* Number of words (32-bit) */
} EvgInstInfo;


CLASS_BEGIN(EvgInst, Object)

	/* Disassembler */
	EvgAsm *as;

	/* Decoded instruction information */
	EvgInstInfo *info;

	/* Instruction words */
	EvgInstBytes words[EVG_INST_MAX_WORDS];

	/* For ALU instructions, identifier of assigned ALU */
	EvgInstAlu alu;

	/* ALU group that instruction belongs to */
	EvgALUGroup *alu_group;

CLASS_END(EvgInst)


void EvgInstCreate(EvgInst *self, EvgAsm *as);
void EvgInstDestroy(EvgInst *self);

void EvgInstClear(EvgInst *self);

/* Decode an instruction from a control-flow, ALU, or texture clause in the
 * buffer given in 'buf'. The functions return the next position in 'buf' after
 * having read the instruction. */
void *EvgInstDecodeCF(EvgInst *self, void *buf);
void *EvgInstDecodeALU(EvgInst *self, void *buf);
void *EvgInstDecodeTC(EvgInst *self, void *buf);

void EvgInstDump(EvgInst *self, int count, int loop_idx, FILE *f);
void EvgInstDumpBuf(EvgInst *self, int count, int loop_idx,
	char *buf, int size);

void EvgInstSlotDump(EvgInst *self, int count, int loop_idx, int slot, FILE *f);
void EvgInstSlotDumpBuf(EvgInst *self, int count, int loop_idx, int slot,
	char *buf, int size);

void EvgInstDumpGpr(int gpr, int rel, int chan, int im, FILE *f);

void EvgInstGetOpSrc(EvgInst *self, int src_idx,
	int *sel, int *rel, int *chan, int *neg, int *abs);
#endif

