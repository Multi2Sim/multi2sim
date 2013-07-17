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

#ifndef ARCH_X86_EMU_REGS_H
#define ARCH_X86_EMU_REGS_H

#include <stdio.h>


struct x86_regs_t
{
	/* Integer registers */
	unsigned int eax, ecx, edx, ebx;
	unsigned int esp, ebp, esi, edi;
	unsigned short int es, cs, ss, ds, fs, gs;
	unsigned int eip;
	unsigned int eflags;

	/* Floating-point unit */
	struct
	{
		unsigned char value[10];
		int valid;
	} fpu_stack[8];
	int fpu_top;  /* top of stack (field 'top' of status register) */
	int fpu_code;  /* field 'code' of status register (C3-C2-C1-C0) */
	unsigned short int fpu_ctrl;  /* fpu control word */

	/* XMM registers (8 128-bit regs) */
	unsigned char xmm[8][16];

} __attribute__((packed));

struct x86_regs_t *x86_regs_create(void);
void x86_regs_free(struct x86_regs_t *regs);

void x86_regs_copy(struct x86_regs_t *dst, struct x86_regs_t *src);
void x86_regs_dump(struct x86_regs_t *regs, FILE *f);
void x86_regs_fpu_stack_dump(struct x86_regs_t *regs, FILE *f);

#endif

