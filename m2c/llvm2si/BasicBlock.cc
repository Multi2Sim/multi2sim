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

#include <lib/cpp/Misc.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Type.h>

#include "BasicBlock.h"
#include "Context.h"
#include "Function.h"


// FIXME - This is against the coding guidelines, but all the code for the
// back-end and disassembler will eventually be restructure to revolve around
// a centralized SI program representation, formed for classes for a module,
// function, basic block, instruction, and argument.
using namespace si2bin;

namespace llvm2si
{

int BasicBlock::id_counter;

int BasicBlock::getLlvmTypeSize(llvm::Type *llvm_type)
{
	// Return size based on type kind
	int num_elems;
	if (llvm_type->isVectorTy())
	{
		num_elems = llvm_type->getVectorNumElements();
		llvm_type = llvm_type->getVectorElementType();
	}
	else
	{
		num_elems = 1;
	}

	// Actions depending on type
	if (llvm_type->isIntegerTy())
	{
		return ((llvm_type->getIntegerBitWidth() + 7) / 8 * num_elems);
	}
	else if (llvm_type->isPointerTy() ||
			llvm_type->isFloatTy())
	{
		return 4 * num_elems;
	}
	else
	{
		llvm_type->dump();
		throw misc::Panic("Unsupported LLVM type kind");
	}
}


int BasicBlock::getPointedLlvmTypeSize(llvm::Type *llvm_type)
{
	// Get pointed type
	assert(llvm_type->isPointerTy());
	llvm_type = llvm_type->getPointerElementType();
	return getLlvmTypeSize(llvm_type);
}


void BasicBlock::EmitAdd(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 32-bit integers
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32))
		throw misc::Panic("Unsupported for 32-bit integers");

	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("Only supported for 2 operands, "
				"%d found", llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Second operand cannot be a constant
	arg2 = std::move(function->ConstToVReg(this, std::move(arg2)));
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);
	arg2->ValidTypes(Argument::TypeVectorRegister);

	// Allocate vector register and create symbol for return value
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(llvm_inst->getName());
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Emit addition.
	// v_add_i32 ret_vreg, vcc, arg_op1, arg_op2
	Instruction *instruction = addInstruction(SI::INST_V_ADD_I32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addSpecialRegister(SI::InstSpecialRegVcc);
	instruction->addArgument(std::move(arg1));
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());
}


