/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_INST_H
#define ARCH_SOUTHERN_ISLANDS_ASM_INST_H

#include <stdio.h>

#include <lib/class/class.h>



/*
 * Class 'SIInst'
 */

/* Microcode Formats
 * NOTE: Update string map 'si_inst_fmt_map' if new elements are added
 * to this enumeration. */
typedef enum
{
	SIInstFormatInvalid = 0,

	/* Scalar ALU and Control Formats */
	SIInstFormatSOP2,
	SIInstFormatSOPK,
	SIInstFormatSOP1,
	SIInstFormatSOPC,
	SIInstFormatSOPP,

	/* Scalar Memory Format */
	SIInstFormatSMRD,

	/* Vector ALU Formats */
	SIInstFormatVOP2,
	SIInstFormatVOP1,
	SIInstFormatVOPC,
	SIInstFormatVOP3a,
	SIInstFormatVOP3b,

	/* Vector Parameter Interpolation Format */
	SIInstFormatVINTRP,

	/* LDS/GDS Format */
	SIInstFormatDS,

	/* Vector Memory Buffer Formats */
	SIInstFormatMUBUF,
	SIInstFormatMTBUF,
	
	/* Vector Memory Image Format */
	SIInstFormatMIMG,

	/* Export Formats */
	SIInstFormatEXP,

	/* Max */
	SIInstFormatCount
} SIInstFormat;



/*
 * String maps
 */

extern struct str_map_t si_inst_fmt_map;
extern struct str_map_t si_inst_sdst_map;
extern struct str_map_t si_inst_ssrc_map;
extern struct str_map_t si_inst_OP16_map;
extern struct str_map_t si_inst_OP8_map;

struct si_inst_format_sop2_t
{
        unsigned int ssrc0    : 8;  /*   [7:0] */
        unsigned int ssrc1    : 8;  /*  [15:8] */
        unsigned int sdst     : 7;  /* [22:16] */
        unsigned int op       : 7;  /* [29:23] */
        unsigned int enc      : 2;  /* [31:30] */
        unsigned int lit_cnst : 32; /* [63:32] */
};

struct si_inst_format_sopk_t
{
        unsigned int simm16 : 16;  /*  [15:0] */
        unsigned int sdst   : 7;   /* [22:16] */
        unsigned int op     : 5;   /* [27:23] */
        unsigned int enc    : 4;   /* [31:28] */
};

struct si_inst_format_sop1_t
{
        unsigned int ssrc0    : 8;  /*   [7:0] */
        unsigned int op       : 8;  /*  [15:8] */
        unsigned int sdst     : 7;  /* [22:16] */
        unsigned int enc      : 9;  /* [31:23] */
        unsigned int lit_cnst : 32; /* [63:32] */
};

struct si_inst_format_sopc_t
{
        unsigned int ssrc0    : 8;  /*   [7:0] */
        unsigned int ssrc1    : 8;  /*  [15:8] */
        unsigned int op       : 7;  /* [22:16] */
        unsigned int enc      : 9;  /* [31:23] */
        unsigned int lit_cnst : 32; /* [63:32] */
};

struct si_inst_format_sopp_t
{
        unsigned int simm16 : 16;  /*  [15:0] */
        unsigned int op     : 7;   /* [22:16] */
        unsigned int enc    : 9;   /* [31:23] */
};

struct si_inst_format_smrd_t 
{
        unsigned int offset : 8;   /*   [7:0] */
        unsigned int imm    : 1;   /*      8  */
        unsigned int sbase  : 6;   /*  [14:9] */
        unsigned int sdst   : 7;   /* [21:15] */
        unsigned int op     : 5;   /* [26:22] */
        unsigned int enc    : 5;   /* [31:27] */
};

struct si_inst_format_vop2_t
{
        unsigned int src0     : 9;   /*   [8:0] */
        unsigned int vsrc1    : 8;   /*  [16:9] */
        unsigned int vdst     : 8;   /* [24:17] */
        unsigned int op       : 6;   /* [30:25] */
        unsigned int enc      : 1;   /*     31  */
        unsigned int lit_cnst : 32;  /* [63:32] */
};

struct si_inst_format_vop1_t
{
        unsigned int src0     : 9;   /*   [8:0] */
        unsigned int op       : 8;   /*  [16:9] */
        unsigned int vdst     : 8;   /* [24:17] */
        unsigned int enc      : 7;   /* [31:25] */
        unsigned int lit_cnst : 32;  /* [63:32] */
};

