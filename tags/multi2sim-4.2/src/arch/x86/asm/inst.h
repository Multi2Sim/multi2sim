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

#ifndef X86_INST_H
#define X86_INST_H

#include <stdio.h>


/* List of opcodes */
enum x86_inst_opcode_t
{
	x86_inst_opcode_invalid = 0,

#define DEFINST(name, op1, op2, op3, modrm, imm, prefixes) \
	x86_inst_##name,
#include "asm.dat"
#undef DEFINST

	x86_inst_opcode_count
};


/* Prefixes */
enum x86_inst_prefix_t
{
	x86_inst_prefix_none = 0x00,
	x86_inst_prefix_rep = 0x01,
	x86_inst_prefix_repz = 0x01,  /* same value as rep */
	x86_inst_prefix_repnz = 0x02,
	x86_inst_prefix_lock = 0x04,
	x86_inst_prefix_addr = 0x08,  /* address-size override */
	x86_inst_prefix_op = 0x10  /* operand-size override */
};


/* Registers */
extern struct str_map_t x86_inst_reg_map;
enum x86_inst_reg_t
{
	x86_inst_reg_none = 0,

	x86_inst_reg_eax,
	x86_inst_reg_ecx,
	x86_inst_reg_edx,
	x86_inst_reg_ebx,
	x86_inst_reg_esp,
	x86_inst_reg_ebp,
	x86_inst_reg_esi,
	x86_inst_reg_edi,

	x86_inst_reg_ax,
	x86_inst_reg_cx,
	x86_inst_reg_dx,
	x86_inst_reg_bx,
	x86_inst_reg_sp,
	x86_inst_reg_bp,
	x86_inst_reg_si,
	x86_inst_reg_di,

	x86_inst_reg_al,
	x86_inst_reg_cl,
	x86_inst_reg_dl,
	x86_inst_reg_bl,
	x86_inst_reg_ah,
	x86_inst_reg_ch,
	x86_inst_reg_dh,
	x86_inst_reg_bh,

	x86_inst_reg_es,
	x86_inst_reg_cs,
	x86_inst_reg_ss,
	x86_inst_reg_ds,
	x86_inst_reg_fs,
	x86_inst_reg_gs,

	x86_inst_reg_count
};


/* Flags */
enum x86_inst_flag_t
{
	x86_inst_flag_cf = 0,
	x86_inst_flag_pf = 2,
	x86_inst_flag_af = 4,
	x86_inst_flag_zf = 6,
	x86_inst_flag_sf = 7,
	x86_inst_flag_df = 10,
	x86_inst_flag_of = 11
};


/* XMM register */
union x86_inst_xmm_reg_t
{
	unsigned char as_uchar[16];
	signed char as_char[16];

	unsigned short as_ushort[8];
	signed short as_short[8];

	unsigned int as_uint[4];
	signed int as_int[4];

	unsigned long long as_uint64[2];
	signed long long as_int64[2];

	float as_float[4];
	double as_double[2];
};


/* This structure contains information derived from 'asm.dat'. */
struct x86_inst_info_t
{
	enum x86_inst_opcode_t opcode;

	unsigned int op1;
	unsigned int op2;
	unsigned int op3;
	unsigned int modrm;
	unsigned int imm;

	/* Mask of prefixes of type 'enum x86_inst_prefix_t' */
	int prefixes;

	/* Format string */
	char *fmt;

	/* Derived fields */
	unsigned int match_mask;
	unsigned int match_result;

	unsigned int nomatch_mask;
	unsigned int nomatch_result;

	int opindex_shift;  /* Pos to shift inst to obtain index of op1/op2 if any */
	int impl_reg;  /* Implied register in op1 (0-7) */
	int opcode_size;  /* Size of opcode (1 or 2), not counting the modrm part. */
	int modrm_size;  /* Size of modrm field (0 or 1) */
	int imm_size;  /* Immediate size (0, 1, 2, or 4) */
};


/* Containers for instruction infos. We need this because an info can belong to
 * different lists when there are registers embedded in the opcodes. */
struct x86_inst_info_elem_t
{
	struct x86_inst_info_t *info;
	struct x86_inst_info_elem_t *next;
};


/* x86 Instruction */
struct x86_inst_t
{
	unsigned int eip;  /* position inside the code */
	int size;  /* number of instruction bytes */
	enum x86_inst_opcode_t opcode;
	char *format;  /* format of the instruction */
	
	/* Size of fields */
	int prefix_size;
	int opcode_size;
	int modrm_size;
	int sib_size;
	int disp_size;
	int imm_size;

	/* Index in the opcode. This is a value between 0 and 7, which
	 * is present in some instructions at op1 or op2. */
	int opindex;

	/* Prefixes */
	enum x86_inst_reg_t segment;  /* Reg. used to override segment */
	int prefixes;  /* Mask of prefixes of type 'enum x86_inst_prefix_t' */
	int op_size;  /* Operand size: 2 or 4, default 4 */
	int addr_size;  /* Address size: 2 or 4, default 4 */
	
	/* ModR/M Field */
	unsigned char modrm;  /* ModR/M field */
	unsigned char modrm_mod;  /* mod field of ModR/M */
	unsigned char modrm_reg;  /* reg field of ModR/M */
	unsigned char modrm_rm;  /* rm field of ModR/M */

	/* SIB Field */
	unsigned char sib;  /* SIB field */
	unsigned char sib_scale;  /* Scale field of SIB */
	unsigned char sib_index;  /* Index field of SIB */
	unsigned char sib_base;  /* Base field of SIB */

	/* Displacement and Immediate */
	int disp;
	union {
		unsigned char b;
		unsigned short w;
		unsigned int d;
	} imm;

	/* Effective address */
	enum x86_inst_reg_t ea_base;
	enum x86_inst_reg_t ea_index;
	unsigned int ea_scale;

	/* Register */
	int reg;  /* same as modrm_reg */
};


struct x86_inst_t *x86_inst_create(void);
void x86_inst_free(struct x86_inst_t *inst);

void x86_inst_dump(struct x86_inst_t *inst, FILE *f);
void x86_inst_dump_buf(struct x86_inst_t *inst, char *buf, int size);

/* Return an instruction name given an opcode, or string '<invalid>' if the
 * opcode value does not exist. */
char *x86_inst_get_name(enum x86_inst_opcode_t opcode);

/* Populate fields of instruction 'inst' after decoding the instruction bytes
 * provided in 'buf'. The value in 'eip' should give the virtual address of
 * the instruction, used for branch decoding purposes. */
void x86_inst_decode(struct x86_inst_t *inst, unsigned int eip, void *buf);


#endif