void BasicBlock::EmitCall(llvm::CallInst *llvm_inst)
{
	// Get called function, found in last operand of the operand list, as
	// returned by LLVMGetOperand.
	llvm::Function *llvm_function = llvm_inst->getCalledFunction();
	std::string func_name = llvm_function->getName();

	// Get return argument name
	std::string var_name = llvm_inst->getName();
	if (var_name.empty())
		throw misc::Panic("Invalid return variable");

	// Number of arguments
	if (llvm_inst->getNumOperands() - 1 !=
			llvm_function->getArgumentList().size())
		throw Error(misc::fmt("%d arguments expected for "
				"'%s', %d found",
				llvm_inst->getNumOperands() - 1,
				func_name.c_str(),
				(int) llvm_function->getArgumentList().size()));

	// Get argument and check type
	llvm::Value *llvm_op = llvm_inst->getOperand(0);
	llvm::Type *llvm_type = llvm_op->getType();
	if (!llvm_type->isIntegerTy() || !llvm::isa<llvm::ConstantInt>(llvm_op))
		throw Error("Argument should be an integer constant");

	// Get argument value and check bounds
	llvm::ConstantInt *llvm_const = llvm::cast<llvm::ConstantInt>(llvm_op);
	int dim = llvm_const->getZExtValue();
	if (!misc::inRange(dim, 0, 2))
		throw Error("Constant in range [0..2] expected");

	// Built-in functions
	if (func_name == "__get_global_id_u32")
	{
		// Create new symbol associating it with the vector register
		// containing the global ID in the given dimension.
		Symbol *ret_symbol = function->addSymbol(var_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister,
				function->getVRegGid() + dim);
	}
	else if (func_name == "__get_local_id_u32")
	{
		// Create new symbol associating it with the vector register
		// containing the global ID in the given dimension.
		Symbol *ret_symbol = function->addSymbol(var_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister,
				function->getVRegLid() + dim);
	}
	else if (func_name == "__get_group_id_u32")
	{
		// Allocate a new vector register to copy work group id.
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(var_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Create new vector register containing the work group id.
		// v_mov_b32 vreg, s[wgid + dim]
		Instruction *instruction = addInstruction(SI::INST_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(function->getSRegWGid() + dim);
		assert(instruction->hasValidArguments());

		// Create new symbol associating it with the scalar register
		// containing the group ID in the given dimension.
		/* Symbol *ret_symbol = function->addSymbol(var_name);
		ret_symbol->setRegister(Symbol::TypeScalarRegister,
					function->getSRegWGid() + dim); */
	}
	else if (func_name == "__get_global_size_u32")
	{
		// Allocate a new vector register to copy global size.
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(var_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Create new vector register containing the global size.
		// v_mov_b32 vreg, s[gsize + dim]
		Instruction *instruction = addInstruction(SI::INST_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(function->getSRegGSize() + dim);
		assert(instruction->hasValidArguments());
	}
	else if (func_name == "__get_local_size_u32")
	{
		// Allocate a new vector register to copy local size.
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(var_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Create new vector register containing the local size.
		// v_mov_b32 vreg, s[lsize + dim]
		Instruction *instruction = addInstruction(SI::INST_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(function->getSRegLSize() + dim);
		assert(instruction->hasValidArguments());
	}
	else if (func_name == "__get_num_groups_u32")
	{
		// Allocate a new vector register to copy local size.
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(var_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Create new vector register containing the local size.
		// v_mov_b32 vreg, s[num_of_workgroups + dim]
		Instruction *instruction = addInstruction(SI::INST_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(function->getNumOfWG() + dim);
		assert(instruction->hasValidArguments());
	}
	else
	{
		throw Error("Invalid built-in function: " + func_name);
	}
}


void BasicBlock::EmitGetElementPtr(llvm::GetElementPtrInst *llvm_inst)
{
	// Only supported for 2 operands (pointer + 1 index)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get pointer operand (vreg)
	llvm::Value *llvm_arg_ptr = llvm_inst->getPointerOperand();
	Symbol *ptr_symbol;
	std::unique_ptr<Argument> arg_ptr = function->TranslateValue(
			llvm_arg_ptr, ptr_symbol);
	arg_ptr->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeScalarRegister);

	// If arg_ptr is a scalar register convert it to a vector register
	if (arg_ptr->getType() == Argument::TypeScalarRegister)
	{	
		ArgScalarRegister *arg_scalar = 
				dynamic_cast<ArgScalarRegister *>(arg_ptr.get());
		
		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_arg_ptr->getName();
		int ret_vreg = function->AllocVReg();
		ptr_symbol->setType(Symbol::TypeVectorRegister);
		ptr_symbol->setRegister(ret_vreg);

		// Emit instruction
		// v_mov_b32 ret_vreg, arg1
		Instruction *instruction = addInstruction(SI::INST_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(arg_scalar->getId());
		assert(instruction->hasValidArguments());

		arg_ptr = misc::new_unique<ArgVectorRegister>(ret_vreg);
	}


	// Address must be a symbol with UAV
	assert(ptr_symbol && "symbol not found");
	assert(ptr_symbol->isAddress() && "no UAV for symbol");
	
	// Get size of pointed value
	llvm::Type *llvm_type_ptr = llvm_arg_ptr->getType();
	int ptr_size = getPointedLlvmTypeSize(llvm_type_ptr);

	// Get index operand (vreg, literal)
	llvm::Value *llvm_arg_index = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg_index = function->TranslateValue(llvm_arg_index);
	arg_index->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);
	ret_symbol->setUAVIndex(ptr_symbol->getUAVIndex());

	// Calculate offset as the multiplication between 'arg_index' and the
	// size of the pointed element ('ptr_size'). If 'arg_index' is a
	// literal, we can pre-calculate it here. If 'arg_index' is a vector
	// register, we need to emit an instruction.
	std::unique_ptr<Argument> arg_offset;
	if (arg_index->getType() == Argument::TypeLiteral ||
			arg_index->getType() == Argument::TypeLiteralReduced)
	{
		// Argument 'arg_offset' is just a modification of
		// 'arg_index'.
		auto *arg_offset_literal = dynamic_cast<ArgLiteral *>(arg_index.get());
		assert(arg_offset_literal);
		arg_offset_literal->setValue(arg_offset_literal->getValue() * ptr_size);
		arg_offset.reset(arg_offset_literal);
	}
	else
	{
		// Allocate one register and create 'arg_offset' with it
		int tmp_vreg = function->AllocVReg();
		arg_offset = misc::new_unique<ArgVectorRegister>(tmp_vreg);

		// Emit calculation of offset as the multiplication between the
		// index argument and the pointed element size.
		//
		// v_mul_i32_i24 tmp_vreg, ptr_size, arg_index
		//
		Instruction *instruction = addInstruction(SI::INST_V_MUL_I32_I24);
		instruction->addVectorRegister(tmp_vreg);
		instruction->addLiteral(ptr_size);
		instruction->addArgument(std::move(arg_index));
		assert(instruction->hasValidArguments());
	}

	// Emit effective address calculation as the addition between the
	// original pointer and the offset.
	//
	// v_add_i32 ret_vreg, vcc, arg_offset, arg_pointer
	//
	Instruction *instruction = addInstruction(SI::INST_V_ADD_I32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addSpecialRegister(SI::InstSpecialRegVcc);
	instruction->addArgument(std::move(arg_offset));
	instruction->addArgument(std::move(arg_ptr));
	assert(instruction->hasValidArguments());
}


void BasicBlock::EmitICmp(llvm::ICmpInst *llvm_inst)
{
	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Only supported for 32-bit integers
	llvm::Type *llvm_type = llvm_arg1->getType();
	if (!llvm_type->isIntegerTy(32))
		throw misc::Panic("Only supported for 32-bit integers");

	// Only the first argument can be a literal. If the second argument is
	// a literal, flip them and invert comparison predicate later.
	bool invert = false;
	if (arg2->getType() != Argument::TypeVectorRegister)
	{
		arg1.swap(arg2);
		invert = true;
	}

	// Valid argument types. Argument 2 cannot be a literal.
	arg2 = std::move(function->ConstToVReg(this, std::move(arg2)));
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);
	arg2->ValidTypes(Argument::TypeVectorRegister);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_sreg_series = function->AllocSReg(2, 2);
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeScalarRegister,
			ret_sreg_series, ret_sreg_series + 1);

	// Choose instruction based on predicate
	SI::InstOpcode opcode = SI::InstOpcodeInvalid;
	llvm::CmpInst::Predicate llvm_pred = llvm_inst->getPredicate();
	switch (llvm_pred)
	{

	case llvm::CmpInst::ICMP_EQ:

		opcode = SI::INST_V_CMP_EQ_I32;
		break;

	case llvm::CmpInst::ICMP_NE:

		opcode = SI::INST_V_CMP_NE_I32;
		break;

	case llvm::CmpInst::ICMP_UGT:

		opcode = SI::INST_V_CMP_GT_U32;
		if (invert)
			opcode = SI::INST_V_CMP_LT_U32;
		break;

	case llvm::CmpInst::ICMP_UGE:

		opcode = SI::INST_V_CMP_GE_U32;
		if (invert)
			opcode = SI::INST_V_CMP_LE_U32;
		break;

	case llvm::CmpInst::ICMP_ULT:

		opcode = SI::INST_V_CMP_LT_U32;
		if (invert)
			opcode = SI::INST_V_CMP_GT_U32;
		break;

	case llvm::CmpInst::ICMP_ULE:

		opcode = SI::INST_V_CMP_LE_U32;
		if (invert)
			opcode = SI::INST_V_CMP_GE_U32;
		break;

	case llvm::CmpInst::ICMP_SGT:

		opcode = SI::INST_V_CMP_GT_I32;
		if (invert)
			opcode = SI::INST_V_CMP_LT_I32;
		break;

	case llvm::CmpInst::ICMP_SGE:

		opcode = SI::INST_V_CMP_GE_I32;
		if (invert)
			opcode = SI::INST_V_CMP_LE_I32;
		break;

	case llvm::CmpInst::ICMP_SLT:

		opcode = SI::INST_V_CMP_LT_I32;
		if (invert)
			opcode = SI::INST_V_CMP_GT_I32;
		break;

	case llvm::CmpInst::ICMP_SLE:

		opcode = SI::INST_V_CMP_LE_I32;
		if (invert)
			opcode = SI::INST_V_CMP_GE_I32;
		break;

	default:

		throw misc::Panic(misc::fmt("Unsupported predicate (%d)",
				llvm_pred));
	}

	// Emit comparison.
	//
	// v_cmp_<pred>_<type> vcc, arg_op1, arg_op2
	//
	Instruction *instruction = addInstruction(opcode);
	instruction->addSpecialRegister(SI::InstSpecialRegVcc);
	instruction->addArgument(std::move(arg1));
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());

	// Store 'vcc' in scalar register
	//
	// s_mov_b64 ret_sreg_series, vcc
	//
	instruction = addInstruction(SI::INST_S_MOV_B64);
	instruction->addScalarRegisterSeries(ret_sreg_series, ret_sreg_series + 1);
	instruction->addSpecialRegister(SI::InstSpecialRegVcc);
	assert(instruction->hasValidArguments());
}


void BasicBlock::EmitLoad(llvm::LoadInst *llvm_inst)
{
	// Only supported for 1 operand (address)
	if (llvm_inst->getNumOperands() != 1)
		throw misc::Panic(misc::fmt("1 operand supported, %d found",
				llvm_inst->getNumOperands()));

	// Get address operand (vreg)
	llvm::Value *llvm_arg_addr = llvm_inst->getOperand(0);
	Symbol *addr_symbol;
	std::unique_ptr<Argument> arg_addr = function->TranslateValue(llvm_arg_addr, addr_symbol);
	arg_addr->ValidTypes(Argument::TypeVectorRegister);

	// Address must be a symbol with UAV
	if (!addr_symbol || !addr_symbol->isAddress())
		throw Error("No UAV for symbol: " + addr_symbol->getName());

	// Get UAV
	FunctionUAV *uav = function->getUAV(addr_symbol->getUAVIndex());
	if (!uav)
		throw Error(misc::fmt("Invalid UAV index (%d)",
				addr_symbol->getUAVIndex()));

	// Get address space - only 1 (global mem.) supported for now
	llvm::Type *llvm_type = llvm_arg_addr->getType();
	int addr_space = llvm_type->getPointerAddressSpace();
	if (addr_space != 1)
		throw Error(misc::fmt("Address space 1 expected, %d found",
				addr_space));

	// Get return type (data) - only support 4-byte types for now
	llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32) && !llvm_type->isFloatTy())
		throw misc::Panic("Only 4-byte int/float types supported");

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Emit memory load instruction.
	//
	// tbuffer_load_format_x v[value_symbol->vreg], v[pointer_symbol->vreg],
	// 	s[sreg_uav,sreg_uav+3],
	//	0 offen format:[BUF_DATA_FORMAT_32,BUF_NUM_FORMAT_FLOAT]
	//
	Instruction *instruction = addInstruction(SI::INST_TBUFFER_LOAD_FORMAT_X);
	instruction->addVectorRegister(ret_vreg);
	instruction->addArgument(std::move(arg_addr));
	instruction->addScalarRegisterSeries(uav->getSReg(), uav->getSReg() + 3);
	instruction->addMemoryAddress(new ArgLiteral(0),
			new ArgMaddrQual(true, false, 0),
			SI::InstBufDataFormat32,
			SI::InstBufNumFormatFloat);
	assert(instruction->hasValidArguments());
	
	instruction = addInstruction(SI::INST_S_WAITCNT);
	instruction->addWaitCounter(ArgWaitCounter::CounterTypeVmCnt);
	assert(instruction->hasValidArguments());
}


void BasicBlock::EmitMul(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Only supported for 32-bit integers
	llvm::Type *type = llvm_inst->getType();
	if (!type->isIntegerTy(32))
		throw misc::Panic("Only supported for 32-bit integers");

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);
	
	// If arg1 is a scalar register convert it to a vector register
	if (arg1->getType() == Argument::TypeScalarRegister)
	{	
		ArgScalarRegister *arg_scalar = 
				dynamic_cast<ArgScalarRegister *>(arg1.get());
		
		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_arg1->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Emit instruction
		//
		// v_mov_b32 ret_vreg, arg1
		//
		Instruction *instruction = addInstruction(SI::INST_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(arg_scalar->getId());
		assert(instruction->hasValidArguments());

		arg1 = misc::new_unique<ArgVectorRegister>(ret_vreg);
	}
	
	// If arg2 is a scalar register convert it to a vector register
	if (arg2->getType() == Argument::TypeScalarRegister)
	{	
		ArgScalarRegister *arg_scalar = 
				dynamic_cast<ArgScalarRegister *>(arg2.get());
		
		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_arg2->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);


		// Emit instruction
		//
		// v_mov_b32 ret_vreg, arg1
		//
		Instruction *instruction = addInstruction(SI::INST_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(arg_scalar->getId());
		assert(instruction->hasValidArguments());

		arg2 = misc::new_unique<ArgVectorRegister>(ret_vreg);
	}

	// Only the first operand can be a constant, so swap them if there is
	// a constant in the second.
	if (arg2->getType() != Argument::TypeVectorRegister)
		arg1.swap(arg2);
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);
	arg2->ValidTypes(Argument::TypeVectorRegister);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Emit effective address calculation.
	//
	// v_mul_lo_i32 ret_vreg, arg_op1, arg_op2
	//
	Instruction *instruction = addInstruction(SI::INST_V_MUL_LO_U32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addArgument(std::move(arg1));
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());
}


void BasicBlock::EmitPhi(llvm::PHINode *llvm_inst)
{
	// Only supported for 32-bit integers
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32))
		throw misc::Panic("Only supported for 32-bit integers");

	// Argument list for output Phi instruction
	std::vector<std::unique_ptr<Argument>> arg_list;

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Add destination argument
	ArgVectorRegister *ret_arg = new ArgVectorRegister(ret_vreg);
	arg_list.emplace_back(ret_arg);

	// Process arguments
	for (unsigned i = 0; i < llvm_inst->getNumIncomingValues(); i++)
	{
		// Get source label
		llvm::BasicBlock *llvm_basic_block = llvm_inst->getIncomingBlock(i);
		std::string label = llvm_basic_block->getName();

		// Get source vector register mapped to LLVM value
		llvm::Value *src_value = llvm_inst->getIncomingValue(i);
		Argument *src_arg = function->TranslateValue(src_value).get();
		ArgVectorRegister *src_arg_vreg = misc::cast<ArgVectorRegister *>(src_arg);
		int src_vreg = src_arg_vreg->getId();

		// Create Phi argument and set vector register
		ArgPhi *arg = new si2bin::ArgPhi(label);
		arg->setVectorRegister(src_vreg);
		arg_list.emplace_back(arg);
	}

	// Emit Phi instruction
	Instruction *instruction = addInstruction(SI::INST_PHI);
	for (auto &arg: arg_list)
		instruction->addArgument(std::move(arg));
	assert(instruction->hasValidArguments());

	// Process arguments
	/*for (unsigned i = 0; i < llvm_inst->getNumIncomingValues(); i++)
	{
		// Get item
		llvm::BasicBlock *llvm_basic_block = llvm_inst->getIncomingBlock(i);
		llvm::Value *value = llvm_inst->getIncomingValue(i);

		// Find node
		std::string name = llvm_basic_block->getName();
		comm::Tree *tree = function->getTree();
		comm::LeafNode *node = tree->getLeafNode(name);
		if (!node)
			throw misc::Panic("Cannot find node: " + name);

		// Create destination argument
		Arg *arg = new ArgVectorRegister(ret_vreg);

		// Create 'phi' element and add it.
		Phi *phi = new Phi(node, value, arg);
		function->AddPhi(phi);
	}*/
}


