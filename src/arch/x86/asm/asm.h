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

#ifndef X86_ASM_H
#define X86_ASM_H

#include <stdio.h>
#include <stdint.h>

#include <lib/mhandle/mhandle.h>


/* List of opcodes */
enum x86_opcode_t
{
	x86_op_none = 0,

#define DEFINST(name,op1,op2,op3,modrm,imm,prefixes) op_##name,
#include "asm.dat"
#undef DEFINST

	x86_opcode_count
};


/* Prefixes */
enum x86_prefix_t
{
	x86_prefix_none = 0x00,
	x86_prefix_rep = 0x01,
	x86_prefix_repz = 0x01,  /* same value as rep */
	x86_prefix_repnz = 0x02,
	x86_prefix_lock = 0x04,
	x86_prefix_addr = 0x08,  /* address-size override */
	x86_prefix_op = 0x10  /* operand-size override */
};


/* Register identifiers and names. */
enum x86_reg_t
{
	x86_reg_none = 0,

	x86_reg_eax,
	x86_reg_ecx,
	x86_reg_edx,
	x86_reg_ebx,
	x86_reg_esp,
	x86_reg_ebp,
	x86_reg_esi,
	x86_reg_edi,

	x86_reg_ax,
	x86_reg_cx,
	x86_reg_dx,
	x86_reg_bx,
	x86_reg_sp,
	x86_reg_bp,
	x86_reg_si,
	x86_reg_di,

	x86_reg_al,
	x86_reg_cl,
	x86_reg_dl,
	x86_reg_bl,
	x86_reg_ah,
	x86_reg_ch,
	x86_reg_dh,
	x86_reg_bh,

	x86_reg_es,
	x86_reg_cs,
	x86_reg_ss,
	x86_reg_ds,
	x86_reg_fs,
	x86_reg_gs,

	x86_reg_count
};

extern char *x86_reg_name[x86_reg_count];


/* Names of flags */
enum x86_flag_t
{
	x86_flag_cf = 0,
	x86_flag_pf = 2,
	x86_flag_af = 4,
	x86_flag_zf = 6,
	x86_flag_sf = 7,
	x86_flag_df = 10,
	x86_flag_of = 11
};

/* XMM register */
union x86_xmm_reg_t
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


/* x86 Instruction */
struct x86_inst_t
{
	unsigned int eip;  /* position inside the code */
	int size;  /* number of instruction bytes */
	enum x86_opcode_t opcode;
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
	enum x86_reg_t segment;  /* Reg. used to override segment */
	int prefixes;  /* Mask of prefixes of type 'enum x86_prefix_t' */
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
	enum x86_reg_t ea_base;
	enum x86_reg_t ea_index;
	unsigned int ea_scale;

	/* Register */
	int reg;  /* same as modrm_reg */
};

void x86_inst_dump_buf(struct x86_inst_t *inst, char *buf, int size);
void x86_inst_dump(struct x86_inst_t *inst, FILE *f);
char *x86_inst_name(enum x86_opcode_t opcode);




/*
 * X86 Disassembler
 */

/* Initialization and finalization routines */
void x86_disasm_init(void);
void x86_disasm_done(void);

/* Disassemble and dump */
void x86_disasm(void *buf, uint32_t eip, volatile struct x86_inst_t *inst);
void x86_disasm_file(char *file_name);


#endif