struct si_inst_format_vopc_t
{
        unsigned int src0     : 9;   /*   [8:0] */
        unsigned int vsrc1    : 8;   /*  [16:9] */
        unsigned int op       : 8;   /* [24:17] */
        unsigned int enc      : 7;   /* [31:25] */
        unsigned int lit_cnst : 32;  /* [63:32] */
};

struct si_inst_format_vop3a_t
{
        unsigned int vdst     : 8;   /*   [7:0] */
        unsigned int abs      : 3;   /*  [10:8] */
        unsigned int clamp    : 1;   /*     11  */
        unsigned int reserved : 5;   /* [16:12] */
        unsigned int op       : 9;   /* [25:17] */
        unsigned int enc      : 6;   /* [31:26] */
        unsigned int src0     : 9;   /* [40:32] */
        unsigned int src1     : 9;   /* [49:41] */
        unsigned int src2     : 9;   /* [57:49] */
        unsigned int omod     : 2;   /* [59:58] */
        unsigned int neg      : 3;   /* [63:60] */
};

struct si_inst_format_vop3b_t
{
        unsigned int vdst     : 8;   /*   [7:0] */
        unsigned int sdst     : 7;   /*  [14:8] */
        unsigned int reserved : 2;   /* [16:15] */
        unsigned int op       : 9;   /* [25:17] */
        unsigned int enc      : 6;   /* [31:26] */
        unsigned int src0     : 9;   /* [40:32] */
        unsigned int src1     : 9;   /* [49:41] */
        unsigned int src2     : 9;   /* [57:49] */
        unsigned int omod     : 2;   /* [59:58] */
        unsigned int neg      : 3;   /* [63:60] */
};

struct si_inst_format_vintrp_t
{
        unsigned int vsrc     : 8;   /*   [7:0] */
        unsigned int attrchan : 2;   /*   [9:8] */
        unsigned int attr     : 6;   /* [15:10] */
        unsigned int op       : 2;   /* [17:16] */
        unsigned int vdst     : 8;   /* [25:18] */
        unsigned int enc      : 6;   /* [31:26] */
};

struct si_inst_format_ds_t
{
        unsigned int offset0  : 8;   /*   [7:0] */
        unsigned int offset1  : 8;   /*  [15:8] */
        unsigned int reserved : 1;   /*     16  */
        unsigned int gds      : 1;   /*     17  */
        unsigned int op       : 8;   /* [25:18] */
        unsigned int enc      : 6;   /* [31:26] */
        unsigned int addr     : 8;   /* [39:32] */
        unsigned int data0    : 8;   /* [47:40] */
        unsigned int data1    : 8;   /* [55:48] */
        unsigned int vdst     : 8;   /* [63:56] */
};

struct si_inst_format_mtbuf_t
{
        unsigned int offset   : 12;  /*  [11:0] */
        unsigned int offen    : 1;   /*     12  */
        unsigned int idxen    : 1;   /*     13  */
        unsigned int glc      : 1;   /*     14  */
        unsigned int addr64   : 1;   /*     15  */
        unsigned int op       : 3;   /* [18:16] */
        unsigned int dfmt     : 4;   /* [22:19] */
        unsigned int nfmt     : 3;   /* [25:23] */
        unsigned int enc      : 6;   /* [31:26] */
        unsigned int vaddr    : 8;   /* [39:32] */
        unsigned int vdata    : 8;   /* [47:40] */
        unsigned int srsrc    : 5;   /* [52:48] */
        unsigned int reserved : 1;   /*     53  */
        unsigned int slc      : 1;   /*     54  */
        unsigned int tfe      : 1;   /*     55  */
        unsigned int soffset  : 8;   /* [63:56] */
};

struct si_inst_format_mubuf_t
{
        unsigned int offset    : 12;  /*  [11:0] */
        unsigned int offen     : 1;   /*     12  */
        unsigned int idxen     : 1;   /*     13  */
        unsigned int glc       : 1;   /*     14  */
        unsigned int addr64    : 1;   /*     15  */
        unsigned int lds       : 1;   /*     16  */
        unsigned int reserved0 : 1;   /*     17  */
        unsigned int op        : 7;   /* [24:18] */
        unsigned int reserved1 : 1;   /*     25  */
        unsigned int enc       : 6;   /* [31:26] */
        unsigned int vaddr     : 8;   /* [39:32] */
        unsigned int vdata     : 8;   /* [47:40] */
        unsigned int srsrc     : 5;   /* [52:48] */
        unsigned int reserved2 : 1;   /*     53  */
        unsigned int slc       : 1;   /*     54  */
        unsigned int tfe       : 1;   /*     55  */
        unsigned int soffset   : 8;   /* [63:56] */
};

