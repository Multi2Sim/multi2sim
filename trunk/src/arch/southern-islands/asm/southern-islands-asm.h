/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef SOUTHERN_ISLANDS_ASM_H
#define SOUTHERN_ISLANDS_ASM_H

#include <stdio.h>
#include "elf-format.h"


/* Microcode Formats */
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

	/* Max */
	SI_FMT_COUNT
};



/*
 * String maps
 */

/* TODO Replace with SI maps */
/*
extern struct string_map_t si_pv_map;
extern struct string_map_t si_alu_map;
extern struct string_map_t si_bank_swizzle_map;
extern struct string_map_t si_rat_inst_map;
extern struct string_map_t si_cf_cond_map;
extern struct string_map_t si_src_sel_map;
extern struct string_map_t si_dst_sel_map;

extern struct string_map_t si_fmt_vtx_fetch_type_map;
extern struct string_map_t si_fmt_vtx_data_format_map;
extern struct string_map_t si_fmt_vtx_num_format_map;
extern struct string_map_t si_fmt_vtx_format_comp_map;
extern struct string_map_t si_fmt_vtx_srf_mode_map;
extern struct string_map_t si_fmt_vtx_endian_swap_map;
extern struct string_map_t si_fmt_lds_op_map;
*/



/*
 * Structure of Microcode Format
 */

struct si_fmt_sop2_t
{
        unsigned int ssrc0 : 8;  /*   [7:0] */
        unsigned int ssrc1 : 8;  /*  [15:8] */
        unsigned int sdst  : 7;  /* [22:16] */
        unsigned int op    : 7;  /* [29:23] */
        unsigned int enc   : 2;  /* [31:30] */
	int lit_cnst	   : 32; /* [63:32] */
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
        unsigned int ssrc0 : 8;  /*   [7:0] */
        unsigned int op    : 8;  /*  [15:8] */
        unsigned int sdst  : 7;  /* [22:16] */
        unsigned int enc   : 9;  /* [31:23] */
};

struct si_fmt_sopc_t
{
        unsigned int ssrc0 : 8;  /*   [7:0] */
        unsigned int ssrc1 : 8;  /*  [15:8] */
        unsigned int op    : 7;  /* [22:16] */
        unsigned int enc   : 9;  /* [31:32] */
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
        unsigned int src0  : 9;   /*   [8:0] */
        unsigned int vsrc1 : 8;   /*  [16:9] */
        unsigned int vdst  : 8;   /* [24:17] */
        unsigned int op    : 6;   /* [30:25] */
        unsigned int enc   : 1;   /*     31  */
	int lit_cnst       : 32;  /* [63:32] */
};

struct si_fmt_vop1_t
{
        unsigned int src0 : 9;   /*   [8:0] */
        unsigned int op   : 8;   /*  [16:9] */
        unsigned int vdst : 8;   /* [24:17] */
        unsigned int enc  : 7;   /* [31:25] */
	int lit_cnst      : 32;  /* [63:32] */
};

struct si_fmt_vopc_t
{
        unsigned int src0  : 9;   /*   [8:0] */
        unsigned int vsrc1 : 8;   /*  [16:9] */
        unsigned int op    : 8;   /* [24:17] */
        unsigned int enc   : 7;   /* [31:25] */
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

struct si_fmt_mubuf_t
{
        unsigned int offset   : 12;  /*  [11:0] */
        unsigned int offen    : 1;   /*     12  */
        unsigned int index    : 1;   /*     13  */
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

struct si_fmt_mtbuf_t
{
        unsigned int offset    : 12;  /*  [11:0] */
        unsigned int offen     : 1;   /*     12  */
        unsigned int index     : 1;   /*     13  */
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



extern struct string_map_t si_inst_category_map;

enum si_inst_category_enum
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


#if 0
/* TODO Replace these with SI flags for emulation */
enum si_inst_flag_enum
{
	SI_INST_FLAG_NONE         = 0x0000,
	SI_INST_FLAG_TRANS_ONLY   = 0x0001,  /* Only executable in transcendental unit */
	SI_INST_FLAG_INC_LOOP_IDX = 0x0002,  /* CF inst increasing loop depth index */
	SI_INST_FLAG_DEC_LOOP_IDX = 0x0004,  /* CF inst decreasing loop index */
	SI_INST_FLAG_DST_INT      = 0x0008,  /* Inst with integer dest operand */
	SI_INST_FLAG_DST_UINT     = 0x0010,  /* Inst with unsigned int dest op */
	SI_INST_FLAG_DST_FLOAT    = 0x0020,  /* Inst with float dest op */
	SI_INST_FLAG_ACT_MASK     = 0x0040,  /* Inst affects the active mask (control flow) */
	SI_INST_FLAG_LDS          = 0x0080,  /* Access to local memory */
	SI_INST_FLAG_MEM          = 0x0100,  /* Access to global memory */
	SI_INST_FLAG_MEM_READ     = 0x0200,  /* Read to global memory */
	SI_INST_FLAG_MEM_WRITE    = 0x0400,  /* Write to global memory */
	SI_INST_FLAG_PRED_MASK    = 0x0800   /* Inst affects the predicate mask */
};
#endif 


enum si_inst_enum
{
	SI_INST_NONE = 0,

#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size) \
	SI_INST_##_name,
#include "southern-islands-asm.dat"
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


#define SI_INST_MAX_WORDS  2

struct si_inst_info_t
{
	enum si_inst_enum inst;
	enum si_inst_category_enum category;
	char *name;
	char *fmt_str;
	enum si_fmt_enum fmt;  /* Word formats */
	int opcode;  /* Operation code */
	//enum si_inst_flag_enum flags;  /* Flag bitmap */
	/* FIXME Can we add the size to the .dat file? */
	int size;  /* Size of microcode inst (bytes) */
};

union si_reg_t
{
	signed int as_int;
	unsigned int as_uint;

