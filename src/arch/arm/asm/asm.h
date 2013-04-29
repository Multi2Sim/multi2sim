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

#ifndef ARM_ASM_H
#define ARM_ASM_H

#include <stdio.h>
#include "asm-thumb.h"

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

struct arm_fmt_dpr_t
{
	unsigned int op2 	: 12; /* [11:0] */
	unsigned int dst_reg 	: 4; /* [15:12] */
	unsigned int op1_reg 	: 4; /* [19:16] */
	unsigned int s_cond 	: 1; /* [20] */
	unsigned int opc_dpr 	: 4; /* [24:21] */
	unsigned int imm 	: 1; /* [25] */
	unsigned int __reserved0: 2; /* [27:26] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_dpr_sat_t
{
	unsigned int op2 	: 12; /* [11:0] */
	unsigned int dst_reg 	: 4; /* [15:12] */
	unsigned int op1_reg 	: 4; /* [19:16] */
	unsigned int s_cond 	: 1; /* [20] */
	unsigned int opc_dpr 	: 4; /* [24:21] */
	unsigned int imm 	: 1; /* [25] */
	unsigned int __reserved0: 2; /* [27:26] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_psr_t
{
	unsigned int op2 	: 12; /* [11:0] */
	unsigned int dst_reg 	: 4; /* [15:12] */
	unsigned int __reserved0: 6; /* [21:16] */
	unsigned int psr_loc	: 1; /* [22] */
	unsigned int __reserved1: 2; /* [24:23] */
	unsigned int imm 	: 1; /* [25] */
	unsigned int __reserved2: 2; /* [27:26] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_mult_t
{
	unsigned int op0_rm 	: 4; /* [3:0] */
	unsigned int __reserved0: 4; /* [7:4] */
	unsigned int op1_rs 	: 4; /* [11:8] */
	unsigned int op2_rn 	: 4; /* [15:12] */
	unsigned int dst_rd 	: 4; /* [19:16] */
	unsigned int s_cond 	: 1; /* [20] */
	unsigned int m_acc 	: 1; /* [21] */
	unsigned int __reserved1: 6; /* [27:22] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_mul_ln_t
{
	unsigned int op0_rm 	: 4; /* [3:0] */
	unsigned int __reserved0: 4; /* [7:4] */
	unsigned int op1_rs 	: 4; /* [11:8] */
	unsigned int dst_lo 	: 4; /* [15:12] */
	unsigned int dst_hi 	: 4; /* [19:16] */
	unsigned int s_cond 	: 1; /* [20] */
	unsigned int m_acc 	: 1; /* [21] */
	unsigned int sign 	: 1; /* [22] */
	unsigned int __reserved1: 5; /* [27:23] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_sngl_dswp_t
{
	unsigned int op0_rm 	: 4; /* [3:0] */
	unsigned int __reserved0: 8; /* [11:4] */
	unsigned int dst_rd 	: 4; /* [15:12] */
	unsigned int base_rn 	: 4; /* [19:16] */
	unsigned int __reserved1: 2; /* [21:20] */
	unsigned int d_type 	: 1; /* [22] */
	unsigned int __reserved2: 5; /* [27:23] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_bax_t
{
	unsigned int op0_rn 	: 4; /* [3:0] */
	unsigned int __reserved0: 24; /* [27:4] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_hfwrd_tns_reg_t
{
	unsigned int off_reg 	: 4; /* [3:0] */
	unsigned int __reserved0: 1; /* [4] */
	unsigned int sh_comb 	: 2; /* [6:5] */
	unsigned int __reserved1: 5; /* [11:7] */
	unsigned int dst_rd 	: 4; /* [15:12] */
	unsigned int base_rn 	: 4; /* [19:16] */
	unsigned int ld_st 	: 1; /* [20] */
	unsigned int wb 	: 1; /* [21] */
	unsigned int __reserved2: 1; /* [22] */
	unsigned int up_dn 	: 1; /* [23] */
	unsigned int idx_typ 	: 1; /* [24] */
	unsigned int __reserved3: 3; /* [27:25] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_hfwrd_tns_imm_t
{
	unsigned int imm_off_lo : 4; /* [3:0] */
	unsigned int __reserved0: 1; /* [4] */
	unsigned int sh_comb 	: 2; /* [6:5] */
	unsigned int __reserved1: 1; /* [7] */
	unsigned int imm_off_hi : 4; /* [11:8] */
	unsigned int dst_rd 	: 4; /* [15:12] */
	unsigned int base_rn 	: 4; /* [19:16] */
	unsigned int ld_st 	: 1; /* [20] */
	unsigned int wb 	: 1; /* [21] */
	unsigned int __reserved2: 1; /* [22] */
	unsigned int up_dn 	: 1; /* [23] */
	unsigned int idx_typ 	: 1; /* [24] */
	unsigned int __reserved3: 3; /* [27:25] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_sdtr_t
{
	unsigned int off 	: 12; /* [11:0] */
	unsigned int src_dst_rd : 4; /* [15:12] */
	unsigned int base_rn 	: 4; /* [19:16] */
	unsigned int ld_st 	: 1; /* [20] */
	unsigned int wb 	: 1; /* [21] */
	unsigned int d_type 	: 1; /* [22] */
	unsigned int up_dn 	: 1; /* [23] */
	unsigned int idx_typ 	: 1; /* [24] */
	unsigned int imm 	: 1; /* [25] */
	unsigned int __reserved0: 2; /* [27:26] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_bdtr_t
{
	unsigned int reg_lst 	: 16; /* [15:0] */
	unsigned int base_rn 	: 4; /* [19:16] */
	unsigned int ld_st 	: 1; /* [20] */
	unsigned int wb 	: 1; /* [21] */
	unsigned int psr_frc 	: 1; /* [22] */
	unsigned int up_dn 	: 1; /* [23] */
	unsigned int idx_typ 	: 1; /* [24] */
	unsigned int __reserved0: 3; /* [27:25] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_brnch_t
{
	signed int off 		: 24; /* [23:0] */
	unsigned int link 	: 1; /* [24] */
	unsigned int __reserved0: 3; /* [27:25] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_cpr_dtr_t
{
	unsigned int off 	: 8; /* [7:0] */
	unsigned int cpr_num 	: 4; /* [11:8] */
	unsigned int cpr_sr_dst : 4; /* [15:12] */
	unsigned int base_rn 	: 4; /* [19:16] */
	unsigned int ld_st 	: 1; /* [20] */
	unsigned int wb 	: 1; /* [21] */
	unsigned int tr_len 	: 1; /* [22] */
	unsigned int up_dn 	: 1; /* [23] */
	unsigned int idx_typ	: 1; /* [24] */
	unsigned int __reserved0: 3; /* [27:25] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_cpr_dop_t
{
	unsigned int cpr_op_rm 	: 4; /* [3:0] */
	unsigned int __reserved0: 1; /* [4] */
	unsigned int cpr_info 	: 3; /* [7:5] */
	unsigned int cpr_num 	: 4; /* [11:8] */
	unsigned int cpr_dst 	: 4; /* [15:12] */
	unsigned int cpr_rn 	: 4; /* [19:16] */
	unsigned int cpr_opc 	: 4; /* [23:20] */
	unsigned int __reserved1: 4; /* [27:24] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_cpr_rtr_t
{
	unsigned int cpr_op_rm 	: 4; /* [3:0] */
	unsigned int __reserved0: 1; /* [4] */
	unsigned int cpr_info 	: 3; /* [7:5] */
	unsigned int cpr_num 	: 4; /* [11:8] */
	unsigned int rd 	: 4; /* [15:12] */
	unsigned int cpr_rn 	: 4; /* [19:16] */
	unsigned int ld_st 	: 1; /* [20] */
	unsigned int cpr_opm 	: 1; /* [23:21] */
	unsigned int __reserved1: 2; /* [27:24] */
	unsigned int cond 	: 4; /* [31:28] */
};