struct si_inst_format_mimg_t
{
        unsigned int reserved0 : 8;   /*   [7:0] */
        unsigned int dmask     : 4;   /*  [11:8] */
        unsigned int unorm     : 1;   /*     12  */
        unsigned int glc       : 1;   /*     13  */
        unsigned int da        : 1;   /*     14  */
        unsigned int r128      : 1;   /*     15  */
        unsigned int tfe       : 1;   /*     16  */
        unsigned int lwe       : 1;   /*     17  */
        unsigned int op        : 7;   /* [24:18] */
        unsigned int slc       : 1;   /*     25  */
        unsigned int enc       : 6;   /* [31:26] */
        unsigned int vaddr     : 8;   /* [39:32] */
        unsigned int vdata     : 8;   /* [47:40] */
        unsigned int srsrc     : 5;   /* [52:48] */
        unsigned int ssamp     : 5;   /* [57:53] */
        unsigned int reserved1  : 6;   /* [63:58] */
};

struct si_inst_format_exp_t
{
        unsigned int en       : 4;   /*   [3:0] */
        unsigned int tgt      : 6;   /*   [9:4] */
        unsigned int compr    : 1;   /*     10  */
        unsigned int done     : 1;   /*     11  */
        unsigned int vm       : 1;   /*     12  */
        unsigned int reserved : 13;  /* [25:13] */
        unsigned int enc      : 6;   /* [31:26] */
        unsigned int vsrc0    : 8;   /* [39:32] */
        unsigned int vsrc1    : 8;   /* [47:40] */
        unsigned int vsrc2    : 8;   /* [55:48] */
        unsigned int vsrc3    : 8;   /* [63:56] */
};



extern struct str_map_t si_inst_category_map;
typedef enum
{
	SIInstCategoryInvalid = 0,

	/* Scalar ALU and Control Instructions */
	SIInstCategorySOP2,
	SIInstCategorySOPK,
	SIInstCategorySOP1,
	SIInstCategorySOPC,
	SIInstCategorySOPP,

	/* Scalar Memory Instruction */
	SIInstCategorySMRD,

	/* Vector ALU Instructions */
	SIInstCategoryVOP2,
	SIInstCategoryVOP1,
	SIInstCategoryVOPC,
	SIInstCategoryVOP3a,
	SIInstCategoryVOP3b,

	/* Vector Parameter Interpolation Instruction */
	SIInstCategoryVINTRP,

	/* LDS/GDS Instruction */
	SIInstCategoryDS,

	/* Vector Memory Buffer Instructions */
	SIInstCategoryMUBUF,
	SIInstCategoryMTBUF,
	
	/* Vector Memory Image Instruction */
	SIInstCategoryMIMG,

	SIInstCategoryCount
} SIInstCategory;


extern struct str_map_t si_inst_special_reg_map;
enum si_inst_special_reg_t
{
	si_inst_special_reg_invalid = 0,
	si_inst_special_reg_vcc,
	si_inst_special_reg_scc,
	si_inst_special_reg_exec,
	si_inst_special_reg_tma
};


extern struct str_map_t si_inst_buf_data_format_map;
enum si_inst_buf_data_format_t
{
	si_inst_buf_data_format_invalid = 0,
	si_inst_buf_data_format_8 = 1,
	si_inst_buf_data_format_16 = 2,
	si_inst_buf_data_format_8_8 = 3,
	si_inst_buf_data_format_32 = 4,
	si_inst_buf_data_format_16_16 = 5,
	si_inst_buf_data_format_10_11_11 = 6,
	si_inst_buf_data_format_11_10_10 = 7,
	si_inst_buf_data_format_10_10_10_2 = 8,
	si_inst_buf_data_format_2_10_10_10 = 9,
	si_inst_buf_data_format_8_8_8_8 = 10,
	si_inst_buf_data_format_32_32 = 11,
	si_inst_buf_data_format_16_16_16_16 = 12,
	si_inst_buf_data_format_32_32_32 = 13,
	si_inst_buf_data_format_32_32_32_32 = 14,
	si_inst_buf_data_format_reserved = 15
};


