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

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <m2c/common/BasicBlock.h>
#include <m2c/common/Node.h>
#include <m2c/si2bin/Instruction.h>

#include <src/lib/cpp/Bitmap.h>


namespace llvm2si
{

// Forward declarations
class Function;

class BasicBlock : public comm::BasicBlock
{
	static int id_counter;

	int global_id;

	// Function that the basic block belongs to
	Function *function;

	// List list of instructions forming the basic block.
	std::list<std::unique_ptr<si2bin::Instruction>> instructions;

	// Comment stored temporarily in the basic block to be attached to the
	// next instruction added.
	std::string comment;

	// Return the size in bytes of an LLVM type
	static int getLlvmTypeSize(llvm::Type *llvm_type);

	// Return the size in bytes of the LLVM type pointed to by an
	// LLVM pointer.
	static int getPointedLlvmTypeSize(llvm::Type *llvm_type);

	// Emit instructions
	void EmitAdd(llvm::BinaryOperator *llvm_inst);
	void EmitCall(llvm::CallInst *llvm_inst);
	void EmitGetElementPtr(llvm::GetElementPtrInst *llvm_inst);
	void EmitICmp(llvm::ICmpInst *llvm_inst);
	void EmitLoad(llvm::LoadInst *llvm_inst);
	void EmitMul(llvm::BinaryOperator *llvm_inst);
	void EmitPhi(llvm::PHINode *llvm_inst);
	void EmitRet(llvm::ReturnInst *llvm_inst);
	void EmitStore(llvm::StoreInst *llvm_inst);
	void EmitSub(llvm::BinaryOperator *llvm_inst);
	void EmitFAdd(llvm::BinaryOperator *llvm_inst);
	void EmitFSub(llvm::BinaryOperator *llvm_inst);
	void EmitFMul(llvm::BinaryOperator *llvm_inst);
	void EmitFDiv(llvm::BinaryOperator *llvm_inst);
	void EmitAnd(llvm::BinaryOperator *llvm_inst);
	void EmitOr(llvm::BinaryOperator *llvm_inst);
	void EmitXor(llvm::BinaryOperator *llvm_inst);
	void EmitSExt(llvm::SExtInst *llvm_inst);
	void EmitExtractElement(llvm::ExtractElementInst *llvm_inst);
	void EmitInsertElement(llvm::InsertElementInst *llvm_inst);

public:

	/// Constructor
	BasicBlock(Function *function, comm::LeafNode *node) :
			comm::BasicBlock(node),
			function(function)
	{
		global_id = (BasicBlock::id_counter)++;
	}

	/// Return a reference to the list of instructions
	std::list<std::unique_ptr<si2bin::Instruction>> &getInstructions()
	{
		return instructions;
	}

	/// Dump basic block
	void Dump(std::ostream &os = std::cout);

	/// Alternative syntax for Dump()
	friend std::ostream &operator<<(std::ostream &os,
			BasicBlock &basic_block)
	{
		basic_block.Dump(os);
		return os;
	}

	/// Return the function that the basic block belongs to
	Function *getFunction() { return function; }

	/// Return global id of the basic block
	int getId() const { return global_id; }

	/// Add one SI instruction to the 'inst_list' field of the basic block.
	/// FIXME - This is the old version with asymmetric memory allocation.
	/// This function should be removed.
	void AddInst(si2bin::Instruction *inst);

	/// Add an instruction to the end of the basic block, constructed with
	/// an opcode.
	si2bin::Instruction *addInstruction(SI::InstOpcode opcode)
	{
		instructions.emplace_back(
				misc::new_unique<si2bin::Instruction>
				(this, opcode));
		return instructions.back().get();
	}

	/// Add an instruction to the basic block at the position indicated by
	/// \a it. The instruction is constructed by its opcode.
	si2bin::Instruction *addInstruction(
			std::list<std::unique_ptr<si2bin::Instruction>>
			::iterator it,
			SI::InstOpcode opcode)
	{
		it = instructions.emplace(it,
				misc::new_unique<si2bin::Instruction>
				(this, opcode));
		return it->get();
	}

	/// Set a comment to a basic block. The comment will be attached to the
	/// next instruction added to the block. If no other instruction is
	/// added to the basic block, the comment won't have any effect.
	void setComment(const std::string &comment) { this->comment = comment; }

	/// Translate an LLVM basic block into Southern Islands instructions.
	/// The produced instructions are then available in the internal
	/// instruction list, available through getInstructions().
	void Emit(llvm::BasicBlock *llvm_basic_block);

	/// Return an iterator to the first instruction that was emitted by
	/// the control flow pass in the basic block. If there is no control
	/// flow instruction, a past-the-end iterator is returned.
	std::list<std::unique_ptr<si2bin::Instruction>>::iterator
			getFirstControlFlowInstruction();

	/// Perform analysis on live variables inside the llvm function to allow
	/// for memory efficient register allocation at an instruction level
	/// granularity.
	///
	/// Pseudo-code:
	///
	/// Populate def and use bitmaps for each instruction
	///
	/// For all instructions n: in[n] = out[n] = 0
	/// current = last instruction in basic block
	///
	/// while (current is not null)
	/// 	if (current is last in basic block)
	/// 		out[current] = out[basic_block]
	/// 	else
	///		out[current] = in[successor]
	///
	/// 	in[current] = ( out[current] - def[current] ) U use[current]
	///
	///	current = predecessor
	///
	/// assert (in[first instruction] == in[basic_block])
	///
	void LiveRegisterAnalysis();
};


}  // namespace llvm2si

#endif