struct arm_fmt_swi_svc_t
{
	unsigned int cmnt 	: 24; /* [23:0] */
	unsigned int __reserved0: 4;  /* [27:24] */
	unsigned int cond 	: 4;  /* [31:28] */
};

struct arm_fmt_vfp_mv_t
{
	unsigned int immd8	: 8; /* [7:0] */
	unsigned int __reserved0: 4; /* [11:8] */
	unsigned int vd		: 4; /* [15:12] */
	unsigned int vfp_rn	: 4; /* [19:16] */
	unsigned int __reserved1: 1; /* [20] */
	unsigned int w		: 1; /* [21] */
	unsigned int d		: 1; /* [22] */
	unsigned int u		: 1; /* [23] */
	unsigned int p		: 1; /* [24] */
	unsigned int __reserved2: 3; /* [27:25] */
	unsigned int cond	: 4; /* [31:28] */
};

struct arm_fmt_vfp_streg_tr_t
{
	unsigned int __reserved0:12; /* [11:0] */
	unsigned int vfp_rt	: 4; /* [15:12] */
	unsigned int __reserved1:12; /* [27:16] */
	unsigned int cond	: 4; /* [31:28] */
};

union arm_inst_dword_t
{
	unsigned char bytes[4];

	struct arm_fmt_dpr_t dpr_ins;
	struct arm_fmt_dpr_sat_t dpr_sat_ins;
	struct arm_fmt_psr_t psr_ins;
	struct arm_fmt_mult_t mult_ins;
	struct arm_fmt_mul_ln_t mul_ln_ins;
	struct arm_fmt_sngl_dswp_t sngl_dswp_ins;
	struct arm_fmt_bax_t bax_ins;
	struct arm_fmt_hfwrd_tns_reg_t hfwrd_reg_ins;
	struct arm_fmt_hfwrd_tns_imm_t hfwrd_imm_ins;
	struct arm_fmt_sdtr_t sdtr_ins;
	struct arm_fmt_bdtr_t bdtr_ins;
	struct arm_fmt_brnch_t brnch_ins;
	struct arm_fmt_cpr_dtr_t cpr_dtr_ins;
	struct arm_fmt_cpr_dop_t cpr_dop_ins;
	struct arm_fmt_cpr_rtr_t cpr_rtr_ins;
	struct arm_fmt_swi_svc_t swi_svc_ins;
	struct arm_fmt_vfp_mv_t vfp_mv_ins;
	struct arm_fmt_vfp_streg_tr_t vfp_strreg_tr_ins;
};


