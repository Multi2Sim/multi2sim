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

#ifndef ARM_ASM_ASM_H_
#define ARM_ASM_ASM_H_

#include <arch/common/asm.h>
#include <lib/class/class.h>
#include <stdio.h>

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

struct arm_inst_info_t
{
	ARMInstOpcode inst;
	ARMInstCategory category;
	char* name;
	char* fmt_str;
	unsigned int opcode;
	int size;
};

struct arm_thumb16_inst_info_t
{
	ARMInstThumb16Opcode inst_16;
	ARMThumb16InstCategory cat16;
	char* name;
	char* fmt_str;
	unsigned int opcode;
	int size;
	struct arm_thumb16_inst_info_t *next_table;
	int next_table_low;
	int next_table_high;
};

struct arm_thumb32_inst_info_t
{
	ARMInstThumb32Opcode inst_32;
	ARMThumb32InstCategory cat32;
	char* name;
	char* fmt_str;
	unsigned int opcode;
	int size;
	struct arm_thumb32_inst_info_t *next_table;
	int next_table_low;
	int next_table_high;
};

/* ARM instruction table */
struct arm_inst_info_t arm_inst_info[(256*16)];



/*
 * Class 'ARMAsm'
 */

CLASS_BEGIN(ARMAsm, Asm)



/* Pointers to the tables of instructions Thumb16*/

struct arm_thumb16_inst_info_t *arm_thumb16_asm_table;
struct arm_thumb16_inst_info_t *arm_thumb16_shft_ins_table;
struct arm_thumb16_inst_info_t *arm_thumb16_shft_ins_lv2_table;

struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv1_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv2_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv3_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv4_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv5_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv6_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv7_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv8_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv9_table;


struct arm_thumb16_inst_info_t *arm_thumb16_data_proc_table;
struct arm_thumb16_inst_info_t *arm_thumb16_spcl_data_brex_table;
struct arm_thumb16_inst_info_t *arm_thumb16_spcl_data_brex_lv1_table;

struct arm_thumb16_inst_info_t *arm_thumb16_ld_st_table;
struct arm_thumb16_inst_info_t *arm_thumb16_ld_st_lv1_table;
struct arm_thumb16_inst_info_t *arm_thumb16_ld_st_lv2_table;

struct arm_thumb16_inst_info_t *arm_thumb16_misc_table;
struct arm_thumb16_inst_info_t *arm_thumb16_it_table;
struct arm_thumb16_inst_info_t *current_table;



/* Pointers to the tables of instructions Thumb32 */

struct arm_thumb32_inst_info_t *arm_thumb32_asm_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv6_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv7_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv8_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv9_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv10_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv11_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv12_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv13_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv14_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_lv15_table;


struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_mul6_table;

struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst_dual_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst1_dual_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst2_dual_table;
struct arm_thumb32_inst_info_t *arm_thumb32_asm_ldst3_dual_table;

struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_shft_reg6_table;

struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_imm6_table;

struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg6_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_reg7_table;

struct arm_thumb32_inst_info_t *arm_thumb32_dproc_misc_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_misc1_table;

struct arm_thumb32_inst_info_t *arm_thumb32_st_single_table;
struct arm_thumb32_inst_info_t *arm_thumb32_st_single1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_st_single2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_st_single3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_st_single4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_st_single5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_st_single6_table;

struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte3_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte4_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte5_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_byte6_table;

struct arm_thumb32_inst_info_t *arm_thumb32_ld_hfword_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_hfword1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_hfword2_table;

struct arm_thumb32_inst_info_t *arm_thumb32_ld_word_table;
struct arm_thumb32_inst_info_t *arm_thumb32_ld_word1_table;

struct arm_thumb32_inst_info_t *arm_thumb32_mult_table;
struct arm_thumb32_inst_info_t *arm_thumb32_mult1_table;

struct arm_thumb32_inst_info_t *arm_thumb32_dproc_bin_imm_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_bin_imm1_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_bin_imm2_table;
struct arm_thumb32_inst_info_t *arm_thumb32_dproc_bin_imm3_table;

struct arm_thumb32_inst_info_t *arm_thumb32_mult_long_table;

struct arm_thumb32_inst_info_t *arm_thumb32_brnch_ctrl_table;
struct arm_thumb32_inst_info_t *arm_thumb32_brnch_ctrl1_table;

struct arm_thumb32_inst_info_t *arm_thumb32_mov_table;
struct arm_thumb32_inst_info_t *arm_thumb32_mov1_table;

CLASS_END(ARMAsm)


void ARMAsmCreate(ARMAsm *self);
void ARMAsmDestroy(ARMAsm *self);

void arm_inst_hex_dump(FILE *f, void *inst_ptr, unsigned int inst_addr);

unsigned int arm_rotl(unsigned int value, unsigned int shift);
unsigned int arm_rotr(unsigned int value, unsigned int shift);


/* Thumb Disassembler Functions */

int ARMTestThumb32(void *inst_ptr);
void ARMThumb16InstDebugDump(ARMInst *inst, FILE *f );
void ARMThumb32InstDebugDump(ARMInst *inst, FILE *f );
void ARMThumb32SetupTable(char* name , char* fmt_str ,
	ARMThumb32InstCategory cat32 , int op1 , int op2 , int op3 ,
	int op4 , int op5 , int op6, int op7, int op8, ARMInstThumb32Opcode inst_name, ARMAsm *as);
void ARMThumb16SetupTable(char* name , char* fmt_str ,
	ARMThumb16InstCategory cat16 , int op1 , int op2 , int op3 ,
	int op4 , int op5 , int op6, ARMInstThumb16Opcode inst_name, ARMAsm *as);

/*
 * Arm Disassembler
 */

void ARMAsmDisassembleBinary(ARMAsm *self, char *path);

void arm_disasm(void *buf, unsigned int ip, volatile ARMInst *inst);

//void thumb16_disasm(void *buf, unsigned int ip, volatile struct arm_thumb16_inst_t *inst);
//void thumb32_disasm(void *buf, unsigned int ip, volatile struct arm_thumb32_inst_t *inst);

#endif
