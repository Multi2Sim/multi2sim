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
#include <memory>

#include <arch/common/Asm.h>
#include <lib/cpp/Error.h>

#include "Inst.h"


namespace Fermi
{

class Asm : public comm::Asm
{
	// Max number of operations per category
	static const unsigned InstOpCountPerCategory = 64;

	// Instruction information table. The 1st level is indexed by the
	// instruction category. The 2nd level is indexed by the opcode bits in
	// the category.
	InstInfo inst_info_table[InstCategoryCount][InstOpCountPerCategory];

	// Constructor
	Asm();

	// Unique instance of Fermi disassembler
	static std::unique_ptr<Asm> instance;

public:

	/// For exceptions related with the Fermi disassembler
	class Error : public misc::Error
	{
	public:
		Error(const std::string &message) :
				misc::Error(message)
		{
			AppendPrefix("Fermi Disassembler");
		}
	};

	/// Get the only instance of the Fermi disassembler. If the instance
	/// does not exist yet, it will be created, and will remain allocated
	/// until the end of the execution.
	static Asm *getInstance();

	/// Disassembler
	void DisassembleBinary(const std::string &path) const;

	/// Get instruction information
	InstInfo *GetInstInfo(unsigned cat, unsigned op_in_cat)
	{
		assert(cat >= 0 && cat < InstCategoryCount);
		assert(op_in_cat >= 0 && op_in_cat < InstOpCountPerCategory);
		return &inst_info_table[cat][op_in_cat];
	}
};


}  // namespace Fermi

#endif  // FERMI_ASM_ASM_H