enum arm_inst_enum
{
	ARM_INST_NONE = 0,

#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) \
	ARM_INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
	ARM_INST_COUNT
};

enum arm_cat_enum
{
	ARM_CAT_NONE = 0,

	ARM_CAT_DPR_REG, 	/* Data Processing Register instructions */
	ARM_CAT_DPR_IMM,	/* Data Processing Immediate instructions */
	ARM_CAT_DPR_SAT, 	/* Data Processing saturation instructions */
	ARM_CAT_PSR, 		/* Processor Status Register (PSR) instructions */
	ARM_CAT_MULT,  		/* Multiplication instructions */
	ARM_CAT_MULT_SIGN,	/* Multiplication instructions */
	ARM_CAT_MULT_LN, 	/* Long Multiplication instructions */
	ARM_CAT_MULT_LN_SIGN, 	/* Long Multiplication instructions */
	ARM_CAT_SDSWP, 		/* Single Data Swap instructions */
	ARM_CAT_BAX, 		/* Branch and Exchange instructions */
	ARM_CAT_HFWRD_REG,	/* Halfword data transfer Register offset */
	ARM_CAT_HFWRD_IMM, 	/* Halfword data transfer Immediate offset */
	ARM_CAT_SDTR, 		/* Single Data Transfer instructions */
	ARM_CAT_BDTR, 		/* Block Data Transfer instructions */
	ARM_CAT_BRNCH, 		/* Branch instructions */
	ARM_CAT_CPR_DTR, 	/* Coprocessor Data Transfer instructions */
	ARM_CAT_CPR_DOP, 	/* Coprocessor Data Operation instructions */
	ARM_CAT_CPR_RTR, 	/* Coprocessor Register Transfer instructions */
	ARM_CAT_SWI_SVC, 	/* Software Interrupt / SVC Angel trap instructions */
	ARM_CAT_VFP,		/* Vector Floating Point Instructions */

	ARM_CAT_UNDEF,

	ARM_CAT_COUNT
};

struct arm_inst_info_t
{
	enum arm_inst_enum inst;
	enum arm_cat_enum category;
	char* name;
	char* fmt_str;
	unsigned int opcode;
	int size;
};

struct arm_inst_t
{
	unsigned int addr;
	union arm_inst_dword_t dword;
	struct arm_inst_info_t *info;
};

struct arm_inst_info_t arm_inst_info[(256*16)];

//!TODO Define functions for the ARM_INST

void arm_disasm_init();

void arm_inst_decode(struct arm_inst_t *inst);

void arm_inst_hex_dump(FILE *f, void *inst_ptr, unsigned int inst_addr);

void arm_inst_dump(FILE *f , char *str , int inst_str_size , void *inst_ptr ,
		unsigned int inst_index, unsigned int inst_addr);

void arm_inst_dump_RD(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_RS(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_RM(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_RN(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_RDLO(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_RDHI(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_OP2(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_COND(char **inst_str_ptr, int *inst_str_size,
                struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_OP2_PSR(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_PSR(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_AMODE_2(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_AMODE_3(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_IDX(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_BADDR(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat,
	unsigned int inst_addr);

void arm_inst_dump_REGS(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_IMMD24(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_IMMD16(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_COPR(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_AMODE_5(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_VFP_REGS(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_VFP2(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_VFP1STM(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_VFP1LDM(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_FREG(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_FP(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

void arm_inst_dump_RT(char **inst_str_ptr, int *inst_str_size,
	struct arm_inst_t *inst, enum arm_cat_enum cat);

unsigned int arm_rotl(unsigned int value, unsigned int shift);
unsigned int arm_rotr(unsigned int value, unsigned int shift);


/*
 * Arm Disassembler
 */

void arm_emu_disasm(char *path);

void arm_disasm(void *buf, unsigned int ip, volatile struct arm_inst_t *inst);
void arm_disasm_done();
void arm_inst_debug_dump(struct arm_inst_t *inst, FILE *f );

//void thumb16_disasm(void *buf, unsigned int ip, volatile struct arm_thumb16_inst_t *inst);
//void thumb32_disasm(void *buf, unsigned int ip, volatile struct arm_thumb32_inst_t *inst);

#endif