extern struct str_map_t si_inst_buf_num_format_map;
enum si_inst_buf_num_format_t
{
	si_inst_buf_num_format_unorm = 0,
	si_inst_buf_num_format_snorm = 1,
	si_inst_buf_num_format_unscaled = 2,
	si_inst_buf_num_format_sscaled = 3,
	si_inst_buf_num_format_uint = 4,
	si_inst_buf_num_format_sint = 5,
	si_inst_buf_num_format_snorm_nz = 6,
	si_inst_buf_num_format_float = 7,
	si_inst_buf_num_format_reserved = 8,
	si_inst_buf_num_format_srgb = 9,
	si_inst_buf_num_format_ubnorm = 10,
	si_inst_buf_num_format_ubnorm_nz = 11,
	si_inst_buf_num_format_ubint = 12,
	si_inst_buf_num_format_ubscaled = 13
};



typedef enum
{
	SIInstFlagNone = 0x0000,
	SIInstFlagOp8 = 0x0001,  /* Opcode represents 8 comparison instructions */
	SIInstFlagOp16 = 0x0002  /* Opcode represents 16 comparison instructions */
} SIInstFlag;


typedef enum
{
	SIInstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size, _flags) \
	SI_INST_##_name,

#include "asm.dat"
#undef DEFINST

	/* Max */
	SIInstOpcodeCount
} SIInstOpcode;


union si_inst_microcode_t
{
	unsigned char byte[8];
	unsigned int word[2];
	unsigned long long dword;

	struct si_inst_format_sop2_t sop2;
	struct si_inst_format_sopk_t sopk;
	struct si_inst_format_sop1_t sop1;
	struct si_inst_format_sopc_t sopc;
	struct si_inst_format_sopp_t sopp;
	struct si_inst_format_smrd_t smrd;
	struct si_inst_format_vop2_t vop2;
	struct si_inst_format_vop1_t vop1;
	struct si_inst_format_vopc_t vopc;
	struct si_inst_format_vop3a_t vop3a;
	struct si_inst_format_vop3b_t vop3b;
	struct si_inst_format_vintrp_t vintrp;
	struct si_inst_format_ds_t ds;
	struct si_inst_format_mubuf_t mubuf;
	struct si_inst_format_mtbuf_t mtbuf;
	struct si_inst_format_mimg_t mimg;
	struct si_inst_format_exp_t exp;
};


struct si_inst_info_t
{
	SIInstOpcode inst;
	SIInstCategory category;

	char *name;
	char *fmt_str;
	SIInstFormat fmt;  /* Word formats */
	int opcode;  /* Operation code */
	SIInstFlag flags;  /* Flag bitmap */
	int size;  /* Size of microcode inst (bytes) */
};


/* Table containing information for all instructions, filled out with the
 * fields found in 'asm.dat'. */
extern struct si_inst_info_t si_inst_info[SIInstOpcodeCount];


typedef union
{
	signed int as_int;
	unsigned int as_uint;

	signed short int as_short[2];
	unsigned short int as_ushort[2];

	signed char as_byte[4];
	unsigned char as_ubyte[4];

	float as_float;
} SIInstReg;


CLASS_BEGIN(SIInst, Object)

	/* Basic instruction info */
	struct si_inst_info_t *info;  /* Pointer to 'amd_inst_info' table */
	union si_inst_microcode_t micro_inst;

CLASS_END(SIInst)


void SIInstCreate(SIInst *self);
void SIInstDestroy(SIInst *self);

void SIInstClear(SIInst *self);
int SIInstDecode(SIInst *self, void *buf, unsigned int offset);

void SIInstDump(SIInst *self, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size);
void SIInstDump_SSRC(SIInst *self, unsigned int ssrc, char *operand_str, char **inst_str, int str_size);
void SIInstDump_64_SSRC(SIInst *self, unsigned int ssrc, char *operand_str, char **inst_str, int str_size);
void SIInstDump_VOP3_SRC(SIInst *self, unsigned int src, int neg, char *operand_str, char **inst_str, int str_size);
void SIInstDump_VOP3_64_SRC(SIInst *self, unsigned int src, int neg, char *operand_str, char **inst_str, int str_size);
void SIInstDump_SERIES_VDATA(unsigned int vdata, int op, char *operand_str, char **inst_str, int str_size);
void SIInstDump_MADDR(SIInst *self, char *operand_str, char **inst_str, int str_size);
void SIInstDump_DUG(SIInst *self, char *operand_str, char **inst_str, int str_size);

#endif
