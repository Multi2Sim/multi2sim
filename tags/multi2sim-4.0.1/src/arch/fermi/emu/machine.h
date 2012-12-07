/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FERMI_EMU_MACHINE_H
#define FERMI_EMU_MACHINE_H


/* List of functions implementing GPU instructions 'amd_inst_XXX_impl' */
struct frm_inst_t;
typedef void (*frm_isa_inst_func_t)(struct frm_thread_t *thread, struct frm_inst_t *inst);
extern frm_isa_inst_func_t *frm_isa_inst_func;

/* Declarations of function prototypes implementing Fermi ISA */
#define DEFINST(_name, _fmt_str, _fmt, _category, _opcode) \
        extern void frm_isa_##_name##_impl(struct frm_thread_t *thread, \
                        struct frm_inst_t *inst);
#include <arch/fermi/asm/asm.dat>
#undef DEFINST


#endif

