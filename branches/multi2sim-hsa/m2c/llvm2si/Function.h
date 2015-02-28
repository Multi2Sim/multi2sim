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

#ifndef M2C_LLVM2SI_FUNCTION_H
#define M2C_LLVM2SI_FUNCTION_H

#include <iostream>
#include <memory>
#include <list>

#include <arch/southern-islands/asm/Arg.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <m2c/common/Node.h>
#include <m2c/common/Function.h>
#include <m2c/common/Tree.h>
#include <m2c/si2bin/Argument.h>

#include "BasicBlock.h"
#include "Symbol.h"


namespace llvm2si
{

// Forward declarations
class Module;
class Function;

class FunctionArg
{
	friend class Function;

	std::unique_ptr<SI::Arg> arg;

	std::string name;
	
	// Associated LLVM argument
	llvm::Argument *llvm_arg;

	// The fields below are populated when the argument is inserted into
	// a function with a call to Function::AddArg().
	Function *function;

	// Index occupied in function argument list
	int index;
	
	// For arguments of type SI::ArgTypePointer, and scope
	// SI::ArgScopeUAV
	int uav_index;

	// Scalar register identifier containing the argument
	int sreg;

	// Vector register identifier containing the argument
	int vreg;

public:

	/// Constructor
	///
	/// \param llvm_arg
	///	Associated LLVM argument
	FunctionArg(llvm::Argument *llvm_arg);

	/// Dump argument
	void Dump(std::ostream &os = std::cout);

	/// Alternative syntax for Dump()
	friend std::ostream &operator<<(std::ostream &os, FunctionArg &arg)
	{
		arg.Dump(os);
		return os;
	}

	/// Return a Southern Islands argument type from an LLVM type.
	static SI::ArgDataType getDataType(llvm::Type *llvm_type);

	/// Return the number of elements in a vector type, or 1 if the LLVM
	/// type passed is not a vector type.
	static int getNumElements(llvm::Type *llvm_type);
};


class FunctionUAV
{
	friend class Function;

	// Function where it belongs
	Function *function;

	// UAV index in 'function->uav_list'. Uav10 has an index 0, uav11 has
	// index 1, etc.
	int index;

	// Base scalar register of a group of 4 assigned to the UAV. This
	// register identifier is a multiple of 4.
	int sreg;

public:

	/// Return the base scalar register of a group of 4 representing the
	/// UAV descriptor.
	int getSReg() { return sreg; }
};


class Function : public comm::Function
{
	std::string name;

	// Module that the function belongs to.
	Module *module;

	// Associated LLVM function
	llvm::Function *llvm_function;

	// Number of used scalar registers
	int num_sregs;  // Scalar

	int sreg_uav_table;  // UAV table (2 registers)
	int sreg_cb0;  // CB0 (4 registers)
	int sreg_cb1;  // CB1 (4 registers)
	int sreg_wgid;  // Work-group ID (3 registers)
	int sreg_gsize;  // Global size (3 register)
	int sreg_lsize;  // Local size (3 registers)
	int sreg_offs;  // Global offset (3 registers)

	int vreg_lid;  // Local ID (3 registers)
	int vreg_gid;  // Global ID (4 registers)

	// List of arguments
	std::list<std::unique_ptr<FunctionArg>> arg_list;

	// Array of UAVs, starting at uav10. Each UAV is associated with one
	// function argument using a buffer in global memory.
	std::vector<std::unique_ptr<FunctionUAV>> uav_list;

	// List of basic blocks belonging to the function
	std::vector<std::unique_ptr<BasicBlock>> basic_blocks;

	// Predefined nodes
	comm::LeafNode *header_node;
	comm::LeafNode *uavs_node;
	comm::LeafNode *args_node;
	comm::LeafNode *body_node;

	// Symbol table associated with the function, storing LLVM variables
	SymbolTable symbol_table;

	// Control tree
	comm::Tree tree;

