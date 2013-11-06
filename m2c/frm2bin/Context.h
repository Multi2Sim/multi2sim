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

#ifndef FRM2BIN_INST_CONTEXT_H
#define FRM2BIN_INST_CONTEXT_H

#include <arch/fermi/asm/Asm.h>
#include <arch/fermi/asm/Inst.h>

#ifdef __cplusplus

#include <array>
#include <unordered_map>
#include <vector>
#include <string>
#include "Arg.h"
#include "Token.h"

namespace frm2bin
{
class InstInfo
{
	/* There can be multiple instruction encodings for the same instruction
	 * name. This points to the next one in the list. */
	InstInfo *next;

	/* Associated info structure in disassembler */
	Fermi::InstInfo *info;

	/* List of tokens in format string */
	std::vector<std::string> str_tokens;
	std::vector<std::unique_ptr<Token>> tokens;

	std::string name; // not necessarily the 1st of tokens;

	/* List of modifiers */
	std::vector<std::string> str_mods;
	std::vector<std::unique_ptr<Mod>> mods;

public:
	InstInfo(Fermi::InstInfo *info);

	/* Getters */

	const std::string &getName() { return name; }
	Fermi::InstInfo *getInfo() { return info; }
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

	const std::string &getModStr(size_t index) {
		assert(index < mods.size());
		return str_mods[index];
	}

	Mod *getMod(size_t index) {
		assert(index < mods.size());
		return mods[index].get();
	}

	/* Insert after another object of type InstInfo with the same name in
	 * the main hash table. */
	void InsertAfter(InstInfo *info) {
		assert(!info->next);
		info->next = this;
	}
};

class Context
{
	/* Information with all Fermi instructions */
	std::array<std::unique_ptr<InstInfo>, Fermi::InstOpcodeCount> inst_info_array;

	/* Hash table indexed by an instruction name. Each entry contains a
	 * linked list of instructions with that name. */
	std::unordered_map<std::string, InstInfo *> inst_info_table;

	/* Fermi disassembler */
	Fermi::Asm as;

public:

	/* Constructor */
	Context();

	/* Return instruction information associated with a given opcode, or
	 * null if the opcode is invalid. */
	InstInfo *getInstInfo(Fermi::InstOpcode opcode) {
		return (opcode > Fermi::InstOpcodeInvalid &&
				opcode < Fermi::InstOpcodeCount) ?
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
extern Context context;

} /* namespace frm2bin */

#endif  /* __cplusplus */


#endif /* FRM2BIN_INST_CONTEXT_H */
