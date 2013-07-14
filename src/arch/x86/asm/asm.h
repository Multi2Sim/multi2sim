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

#include <arch/common/asm.h>
#include <lib/util/class.h>

struct x86_inst_t;


/*
 * Class 'X86Asm'
 */

CLASS_BEGIN(X86Asm, Asm)

	/* Array containing 'x86_inst_opcode_count' elements of type
	 * 'x86_inst_info_t' allocated contiguously, and storing instructions
	 * information as given in 'asm.dat'. */
	struct x86_inst_info_t *inst_info_list;

	/* Arrays containing 256 elements of type 'x86_inst_info_elem_t *'
	 * allocated contiguously. These tables are used for lookups when
	 * decoding instructions. */
	struct x86_inst_info_elem_t *inst_info_table[0x100];
	struct x86_inst_info_elem_t *inst_info_table_0f[0x100];

	/* Look-up table returning true if a byte is an x86 prefix */
	unsigned char is_prefix[0x100];

CLASS_END(X86Asm)


void X86AsmCreate(X86Asm *self);
void X86AsmDestroy(X86Asm *self);




/*
 * Non-class Public Functions
 */

/* One public instance of the x86 disassembler */
extern X86Asm *x86_asm;

void x86_asm_init(void);
void x86_asm_done(void);

void x86_asm_disassemble_binary(char *path);


#endif
