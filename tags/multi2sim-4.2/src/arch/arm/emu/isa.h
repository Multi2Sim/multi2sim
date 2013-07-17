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

#ifndef ARCH_ARM_EMU_ISA_H
#define ARCH_ARM_EMU_ISA_H


#define arm_isa_call_debug(...) debug(arm_isa_call_debug_category, __VA_ARGS__)
#define arm_isa_inst_debug(...) debug(arm_isa_inst_debug_category, __VA_ARGS__)

extern int arm_isa_call_debug_category;
extern int arm_isa_inst_debug_category;

enum arm_isa_op2_cat_t
{
	immd = 0,
	reg
};


int arm_isa_op2_get(struct arm_ctx_t *ctx, unsigned int op2 , enum arm_isa_op2_cat_t cat);
unsigned int arm_isa_get_addr_amode2(struct arm_ctx_t *ctx);
int arm_isa_get_addr_amode3_imm(struct arm_ctx_t *ctx);
void arm_isa_reg_store(struct arm_ctx_t *ctx, unsigned int reg_no,
	int value);
void arm_isa_reg_store_safe(struct arm_ctx_t *ctx, unsigned int reg_no,
	unsigned int value);
void arm_isa_reg_load(struct arm_ctx_t *ctx, unsigned int reg_no,
	 int *value);
void arm_isa_branch(struct arm_ctx_t *ctx);
int arm_isa_check_cond(struct arm_ctx_t *ctx);
void arm_isa_amode4s_str(struct arm_ctx_t *ctx);
void arm_isa_amode4s_ld(struct arm_ctx_t *ctx);
void arm_isa_cpsr_print(struct arm_ctx_t *ctx);
unsigned int arm_isa_ret_cpsr_val(struct arm_ctx_t *ctx);
void arm_isa_set_cpsr_val(struct arm_ctx_t *ctx, unsigned int op2);
void arm_isa_subtract(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3);
void arm_isa_subtract_rev(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3);
void arm_isa_add(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3);
void arm_isa_multiply(struct arm_ctx_t *ctx);
int arm_isa_op2_carry(struct arm_ctx_t *ctx,  unsigned int op2 , enum arm_isa_op2_cat_t cat);
unsigned int arm_isa_bit_count(unsigned int ip_num);

void arm_isa_syscall(struct arm_ctx_t *ctx);

unsigned int arm_isa_invalid_addr_str(unsigned int addr, int value, struct arm_ctx_t *ctx);
unsigned int arm_isa_invalid_addr_ldr(unsigned int addr, unsigned int* value, struct arm_ctx_t *ctx);

void arm_isa_execute_inst(struct arm_ctx_t *ctx);


void arm_thumb_add_isa(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3, unsigned int flag);
void arm_thumb_isa_subtract(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3, unsigned int flag_set);
unsigned int arm_thumb32_isa_immd12(struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat);
void arm_thumb32_isa_branch_link(struct arm_ctx_t *ctx);
void arm_thumb32_isa_branch(struct arm_ctx_t *ctx);
unsigned int arm_isa_thumb_check_cond(struct arm_ctx_t *ctx, unsigned int cond);
void arm_thumb_isa_rev_subtract(struct arm_ctx_t *ctx, unsigned int rd, unsigned int rn, int op2,
	unsigned int op3, unsigned int flag_set);
void arm_thumb_isa_iteq(struct arm_ctx_t *ctx);
unsigned int arm_isa_thumb_immd_extend(unsigned int immd);


#define CALL_STACK_SIZE 10000

struct arm_isa_cstack_t
{
	char *sym_name[CALL_STACK_SIZE];
	unsigned int top;
};
struct arm_isa_cstack_t *arm_isa_cstack_create(struct arm_ctx_t *ctx);
void arm_isa_cstack_push(struct arm_ctx_t *ctx, char *str);
char *arm_isa_cstack_pop(struct arm_ctx_t *ctx);

#endif

