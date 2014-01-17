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

#ifndef ARCH_X86_EMU_MACHINE_H
#define ARCH_X86_EMU_MACHINE_H


extern long x86_context_host_flags;

#define __X86_ISA_ASM_START__ asm volatile ( \
	"pushf\n\t" \
	"pop %0\n\t" \
	: "=m" (x86_context_host_flags));

#define __X86_ISA_ASM_END__ asm volatile ( \
	"push %0\n\t" \
	"popf\n\t" \
	: "=m" (x86_context_host_flags));


extern unsigned char x86_context_host_fpenv[28];

#define __X86_ISA_FP_ASM_START__ asm volatile ( \
	"pushf\n\t" \
	"pop %0\n\t" \
	"fnstenv %1\n\t" /* store host FPU environment */ \
	"fnclex\n\t" /* clear host FP exceptions */ \
	"fldcw %2\n\t" \
	: "=m" (x86_context_host_flags), "=m" (*x86_context_host_fpenv) \
	: "m" (ctx->regs->fpu_ctrl));

#define __X86_ISA_FP_ASM_END__ asm volatile ( \
	"push %0\n\t" \
	"popf\n\t" \
	"fnstcw %1\n\t" \
	"fldenv %2\n\t" /* restore host FPU environment */ \
	: "=m" (x86_context_host_flags), "=m" (ctx->regs->fpu_ctrl) \
	: "m" (*x86_context_host_fpenv));


/* References to functions emulating x86 instructions */
#define DEFINST(name, op1, op2, op3, modrm, imm, pfx) \
	void x86_isa_##name##_impl(X86Context *ctx);
#include <arch/x86/asm/asm.dat>
#undef DEFINST


#endif

