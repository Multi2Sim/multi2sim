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
#include <vector>

#include <arch/southern-islands/asm/Asm.h>

#include "Inst.h"


namespace si2bin
{

class InstInfo
{
	/* There can be multiple instruction encodings for the same instruction
	 * name. This points to the next one in the list. */
	InstInfo *next;

	/* Associated info structure in disassembler */
	SI::InstInfo *info;

	/* List of tokens in format string */
	std::vector<std::string> str_tokens;
	std::vector<std::unique_ptr<Token>> tokens;

	/* Instruction name. This string is equal to str_tokens[0] */
	std::string name;

	/* Instruction opcode as a unique integer identifier */
	SI::InstOpcode opcode;

public:

	InstInfo(SI::InstInfo *info);

	/* Getters */

	const std::string &getName() { return name; }
	SI::InstOpcode getOpcode() { return opcode; }
	SI::InstInfo *getInfo() { return info; }
	InstInfo *getNext() { return next; }
	size_t getNumTokens() { return tokens.size(); }

	const std::string &getTokenStr(size_t index) {
		assert(index < tokens.size());
		return str_tokens[index];
	}

	Token *getToken(size_t index) {
		assert(index < tokens.size());
		return tokens[index].get();
	}

	/* Insert after another object of type InstInfo with the same name in
	 * the main hash table. */
	void InsertAfter(InstInfo *info) {
		assert(!info->next);
		info->next = this;
	}
};



class Builder
{
	/* Information with all Southern Islands instructions */
	std::array<std::unique_ptr<InstInfo>, SI::InstOpcodeCount> inst_info_array;

	/* Hash table indexed by an instruction name. Each entry contains a
	 * linked list of instructions with that name. */
	std::unordered_map<std::string, InstInfo *> inst_info_table;

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
			inst_info_array[opcode].get() : nullptr;
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
