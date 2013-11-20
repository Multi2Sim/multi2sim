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


#include <cassert>

#include <arch/common/Asm.h>

#include "Inst.h"


namespace Fermi
{


class Asm : public Common::Asm
{
	static const int inst_cat_count = 16;
	static const int func_count = 64;

	// Instruction information table
	InstInfo inst_info[InstOpcodeCount];

	// Decoding table. The 1st level is indexed by the instruction category
	// bits, and the 2nd level is indexed by the function bits in the
	// category.
	InstInfo *dec_table[inst_cat_count][func_count];

public:
	// Constructor
	Asm();

	// Disassemblers
	void DisassembleBinary(std::string path);
	void DisassembleBuffer(char *buffer, unsigned int size);

	// Getters
	InstInfo *GetInstInfo(InstOpcode opcode) { assert(opcode >= 0 &&
			opcode < InstOpcodeCount); return &inst_info[opcode]; }
	InstInfo *GetDecTable(int cat, int func) { assert(cat >= 0 && 
			cat < inst_cat_count); assert(func >= 0 && 
				func < func_count); return dec_table[cat][func]; }
};

}  // namespace Fermi


#endif  // FERMI_ASM_ASM_H
