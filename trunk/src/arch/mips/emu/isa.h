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

#ifndef ARCH_MIPS_EMU_ISA_H
#define ARCH_MIPS_EMU_ISA_H

#include "machine.h"
#include "syscall.h"

#define mips_isa_call_debug(...) debug(mips_isa_call_debug_category, __VA_ARGS__)
#define mips_isa_inst_debug(...) debug(mips_isa_inst_debug_category, __VA_ARGS__)

extern int mips_isa_call_debug_category;
extern int mips_isa_inst_debug_category;

void mips_isa_syscall(struct mips_ctx_t *ctx);


void mips_isa_execute_inst(struct mips_ctx_t *ctx);

#define CALL_STACK_SIZE 10000

struct mips_isa_cstack_t
{
	char *sym_name[CALL_STACK_SIZE];
	unsigned int top;
};
struct mips_isa_cstack_t *mips_isa_cstack_create(struct mips_ctx_t *ctx);
void mips_isa_cstack_push(struct mips_ctx_t *ctx, char *str);
char *mips_isa_cstack_pop(struct mips_ctx_t *ctx);

void mips_isa_branch(struct mips_ctx_t *ctx, unsigned int dest);

void mips_isa_rel_branch(struct mips_ctx_t *ctx, unsigned int dest);

unsigned int mips_gpr_get_value(struct mips_ctx_t* ctx, unsigned int reg_no);
void mips_gpr_set_value(struct mips_ctx_t *ctx, unsigned int reg_no, unsigned int value);

#endif

