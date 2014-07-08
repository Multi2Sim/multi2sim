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

	// List list of instructions forming the basic block.
	std::list<std::unique_ptr<si2bin::Instruction>> inst_list;

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
	void EmitAnd(llvm::BinaryOperator *llvm_inst);
	void EmitOr(llvm::BinaryOperator *llvm_inst);
	void EmitXor(llvm::BinaryOperator *llvm_inst);
	void EmitSExt(llvm::SExtInst *llvm_inst);
	void EmitExtractElement(llvm::ExtractElementInst *llvm_inst);
	void EmitInsertElement(llvm::InsertElementInst *llvm_inst);

public:

	// Function where the basic block belongs.
	Function *function;

	// Bitmaps to hold live register analysis intermediate data
	misc::Bitmap *def;
	misc::Bitmap *use;

	misc::Bitmap *in;
	misc::Bitmap *out;

	// Constructor
	BasicBlock(Function *function, comm::LeafNode *node) :
			comm::BasicBlock(node),
			function(function)
	{
	}

	/// Return a reference to the list of instructions
	std::list<std::unique_ptr<si2bin::Instruction>> &getInstList()
	{
		return inst_list;
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

	// Add one SI instruction to the 'inst_list' field of the basic block.
	void AddInst(si2bin::Instruction *inst);

	/// Add a comment to a basic block. The comment will be attached to the
	/// next instruction added to the block. If no other instruction is
	/// added to the basic block, the comment won't have any effect.
	void AddComment(const std::string &comment) { this->comment = comment; }

	/// Emit SI code for the LLVM basic block into field `inst_list`.
	void Emit(llvm::BasicBlock *llvm_basic_block);

	/// Return an iterator to the first instruction that was emitted by
	/// the control flow pass in the basic block. If there is no control
	/// flow instruction, a past-the-end iterator is returned.
	std::list<std::unique_ptr<si2bin::Instruction>>::iterator
			getFirstControlFlowInst();

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