void BasicBlock::EmitRet(llvm::ReturnInst *llvm_inst)
{
	// Emit program end instruction.
	//
	// s_endpgm
	//
	addInstruction(SI::INST_S_ENDPGM);
}


void BasicBlock::EmitStore(llvm::StoreInst *llvm_inst)
{
	// Only supported for 2 operand (address, data)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get data operand (vreg)
	llvm::Value *llvm_arg_data = llvm_inst->getOperand(0);
	std::unique_ptr<Argument> arg_data = function->TranslateValue(
			llvm_arg_data);
	arg_data = std::move(function->ConstToVReg(this, std::move(arg_data)));
	arg_data->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeScalarRegister);

	// If arg_data is a scalar register convert it to a vector register
	if (arg_data->getType() == Argument::TypeScalarRegister)
	{	
		ArgScalarRegister *arg_scalar = 
				dynamic_cast<ArgScalarRegister *>(arg_data.get());
		
		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_arg_data->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);


		// Emit instruction
		//
		// v_mov_b32 ret_vreg, arg1
		//
		Instruction *instruction = addInstruction(SI::INST_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(arg_scalar->getId());
		assert(instruction->hasValidArguments());

		arg_data = misc::new_unique<ArgVectorRegister>(ret_vreg);
	}

	// Get address operand (vreg)
	llvm::Value *llvm_arg_addr = llvm_inst->getOperand(1);
	Symbol *addr_symbol;
	std::unique_ptr<Argument> arg_addr = function->TranslateValue(llvm_arg_addr, addr_symbol);
	arg_addr->ValidTypes(Argument::TypeVectorRegister);

	// Address must be a symbol with UAV
	if (!addr_symbol || !addr_symbol->isAddress())
		throw Error("No UAV for symbol: " + addr_symbol->getName());

	// Get UAV
	FunctionUAV *uav = function->getUAV(addr_symbol->getUAVIndex());
	if (!uav)
		throw Error(misc::fmt("Invalid UAV index (%d)",
				addr_symbol->getUAVIndex()));

	// Get address space - only 1 (global mem.) supported for now
	llvm::Type *llvm_type = llvm_arg_addr->getType();
	int addr_space = llvm_type->getPointerAddressSpace();
	if (addr_space != 1)
		throw misc::Panic(misc::fmt("Only address space 1 supported, "
				"%d found", addr_space));

	// Get type of data - only support 4-byte types for now
	llvm_type = llvm_arg_data->getType();
	if (!llvm_type->isIntegerTy(32) && !llvm_type->isFloatTy())
		throw misc::Panic("Only 4-byte int/float types supported");

	// Emit memory write.
	//
	// tbuffer_store_format_x v[value_symbol->vreg], s[pointer_symbol->vreg],
	// 	s[sreg_uav,sreg_uav+3], 0 offen format:[BUF_DATA_FORMAT_32,
	// 	BUF_NUM_FORMAT_FLOAT]
	//
	Instruction *instruction = addInstruction(SI::INST_TBUFFER_STORE_FORMAT_X);
	instruction->addArgument(std::move(arg_data));
	instruction->addArgument(std::move(arg_addr));
	instruction->addScalarRegisterSeries(uav->getSReg(), uav->getSReg() + 3);
	instruction->addMemoryAddress(new ArgLiteral(0),
			new ArgMaddrQual(true, false, 0),
			SI::InstBufDataFormat32,
			SI::InstBufNumFormatFloat);
	assert(instruction->hasValidArguments());

	instruction = addInstruction(SI::INST_S_WAITCNT);
	instruction->addWaitCounter(ArgWaitCounter::CounterTypeExpCnt);
	assert(instruction->hasValidArguments());
}


