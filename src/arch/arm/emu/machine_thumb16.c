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

#include <lib/util/debug.h>
#include <mem-system/memory.h>

#include "context.h"
#include "isa.h"
#include "machine.h"
#include "regs.h"


char *arm_th16_err_isa_note =
	"\tThe ARM instruction set is partially supported by Multi2Sim. If\n"
	"\tyour program is using an unimplemented instruction, please email\n"
	"\t'development@multi2sim.org' to request support for it.\n";

#define __ARM_TH16_NOT_IMPL__ \
	fatal("%s: Arm instruction '%s' not implemented\n%s", \
		__FUNCTION__, ctx->inst.info->name, arm_th16_err_isa_note);




void arm_th16_isa_LSL_imm_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LSR_imm_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ASR_imm_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_MOV_imm_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CMP_imm_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_imm_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SUB_imm_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SUB_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SUB_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_AND_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_EOR_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LSL_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LSR_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ASR_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADC_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SBC_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ROR_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_TST_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_RSB_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CMP_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CMN_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ORR_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_MUL_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_BIC_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_MVN_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_reg_lo_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_reg_hi1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_reg_hi2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CMP_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CMP_reg_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_MOV_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_MOV_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_BX_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_BLX_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_lit1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_lit2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRH_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRB_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRSB_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRH_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRB_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRSH_reg1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_imm1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_imm1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRB_imm1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRB_imm1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRH_imm1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRH_imm1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_imm2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_imm2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRH_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRB_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRSB_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRH_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRB_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRSH_reg2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRB_imm2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRB_imm2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRH_imm2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRH_imm2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_imm4_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_imm4_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRH_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRB_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRSB_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRH_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRB_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRSH_reg3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_immd5_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_immd5_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRB_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRB_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STRH_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDRH_immd3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STR_imm6_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDR_imm6_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADR_PC_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_SP_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_STM_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_LDM_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SVC_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_0_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_4_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_5_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_6_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_7_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_8_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_9_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_10_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_11_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_12_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_13_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_14_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_B_15_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_SP1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_SP2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_SP3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_ADD_SP4_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SUB_SP1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SUB_SP2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SUB_SP3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SUB_SP4_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SXTH_SP1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SXTH_SP2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SXTB_SP1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_SXTB_SP2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_UXTH_SP1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_UXTH_SP2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_UXTB_SP1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_UXTB_SP2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_0_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_4_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_5_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_6_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_7_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_8_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_9_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_10_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_11_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_12_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_13_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_14_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_PUSH_15_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_0_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_2_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_3_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_4_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_5_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_6_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_7_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_8_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_9_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_10_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_11_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_12_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_13_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_14_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_POP_15_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_8_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_9_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_10_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_11_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_12_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_13_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_14_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_15_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_18_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_19_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_110_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_111_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_112_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_113_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_114_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBZ_115_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_58_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_59_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_510_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_511_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_512_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_513_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_514_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_515_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_48_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_49_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_410_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_411_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_412_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_413_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_414_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_CBNZ_415_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_NOP_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_41_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_42_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_43_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_44_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_45_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_46_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_47_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_48_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_49_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_410_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_411_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_412_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_413_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_414_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_415_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_51_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_52_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_53_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_54_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_55_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_56_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_57_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_58_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_59_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_510_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_511_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_512_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_513_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_514_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_515_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_61_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_62_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_63_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_64_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_65_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_66_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_67_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_68_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_69_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_610_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_611_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_612_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_613_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_614_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_615_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_71_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_72_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_73_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_74_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_75_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_76_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_77_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_78_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_79_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_710_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_711_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_712_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_713_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_714_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_715_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_81_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_82_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_83_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_84_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_85_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_86_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_87_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_88_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_89_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_810_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_811_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_812_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_813_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_814_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_815_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_91_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_92_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_93_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_94_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_95_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_96_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_97_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_98_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_99_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_910_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_911_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_912_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_913_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_914_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_915_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_101_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_102_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_103_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_104_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_105_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_106_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_107_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_108_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_109_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1010_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1011_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1012_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1013_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1014_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1015_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_111_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_112_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_113_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_114_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_115_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_116_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_117_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_118_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_119_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1110_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1111_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1112_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1113_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1114_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_IT_1115_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_REV_0_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}

void arm_th16_isa_REV_1_impl(struct arm_ctx_t *ctx)
{
 __ARM_TH16_NOT_IMPL__
}
