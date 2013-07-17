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

#ifndef ARCH_MIPS_EMU_MACHINE_H
#define ARCH_MIPS_EMU_MACHINE_H

/* Table of functions implementing the MIPS ISA */
typedef void (*mips_isa_inst_func_t)(struct mips_ctx_t *ctx);

/* Declarations of function prototypes implementing MIPS ISA */
#define DEFINST(_name, _fmt_str, _op0, _op1, _op2, _op3) \
	extern void mips_isa_##_name##_impl(struct mips_ctx_t *ctx);
#include <arch/mips/asm/asm.dat>
#undef DEFINST


#endif