void BasicBlock::EmitSub(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 32-bit integers
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32))
		throw misc::Panic("Only supported for 32-bit integers");

	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Operand 2 cannot be a constant
	arg2 = std::move(function->ConstToVReg(this, std::move(arg2)));
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);
	arg2->ValidTypes(Argument::TypeVectorRegister);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Emit subtraction.
	//
	// v_sub_i32 ret_vreg, vcc, arg_op1, arg_op2
	//
	Instruction *instruction = addInstruction(SI::INST_V_SUB_I32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addSpecialRegister(SI::InstSpecialRegVcc);
	instruction->addArgument(std::move(arg1));
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());
}


void BasicBlock::EmitFAdd(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for floats
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isFloatTy())
		throw misc::Panic("Only supported for float type arguments");

	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Second operand cannot be a constant
	arg2 = std::move(function->ConstToVReg(this, std::move(arg2)));
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);
	arg2->ValidTypes(Argument::TypeVectorRegister);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Emit addition.
	//
	// v_add_f32 ret_vreg, arg_op1, arg_op2
	//
	Instruction *instruction = addInstruction(SI::INST_V_ADD_F32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addArgument(std::move(arg1));
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());
}


void BasicBlock::EmitFSub(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for floats
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isFloatTy())
		throw misc::Panic("Only supported for float type arguments");

	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Second operand cannot be a constant
	arg2 = std::move(function->ConstToVReg(this, std::move(arg2)));
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);
	arg2->ValidTypes(Argument::TypeVectorRegister);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Emit addition.
	//
	// v_sub_f32 ret_vreg, arg_op1, arg_op2
	//
	Instruction *instruction = addInstruction(SI::INST_V_SUB_F32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addArgument(std::move(arg1));
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());
}


