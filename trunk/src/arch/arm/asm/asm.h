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

#ifndef ARM_ASM_ASM_H
#define ARM_ASM_ASM_H

#include <arch/common/asm.h>
#include <lib/class/class.h>
#include <stdio.h>

#include "asm-thumb.h"
#include "inst.h"


/* Disassembly Mode */
enum arm_disassembly_mode_t
{
	ARM_DISASM = 1,
	THUMB_DISASM
};


/* Shift operators */
enum arm_op2_shift_t
{
	ARM_OPTR_LSL = 0,
	ARM_OPTR_LSR,
	ARM_OPTR_ASR,
	ARM_OPTR_ROR
};

/* Condition Fields */
enum arm_cond_codes_t
{
	EQ = 0, /* Equal */
	NE,	/* Not Equal */
	CS,	/* Unsigned higher */
	CC,	/* Unsigned Lower */
	MI,	/* Negative */
	PL,	/* Positive or Zero */
	VS,	/* Overflow */
	VC,	/* No Overflow */
	HI,	/* Unsigned Higher */
	LS,	/* Unsigned Lower */
	GE,	/* Greater or Equal */
	LT,	/* Less Than */
	GT,	/* Greater than */
	LE,	/* Less than or equal */
	AL	/* Always */
};

enum arm_user_regs_t
{
	r0 = 0,
	r1,
	r2,
	r3,
	r4,
	r5,
	r6,
	r7,
	r8,
	r9,
	r10,
	r11,
	r12,
	r13,	/* Stack Pointer sp */
	r14,	/* Link register lr */
	r15	/* Program Counter pc */
};

enum arm_psr_regs_t
{
	CPSR = 0,
	SPSR
};




/*
 * Structure of Instruction Format
 */

struct arm_inst_info_t
{
	ARMInstOpcode inst;
	ARMInstCategory category;
	char* name;
	char* fmt_str;
	unsigned int opcode;
	int size;
};


struct arm_inst_info_t arm_inst_info[(256*16)];

//!TODO Define functions for the ARM_INST




/*
 * Class 'ARMAsm'
 */

CLASS_BEGIN(ARMAsm, Asm)

CLASS_END(ARMAsm)


void ARMAsmCreate(ARMAsm *self);
void ARMAsmDestroy(ARMAsm *self);

void arm_inst_hex_dump(FILE *f, void *inst_ptr, unsigned int inst_addr);

unsigned int arm_rotl(unsigned int value, unsigned int shift);
unsigned int arm_rotr(unsigned int value, unsigned int shift);


/*
 * Arm Disassembler
 */

void arm_emu_disasm(char *path);

void arm_disasm(void *buf, unsigned int ip, volatile ARMInst *inst);

//void thumb16_disasm(void *buf, unsigned int ip, volatile struct arm_thumb16_inst_t *inst);
//void thumb32_disasm(void *buf, unsigned int ip, volatile struct arm_thumb32_inst_t *inst);

#endif
