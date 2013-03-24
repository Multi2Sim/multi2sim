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

#ifndef ARCH_X86_EMU_ISA_H
#define ARCH_X86_EMU_ISA_H

#include <arch/x86/asm/asm.h>

/* Forward type declarations */
struct x86_ctx_t;


#define x86_isa_call_debug(...) debug(x86_isa_call_debug_category, __VA_ARGS__)
#define x86_isa_inst_debug(...) debug(x86_isa_inst_debug_category, __VA_ARGS__)

extern int x86_isa_call_debug_category;
extern int x86_isa_inst_debug_category;

void x86_isa_error(struct x86_ctx_t *ctx, char *fmt, ...);

void x86_isa_mem_read(struct x86_ctx_t *ctx, unsigned int addr, int size, void *buf);
void x86_isa_mem_write(struct x86_ctx_t *ctx, unsigned int addr, int size, void *buf);

void x86_isa_dump_flags(struct x86_ctx_t *ctx, FILE *f);
void x86_isa_set_flag(struct x86_ctx_t *ctx, enum x86_flag_t flag);
void x86_isa_clear_flag(struct x86_ctx_t *ctx, enum x86_flag_t flag);
int x86_isa_get_flag(struct x86_ctx_t *ctx, enum x86_flag_t flag);

unsigned int x86_isa_load_reg(struct x86_ctx_t *ctx, enum x86_reg_t reg);
void x86_isa_store_reg(struct x86_ctx_t *ctx, enum x86_reg_t reg, unsigned int value);

unsigned char x86_isa_load_rm8(struct x86_ctx_t *ctx);
unsigned short x86_isa_load_rm16(struct x86_ctx_t *ctx);
unsigned int x86_isa_load_rm32(struct x86_ctx_t *ctx);
unsigned short x86_isa_load_r32m16(struct x86_ctx_t *ctx);
unsigned long long x86_isa_load_m64(struct x86_ctx_t *ctx);
void x86_isa_store_rm8(struct x86_ctx_t *ctx, unsigned char value);
void x86_isa_store_rm16(struct x86_ctx_t *ctx, unsigned short value);
void x86_isa_store_rm32(struct x86_ctx_t *ctx, unsigned int value);
void x86_isa_store_m64(struct x86_ctx_t *ctx, unsigned long long value);

#define x86_isa_load_r8(ctx) x86_isa_load_reg(ctx, ctx->inst.reg + x86_reg_al)
#define x86_isa_load_r16(ctx) x86_isa_load_reg(ctx, ctx->inst.reg + x86_reg_ax)
#define x86_isa_load_r32(ctx) x86_isa_load_reg(ctx, ctx->inst.reg + x86_reg_eax)
#define x86_isa_load_sreg(ctx) x86_isa_load_reg(ctx, ctx->inst.reg + x86_reg_es)
#define x86_isa_store_r8(ctx, value) x86_isa_store_reg(ctx, ctx->inst.reg + x86_reg_al, value)
#define x86_isa_store_r16(ctx, value) x86_isa_store_reg(ctx, ctx->inst.reg + x86_reg_ax, value)
#define x86_isa_store_r32(ctx, value) x86_isa_store_reg(ctx, ctx->inst.reg + x86_reg_eax, value)
#define x86_isa_store_sreg(ctx, value) x86_isa_store_reg(ctx, ctx->inst.reg + x86_reg_es, value)

#define x86_isa_load_ir8(ctx) x86_isa_load_reg(ctx, ctx->inst.opindex + x86_reg_al)
#define x86_isa_load_ir16(ctx) x86_isa_load_reg(ctx, ctx->inst.opindex + x86_reg_ax)
#define x86_isa_load_ir32(ctx) x86_isa_load_reg(ctx, ctx->inst.opindex + x86_reg_eax)
#define x86_isa_store_ir8(ctx, value) x86_isa_store_reg(ctx, ctx->inst.opindex + x86_reg_al, value)
#define x86_isa_store_ir16(ctx, value) x86_isa_store_reg(ctx, ctx->inst.opindex + x86_reg_ax, value)
#define x86_isa_store_ir32(ctx, value) x86_isa_store_reg(ctx, ctx->inst.opindex + x86_reg_eax, value)

void x86_isa_load_fpu(struct x86_ctx_t *ctx, int index, unsigned char *value);
void x86_isa_store_fpu(struct x86_ctx_t *ctx, int index, unsigned char *value);
void x86_isa_pop_fpu(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_push_fpu(struct x86_ctx_t *ctx, unsigned char *value);

float x86_isa_load_float(struct x86_ctx_t *ctx);
double x86_isa_load_double(struct x86_ctx_t *ctx);
void x86_isa_load_extended(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_store_float(struct x86_ctx_t *ctx, float value);
void x86_isa_store_double(struct x86_ctx_t *ctx, double value);
void x86_isa_store_extended(struct x86_ctx_t *ctx, unsigned char *value);

void x86_isa_dump_xmm(struct x86_ctx_t *ctx, unsigned char *value, FILE *f);
void x86_isa_load_xmm(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_store_xmm(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_load_xmmm32(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_store_xmmm32(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_load_xmmm64(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_store_xmmm64(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_load_xmmm128(struct x86_ctx_t *ctx, unsigned char *value);
void x86_isa_store_xmmm128(struct x86_ctx_t *ctx, unsigned char *value);

void x86_isa_double_to_extended(double f, unsigned char *e);
double x86_isa_extended_to_double(unsigned char *e);
void x86_isa_float_to_extended(float f, unsigned char *e);
float x86_isa_extended_to_float(unsigned char *e);

void x86_isa_store_fpu_code(struct x86_ctx_t *ctx, unsigned short status);
unsigned short x86_isa_load_fpu_status(struct x86_ctx_t *ctx);

unsigned int x86_isa_effective_address(struct x86_ctx_t *ctx);
unsigned int x86_isa_moffs_address(struct x86_ctx_t *ctx);

void x86_isa_init(void);
void x86_isa_done(void);

void x86_isa_execute_inst(struct x86_ctx_t *ctx);

void x86_isa_trace_call_init(char *filename);
void x86_isa_trace_call_done(void);

void x86_isa_inst_stat_dump(FILE *f);
void x86_isa_inst_stat_reset(void);

#endif