	signed short int as_short[2];
	unsigned short int as_ushort[2];

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
void si_disasm_buffer(struct elf_buffer_t *buffer, FILE *f);

#if 0
/* TODO Remove? */
void si_inst_slot_dump_buf(struct si_inst_t *inst, int count, int loop_idx, int slot, char *buf, int size);
void si_inst_dump_buf(struct si_inst_t *inst, int count, int loop_idx, char *buf, int size);

void si_inst_word_dump(void *buf, enum si_fmt_enum fmt, FILE *f);
void si_inst_dump_gpr(int gpr, int rel, int chan, int im, FILE *f);
void si_inst_slot_dump(struct si_inst_t *inst, int count, int loop_idx, int slot, FILE *f);
void si_inst_dump(struct si_inst_t *inst, int count, int loop_idx, FILE *f);
void si_inst_dump_debug(struct si_inst_t *inst, int count, int loop_idx, FILE *f);
void si_inst_words_dump(struct si_inst_t *inst, FILE *f);

/* Copy instruction */
void si_inst_copy(struct si_inst_t *dest, struct si_inst_t *src);

/* Obtaining source operand fields for ALU instructions */
void si_inst_get_op_src(struct si_inst_t *inst, int src_idx,
	int *sel, int *rel, int *chan, int *neg, int *abs);
#endif


/*
 * AMD Southern Islands Binary File (Internal ELF)
 */


/* Encoding dictionary entry header (as encoded in ELF file) */
struct si_bin_enc_dict_entry_header_t
{
	Elf32_Word d_machine;
	Elf32_Word d_type;
	Elf32_Off d_offset;  /* Offset for encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_size;  /* Size of encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_flags;
};


/* Constats embedded in the '.data' section */
struct si_bin_enc_dict_entry_consts_t
{
	float float_consts[256][4];
	unsigned int int_consts[32][4];
	unsigned int bool_consts[32];
};


/* Encoding dictionary entry */
struct si_bin_enc_dict_entry_t
{
	/* Header (pointer to ELF buffer contents) */
	struct si_bin_enc_dict_entry_header_t *header;

	/* Buffers containing PT_LOAD and PT_NOTE segments */
	struct elf_buffer_t pt_load_buffer;
	struct elf_buffer_t pt_note_buffer;

	/* Buffers containing sections */
	struct elf_buffer_t sec_text_buffer;
	struct elf_buffer_t sec_data_buffer;
	struct elf_buffer_t sec_symtab_buffer;
	struct elf_buffer_t sec_strtab_buffer;

	/* Constants extract from '.data' section */
	struct si_bin_enc_dict_entry_consts_t *consts;

	/* Info read from pt_notes */
	int num_gpr_used;
	int lds_size_used;
	int stack_size_used;
};


/* Binary file */
struct si_bin_file_t
{
	/* Associated ELF file */
	struct elf_file_t *elf_file;

	/* Encoding dictionary.
	 * Elements are of type 'struct si_bin_enc_dict_entry_t'
	 * Each element of the dictionary contains the binary for a different architecture
	 * (Evergreen, x86, etc.) */
	struct list_t *enc_dict;

	/* Encoding dictionary entry containing the Southern Islands kernel.
	 * This is a member of the 'enc_dict' list. */
	struct si_bin_enc_dict_entry_t *enc_dict_entry_southern_islands;
};

struct si_bin_file_t *si_bin_file_create(void *ptr, int size, char *name);
void si_bin_file_free(struct si_bin_file_t *bin);
void si_disasm(char* path);
int si_inst_decode(void *buf, struct si_inst_t *inst);

#endif

