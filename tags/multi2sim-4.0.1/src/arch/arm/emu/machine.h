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

#ifndef ARCH_ARM_EMU_MACHINE_H
#define ARCH_ARM_EMU_MACHINE_H

/* Table of functions implementing implementing the Evergreen ISA */
typedef void (*arm_isa_inst_func_t)(struct arm_ctx_t *ctx);

/* Declarations of function prototypes implementing Evergreen ISA */
#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) \
	extern void arm_isa_##_name##_impl(struct arm_ctx_t *ctx);
#include <arch/arm/asm/asm.dat>
#undef DEFINST


#endif