void BasicBlock::EmitFMul(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Only supported for 32-bit integers
	llvm::Type *type = llvm_inst->getType();
	if (!type->isFloatTy())
		throw misc::Panic("Only supported for float type arguments");

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Only the first operand can be a constant, so swap them if there is
	// a constant in the second.
	if (arg2->getType() != Argument::TypeVectorRegister)
		arg1.swap(arg2);
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);
	arg2->ValidTypes(Argument::TypeVectorRegister);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Emit effective address calculation.
	//
	// v_mul_f32 ret_vreg, arg_op1, arg_op2
	//
	Instruction *instruction = addInstruction(SI::INST_V_MUL_F32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addArgument(std::move(arg1));
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());
}


void BasicBlock::EmitFDiv(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Only supported for 32-bit integers
	llvm::Type *type = llvm_inst->getType();
	if (!type->isFloatTy())
		throw misc::Panic("Only supported for float type arguments");

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Only the first operand can be a constant, so swap them if there is
	// a constant in the second.
	if (arg2->getType() != Argument::TypeVectorRegister)
		arg1.swap(arg2);
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);
	arg2->ValidTypes(Argument::TypeVectorRegister);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// For float division, the compiler provides extra steps to prevent
	// possible overflow/downflow.
	//
	// LLVM IR is shown below to calcuate a = b/c.
	//
	// 		%a = fdiv float %b, %c
	//
	// Here is the corresponding SI assembly code.
	//
	// 		s_mov_b32     s0, 0x6f800000
	//  	v_mov_b32     v2, 0x2f800000
	//		s_waitcnt     vmcnt(1)
	//		v_cmp_gt_f32  vcc, abs(v1), s0
	//		v_cndmask_b32  v2, 1.0, v2, vcc
	//		v_mul_f32     v1, v1, v2
	//		v_rcp_f32     v1, v1
	//		s_waitcnt     vmcnt(0)
	//		v_mul_f32     v1, v3, v1
	//		v_mul_f32     v1, v2, v1
	//
	// The final result of a is stored in v1. The SSA format is shown here,
	// in order to better understand this implementation. The %a is saved in v7.
	//
	//		s_mov_b32     s0, 1.0 * 2^96
	//		v_mov_b32     v2, 1.0 * 2^-32
	//		v_cmp_gt_f32  vcc, abs(v1), s0
	//		v_cndmask_b32  v3, 1.0, v2, vcc
	//		v_mul_f32     v4, v1, v3
	//		v_rcp_f32     v5, v4
	//		v_mul_f32     v6, v0, v5
	//		v_mul_f32     v7, v3, v6
	//
	// The pseudocode is illustrated below.
	//
	//		if |%c| > 2^96
	//			v3 = 2^-32
	//		else
	//			v3 = 1.0
	//
	//		v4 = %c * v3
	//
	//		v5 = 1 / (%c * v3)
	//
	//		v6 = %b / (%c * v3)
	//
	//		%a = ( v3 * %b ) / (v3 * %c) = %b / %c
	//
	// Currently, the v3 step has not been implemented yet. A simplified version of
	// floating point division is carried out.

	int arg2_rcp_id = function->AllocVReg();

	Instruction *instruction = addInstruction(SI::INST_V_RCP_F32);
	instruction->addVectorRegister(arg2_rcp_id);
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());

	instruction = addInstruction(SI::INST_V_MUL_F32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addArgument(std::move(arg1));
	instruction->addVectorRegister(arg2_rcp_id);
	assert(instruction->hasValidArguments());
}


