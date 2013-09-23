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

#ifndef FERMI_ASM_ASM_H
#define FERMI_ASM_ASM_H


#ifdef __cplusplus

#include <arch/common/Asm.h>

#include "Inst.h"


namespace Fermi
{


class Asm : public Common::Asm
{
public:
	static const int dec_table_size = 1024;

	/* Instruction information table, indexed with an instruction opcode
	 * enumeration. */
	InstInfo inst_info[InstOpcodeCount];

	/* Decoding table. This table is indexed by the opcode bits of the
	 * instruction bytes. */
	InstInfo *dec_table[dec_table_size];

	/* Constructor */
	Asm();

	/* Disassemblers */
	void DisassembleBinary(std::string path);
	void DisassembleBuffer(char *buffer, unsigned int size);
};

}  /* namespace Fermi */

#endif  /* __cplusplus */



/*
 * C Wrapper
 */

#ifdef __cplusplus
extern "C" {
#endif

struct FrmAsmWrap *FrmAsmWrapCreate(void);
void FrmAsmWrapFree(struct FrmAsmWrap *self);

void FrmAsmWrapDisassembleBinary(struct FrmAsmWrap *self, char *path);
void FrmAsmWrapDisassembleBuffer(struct FrmAsmWrap *self, char *buffer, unsigned int size);

#ifdef __cplusplus
}
#endif




#endif  /* FERMI_ASM_ASM_H */