	// While code is generated, this variable keeps track of the total
	// amount of bytes pushed into the stack for this function.
	unsigned int stack_size;

	// Add a UAV to the UAV list. This function allocates a series of 4
	// aligned scalar registers to the UAV, populating its 'index' and
	// 'sreg' fields. The UAV object will be freed automatically after
	// calling this function. Emit the code needed to load UAV into
	// 'function->basic_block_uavs'
	void AddUAV(FunctionUAV *uav);

	// Add argument 'arg' into the list of arguments of 'function', and
	// emit code to load it into 'function->basic_block_args'.
	int AddArg(FunctionArg *arg, int num_elem, int offset);

	void DumpData(std::ostream &os);

	void EmitIfThen(comm::AbstractNode *node);
	void EmitIfThenElse(comm::AbstractNode *node);
	void EmitWhileLoop(comm::AbstractNode *node);

	std::unique_ptr<si2bin::Argument> TranslateConstant(llvm::Constant *llvm_const);

public:

	// Number of used vector registers
	int num_vregs;  // Vector
	
	/// Constructor
	///
	/// \param llvm_function
	///	Associated function in the LLVM code.
	explicit Function(Module *module, llvm::Function *llvm_function);

	/// Return the control tree associated with this function
	comm::Tree *getTree() { return &tree; }

	int getVRegGid() { return vreg_gid; }

	int getVRegLid() { return vreg_lid; }

	int getSRegWGid() { return sreg_wgid; }

	int getSRegGSize() { return sreg_gsize; }

	int getSRegLSize() { return sreg_lsize; }

	int getNumOfWG()
	{
		// Copying to a float variable to avoid type casting gsize
		float temp_gsize = sreg_gsize;
		return ceil(temp_gsize/sreg_lsize);
	}

	/// Return number of scalar registers
	int getNumScalarRegisters() const { return num_sregs; }

	/// Return number of vector registers
	int getNumVectorRegisters() const { return num_vregs; }

	/// Return a UAV object given its index, or null if there is not UAV
	/// with that index.
	FunctionUAV *getUAV(int index)
	{
		return index >= 0 && index < (int) uav_list.size() ?
				uav_list[index].get() :
				nullptr;
	}

	// Dump function
	void Dump(std::ostream &os = std::cout);

	/// Alternative syntax for Dump()
	friend std::ostream &operator<<(std::ostream &os, Function &function)
	{
		function.Dump(os);
		return os;
	}

	/// Return a pointers to basic block list
	std::vector<std::unique_ptr<BasicBlock>> *getBasicBlocks()
	{
		return &basic_blocks;
	}

	/// Return number of basic blocks in the function
	int getNumBasicBlocks()
	{
		return basic_blocks.size();
	}

	/// Create a basic block that belongs to the function, and return a
	/// pointer to it. The new basic block will be internally freed when the
	/// function object is destroyed.
	BasicBlock *newBasicBlock(comm::LeafNode *node)
	{
		basic_blocks.emplace_back(new BasicBlock(this, node));
		return basic_blocks.back().get();
	}

	/// Add symbol to symbol table, constructed with symbol name, return
	/// a pointer the symbol
	Symbol *addSymbol(const std::string &name) 
	{
		return symbol_table.addSymbol(name);
	}

	/// Generate initialization code for the function in basic block
	/// 'basic_block_header'.
	void EmitHeader();

	/// Emit code to load arguments into registers. The code will be emitted
	/// in 'args_node'. UAVs will be created and loaded in 'uavs_node', as
	/// they are needed by new arguments.
	void EmitArgs();

	/// Emit code for the function body. The first basic block of the
	/// function will be added at the end of 'basic_block', which should be
	/// already part of the function. As the code emission progresses, new
	/// basic blocks will be created.
	void EmitBody();

