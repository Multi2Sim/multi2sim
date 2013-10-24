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

#ifndef M2C_SI2BIN_BUILDER_H
#define M2C_SI2BIN_BUILDER_H

#include <array>
#include <unordered_map>

#include <arch/southern-islands/asm/Asm.h>

#include "Inst.h"


namespace si2bin
{

class Builder
{
	/* Information with all Southern Islands instructions */
	std::array<InstInfo, SI::InstOpcodeCount> inst_info_array;

	/* Hash table indexed by an instruction name. Each entry contains a
	 * linked list of instructions with that name. */
	std::unordered_map<std::string, InstInfo*> inst_info_table;

	/* Southern Islands disassembler */
	SI::Asm as;

public:

	/* Constructor */
	Builder();

	/* Return instruction information associated with a given opcode, or
	 * null if the opcode is invalid. */
	InstInfo *getInstInfo(SI::InstOpcode opcode) {
		return (opcode > SI::InstOpcodeInvalid &&
				opcode < SI::InstOpcodeCount) ?
			&inst_info_array[opcode] :
			nullptr;
	}

	/* Return the head of a linked list of InstInfo structures associated
	 * with an instruction name, or null if the name is invalid. */
	InstInfo *getInstInfo(const std::string &name) {
		auto it = inst_info_table.find(name);
		return it == inst_info_table.end() ? nullptr : it->second;
	}
};


/* Global variable */
extern Builder builder;


}  /* namespace si2bin */

#endif
