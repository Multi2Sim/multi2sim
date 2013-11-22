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

#ifndef ARCH_X86_ASM_ASM_OLD_H
#define ARCH_X86_ASM_ASM_OLD_H

#include <arch/common/asm.h>
#include <lib/class/class.h>


/*
 * Class 'X86Asm'
 */

CLASS_BEGIN(X86Asm, Asm)

	/* Array containing 'X86InstOpcodeCount' elements of type
	 * 'X86InstInfo' allocated contiguously, and storing instructions
	 * information as given in 'asm.dat'. */
	struct x86_inst_info_t *inst_info_list;

	/* Arrays containing 256 elements of type 'X86InstInfoElem *'
	 * allocated contiguously. These tables are used for lookups when
	 * decoding instructions. */
	struct x86_inst_info_elem_t *inst_info_table[0x100];
	struct x86_inst_info_elem_t *inst_info_table_0f[0x100];

	/* Look-up table returning true if a byte is an x86 prefix */
	unsigned char is_prefix[0x100];

CLASS_END(X86Asm)


void X86AsmCreate(X86Asm *self);
void X86AsmDestroy(X86Asm *self);

/* Return an instruction name given an opcode, or string '<invalid>' if the
 * opcode value does not exist. */
char *X86AsmGetInstName(X86Asm *self, int opcode);

/* Disassemble the content of file given in 'path' and dump the output in
 * 'stdout', with a similar format as tool 'objdump'. */
void X86AsmDisassembleBinary(X86Asm *self, char *path);


#endif
