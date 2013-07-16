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

#ifndef FERMI_ASM_H
#define FERMI_ASM_H

#include <stdio.h>

/* Forward declarations */
struct elf_enc_buffer_t;


/* Microcode Formats */
enum frm_fmt_enum
{
	FRM_FMT_NONE = 0, 

#define DEFINST(_name, _fmt_str, _fmt, _category, _opcode) \
	FRM_FMT_##_fmt,
#include "asm.dat"
#undef DEFINST

	/* Max */
	FRM_FMT_COUNT
};


/* Special Registers */
enum frm_sr_enum
{
	FRM_SR_Laneld = 0, 
	FRM_SR_VirtCfg = 2, 
	FRM_SR_VirtId, 
	FRM_SR_PM0, 
	FRM_SR_PM1, 
	FRM_SR_PM2, 
	FRM_SR_PM3, 
	FRM_SR_PM4, 
	FRM_SR_PM5, 
	FRM_SR_PM6, 
	FRM_SR_PM7, 
	FRM_SR_PRIM_TYPE = 16, 
	FRM_SR_INVOCATION_ID, 
	FRM_SR_Y_DIRECTION, 
	FRM_SR_MACHINE_ID_0 = 24, 
	FRM_SR_MACHINE_ID_1, 
	FRM_SR_MACHINE_ID_2, 
	FRM_SR_MACHINE_ID_3, 
	FRM_SR_AFFINITY, 
	FRM_SR_Tid = 32,  
	FRM_SR_Tid_X, 
	FRM_SR_Tid_Y, 
	FRM_SR_Tid_Z, 
	FRM_SR_CTAParam, 
	FRM_SR_CTAid_X, 
	FRM_SR_CTAid_Y, 
	FRM_SR_CTAid_Z, 
	FRM_SR_NTid, 
	FRM_SR_NTid_X, 
	FRM_SR_NTid_Y, 
	FRM_SR_NTid_Z, 
	FRM_SR_GridParam, 
	FRM_SR_NCTAid_X, 
	FRM_SR_NCTAid_Y, 
	FRM_SR_NCTAid_Z, 
	FRM_SR_SWinLo, 
	FRM_SR_SWINSZ, 
	FRM_SR_SMemSz, 
	FRM_SR_SMemBanks, 
	FRM_SR_LWinLo, 
	FRM_SR_LWINSZ, 
	FRM_SR_LMemLoSz, 
	FRM_SR_LMemHiOff, 
	FRM_SR_EqMask, 
	FRM_SR_LtMask, 
	FRM_SR_LeMask, 
	FRM_SR_GtMask, 
	FRM_SR_GeMask, 
	FRM_SR_ClockLo = 80, 
	FRM_SR_ClockHi, 

	/* Max */
	FRM_SR_COUNT
};



/* 1st level struct */

struct frm_fmt_general0_t
{
	unsigned long long int op0 : 4; /* 3:0 */
	unsigned long long int mod0 : 6; /* 9:4 */
	unsigned long long int pred : 4; /* 13:10 */
	unsigned long long int dst : 6; /* 19:14 */
	unsigned long long int src1 : 6; /* 25:20 */
	unsigned long long int src2 : 20; /* 45:26 */
	unsigned long long int src2_mod : 2; /* 47:46 */
	unsigned long long int dst_cc : 1; /* 48 */
	unsigned long long int mod1 : 9; /* 57:49 */
	unsigned long long int op1 : 6; /* 63:58 */
};

struct frm_fmt_general1_t
{
	unsigned long long int op0 : 4; /* 3:0 */
	unsigned long long int mod0 : 6; /* 9:4 */
	unsigned long long int pred : 4; /* 13:10 */
	unsigned long long int dst : 6; /* 19:14 */
	unsigned long long int src1 : 6; /* 25:20 */
	unsigned long long int src2 : 20; /* 45:26 */
	unsigned long long int src2_mod : 2; /* 47:46 */
	unsigned long long int dst_cc : 1; /* 48 */
	unsigned long long int R : 3; /* 51:49 */
	unsigned long long int _reserved0: 1; /* 52 */
	unsigned long long int logic : 2; /* 54:53 */
	unsigned long long int cmp : 4; /* 58:55 */
	unsigned long long int op1 : 5; /* 63:59 */
};