	/// Process all virtual Phi instructions and replace them by Move
	/// instructions in the predecessor basic blocks.
	void EmitPhi();
	void EmitPhiMoves(si2bin::Instruction *inst);

	/// Emit additional instructions managing active masks and active mask
	/// stacks related with the function control flow.
	void EmitControlFlow();

	/// Perform analysis on live variables inside the llvm function to allow
	/// for memory efficient register allocation at a basic block level
	/// granularity.
	///
	/// Pseudo-code:
	///
	/// Populate def and use bitmaps for each basic block
	///
	/// For all basic blocks n: in[n] = out[n] = 0
	/// worklist = all exit basic blocks
	///
	/// while (worklist is not empty)
	///	remove n from worklist
	/// 	in[n] = ( out[n] - def[n] ) U use[n]
	///
	///	for each predecessor n':
	///		out[n'] = out[n'] U in[n]
	///		if ( out[n'] changed )
	///			add n' to worklist
	///
	void LiveRegisterAnalysis();

	/// Creates an interference graph populated by the register lifetimes
	/// seen from conducting live register analysis. Uses this interference
	/// graph to map and allocate registers to each pre-mapped register.
	///
	/// Pseudo-code:
	///
	/// n = number of registers available
	/// b = bitmap specifying previously defined interfering registers'
	///	mappings
	/// registerMap = T -> R
	/// T = used temporaries
	/// R = available registers
	///
	/// for (temp (column) in interference graph) {
	///	Bitmap b(n) // clear
	///
	///	for (each row in the temp's column; row < column)
	///		if (if vertex c -> r in int. graph is true)
	///			b[registerMap[row]] = true
	///
	///	iterate over b to find first 0 at index i
	///	registerMap[column] = i;
	/// }
	///
	void LiveRegisterAllocation();

	/// Dump the results of the live register analysis.
	void LiveRegisterAnalysisBitmapDump();

	/// Create a Southern Islands instruction argument from an LLVM value.
	/// The type of argument created depends on the LLVM value as follows:
	///
	/// - If the LLVM value is an integer constant, the Southern Islands
	///   argument will be of type integer literal.
	/// - If the LLVM value is an LLVM identifier, the Southern Islands
	///   argument will be the vector register associated with that symbol.
	///   In this case, the symbol is returned in argument 'symbol'.
	///
	std::unique_ptr<si2bin::Argument> TranslateValue(llvm::Value *llvm_value, Symbol *&symbol);

	/// Alternative syntax for TranslateValue() where the \a symbol output
	/// argument is omitted.
	std::unique_ptr<si2bin::Argument> TranslateValue(llvm::Value *llvm_value)
	{
		Symbol *symbol;
		return TranslateValue(llvm_value, symbol);
	}

	/// Convert an argument of type literal (any variant) into a vector
	/// register by emitting a 'v_mob_b32' instruction. The original
	/// argument is consumed and make part of the new instruction, while a
	/// new argument instance is returned containing the new vector
	/// register. If the original argument was not a literal, it will be
	/// returned directly, and no instruction is emitted.
	std::unique_ptr<si2bin::Argument>ConstToVReg(BasicBlock *basic_block, 
			std::unique_ptr<si2bin::Argument> arg);

	/// Allocate a scalar register
	///
	/// \param count
	///	Number of contiguous scalar registers to allocate.
	///
	/// \param align
	///	Alignment of the first scalar register in the group of allocated
	///	registers.
	///
	/// \return
	///	The function returns the first scalar register allocated in the
	///	group.
	int AllocSReg(int count = 1, int align = 1);
	
	/// Allocate a vector register
	///
	/// \param count
	///	Number of contiguous vector registers to allocate.
	///
	/// \param align
	///	Alignment of the first vector register in the group of allocated
	///	registers.
	///
	/// \return
	///	The function returns the first vector register allocated in the
	///	group.
	int AllocVReg(int count = 1, int align = 1);
};

}  // namespace llvm2si

#endif

