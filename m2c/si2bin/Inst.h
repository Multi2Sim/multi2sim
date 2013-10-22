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

#ifndef M2C_SI2BIN_INST_H
#define M2C_SI2BIN_INST_H

#include <iostream>
#include <list>
#include <memory>

#include <arch/southern-islands/asm/Inst.h>

#include "Token.h"


/* Forward declarations */
namespace llvm2si {
class BasicBlock;
}


namespace si2bin
{


struct InstInfo
{
	/* There can be multiple instruction encodings for the same instruction
	 * name. This points to the next one in the list. */
	InstInfo *next;

	/* Associated info structure in disassembler */
	SI::InstInfo *info;

	/* List of tokens in format string */
	std::list<std::string> str_tokens;
	std::list<std::unique_ptr<Token>> tokens;

	/* Instruction name. This string is equal to str_tokens[0] */
	std::string name;
};


class Inst
{
	/* Instruction opcode. This field should match the content of
	 * info->info->opcode. */
	SI::InstOpcode opcode;

	/* Instruction size in bytes (4 or 8). This value is produced after a
	 * call to Inst::Encode() */
	int size;

	/* Instruction bytes. This value is produced after a call to
	 * Inst::Encode(). */
	SI::InstBytes bytes;

	/* Invariable information related with this instruction */
	InstInfo *info;

	/* List of arguments */
	std::list<std::unique_ptr<Arg>> args;

	/* For LLVM-to-SI back-end: basic block that the instruction
	 * belongs to. */
	llvm2si::BasicBlock *basic_block;

	/* Comment attached to the instruction, which will be dumped together
	 * with it. */
	std::string comment;

	/* Construction based on opcode + argument list */
	void Initialize(SI::InstOpcode opcode);
	template<typename... Args> void Initialize(SI::InstOpcode opcode,
			Arg *arg, Args&&... args)
	{
		this->args.emplace_back(arg);
		Initialize(opcode, args...);
	}

	/* Construction based on name + argument list */
	void Initialize(const std::string &name);
	template<typename... Args> void Initialize(const std::string &name,
			Arg *arg, Args&&... args)
	{
		this->args.emplace_back(arg);
		Initialize(name, args...);
	}

public:

	/* Create a new instruction with the specified opcode, as defined in the
	 * Southern Islands disassembler. The arguments contained in the list
	 * will be freed automatically in the destructor of this class. */
	template<typename... Args> Inst(SI::InstOpcode opcode, Args&&... args)
			{ Initialize(opcode, args...); }

	/* Create a new instruction with one of the possible opcodes
	 * corresponding to a name. The arguments contained in the list will be
	 * adopted by the instruction and freed in the destructor. */
	template<typename... Args> Inst(const std::string &name, Args&&... args)
			{ Initialize(name, args...); }

	/* Dump instruction in a human-ready way */
	void Dump(std::ostream &os);
	friend std::ostream &operator<<(std::ostream &os, Inst &inst) {
		inst.Dump(os);
		return os;
	}

	/* Getters/setters */
	llvm2si::BasicBlock *GetBasicBlock() { return basic_block; }
	void SetBasicBlock(llvm2si::BasicBlock *basic_block) {
		this->basic_block = basic_block; }

	/* Attach a comment to the instruction */
	void SetComment(const std::string &comment) { this->comment = comment; }

	/* Encode the instruction, internally populating the 'bytes' and 'size'
	 * fields. A call to Inst::Write() can be performed after this to dump
	 * the instructions bytes. */
	void Encode();

	/* Write the instruction bytes into output stream. */
	void Write(std::ostream &os);
};


}  /* namespace si2bin */

#endif
