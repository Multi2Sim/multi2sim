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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_H
#define ARCH_SOUTHERN_ISLANDS_ASM_H

#include <stdio.h>


/* Microcode Formats
 * NOTE: Update string map 'si_inst_fmt_map' if new elements are added
 * to this enumeration. */
enum si_fmt_enum
{
	SI_FMT_NONE = 0,

	/* Scalar ALU and Control Formats */
	SI_FMT_SOP2,
	SI_FMT_SOPK,
	SI_FMT_SOP1,
	SI_FMT_SOPC,
	SI_FMT_SOPP,

	/* Scalar Memory Format */
	SI_FMT_SMRD,

	/* Vector ALU Formats */
	SI_FMT_VOP2,
	SI_FMT_VOP1,
	SI_FMT_VOPC,
	SI_FMT_VOP3a,
	SI_FMT_VOP3b,

	/* Vector Parameter Interpolation Format */
	SI_FMT_VINTRP,

	/* LDS/GDS Format */
	SI_FMT_DS,

	/* Vector Memory Buffer Formats */
	SI_FMT_MUBUF,
	SI_FMT_MTBUF,
	
	/* Vector Memory Image Format */
	SI_FMT_MIMG,

	/* Export Formats */
	SI_FMT_EXP,

	/* Max */
	SI_FMT_COUNT
};



/*
 * String maps
 */

extern struct str_map_t si_inst_fmt_map;
extern struct str_map_t si_inst_sdst_map;
extern struct str_map_t si_inst_ssrc_map;
extern struct str_map_t si_inst_OP16_map;
extern struct str_map_t si_inst_OP8_map;




/*
 * Structure of Microcode Format
 */

struct si_fmt_sop2_t
{
        unsigned int ssrc0    : 8;  /*   [7:0] */
        unsigned int ssrc1    : 8;  /*  [15:8] */
        unsigned int sdst     : 7;  /* [22:16] */
        unsigned int op       : 7;  /* [29:23] */
        unsigned int enc      : 2;  /* [31:30] */
        unsigned int lit_cnst : 32; /* [63:32] */
};

struct si_fmt_sopk_t
{
        unsigned int simm16 : 16;  /*  [15:0] */
        unsigned int sdst   : 7;   /* [22:16] */
        unsigned int op     : 5;   /* [27:23] */
        unsigned int enc    : 4;   /* [31:28] */
};

struct si_fmt_sop1_t
{
        unsigned int ssrc0    : 8;  /*   [7:0] */
        unsigned int op       : 8;  /*  [15:8] */
        unsigned int sdst     : 7;  /* [22:16] */
        unsigned int enc      : 9;  /* [31:23] */
        unsigned int lit_cnst : 32; /* [63:32] */
};

struct si_fmt_sopc_t
{
        unsigned int ssrc0    : 8;  /*   [7:0] */
        unsigned int ssrc1    : 8;  /*  [15:8] */
        unsigned int op       : 7;  /* [22:16] */
        unsigned int enc      : 9;  /* [31:23] */
        unsigned int lit_cnst : 32; /* [63:32] */
};

struct si_fmt_sopp_t
{
        unsigned int simm16 : 16;  /*  [15:0] */
        unsigned int op     : 7;   /* [22:16] */
        unsigned int enc    : 9;   /* [31:23] */
};

struct si_fmt_smrd_t 
{
        unsigned int offset : 8;   /*   [7:0] */
        unsigned int imm    : 1;   /*      8  */
        unsigned int sbase  : 6;   /*  [14:9] */
        unsigned int sdst   : 7;   /* [21:15] */
        unsigned int op     : 5;   /* [26:22] */
        unsigned int enc    : 5;   /* [31:27] */
};

struct si_fmt_vop2_t
{
        unsigned int src0     : 9;   /*   [8:0] */
        unsigned int vsrc1    : 8;   /*  [16:9] */
        unsigned int vdst     : 8;   /* [24:17] */
        unsigned int op       : 6;   /* [30:25] */
        unsigned int enc      : 1;   /*     31  */
        unsigned int lit_cnst : 32;  /* [63:32] */
};

struct si_fmt_vop1_t
{
        unsigned int src0     : 9;   /*   [8:0] */
        unsigned int op       : 8;   /*  [16:9] */
        unsigned int vdst     : 8;   /* [24:17] */
        unsigned int enc      : 7;   /* [31:25] */
        unsigned int lit_cnst : 32;  /* [63:32] */
};

struct si_fmt_vopc_t
{
        unsigned int src0     : 9;   /*   [8:0] */
        unsigned int vsrc1    : 8;   /*  [16:9] */
        unsigned int op       : 8;   /* [24:17] */
        unsigned int enc      : 7;   /* [31:25] */
        unsigned int lit_cnst : 32;  /* [63:32] */
};

struct si_fmt_vop3a_t
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

struct si_fmt_vop3b_t
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

struct si_fmt_vintrp_t
{
        unsigned int vsrc     : 8;   /*   [7:0] */
        unsigned int attrchan : 2;   /*   [9:8] */
        unsigned int attr     : 6;   /* [15:10] */
        unsigned int op       : 2;   /* [17:16] */
        unsigned int vdst     : 8;   /* [25:18] */
        unsigned int enc      : 6;   /* [31:26] */
};

struct si_fmt_ds_t
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

