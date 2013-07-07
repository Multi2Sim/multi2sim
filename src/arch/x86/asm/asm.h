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

#ifndef X86_ASM_H
#define X86_ASM_H

#include <lib/util/class.h>

struct x86_inst_t;


/* Class macros */
#define X86_ASM_TYPE 0xeae356fd
#define X86_ASM(p) (CLASS_REINTERPRET_CAST(p, X86_ASM_TYPE, struct x86_asm_t))
#define X86_ASM_CLASS_OF(p) (CLASS_OF(p, X86_ASM_TYPE))


/* Class:	x86_asm_t
 * Inherits:	asm_t
 */
struct x86_asm_t
{
	/* Class information
	 * WARNING - must be first field */
	struct class_t class_info;

	/* Array containing 'x86_inst_opcode_count' elements of type
	 * 'x86_inst_info_t' allocated contiguously, and storing instructions
	 * information as given in 'asm.dat'. */
	struct x86_inst_info_t *inst_info_list;

	/* Arrays containing 256 elements of type 'x86_inst_info_elem_t *'
	 * allocated contiguously. These tables are used for lookups when
	 * decoding instructions. */
	struct x86_inst_info_elem_t *inst_info_table[0x100];
	struct x86_inst_info_elem_t *inst_info_table_0f[0x100];
};


/* There is only one instance of the x86 disassembler, initialized and finalized
 * in 'x86_asm_init' and 'x86_asm_done'. */
extern struct x86_asm_t *x86_asm;


void x86_disasm_init(void);
void x86_disasm_done(void);

struct x86_asm_t *x86_asm_create(void);
void x86_asm_free(struct x86_asm_t *as);

/* Disassemble and dump */
void x86_disasm(void *buf, unsigned int eip, volatile struct x86_inst_t *inst);
void x86_disasm_file(char *file_name);


#endif