void BasicBlock::EmitAnd(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 32-bit integers
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32))
		throw misc::Panic("Only supported for 32-bit integers");

	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
			llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Second operand cannot be a constant
	arg2 = std::move(function->ConstToVReg(this, std::move(arg2)));
	arg2->ValidTypes(Argument::TypeVectorRegister);
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);

	// Allocate vector register and create symbol for return value
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(llvm_inst->getName());
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Emit AND.
	//
	// v_and_b32 ret_vreg, arg_op1, arg_op2
	//
	Instruction *instruction = addInstruction(SI::INST_V_AND_B32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addArgument(std::move(arg1));
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());
}

void BasicBlock::EmitOr(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 32-bit integers
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32))
		throw misc::Panic("Only supported for 32-bit integers");

	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Second operand cannot be a constant
	arg2 = std::move(function->ConstToVReg(this, std::move(arg2)));
	arg2->ValidTypes(Argument::TypeVectorRegister);
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);

	// Allocate vector register and create symbol for return value
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(llvm_inst->getName());
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Emit OR.
	//
	// v_or_b32 ret_vreg, arg_op1, arg_op2
	//
	Instruction *instruction = addInstruction(SI::INST_V_OR_B32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addArgument(std::move(arg1));
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());
}

void BasicBlock::EmitXor(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 32-bit integers
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32))
		throw misc::Panic("Only supported for 32-bit integers");

	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Second operand cannot be a constant
	arg2 = std::move(function->ConstToVReg(this, std::move(arg2)));
	arg2->ValidTypes(Argument::TypeVectorRegister);
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);

	// Allocate vector register and create symbol for return value
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(llvm_inst->getName());
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Emit XOR.
	//
	// v_xor_b32 ret_vreg, arg_op1, arg_op2
	//
	Instruction *instruction = addInstruction(SI::INST_V_XOR_B32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addArgument(std::move(arg1));
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());
}