struct si_fmt_mtbuf_t
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

struct si_fmt_mubuf_t
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

struct si_fmt_mimg_t
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

struct si_fmt_exp_t
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
enum si_inst_category_t
{
	SI_INST_CAT_NONE = 0,

	/* Scalar ALU and Control Instructions */
	SI_INST_CAT_SOP2,
	SI_INST_CAT_SOPK,
	SI_INST_CAT_SOP1,
	SI_INST_CAT_SOPC,
	SI_INST_CAT_SOPP,

	/* Scalar Memory Instruction */
	SI_INST_CAT_SMRD,

	/* Vector ALU Instructions */
	SI_INST_CAT_VOP2,
	SI_INST_CAT_VOP1,
	SI_INST_CAT_VOPC,
	SI_INST_CAT_VOP3a,
	SI_INST_CAT_VOP3b,

	/* Vector Parameter Interpolation Instruction */
	SI_INST_CAT_VINTRP,

	/* LDS/GDS Instruction */
	SI_INST_CAT_DS,

	/* Vector Memory Buffer Instructions */
	SI_INST_CAT_MUBUF,
	SI_INST_CAT_MTBUF,
	
	/* Vector Memory Image Instruction */
	SI_INST_CAT_MIMG,

	SI_INST_CAT_COUNT
};


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



enum si_inst_flag_t
{
	SI_INST_FLAG_NONE = 0x0000,
	SI_INST_FLAG_OP8 = 0x0001,  /* Opcode represents 8 comparison instructions */
	SI_INST_FLAG_OP16 = 0x0002  /* Opcode represents 16 comparison instructions */
};


enum si_inst_opcode_t
{
	SI_INST_NONE = 0,

#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size, _flags) \
	SI_INST_##_name,

#include "asm.dat"
#undef DEFINST

	/* Max */
	SI_INST_COUNT
};


union si_inst_microcode_t
{
	char bytes[8];

	struct si_fmt_sop2_t sop2;
	struct si_fmt_sopk_t sopk;
	struct si_fmt_sop1_t sop1;
	struct si_fmt_sopc_t sopc;
	struct si_fmt_sopp_t sopp;
	struct si_fmt_smrd_t smrd;
	struct si_fmt_vop2_t vop2;
	struct si_fmt_vop1_t vop1;
	struct si_fmt_vopc_t vopc;
	struct si_fmt_vop3a_t vop3a;
	struct si_fmt_vop3b_t vop3b;
	struct si_fmt_vintrp_t vintrp;
	struct si_fmt_ds_t ds;
	struct si_fmt_mubuf_t mubuf;
	struct si_fmt_mtbuf_t mtbuf;
	struct si_fmt_mimg_t mimg;
	struct si_fmt_exp_t exp;
};


struct si_inst_info_t
{
	enum si_inst_opcode_t inst;
	enum si_inst_category_t category;
	char *name;
	char *fmt_str;
	enum si_fmt_enum fmt;  /* Word formats */
	int opcode;  /* Operation code */
	enum si_inst_flag_t flags;  /* Flag bitmap */
	int size;  /* Size of microcode inst (bytes) */
};


/* Table containing information for all instructions, filled out with the
 * fields found in 'asm.dat'. */
extern struct si_inst_info_t si_inst_info[SI_INST_COUNT];


union si_reg_t
{
	signed int as_int;
	unsigned int as_uint;

	signed short int as_short[2];
	unsigned short int as_ushort[2];

	signed char as_byte[4];
	unsigned char as_ubyte[4];

	float as_float;
};

struct si_inst_t
{
	/* Basic instruction info */
	struct si_inst_info_t *info;  /* Pointer to 'amd_inst_info' table */
	union si_inst_microcode_t micro_inst;
};

typedef void (*si_fmt_dump_func_t)(void *buf, FILE *);

void si_disasm_init(void);
void si_disasm_done(void);

struct elf_buffer_t;
void si_disasm_buffer(struct elf_buffer_t *buffer, FILE *f);

void si_disasm(char* path);
int si_inst_decode(void *buf, struct si_inst_t *inst, unsigned int offset);

/* Functions to dump individual instruction lines for decoded instructions. */
#define MAX_INST_STR_SIZE 200

void si_inst_dump(struct si_inst_t *inst, unsigned int inst_size, unsigned int rel_addr, void *buf, char *line, int line_size);
void si_inst_SSRC_dump(struct si_inst_t *inst, unsigned int ssrc, char *operand_str, char **inst_str, int str_size);
void si_inst_64_SSRC_dump(struct si_inst_t *inst, unsigned int ssrc, char *operand_str, char **inst_str, int str_size);
void si_inst_VOP3_SRC_dump(struct si_inst_t *inst, unsigned int src, int neg, char *operand_str, char **inst_str, int str_size);
void si_inst_VOP3_64_SRC_dump(struct si_inst_t *inst, unsigned int src, int neg, char *operand_str, char **inst_str, int str_size);
void si_inst_SERIES_VDATA_dump(unsigned int vdata, int op, char *operand_str, char **inst_str, int str_size);
void si_inst_MADDR_dump(struct si_inst_t *inst, char *operand_str, char **inst_str, int str_size);
void si_inst_DUG_dump(struct si_inst_t *inst, char *operand_str, char **inst_str, int str_size);

#endif