struct frm_fmt_imm_t
{
	unsigned long long int op0 : 4; /* 3:0 */
	unsigned long long int mod0 : 6; /* 9:4 */
	unsigned long long int pred : 4; /* 13:10 */
	unsigned long long int dst : 6; /* 19:14 */
	unsigned long long int src1 : 6; /* 25:20 */
	unsigned long long int imm32 : 32; /* 57:26 */
	unsigned long long int op1 : 6; /* 63:58 */
};

struct frm_fmt_offs_t
{
	unsigned long long int op0 : 4; /* 3:0 */
	unsigned long long int mod0 : 6; /* 9:4 */
	unsigned long long int pred : 4; /* 13:10 */
	unsigned long long int dst : 6; /* 19:14 */
	unsigned long long int src1 : 6; /* 25:20 */
	unsigned long long int offset : 16; /* 41:26 */
	unsigned long long int mod1 : 16; /* 57:42 */
	unsigned long long int op1 : 6; /* 63:58 */
};

struct frm_fmt_tgt_t
{
	unsigned long long int op0 : 4; /* 3:0 */
	unsigned long long int mod0 : 6; /* 9:4 */
	unsigned long long int pred : 4; /* 13:10 */
	unsigned long long int tgt_mod : 1; /* 14 */
	unsigned long long int u : 1; /* 15 */
	unsigned long long int noinc : 1; /* 16 */
	unsigned long long int _const0 : 9; /* 25:17 */
	unsigned long long int target : 24; /* 49:26 */
	unsigned long long int _reserved0 : 8; /* 57:50 */
	unsigned long long int op1 : 6; /* 63:58 */
};

/* 2nd level struct */

struct frm_fmt_general0_mod1_A_t
{
	unsigned long long int _reserved0 : 49; /* 48:0 */
	unsigned long long int R : 4; /* 52:49 */
	unsigned long long int  _const0: 5; /* 57:53 */
	unsigned long long int _reserved1 : 6; /* 63:58 */
};

struct frm_fmt_general0_mod1_B_t
{
	unsigned long long int _reserved0 : 49; /* 48:0 */
	unsigned long long int src3 : 6; /* 54:49 */
	unsigned long long int rnd: 2; /* 56:55 */
	unsigned long long int _const0 : 6; /* 57 */
	unsigned long long int _reserved1 : 6; /* 63:58 */
};

struct frm_fmt_general0_mod1_C_t
{
	unsigned long long int _reserved0 : 49; /* 48:0 */
	unsigned long long int rnd : 2; /* 50:49 */
	unsigned long long int word0 : 7; /* 57:51 */
	unsigned long long int _reserved1 : 6; /* 63:58 */
};

struct frm_fmt_general0_mod1_D_t
{
	unsigned long long int _reserved0 : 49; /* 48:0 */
	unsigned long long int src3 : 6; /* 54:49 */
	unsigned long long int cmp : 3; /* 57:55 */
	unsigned long long int _reserved1 : 6; /* 63:58 */
};

struct frm_fmt_offs_mod1_A_t
{
	unsigned long long int _reserved0 : 42; /* 41:0 */
	unsigned long long int _const0 : 8; /* 49:42 */
	unsigned long long int trig : 1; /* 50 */
	unsigned long long int op : 4; /* 54:51 */
	unsigned long long int _const1 : 3; /* 57:55 */
	unsigned long long int _reserved1 : 6; /* 63:58 */
};

struct frm_fmt_offs_mod1_B_t
{
	unsigned long long int _reserved0 : 42; /* 41:0 */
	unsigned long long int _const0 : 8; /* 49:42 */
	unsigned long long int R : 3; /* 52:50 */
	unsigned long long int _reserved1 : 5; /* 57:53 */
	unsigned long long int _reserved2 : 6; /* 63:58 */
};

struct frm_fmt_mod0_A_t
{
	unsigned long long int _reserved0 : 4; /* 3:0 */
	unsigned long long int s : 1; /* 4 */
	unsigned long long int satftz : 1; /* 5 */
	unsigned long long int abs_src2 : 1; /* 6 */
	unsigned long long int abs_src1 : 1; /* 7 */
	unsigned long long int neg_src2 : 1; /* 8 */
	unsigned long long int neg_src1 : 1; /* 9 */
	unsigned long long int _reserved1 : 54; /* 63:10 */
};