void BasicBlock::EmitSExt(llvm::SExtInst *llvm_inst)
{
	throw misc::Panic("Unimplemented LLVM instruction");
}


void BasicBlock::EmitExtractElement(llvm::ExtractElementInst *llvm_inst)
{
	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// First argument must be a scalar register and the second must be the
	// offset.
	arg1->ValidTypes(Argument::TypeScalarRegister);
	arg2->ValidTypes(Argument::TypeLiteral,
			Argument::TypeLiteralReduced);

	// Obtain actual scalar register
	ArgScalarRegister *arg1_scalar = dynamic_cast<ArgScalarRegister *>(arg1.get());
	ArgLiteral *arg2_literal = dynamic_cast<ArgLiteral *>(arg2.get());
	assert(arg1_scalar);
	assert(arg2_literal);
	arg1_scalar->setId(arg1_scalar->getId() + arg2_literal->getValue());

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	//int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeScalarRegister, arg1_scalar->getId());
}


void BasicBlock::EmitInsertElement(llvm::InsertElementInst *llvm_inst)
{
	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 3)
		throw misc::Panic(misc::fmt("3 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	llvm::Value *llvm_arg3 = llvm_inst->getOperand(2);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);
	std::unique_ptr<Argument> arg3 = function->TranslateValue(llvm_arg3);

	// First argument must be a scalar register and the second must be the
	// offset.
	arg1->ValidTypes(Argument::TypeScalarRegister,
			Argument::TypeVectorRegister);
	arg2->ValidTypes(Argument::TypeScalarRegister,
			Argument::TypeVectorRegister);
	arg3->ValidTypes(Argument::TypeLiteral,
			Argument::TypeLiteralReduced);

	// Rule out case where dest is vgpr and src is sgpr
	assert((arg1->getType() != Argument::TypeScalarRegister) && 
			(arg2->getType() != Argument::TypeVectorRegister));

	ArgLiteral *arg3_literal = dynamic_cast<ArgLiteral *>(arg3.get());
	assert(arg3_literal);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_arg2->getName();
	
	if (arg1->getType() == Argument::TypeScalarRegister && 
			arg2->getType() == Argument::TypeScalarRegister)
	{
		ArgScalarRegister *arg1_scalar =
			dynamic_cast<ArgScalarRegister *>(arg1.get());
		arg1_scalar->setId(arg1_scalar->getId() + 
				arg3_literal->getValue());
		
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeScalarRegister,
				arg1_scalar->getId());
		
		// Emit instruction
		//
		// s_mov_b32 arg1, arg2
		///
		Instruction *instruction = addInstruction(SI::INST_S_MOV_B32);
		instruction->addArgument(std::move(arg1));
		instruction->addArgument(std::move(arg2));
		assert(instruction->hasValidArguments());
	}
	else
	{
		ArgVectorRegister *arg1_vector = 
				dynamic_cast<ArgVectorRegister *>(arg1.get());
		arg1_vector->setId(arg1_vector->getId() + 
				arg3_literal->getValue());
		
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, 
				arg1_vector->getId());
		
		// Emit instruction
		//
		// v_mov_b32 arg2, arg1
		///
		Instruction *instruction = addInstruction(SI::INST_V_MOV_B32);
		instruction->addArgument(std::move(arg2));
		instruction->addArgument(std::move(arg1));
		assert(instruction->hasValidArguments());
	}
}


void BasicBlock::Dump(std::ostream &os)
{
	// Nothing if basic block is empty
	if (!instructions.size())
		return;

	// Label with node's name
	comm::LeafNode *node = getNode();
	os << "\n" << node->getName() << ":\n";

	// Print list of instructions
	for (auto &instruction : instructions)
		instruction->Dump(os);
}


void BasicBlock::AddInst(Instruction *instruction)
{
	// Check that the instruction does not belong to any other basic
	// block already.
	if (instruction->getBasicBlock())
		throw misc::Panic("Instruction already added to basic block");

	// Add instruction
	instructions.emplace_back(instruction);
	instruction->setBasicBlock(this);

	// If there was a comment added to the basic block, attach it to
	// the instruction being added now.
	if (!comment.empty())
	{
		instruction->setComment(comment);
		comment.clear();
	}
}


