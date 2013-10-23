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
#include <llvm/Argument.h>
#include <llvm/Value.h>
#include <llvm/Type.h>
#include <m2c/common/Node.h>
#include <m2c/common/Tree.h>
#include <m2c/si2bin/Arg.h>

#include "Phi.h"
#include "Symbol.h"


namespace llvm2si
{

/* Forward declarations */
class BasicBlock;
class Function;

class FunctionArg
{
	/* Inherits from 'si_arg_t'.
	 * FIXME: if we port all or part of the SI assembler to work with
	 * classes, this should be a real inheritance. */
	//SIArg *si_arg;

	std::string name;
	
	/* Associated LLVM argument */
	llvm::Argument *llvm_arg;


	/* The fields below are populated when the argument is inserted into
	 * a function with a call to Function::AddArg(). */
	Function *function;

	/* Index occupied in function argument list */
	int index;
	
	/* For arguments of type SI::ArgTypePointer, and scope
	 * SI::ArgScopeUAV */
	int uav_index;

	/* Scalar register identifier containing the argument */
	int sreg;

	/* Vector register identifier containing the argument */
	int vreg;

public:

	/* Constructor */
	FunctionArg(llvm::Argument *llvm_arg);

	/* Dump */
	void Dump(std::ostream &os);
	friend std::ostream &operator<<(std::ostream &os, FunctionArg &arg)
			{ arg.Dump(os); return os; }

	/* Return a Southern Islands argument type from an LLVM type. */
	static SI::ArgDataType GetDataType(llvm::Type *llvm_type);
};


class FunctionUAV
{
	/* Function where it belongs */
	Function *function;

	/* UAV index in 'function->uav_list'. Uav10 has an index 0, uav11 has
	 * index 1, etc. */
	int index;

	/* Base scalar register of a group of 4 assigned to the UAV. This
	 * register identifier is a multiple of 4. */
	int sreg;

public:

	/* Getters */
	int GetSReg() { return sreg; }
};


class Function
{
	std::string name;

	/* Associated LLVM function */
	llvm::Function *llvm_function;

	/* Number of used registers */
	int num_sregs;  /* Scalar */
	int num_vregs;  /* Vector */

	int sreg_uav_table;  /* UAV table (2 registers) */
	int sreg_cb0;  /* CB0 (4 registers) */
	int sreg_cb1;  /* CB1 (4 registers) */
	int sreg_wgid;  /* Work-group ID (3 registers) */
	int sreg_gsize;  /* Global size (3 register) */
	int sreg_lsize;  /* Local size (3 registers) */
	int sreg_offs;  /* Global offset (3 registers) */

	int vreg_lid;  /* Local ID (3 registers) */
	int vreg_gid;  /* Global ID (4 registers) */

	/* List of arguments */
	std::list<std::unique_ptr<FunctionArg>> arg_list;

	/* Array of UAVs, starting at uav10. Each UAV is associated with one
	 * function argument using a buffer in global memory. */
	std::vector<std::unique_ptr<FunctionUAV>> uav_list;

	/* Predefined nodes */
	Common::LeafNode *header_node;
	Common::LeafNode *uavs_node;
	Common::LeafNode *args_node;
	Common::LeafNode *body_node;

	/* Symbol table associated with the function, storing LLVM variables */
	SymbolTable symbol_table;

	/* Control tree */
	Common::Tree tree;

	/* List of elements found in LLVM phi instructions during emission of
	 * the function body. */
	std::list<std::unique_ptr<Phi>> phi_list;

	/* While code is generated, this variable keeps track of the total
	 * amount of bytes pushed into the stack for this function. */
	unsigned int stack_size;

public:

	/* Constructor */
	explicit Function(llvm::Function *llvm_function);

	/* Getters */
	Common::Tree *GetTree() { return &tree; }
	int GetVRegGid() { return vreg_gid; }
	int GetSRegGSize() { return sreg_gsize; }
	FunctionUAV *GetUAV(int index) { return index >= 0 && index <
			(int) uav_list.size() ? uav_list[index].get() :
			nullptr; }

	/* Dump */
	void Dump(std::ostream &os);
	friend std::ostream &operator<<(std::ostream &os, Function &function)
			{ function.Dump(os); return os; }

	/* Add symbol to symbol table */
	void AddSymbol(Symbol *symbol) { symbol_table.AddSymbol(symbol); }

	/* Add phi node to list */
	void AddPhi(Phi *phi) { phi_list.emplace_back(phi); }

	/* Generate initialization code for the function in basic block
	 * 'basic_block_header'. */
	void EmitHeader();

	/* Emit code to load arguments into registers. The code will be emitted
	 * in 'args_node'. UAVs will be created and loaded in 'uavs_node', as
	 * they are needed by new arguments. */
	void EmitArgs();

	/* Emit code for the function body. The first basic block of the
	 * function will be added at the end of 'basic_block', which should be
	 * already part of the function. As the code emission progresses, new
	 * basic blocks will be created. */
	void EmitBody();

	/* Emit code for the phi elements that were encountered during the
	 * emission of the function body, comming from LLVM phi nodes. */
	void EmitPhi();

	/* Emit additional instructions managing active masks and active mask
	 * stacks related with the function control flow. */
	void EmitControlFlow();

	/* Perform analysis on live variables inside the llvm function to allow
	 * for memory efficient register allocation */
	void LiveRegisterAnalysis();
	void LiveRegisterAnalysisBitmapDump();

	/* Create a Southern Islands instruction argument from an LLVM value.
	 * The type of argument created depends on the LLVM value as follows:
	 *   - If the LLVM value is an integer constant, the Southern Islands
	 *     argument will be of type integer literal.
	 *   - If the LLVM value is an LLVM identifier, the Southern Islands
	 *     argument will be the vector register associated with that symbol.
	 *     In this case, the symbol is returned in argument 'symbol'.
	 */
	si2bin::Arg *TranslateValue(llvm::Value *llvm_value, Symbol *&symbol);
	si2bin::Arg *TranslateValue(llvm::Value *llvm_value) { Symbol *symbol;
			return TranslateValue(llvm_value, symbol); }

	/* Convert an argument of type literal (any variant) into a vector
	 * register by emitting a 'v_mob_b32' instruction. The original argument
	 * is consumed and make part of the new instruction, while a new
	 * argument instance is returned containing the new vector register. If
	 * the original argument was not a literal, it will be returned
	 * directly, and no instruction is emitted. */
	si2bin::Arg *ConstToVReg(BasicBlock *basic_block, si2bin::Arg *arg);

	/* Allocate 'count' scalar/vector registers where the first register
	 * identifier is a multiple of 'align'. */
	int AllocSReg(int count = 1, int align = 1);
	int AllocVReg(int count = 1, int align = 1);
};

}  /* namespace llvm2si */

#endif