struct frm_fmt_mod0_B_t
{
	unsigned long long int _reserved0 : 4; /* 3:0 */
	unsigned long long int s : 1; /* 4 */
	unsigned long long int type : 3; /* 7:5 */
	unsigned long long int cop : 2; /* 9:8 */
	unsigned long long int _reserved1 : 54; /* 63:10 */
};

struct frm_fmt_mod0_C_t
{
	unsigned long long int _reserved0 : 4; /* 3:0 */
	unsigned long long int s : 1; /* 4 */
	unsigned long long int shamt : 5; /* 9:5 */
	unsigned long long int _reserved1 : 54; /* 63:10 */
};

struct frm_fmt_mod0_D_t
{
	unsigned long long int _reserved0 : 4; /* 3:0 */
	unsigned long long int s : 1; /* 4 */
	unsigned long long int sat : 1; /* 5 */
	unsigned long long int ftzfmz : 2; /* 7:6 */
	unsigned long long int fma_mod : 2; /* 9:8 */
	unsigned long long int _reserved1 : 54; /* 63:10 */
};




enum frm_inst_category_enum
{
	FRM_INST_CAT_NONE = 0,

	FRM_INST_CAT_FP,  /* Floating point instructions */
	FRM_INST_CAT_INT,  /* Integer instructions */
	FRM_INST_CAT_CONV,  /* Conversion instructions */
	FRM_INST_CAT_MOV,  /* Movement instructions */
	FRM_INST_CAT_PRED,  /* Predicate/CC instructions */
	FRM_INST_CAT_TEX,  /* Texture instructions */
	FRM_INST_CAT_LDST, /* Compute load/store instructions */
	FRM_INST_CAT_SURF, /* Surface memory instructions */
	FRM_INST_CAT_CTRL, /* Control instructions */
	FRM_INST_CAT_MISC, /* Miscellaneous instructions */

	FRM_INST_CAT_COUNT
};


enum frm_inst_enum
{
	FRM_INST_NONE = 0, 

#define DEFINST(_name, _fmt_str, _fmt, _category, _opcode) \
	FRM_INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
	FRM_INST_COUNT
};

union frm_inst_dword_t
{
	unsigned char bytes[8];
	unsigned int word[2];
	unsigned long long int dword;

	struct frm_fmt_general0_t general0;	
	struct frm_fmt_general1_t general1;
	struct frm_fmt_imm_t imm;
	struct frm_fmt_offs_t offs;
	struct frm_fmt_tgt_t tgt;
	struct frm_fmt_general0_mod1_A_t general0_mod1_A;
	struct frm_fmt_general0_mod1_B_t general0_mod1_B;
	struct frm_fmt_general0_mod1_C_t general0_mod1_C;
	struct frm_fmt_general0_mod1_D_t general0_mod1_D;
	struct frm_fmt_offs_mod1_A_t offs_mod1_A;
	struct frm_fmt_offs_mod1_B_t offs_mod1_B;
	struct frm_fmt_mod0_A_t mod0_A;
	struct frm_fmt_mod0_B_t mod0_B;
	struct frm_fmt_mod0_C_t mod0_C;
	struct frm_fmt_mod0_D_t mod0_D;
};



struct frm_inst_info_t
{
	enum frm_inst_enum inst;
	enum frm_inst_category_enum category;
	char *name;
	char *fmt_str;
	enum frm_fmt_enum fmt;
	unsigned int opcode;
	int size;
};

/* Table containing information for all instructions, filled out with the
 * fields found in 'asm.dat'. */
extern struct frm_inst_info_t frm_inst_info[FRM_INST_COUNT];

struct frm_inst_t
{
	unsigned int addr;
	union frm_inst_dword_t dword;
	struct frm_inst_info_t *info;
};

void frm_disasm_init(void);
void frm_disasm_done(void);

void frm_inst_dump(char *str, int inst_str_size, void *buf, int inst_index);
void frm_inst_hex_dump(FILE *f, unsigned char *buf, int inst_index);




/* Copy instruction */
void frm_inst_copy(struct frm_inst_t *dest, struct frm_inst_t *src);

/* Obtaining source operand fields for ALU instructions */
void frm_inst_get_op_src(struct frm_inst_t *inst, int src_idx,
        int *sel, int *rel, int *chan, int *neg, int *abs);

void frm_inst_decode(struct frm_inst_t *inst);

void frm_disasm(char *path);

/* a temporial implementation to disassmbler only text_section */
void frm_disasm_text_section_buffer(struct elf_enc_buffer_t *buffer);

#endif