void BasicBlock::Emit(llvm::BasicBlock *llvm_basic_block)
{
	// Iterate over LLVM instructions
	assert(llvm_basic_block);
	for (auto &llvm_inst : llvm_basic_block->getInstList())
	{
		switch (llvm_inst.getOpcode())
		{

		case llvm::Instruction::Add:

			EmitAdd(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::Br:

			// Ignore branches here
			break;

		case llvm::Instruction::Call:

			EmitCall(misc::cast<llvm::CallInst *>(&llvm_inst));
			break;

		case llvm::Instruction::GetElementPtr:

			EmitGetElementPtr(misc::cast<llvm::GetElementPtrInst *>
					(&llvm_inst));
			break;

		case llvm::Instruction::ICmp:

			EmitICmp(misc::cast<llvm::ICmpInst *>(&llvm_inst));
			break;

		case llvm::Instruction::Load:

			EmitLoad(misc::cast<llvm::LoadInst *>(&llvm_inst));
			break;

		case llvm::Instruction::Mul:

			EmitMul(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::PHI:

			EmitPhi(misc::cast<llvm::PHINode *>(&llvm_inst));
			break;

		case llvm::Instruction::Ret:

			EmitRet(misc::cast<llvm::ReturnInst *>(&llvm_inst));
			break;

		case llvm::Instruction::Store:

			EmitStore(misc::cast<llvm::StoreInst *>(&llvm_inst));
			break;

		case llvm::Instruction::Sub:

			EmitSub(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::FAdd:

			EmitFAdd(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::FSub:

			EmitFSub(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;
		
		case llvm::Instruction::FMul:

			EmitFMul(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::FDiv:

			EmitFDiv(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::ExtractElement:
			
			EmitExtractElement(misc::cast<llvm::ExtractElementInst *>
					(&llvm_inst));
			break;
		
		case llvm::Instruction::InsertElement:
			
			EmitInsertElement(misc::cast<llvm::InsertElementInst *>
					(&llvm_inst));
			break;

		case llvm::Instruction::SExt:

			EmitSExt(misc::cast<llvm::SExtInst *>(&llvm_inst));
			break;

		case llvm::Instruction::And:

			EmitAnd(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::Or:

			EmitOr(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::Xor:

			EmitXor(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;
		
		default:

			throw misc::Panic(misc::fmt("LLVM opcode not supported "
					"(%d)", llvm_inst.getOpcode()));
		}
	}
}


std::list<std::unique_ptr<si2bin::Instruction>>::iterator
		BasicBlock::getFirstControlFlowInstruction()
{
	// If list is empty, return a past-the-end iterator
	if (instructions.empty())
		return instructions.end();

	// Traverse list backward
	auto it = instructions.end();
	do
	{
		// Go to previous element
		--it;

		// Check if this is the first non-control flow instruction
		Instruction *instruction = it->get();
		if (!instruction->getControlFlow())
			return ++it;

	} while (it != instructions.begin());

	// There are only control flow instruction
	return instructions.begin();
}


#if 0
void BasicBlock::LiveRegisterAnalysis()
{
	// Iterate through list of instructions to populate bitmaps
	for (auto &inst : this->getInstList())
	{
		this->def = new misc::Bitmap(this->function->num_vregs);
		this->use = new misc::Bitmap(this->function->num_vregs);

		// Get each argument in the line
		for (auto &arg : inst->getArgs())
		{
			// Currently only deals with scalar registers
			if (arg->getType() == si2bin::Argument::TypeVectorRegister)
			{

				si2bin::ArgVectorRegister *argReg = dynamic_cast<si2bin::ArgVectorRegister *>(arg.get());
				if (!argReg)
					continue;

				if (arg->getToken()->getDirection() == si2bin::TokenDirectionDst)
				{
					this->def->Set(argReg->getId(), true);
				}
				else if (arg->getToken()->getDirection() == si2bin::TokenDirectionSrc)
				{
					// If register wasn't defined in the same basic block
					if (this->def->Test(argReg->getId()) != true)
					{
						this->use->Set(argReg->getId(), true);
					}
				}
			}
		}
	}

	assert(out != NULL);

	si2bin::Instruction *prev_inst = nullptr;
	for (auto it = inst_list.rbegin(), e = inst_list.rend(); it != e; ++it)
	{
		std::unique_ptr<Instruction> &inst = *it;

		// Sets out bitmap of instruction
		if (it == inst_list.rbegin())
		{
			inst->out = new misc::Bitmap(*out);
		}
		else
		{
			inst->out = new misc::Bitmap(*prev_inst->in);
		}

		// Clones out into in so that it can be used to perform calculations
		inst->in = new misc::Bitmap(*inst->out);
		*inst->in -= *inst->def;
		*inst->in |= *inst->use;

		// Iterate
		prev_inst = inst.get();
	}

	// Makes sure first instruction's in bitmap matches basic blocks in bitmap
	//assert(!BitmapCompare(prev_inst->in, basic_block->in));
}
#endif


}  // namespace llvm2si

