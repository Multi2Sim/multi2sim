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

#include <stdio.h>

#include <arch/x86/asm/asm.h>
#include <arch/x86/asm/inst.h>
#include <lib/util/class.h>



/*
 * Class 'X86Context'
 * Additional functions.
 */

void X86ContextError(X86Context *ctx, char *fmt, ...);

void X86ContextMemRead(X86Context *ctx, unsigned int addr, int size, void *buf);
void X86ContextMemWrite(X86Context *ctx, unsigned int addr, int size, void *buf);

void X86ContextSetFlag(X86Context *ctx, enum x86_inst_flag_t flag);
void X86ContextClearFlag(X86Context *ctx, enum x86_inst_flag_t flag);
int X86ContextGetFlag(X86Context *ctx, enum x86_inst_flag_t flag);

unsigned int X86ContextLoadReg(X86Context *ctx, enum x86_inst_reg_t reg);
void X86ContextStoreReg(X86Context *ctx, enum x86_inst_reg_t reg, unsigned int value);

unsigned char X86ContextLoadRm8(X86Context *ctx);
unsigned short X86ContextLoadRm16(X86Context *ctx);
unsigned int X86ContextLoadRm32(X86Context *ctx);
unsigned short X86ContextLoadR32M16(X86Context *ctx);
unsigned long long X86ContextLoadM64(X86Context *ctx);
void X86ContextStoreRm8(X86Context *ctx, unsigned char value);
void X86ContextStoreRm16(X86Context *ctx, unsigned short value);
void X86ContextStoreRm32(X86Context *ctx, unsigned int value);
void X86ContextStoreM64(X86Context *ctx, unsigned long long value);

#define X86ContextLoadR8(ctx) X86ContextLoadReg(ctx, ctx->inst.reg + x86_inst_reg_al)
#define X86ContextLoadR16(ctx) X86ContextLoadReg(ctx, ctx->inst.reg + x86_inst_reg_ax)
#define X86ContextLoadR32(ctx) X86ContextLoadReg(ctx, ctx->inst.reg + x86_inst_reg_eax)
#define X86ContextLoadSReg(ctx) X86ContextLoadReg(ctx, ctx->inst.reg + x86_inst_reg_es)
#define X86ContextStoreR8(ctx, value) X86ContextStoreReg(ctx, ctx->inst.reg + x86_inst_reg_al, value)
#define X86ContextStoreR16(ctx, value) X86ContextStoreReg(ctx, ctx->inst.reg + x86_inst_reg_ax, value)
#define X86ContextStoreR32(ctx, value) X86ContextStoreReg(ctx, ctx->inst.reg + x86_inst_reg_eax, value)
#define X86ContextStoreSReg(ctx, value) X86ContextStoreReg(ctx, ctx->inst.reg + x86_inst_reg_es, value)

#define X86ContextLoadIR8(ctx) X86ContextLoadReg(ctx, ctx->inst.opindex + x86_inst_reg_al)
#define X86ContextLoadIR16(ctx) X86ContextLoadReg(ctx, ctx->inst.opindex + x86_inst_reg_ax)
#define X86ContextLoadIR32(ctx) X86ContextLoadReg(ctx, ctx->inst.opindex + x86_inst_reg_eax)
#define X86ContextStoreIR8(ctx, value) X86ContextStoreReg(ctx, ctx->inst.opindex + x86_inst_reg_al, value)
#define X86ContextStoreIR16(ctx, value) X86ContextStoreReg(ctx, ctx->inst.opindex + x86_inst_reg_ax, value)
#define X86ContextStoreIR32(ctx, value) X86ContextStoreReg(ctx, ctx->inst.opindex + x86_inst_reg_eax, value)

void X86ContextLoadFpu(X86Context *ctx, int index, unsigned char *value);
void X86ContextStoreFpu(X86Context *ctx, int index, unsigned char *value);
void X86ContextPopFpu(X86Context *ctx, unsigned char *value);
void X86ContextPushFpu(X86Context *ctx, unsigned char *value);

float X86ContextLoadFloat(X86Context *ctx);
double X86ContextLoadDouble(X86Context *ctx);
void X86ContextLoadExtended(X86Context *ctx, unsigned char *value);
void X86ContextStoreFloat(X86Context *ctx, float value);
void X86ContextStoreDouble(X86Context *ctx, double value);
void X86ContextStoreExtended(X86Context *ctx, unsigned char *value);

void X86ContextDumpXMM(X86Context *ctx, unsigned char *value, FILE *f);
void X86ContextLoadXMM(X86Context *ctx, unsigned char *value);
void X86ContextStoreXMM(X86Context *ctx, unsigned char *value);
void X86ContextLoadXMMM32(X86Context *ctx, unsigned char *value);
void X86ContextStoreXMMM32(X86Context *ctx, unsigned char *value);
void X86ContextLoadXMMM64(X86Context *ctx, unsigned char *value);
void X86ContextStoreXMMM64(X86Context *ctx, unsigned char *value);
void X86ContextLoadXMMM128(X86Context *ctx, unsigned char *value);
void X86ContextStoreXMMM128(X86Context *ctx, unsigned char *value);

void X86ContextStoreFpuCode(X86Context *ctx, unsigned short status);
unsigned short X86ContextLoadFpuStatus(X86Context *ctx);

unsigned int X86ContextEffectiveAddress(X86Context *ctx);
unsigned int X86ContextMoffsAddress(X86Context *ctx);

void X86ContextExecuteInst(X86Context *ctx);




/*
 * Non-Class Stuff
 */

#define X86ContextDebugCall(...) debug(x86_context_call_debug_category, __VA_ARGS__)
#define X86ContextDebugISA(...) debug(x86_context_isa_debug_category, __VA_ARGS__)

extern int x86_context_call_debug_category;
extern int x86_context_isa_debug_category;

void x86_isa_trace_call_init(char *filename);
void x86_isa_trace_call_done(void);

void x86_isa_inst_stat_dump(FILE *f);
void x86_isa_inst_stat_reset(void);

void X86ContextDoubleToExtended(double f, unsigned char *e);
double X86ContextExtendedToDouble(unsigned char *e);
void X86ContextFloatToExtended(float f, unsigned char *e);
float X86ContextExtendedToFloat(unsigned char *e);

#endif

