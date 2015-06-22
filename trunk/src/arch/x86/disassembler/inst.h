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

#ifndef ARCH_X86_ASM_INST_OLD_H
#define ARCH_X86_ASM_INST_OLD_H

#include <stdio.h>

#include <lib/class/class.h>



/*
 * Class 'X86Inst'
 */

/* List of opcodes */
typedef enum
{
	X86InstOpcodeInvalid = 0,

#define DEFINST(name, op1, op2, op3, modrm, imm, prefixes) \
	x86_inst_##name,
#include "Inst.def"
#undef DEFINST

	X86InstOpcodeCount
} X86InstOpcode;


/* Prefixes */
typedef enum
{
	X86InstPrefixNone = 0x00,
	X86InstPrefixRep = 0x01,
	X86InstPrefixRepz = 0x01,  /* same value as rep */
	X86InstPrefixRepnz = 0x02,
	X86InstPrefixLock = 0x04,
	X86InstPrefixAddr = 0x08,  /* address-size override */
	X86InstPrefixOp = 0x10  /* operand-size override */
} X86InstPrefix;


/* Registers */
extern struct str_map_t x86_inst_reg_map;
typedef enum
{
	X86InstRegNone = 0,

	X86InstRegEax,
	X86InstRegEcx,
	X86InstRegEdx,
	X86InstRegEbx,
	X86InstRegEsp,
	X86InstRegEbp,
	X86InstRegEsi,
	X86InstRegEdi,

	X86InstRegAx,
	X86InstRegCx,
	X86InstRegDx,
	X86InstRegBx,
	X86InstRegSp,
	X86InstRegBp,
	X86InstRegSi,
	X86InstRegDi,

	X86InstRegAl,
	X86InstRegCl,
	X86InstRegDl,
	X86InstRegBl,
	X86InstRegAh,
	X86InstRegCh,
	X86InstRegDh,
	X86InstRegBh,

	X86InstRegEs,
	X86InstRegCs,
	X86InstRegSs,
	X86InstRegDs,
	X86InstRegFs,
	X86InstRegGs,

	X86InstRegCount
} X86InstReg;


/* Flags */
typedef enum
{
	X86InstFlagCF = 0,
	X86InstFlagPF = 2,
	X86InstFlagAF = 4,
	X86InstFlagZF = 6,
	X86InstFlagSF = 7,
	X86InstFlagDF = 10,
	X86InstFlagOF = 11
} X86InstFlag;


/* XMM register */
typedef union
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
} X86InstXMMReg;


/* x86 Instruction */
CLASS_BEGIN(X86Inst, Object)

	/* X86 assembler used to decode instruction */
	X86Asm *as;

	unsigned int eip;  /* position inside the code */
	int size;  /* number of instruction bytes */
	X86InstOpcode opcode;
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
	X86InstReg segment;  /* Reg. used to override segment */
	int prefixes;  /* Mask of prefixes of type 'X86InstPrefix' */
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
	X86InstReg ea_base;
	X86InstReg ea_index;
	unsigned int ea_scale;

	/* Register */
	int reg;  /* same as modrm_reg */

CLASS_END(X86Inst)


void X86InstCreate(X86Inst *self, X86Asm *as);
void X86InstDestroy(X86Inst *self);

void X86InstDump(X86Inst *self, FILE *f);
void X86InstDumpBuf(X86Inst *self, char *buf, int size);

/* Clear all fields. This function is useful to reuse the same instruction
 * object among different calls to X86InstDecode for efficiency. */
void X86InstClear(X86Inst *self);

/* Populate fields of instruction 'inst' after decoding the instruction bytes
 * provided in 'buf'. The value in 'eip' should give the virtual address of
 * the instruction, used for branch decoding purposes. */
void X86InstDecode(X86Inst *self, unsigned int eip, void *buf);




/*
 * Public
 */


/* This structure contains information derived from 'Inst.def'. */
struct x86_inst_info_t
{
	X86InstOpcode opcode;

	unsigned int op1;
	unsigned int op2;
	unsigned int op3;
	unsigned int modrm;
	unsigned int imm;

	/* Mask of prefixes of type 'X86InstPrefix' */
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


#endif
