/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef M2C_LLVM2SI_BASIC_BLOCK_H
#define M2C_LLVM2SI_BASIC_BLOCK_H

#include <iostream>
#include <memory>

#include <llvm/Type.h>
#include <m2c/common/Node.h>
#include <m2c/si2bin/Inst.h>


namespace llvm2si
{

/* Forward declarations */
class Function;

class BasicBlock
{
	/* Comment stored temporarily in the basic block to be attached to the
	 * next instruction added. */
	std::string comment;

	/* Function where the basic block belongs. */
	Function *function;

	/* List list of instructions forming the basic block. */
	std::list<std::unique_ptr<si2bin::Inst>> inst_list;

	/* Return the size in bytes of an LLVM type */
	static int GetLlvmTypeSize(llvm::Type *llvm_type);

	/* Return the size in bytes of the LLVM type pointed to by an
	 * LLVM pointer. */
	static int GetPointerLlvmTypeSize(llvm::Type *llvm_type);

public:
	
	/* Constructor */
	BasicBlock(Function *function, Common::LeafNode *node);

	/* Dump */
	void Dump(std::ostream &os);
	friend std::ostream &operator<<(std::ostream &os, BasicBlock &basic_block)
			{ basic_block.Dump(os); return os; }

	/* Add one SI instruction to the 'inst_list' field of the basic block. */
	void AddInst(si2bin::Inst *inst);

	/* Add a comment to a basic block. The comment will be attached to the
	 * next instruction added to the block. If no other instruction is added
	 * to the basic block, the comment won't have any effect. */
	void AddComment(const std::string &comment);

	/* Emit SI code for the LLVM basic block into field 'inst_list'. */
	void Emit(llvm::BasicBlock *llvm_basic_block);
};


}  /* namespace llvm2si */

#endif

