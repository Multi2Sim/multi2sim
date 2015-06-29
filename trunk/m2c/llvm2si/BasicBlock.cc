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


enum SI::Instruction::BufDataFormat BasicBlock::getBufDataFormat32(int num_elems)
{
	enum SI::Instruction::BufDataFormat buf_data_format = SI::Instruction::BufDataFormatInvalid;

	switch (num_elems)
	{
	case 1:
		buf_data_format = SI::Instruction::BufDataFormat32;
		break;
	case 2:
		buf_data_format = SI::Instruction::BufDataFormat32_32;
		break;
	case 3:
		buf_data_format = SI::Instruction::BufDataFormat32_32_32;
		break;
	case 4:
		buf_data_format = SI::Instruction::BufDataFormat32_32_32_32;
		break;
	default:
		break;
	}

	return buf_data_format;

}


enum SI::Instruction::BufDataFormat BasicBlock::getBufDataFormat(llvm::Type *llvm_type)
{
	enum SI::Instruction::BufDataFormat buf_data_format = SI::Instruction::BufDataFormatInvalid;
	
	if (llvm_type->isIntegerTy() || llvm_type->isFloatTy())
	{
		buf_data_format = SI::Instruction::BufDataFormat32;
	}
	else if (llvm_type->isVectorTy())
	{
		buf_data_format = getBufDataFormatVector(llvm_type);
	}
	else
	{
		llvm_type->dump();
		throw misc::Panic("Unsupported LLVM type kind");
	}

	return buf_data_format;
}


enum SI::Instruction::BufDataFormat BasicBlock::getBufDataFormatVector(llvm::Type *llvm_type)
{
	enum SI::Instruction::BufDataFormat buf_data_format;

	int num_elems = llvm_type->getVectorNumElements();
	llvm::Type *elem_type = llvm_type->getVectorElementType();

	if (elem_type->isIntegerTy() || elem_type->isFloatTy())
	{
		buf_data_format = getBufDataFormat32(num_elems);
	}
	else
	{
		llvm_type->dump();
		throw misc::Panic("Unsupported LLVM type kind");
	}

	return buf_data_format;
}


void BasicBlock::ArgScalarToVector(std::unique_ptr<Argument> &arg,
	llvm::Value *llvm_arg)
{
	auto arg_type = arg->getType();

	switch (arg_type)
	{
	
	case Argument::TypeScalarRegister:
	{
		ArgScalarRegister *arg_scalar = 
				dynamic_cast<ArgScalarRegister *>(arg.get());
		
		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_arg->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Emit instruction
		//
		// v_mov_b32 ret_vreg, arg1
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(arg_scalar->getId());
		assert(instruction->hasValidArguments());

		arg = misc::new_unique<ArgVectorRegister>(ret_vreg);
		break;
	}

	default:
		break;
		
	}
}


void BasicBlock::ArgLiteralToVector(std::unique_ptr<Argument> &arg)
{
	auto arg_type = arg->getType();

	switch (arg_type)
	{
	
	case Argument::TypeLiteral:
	case Argument::TypeLiteralReduced:
	case Argument::TypeLiteralFloat:
	case Argument::TypeLiteralFloatReduced:
	{
		arg = std::move(function->ConstToVReg(this, std::move(arg)));
		break;
	}

	default:
		break;
		
	}
}


void BasicBlock::ArgLiteralToFPFormat(std::unique_ptr<Argument> &arg)
{
	auto arg_type = arg->getType();

	union converter
	{
		uint as_uint;
		float as_float;
	} cvt;

	switch (arg_type)
	{
	
	case Argument::TypeLiteral:
	case Argument::TypeLiteralReduced:
	{
		ArgLiteral *arg_literal = 
				dynamic_cast<ArgLiteral *>(arg.get());
		cvt.as_float = static_cast<float>(arg_literal->getValue());
		arg = misc::new_unique<ArgLiteral>(cvt.as_uint);
		break;
	}
	case Argument::TypeLiteralFloat:
	case Argument::TypeLiteralFloatReduced:
	{
		ArgLiteralFloat *arg_literal = 
				dynamic_cast<ArgLiteralFloat *>(arg.get());
		cvt.as_float = arg_literal->getValue();
		arg = misc::new_unique<ArgLiteral>(cvt.as_uint);
		break;
	}

	default:
		break;
		
	}
}

void BasicBlock::EmitAdd(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("Only supported for 2 operands, "
				"%d found", llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	Symbol *arg1_symbol;
	Symbol *arg2_symbol;
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1, 
		arg1_symbol);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2,
		arg2_symbol);

	// Convert to vector registers if neccesary
	ArgScalarToVector(arg1, llvm_arg1);
	ArgScalarToVector(arg2, llvm_arg2);

	// Second operand cannot be a constant
	if (arg2->getType() != Argument::TypeVectorRegister)
		arg1.swap(arg2);

	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);
	arg2->ValidTypes(Argument::TypeVectorRegister);

	// Emit instructions based on instruction type
	llvm::Type *llvm_type = llvm_inst->getType();
	if (llvm_type->isIntegerTy(32))
	{
		// Allocate vector register and create symbol for return value
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(llvm_inst->getName());
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Emit addition.
		// v_add_i32 ret_vreg, vcc, arg_op1, arg_op2
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_ADD_I32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
		instruction->addArgument(std::move(arg1));
		instruction->addArgument(std::move(arg2));
		assert(instruction->hasValidArguments());
	}
	else if (llvm_type->isVectorTy())
	{

		// Only support 32bit integer type
		llvm::Type *elem_type = llvm_type->getVectorElementType();

		if (elem_type->isIntegerTy(32))
		{
			int num_elems = llvm_type->getVectorNumElements();

			// Sanity check
			assert(num_elems == arg1_symbol->getNumRegisters());
			assert(num_elems == arg2_symbol->getNumRegisters());

			// Allocate vector registers and create symbol for return value
			int ret_vreg = function->AllocVReg(num_elems, 1);
			Symbol *ret_symbol = function->addSymbol(
				llvm_inst->getName());
			ret_symbol->setRegister(Symbol::TypeVectorRegister,
				ret_vreg, ret_vreg + num_elems - 1);

			// Arg1 can be vector/literal
			auto arg1_type = arg1->getType();
			switch (arg1_type)
			{

			case Argument::TypeVectorRegister:
			{
				// Get associated register
				ArgVectorRegister *arg1_reg =
					dynamic_cast<ArgVectorRegister *>(arg1.get());
				ArgVectorRegister *arg2_reg =
					dynamic_cast<ArgVectorRegister *>(arg2.get());
				int arg1_vreg_base = arg1_reg->getId();
				int arg2_vreg_base = arg2_reg->getId();

				for (int inst_count = 0; 
					inst_count < num_elems; ++inst_count)
				{
					// Emit addition.
					// v_add_i32 ret_vreg, vcc, arg_op1, arg_op2
					Instruction *instruction = 
						addInstruction(SI::Instruction::Opcode_V_ADD_I32);
					instruction->addVectorRegister(ret_vreg + inst_count);
					instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
					instruction->addVectorRegister(arg1_vreg_base + inst_count);
					instruction->addVectorRegister(arg2_vreg_base + inst_count);
					assert(instruction->hasValidArguments());
				}
				break;
			}

			case Argument::TypeLiteral:
			case Argument::TypeLiteralReduced:
			case Argument::TypeLiteralFloat:
			case Argument::TypeLiteralFloatReduced:
			{
				// Get associated register
				ArgLiteral *arg1_literal =
					dynamic_cast<ArgLiteral *>(arg1.get());
				ArgVectorRegister *arg2_reg =
					dynamic_cast<ArgVectorRegister *>(arg2.get());
				int arg1_literal_val = arg1_literal->getValue();
				int arg2_vreg_base = arg2_reg->getId();

				for (int inst_count = 0; 
					inst_count < num_elems; ++inst_count)
				{
					// Emit addition.
					// v_add_i32 ret_vreg, vcc, arg_op1, arg_op2
					Instruction *instruction = 
						addInstruction(SI::Instruction::Opcode_V_ADD_I32);
					instruction->addVectorRegister(ret_vreg + inst_count);
					instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
					instruction->addLiteral(arg1_literal_val);
					instruction->addVectorRegister(arg2_vreg_base + inst_count);
					assert(instruction->hasValidArguments());
				}
				break;
			}

			default:
				throw Error("Arg 1 type not supported");
				break;
			}

		}
		else 
			throw Error("Only support int vector type");
	}

}


void BasicBlock::EmitCall(llvm::CallInst *llvm_inst)
{
	// Get called function, found in last operand of the operand list, as
	// returned by LLVMGetOperand.
	llvm::Function *llvm_function = llvm_inst->getCalledFunction();
	std::string func_name = llvm_function->getName();

	// Get return argument name
	std::string var_name = llvm_inst->getName();	

	// TODO: sanity check is not needed for some built-in functions
	// if (var_name.empty())
	// 	throw misc::Panic("Invalid return variable");

	// Number of arguments
	if (llvm_inst->getNumOperands() - 1 !=
			llvm_function->getArgumentList().size())
		throw Error(misc::fmt("%d arguments expected for "
				"'%s', %d found",
				llvm_inst->getNumOperands() - 1,
				func_name.c_str(),
				(int) llvm_function->getArgumentList().size()));

	// Math
	if (llvm_inst->hasFnAttr(llvm::Attribute::NoUnwind))
	{
		llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
		std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);

		if (func_name == "llvm.sqrt.f32")
		{
			// Make sure in vector register and in F32 format
			ArgScalarToVector(arg1, llvm_arg1);
			ArgLiteralToFPFormat(arg1);

			// Allocate a new vector register to copy global size.
			int ret_vreg = function->AllocVReg();
			Symbol *ret_symbol = function->addSymbol(var_name);
			ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

			// v_sqrt_f32 ret_vreg, vreg
			Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_SQRT_F32);
			instruction->addVectorRegister(ret_vreg);
			instruction->addArgument(std::move(arg1));
			assert(instruction->hasValidArguments());
		}
		else
		{
			throw Error("Invalid built-in function: " + func_name);
		}	
	}
	else 
	{
		// Atomic add
		if (func_name == "_Z10atomic_addPVU3AS3jj")
		{
			llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
			llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
			llvm::Type *llvm_arg1_type = llvm_arg1->getType();
			llvm::Type *llvm_arg2_type = llvm_arg2->getType();
			
			// Sanity check
			if (!llvm_arg1_type->isPointerTy() && !llvm_arg2_type->isIntegerTy())
				throw Error("Invalid parameters in atomic_add");

			std::unique_ptr<Argument> arg1 = 
				function->TranslateValue(llvm_arg1);
			std::unique_ptr<Argument> arg2 = 
				function->TranslateValue(llvm_arg2);

			arg1->ValidTypes(Argument::TypeVectorRegister);
			arg2->ValidTypes(Argument::TypeLiteral,
				Argument::TypeLiteralReduced);

			unsigned addr_space = llvm_arg1_type->getPointerAddressSpace();
			switch (addr_space)
			{

			case 0:
			{
				throw Error("atomic_add: not implemented yet");
				break;
			}

			case 3:
			{
				// Has to be in vector register
				ArgLiteralToVector(arg2);

				Instruction *instruction = addInstruction(SI::Instruction::Opcode_DS_ADD_U32);
				instruction->addArgument(std::move(arg1));
				instruction->addArgument(std::move(arg2));
				assert(instruction->hasValidArguments());
				break;				
			}

			default:
			{
				throw Error("atomic_add: address space not supported");
				break;				
			}

			}

			return;
		}

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
			Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MOV_B32);
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
			Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MOV_B32);
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
			Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MOV_B32);
			instruction->addVectorRegister(ret_vreg);
			instruction->addScalarRegister(function->getSRegLSize() + dim);
			assert(instruction->hasValidArguments());
		}
		else if (func_name == "__get_num_groups_u32")
		{
			// Allocate a new scalar register to copy num groups from CB0.
			int num_group_sreg = function->AllocSReg();

			// Load from ConstBuffer 0
			Instruction *instruction = addInstruction(SI::Instruction::Opcode_S_BUFFER_LOAD_DWORD);
			instruction->addScalarRegister(num_group_sreg);
			instruction->addScalarRegisterSeries(function->getSRegCB0(),
							function->getSRegCB0() + 3);
			instruction->addLiteral(8 + dim);
			assert(instruction->hasValidArguments());

			// Move to vector register
			// FIXME: this step can be eliminated
			int ret_vreg = function->AllocVReg();
			Symbol *ret_symbol = function->addSymbol(var_name);
			ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

			// v_mov_b32 vreg, s[num_group_sreg]
			instruction = addInstruction(SI::Instruction::Opcode_V_MOV_B32);
			instruction->addVectorRegister(ret_vreg);
			instruction->addScalarRegister(num_group_sreg);
			assert(instruction->hasValidArguments());
		}
		else if (func_name == "barrier")
		{
			function->addSymbol(var_name);
			Instruction *instruction = addInstruction(SI::Instruction::Opcode_S_BARRIER);
			assert(instruction->hasValidArguments());
		}
		else
		{
			throw Error("Invalid built-in function: " + func_name);
		}
	}
}


void BasicBlock::EmitUitofp(llvm::CastInst *llvm_inst)
{
	// Only supported for 1 operands
	if (llvm_inst->getNumOperands() != 1)
		throw misc::Panic(misc::fmt("1 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);

	llvm::Type *llvm_cast_type = llvm_inst->getDestTy();

	if (llvm_cast_type->isFloatTy())
	{
		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Emit cast
		//
		// v_cvt_f32_u32 ret_vreg, arg1
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_CVT_F32_U32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addArgument(std::move(arg1));
		assert(instruction->hasValidArguments());
	}
	else
		throw Error("Unsupported type in UIToFP");
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

	// Get size of pointed value
	llvm::Type *llvm_type_ptr = llvm_arg_ptr->getType();
	int ptr_size = getPointedLlvmTypeSize(llvm_type_ptr);

	// Get index operand (vreg, literal)
	llvm::Value *llvm_arg_index = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg_index = function->TranslateValue(llvm_arg_index);
	arg_index->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced);

	int addr_space = llvm_inst->getPointerAddressSpace();
	switch (addr_space)
	{

	// Global address space
	case 1:
	{	
		// Address must be a symbol with UAV
		assert(ptr_symbol && "symbol not found");
		assert(ptr_symbol->isAddress() && "no UAV for symbol");

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
			// Argument 'arg_offset'
			auto *arg_offset_literal = dynamic_cast<ArgLiteral *>(arg_index.get());
			assert(arg_offset_literal);
			arg_offset = misc::new_unique<ArgLiteral>(arg_offset_literal->getValue() * ptr_size);
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
			Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MUL_I32_I24);
			instruction->addVectorRegister(tmp_vreg);
			instruction->addLiteral(ptr_size);
			instruction->addArgument(std::move(arg_index));
			assert(instruction->hasValidArguments());
		}

		ArgLiteralToVector(arg_offset);

		// Emit effective address calculation as the addition between the
		// original pointer and the offset.
		//
		// v_add_i32 ret_vreg, vcc, arg_pointer, arg_offset
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_ADD_I32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
		instruction->addArgument(std::move(arg_ptr));
		instruction->addArgument(std::move(arg_offset));
		assert(instruction->hasValidArguments());
		break;
	}
	// Local address space
	case 3:
	{	llvm::Type *llvm_type = llvm_arg_ptr->getType()->getPointerElementType();
		if (llvm_type->isIntegerTy(32) || llvm_type->isFloatTy())
		{
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
				// Argument 'arg_offset'
				auto *arg_offset_literal = dynamic_cast<ArgLiteral *>(arg_index.get());
				assert(arg_offset_literal);
				arg_offset = misc::new_unique<ArgLiteral>(arg_offset_literal->getValue() * ptr_size);
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
				Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MUL_I32_I24);
				instruction->addVectorRegister(tmp_vreg);
				instruction->addLiteral(ptr_size);
				instruction->addArgument(std::move(arg_index));
				assert(instruction->hasValidArguments());
			}

			ArgLiteralToVector(arg_offset);

			// Emit effective address calculation as the addition between the
			// original pointer and the offset.
			//
			// v_add_i32 ret_vreg, vcc, arg_offset, arg_pointer
			//
			Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_ADD_I32);
			instruction->addVectorRegister(ret_vreg);
			instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
			instruction->addArgument(std::move(arg_ptr));
			instruction->addArgument(std::move(arg_offset));
			assert(instruction->hasValidArguments());
		}
		else if (llvm_type->isVectorTy())
		{
			llvm::Type *elem_type = llvm_type->getVectorElementType();
			if (elem_type->isIntegerTy(32) || elem_type->isFloatTy())
			{
				int num_elems = llvm_type->getVectorNumElements();

				// Need several VGRPs to store local memory address
				int num_vregs = num_elems;

				// 1st instruction is fixed
				int num_insts = num_elems - 1;

				// element size
				int elem_size = getLlvmTypeSize(elem_type);

				// Allocate vector registers and create symbol for return value
				std::string ret_name = llvm_inst->getName();
				int ret_vreg = function->AllocVReg(num_vregs);
				Symbol *ret_symbol = function->addSymbol(ret_name);
				ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg, 
							ret_vreg + num_vregs - 1);
				ret_symbol->setUAVIndex(ptr_symbol->getUAVIndex());

				// Calculate offset as the multiplication between 'arg_index' and the
				// size of the pointed element ('ptr_size'). If 'arg_index' is a
				// literal, we can pre-calculate it here. If 'arg_index' is a vector
				// register, we need to emit an instruction.
				std::unique_ptr<Argument> arg_offset;
				if (arg_index->getType() == Argument::TypeLiteral ||
						arg_index->getType() == Argument::TypeLiteralReduced)
				{
					// Argument 'arg_offset'
					auto *arg_offset_literal = dynamic_cast<ArgLiteral *>(arg_index.get());
					assert(arg_offset_literal);
					arg_offset = misc::new_unique<ArgLiteral>(arg_offset_literal->getValue() * ptr_size);
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
					Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MUL_I32_I24);
					instruction->addVectorRegister(tmp_vreg);
					instruction->addLiteral(ptr_size);
					instruction->addArgument(std::move(arg_index));
					assert(instruction->hasValidArguments());
				}

				ArgLiteralToVector(arg_offset);

				// Emit effective address calculation as the addition between the
				// original pointer and the offset.
				//
				// v_add_i32 ret_vreg, vcc, arg_offset, arg_pointer
				//
				
				Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_ADD_I32);
				instruction->addVectorRegister(ret_vreg);
				instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
				instruction->addArgument(std::move(arg_ptr));
				instruction->addArgument(std::move(arg_offset));
				assert(instruction->hasValidArguments());

				for (int inst_count = 0; inst_count < num_insts; ++inst_count)
				{
					Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_ADD_I32);
					instruction->addVectorRegister(ret_vreg + inst_count + 1);
					instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
					instruction->addLiteral(elem_size * (inst_count + 1));
					instruction->addVectorRegister(ret_vreg);
					assert(instruction->hasValidArguments());					
				}

			}
		}
		else
			throw Error("GetElementPtr only support 4 byte int/float scalar/vector types");
		break;
	}
	default:
	{	throw Error(misc::fmt("Address space 1 & 3 expected, %d found",
			addr_space));
		break;
	}
	}
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

	// If arg1 is a scalar register convert it to a vector register
	if (arg1->getType() == Argument::TypeScalarRegister)
	{	
		ArgScalarRegister *arg1_scalar = 
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
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(arg1_scalar->getId());
		assert(instruction->hasValidArguments());

		arg1 = misc::new_unique<ArgVectorRegister>(ret_vreg);
	}

	// If arg2 is a scalar register convert it to a vector register
	if (arg2->getType() == Argument::TypeScalarRegister)
	{	
		ArgScalarRegister *arg2_scalar = 
				dynamic_cast<ArgScalarRegister *>(arg2.get());
		
		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_arg2->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Emit instruction
		//
		// v_mov_b32 ret_vreg, arg2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(arg2_scalar->getId());
		assert(instruction->hasValidArguments());

		arg2 = misc::new_unique<ArgVectorRegister>(ret_vreg);
	}

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
	SI::Instruction::Opcode opcode = SI::Instruction::OpcodeInvalid;
	llvm::CmpInst::Predicate llvm_pred = llvm_inst->getPredicate();
	switch (llvm_pred)
	{

	case llvm::CmpInst::ICMP_EQ:

		opcode = SI::Instruction::Opcode_V_CMP_EQ_I32;
		break;

	case llvm::CmpInst::ICMP_NE:

		opcode = SI::Instruction::Opcode_V_CMP_NE_I32;
		break;

	case llvm::CmpInst::ICMP_UGT:

		opcode = SI::Instruction::Opcode_V_CMP_GT_U32;
		if (invert)
			opcode = SI::Instruction::Opcode_V_CMP_LT_U32;
		break;

	case llvm::CmpInst::ICMP_UGE:

		opcode = SI::Instruction::Opcode_V_CMP_GE_U32;
		if (invert)
			opcode = SI::Instruction::Opcode_V_CMP_LE_U32;
		break;

	case llvm::CmpInst::ICMP_ULT:

		opcode = SI::Instruction::Opcode_V_CMP_LT_U32;
		if (invert)
			opcode = SI::Instruction::Opcode_V_CMP_GT_U32;
		break;

	case llvm::CmpInst::ICMP_ULE:

		opcode = SI::Instruction::Opcode_V_CMP_LE_U32;
		if (invert)
			opcode = SI::Instruction::Opcode_V_CMP_GE_U32;
		break;

	case llvm::CmpInst::ICMP_SGT:

		opcode = SI::Instruction::Opcode_V_CMP_GT_I32;
		if (invert)
			opcode = SI::Instruction::Opcode_V_CMP_LT_I32;
		break;

	case llvm::CmpInst::ICMP_SGE:

		opcode = SI::Instruction::Opcode_V_CMP_GE_I32;
		if (invert)
			opcode = SI::Instruction::Opcode_V_CMP_LE_I32;
		break;

	case llvm::CmpInst::ICMP_SLT:

		opcode = SI::Instruction::Opcode_V_CMP_LT_I32;
		if (invert)
			opcode = SI::Instruction::Opcode_V_CMP_GT_I32;
		break;

	case llvm::CmpInst::ICMP_SLE:

		opcode = SI::Instruction::Opcode_V_CMP_LE_I32;
		if (invert)
			opcode = SI::Instruction::Opcode_V_CMP_GE_I32;
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
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	instruction->addArgument(std::move(arg1));
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());

	// Store 'vcc' in scalar register
	//
	// s_mov_b64 ret_sreg_series, vcc
	//
	instruction = addInstruction(SI::Instruction::Opcode_S_MOV_B64);
	instruction->addScalarRegisterSeries(ret_sreg_series, ret_sreg_series + 1);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
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

	// Get return type
	llvm::Type *llvm_type = llvm_inst->getType();

	// Get address space
	int addr_space = llvm_inst->getPointerAddressSpace();
	switch (addr_space)
	{
	// Load from global memory
	case 1:
	{
		// Address must be a symbol with UAV
		if (!addr_symbol || !addr_symbol->isAddress())
			throw Error("No UAV for symbol: " + addr_symbol->getName());

		// Get UAV
		FunctionUAV *uav = function->getUAV(addr_symbol->getUAVIndex());
		if (!uav)
			throw Error(misc::fmt("Invalid UAV index (%d)",
					addr_symbol->getUAVIndex()));

		if (llvm_type->isIntegerTy(32) || llvm_type->isFloatTy())
		{
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
			Instruction *instruction = addInstruction(SI::Instruction::Opcode_TBUFFER_LOAD_FORMAT_X);
			instruction->addVectorRegister(ret_vreg);
			instruction->addArgument(std::move(arg_addr));
			instruction->addScalarRegisterSeries(uav->getSReg(), uav->getSReg() + 3);
			instruction->addMemoryAddress(new ArgLiteral(0),
					new ArgMaddrQual(true, false, 0),
					SI::Instruction::BufDataFormat32,
					SI::Instruction::BufNumFormatFloat);
			assert(instruction->hasValidArguments());

			instruction = addInstruction(SI::Instruction::Opcode_S_WAITCNT);
			instruction->addWaitCounter(ArgWaitCounter::CounterTypeVmCnt);
			assert(instruction->hasValidArguments());
		}
		else if (llvm_type->isVectorTy())
		{
			llvm::Type *elem_type = llvm_type->getVectorElementType();

			if (elem_type->isIntegerTy(32) || elem_type->isFloatTy())
			{
				int num_elems = llvm_type->getVectorNumElements();

				// Need several vGRPs to store the values
				int num_vregs = num_elems;

				// If # of elements is greater than 4, need to emit several instructions
				int num_insts = (num_elems + 3) / 4;
				int num_vregs_per_inst = num_vregs / num_insts;
				int num_elems_per_inst = num_vregs_per_inst;
				int offset_width = getLlvmTypeSize(elem_type) * num_elems_per_inst;

				// Allocate vector registers and create symbol for return value
				std::string ret_name = llvm_inst->getName();
				int ret_vreg = function->AllocVReg(num_vregs, 1);
				Symbol *ret_symbol = function->addSymbol(ret_name);
				ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg,
					ret_vreg + num_vregs - 1);

				// Get corresponding instruction opcode
				SI::Instruction::Opcode inst_op;
				switch (num_elems_per_inst)
				{
				case 1:
					inst_op = SI::Instruction::Opcode_TBUFFER_LOAD_FORMAT_X;
					break;
				case 2:
					inst_op = SI::Instruction::Opcode_TBUFFER_LOAD_FORMAT_XY;
					break;
				case 3:
					inst_op = SI::Instruction::Opcode_TBUFFER_LOAD_FORMAT_XYZ;
					break;
				case 4:
					inst_op = SI::Instruction::Opcode_TBUFFER_LOAD_FORMAT_XYZW;
					break;
				}

				// Get corresponding buffer data format
				enum SI::Instruction::BufDataFormat buf_data_format = getBufDataFormat32(num_elems_per_inst);

				ArgVectorRegister *arg_addr_reg = dynamic_cast<ArgVectorRegister *>(arg_addr.get());
				int arg_addr_vreg = arg_addr_reg->getId();

				// Emit memory load instruction.
				for (int inst_count = 0; inst_count < num_insts; ++inst_count)
				{
					// VGPR series don't overlap
					int vreg_lo = ret_vreg + inst_count * num_vregs_per_inst;
					int vreg_hi = vreg_lo + num_vregs_per_inst - 1;

					// Make sure in range
					assert(vreg_lo >= ret_vreg);
					assert(vreg_hi <= ret_vreg + num_vregs);
					
					// Offset start from 0 if multiple instructions needed
					int offset = inst_count * offset_width;

					Instruction *instruction = addInstruction(inst_op);
					instruction->addVectorRegisterSeries(vreg_lo, vreg_hi);	
					instruction->addVectorRegister(arg_addr_vreg);
					instruction->addScalarRegisterSeries(uav->getSReg(), uav->getSReg() + 3);
					instruction->addMemoryAddress(new ArgLiteral(offset),
							new ArgMaddrQual(true, false, 0),
							buf_data_format,
							SI::Instruction::BufNumFormatFloat);
					assert(instruction->hasValidArguments());		

					instruction = addInstruction(SI::Instruction::Opcode_S_WAITCNT);
					instruction->addWaitCounter(ArgWaitCounter::CounterTypeVmCnt);
					assert(instruction->hasValidArguments());				
				}
			}
			else
				throw misc::Panic("Only 4-byte int/float vector types supported");
		}
		else
		{
			llvm_type->dump();
			throw misc::Panic("Load Global: Unsupported LLVM type kind");
		}
		break;
	}
	// Load from LDS
	case 3:
	{
		if (llvm_type->isIntegerTy(32) || llvm_type->isFloatTy())
		{
			// Allocate vector register and create symbol for return value
			std::string ret_name = llvm_inst->getName();
			int ret_vreg = function->AllocVReg();
			Symbol *ret_symbol = function->addSymbol(ret_name);
			ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

			// Emit LDS load instruction.
			//
			// ds_read ret_vreg, arg_addr
			Instruction *instruction = addInstruction(SI::Instruction::Opcode_DS_READ_B32);
			instruction->addVectorRegister(ret_vreg);
			instruction->addArgument(std::move(arg_addr));
			assert(instruction->hasValidArguments());

			instruction = addInstruction(SI::Instruction::Opcode_S_WAITCNT);
			instruction->addWaitCounter(ArgWaitCounter::CounterTypeVmCnt);
			assert(instruction->hasValidArguments());
		}
		else if (llvm_type->isVectorTy())
		{
			llvm::Type *elem_type = llvm_type->getVectorElementType();

			if (elem_type->isIntegerTy(32) || elem_type->isFloatTy())
			{
				int num_elems = llvm_type->getVectorNumElements();

				// Need several VGRPs to store local memory address
				int num_vregs = num_elems;
				assert(num_vregs == addr_symbol->getNumRegisters());

				int num_insts = num_elems;

				// Allocate vector registers and create symbol for return value
				std::string ret_name = llvm_inst->getName();
				int ret_vreg = function->AllocVReg(num_vregs, 1);
				Symbol *ret_symbol = function->addSymbol(ret_name);
				ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg,
					ret_vreg + num_vregs - 1);

				// 
				ArgVectorRegister *arg_addr_reg = dynamic_cast<ArgVectorRegister *>(arg_addr.get());
				int arg_addr_vreg = arg_addr_reg->getId();

				// Emit instuctions to caculate LDS memory addresses, store in VGPRs
				// 1st instruction uses 
				Instruction *instruction;
				for (int inst_count = 0; inst_count < num_insts; ++inst_count)
				{
					int dst_vreg = ret_vreg + inst_count;
					int src_vreg = arg_addr_vreg + inst_count;

					// Make sure in range
					assert(dst_vreg <= ret_vreg + num_vregs);
					assert(src_vreg <= arg_addr_vreg + addr_symbol->getNumRegisters());
					
					// Emit LDS load instruction.
					//
					// ds_read dst_vreg, arg_addr
					instruction = addInstruction(SI::Instruction::Opcode_DS_READ_B32);
					instruction->addVectorRegister(dst_vreg);
					instruction->addVectorRegister(src_vreg);
					assert(instruction->hasValidArguments());
				}

				instruction = addInstruction(SI::Instruction::Opcode_S_WAITCNT);
				instruction->addWaitCounter(ArgWaitCounter::CounterTypeLgkmCnt);
				assert(instruction->hasValidArguments());
			}
			else
				throw misc::Panic("Only 4-byte int/float vector types supported");
		}
		else
		{
			llvm_type->dump();
			throw misc::Panic("Load Local: Unsupported LLVM type kind");
		}
		break;
	}
	
	default:
	{
		throw Error(misc::fmt("Address space 1 or 3 expected, %d found",
			addr_space));
		break;
	}
	}
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

	// Get operands
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Sanity check
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeScalarRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);
	arg2->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeScalarRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);

	// Check if scalar instruction can be emitted
	bool emit_scalar_arg1 = arg1->hasValidType(Argument::TypeScalarRegister,
		Argument::TypeLiteral,
		Argument::TypeLiteralReduced,
		Argument::TypeLiteralFloat,
		Argument::TypeLiteralFloatReduced);

	bool emit_scalar_arg2 = arg2->hasValidType(Argument::TypeScalarRegister,
		Argument::TypeLiteral,
		Argument::TypeLiteralReduced,
		Argument::TypeLiteralFloat,
		Argument::TypeLiteralFloatReduced);

	bool emit_scalar = emit_scalar_arg1 && emit_scalar_arg2;
	if (emit_scalar)
	{
		// Allocate scalar register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		int ret_sreg = function->AllocSReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeScalarRegister, ret_sreg);

		// Emit subtraction.
		//
		// s_mul_i32 ret_sreg, arg_op1, arg_op2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_S_MUL_I32);
		instruction->addScalarRegister(ret_sreg);
		instruction->addArgument(std::move(arg1));
		instruction->addArgument(std::move(arg2));
		assert(instruction->hasValidArguments());		
	}
	else
	{
		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Literal to register
		ArgLiteralToVector(arg1);
		ArgLiteralToVector(arg2);

		// Emit effective address calculation.
		//
		// v_mul_lo_i32 ret_vreg, arg_op1, arg_op2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MUL_LO_U32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addArgument(std::move(arg1));
		instruction->addArgument(std::move(arg2));
		assert(instruction->hasValidArguments());
	}
}


void BasicBlock::EmitPhi(llvm::PHINode *llvm_inst)
{
	// Only supported for 32-bit integers and float
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32) && !llvm_type->isFloatTy())
		throw misc::Panic("Only supported for 32-bit integers & float");

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

		// Get source symbol
		llvm::Value *src_value = llvm_inst->getIncomingValue(i);
		std::string symbol = src_value->getName();
		
		// At basic block level, there is no value set for ArgPhi of 
		// non literals. As the whole symbol table is not complete
		// when emitting basic blocks, it often leads to symbol table
		// look up error. The symbol will be looked up and mapped to 
		// resigters later at function level. 

		// Emit constant literal
		llvm::Constant *llvm_const = dynamic_cast<llvm::Constant *>(src_value);
		if (llvm_const)
		{
			llvm::Type *llvm_type = llvm_const->getType();
			std::unique_ptr<Argument> src_arg =
				function->TranslateValue(src_value);

			if (llvm_type->isIntegerTy())
			{
				ArgLiteral *src_arg_literal = 
					misc::cast<ArgLiteral *>(src_arg.get());
				int src_literal = src_arg_literal->getValue();

				// Create Phi argument and set literal
				ArgPhi *arg = new si2bin::ArgPhi(label);
				arg->setLiteral(src_literal);
				arg_list.emplace_back(arg);
			}
			else if (llvm_type->isFloatTy())
			{
				ArgLiteralFloat *src_arg_literal_float = 
					misc::cast<ArgLiteralFloat *>(src_arg.get());
				float src_literal_float = 
					src_arg_literal_float->getValue();

				// Create Phi argument and set scalar register
				ArgPhi *arg = new si2bin::ArgPhi(label);
				arg->setLiteralFloat(src_literal_float);
				arg_list.emplace_back(arg);
			}
			else
				throw Error("EmitPhi: Constant type not supported");
		}
		else // Emit ArgPhi without setting actual register
		{
			ArgPhi *arg = new si2bin::ArgPhi(label, symbol);
			arg_list.emplace_back(arg);
		}
	}

	// Emit Phi instruction
	Instruction *instruction = addInstruction(SI::Instruction::Opcode_PHI);
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
	addInstruction(SI::Instruction::Opcode_S_ENDPGM);
}


void BasicBlock::EmitStore(llvm::StoreInst *llvm_inst)
{
	// Only supported for 2 operand (address, data)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get data operand (vreg)
	llvm::Value *llvm_arg_data = llvm_inst->getOperand(0);
	Symbol *data_symbol;
	std::unique_ptr<Argument> arg_data = function->TranslateValue(
			llvm_arg_data, data_symbol);
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
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(arg_scalar->getId());
		assert(instruction->hasValidArguments());

		arg_data = misc::new_unique<ArgVectorRegister>(ret_vreg);
	}

	// Get address operand (vreg)
	llvm::Value *llvm_arg_addr = llvm_inst->getOperand(1);
	Symbol *addr_symbol;
	std::unique_ptr<Argument> arg_addr = function->TranslateValue(
		llvm_arg_addr, addr_symbol);
	arg_addr->ValidTypes(Argument::TypeVectorRegister);

	// Get address space - only 1 (global mem.) supported for now
	llvm::Type *llvm_type = llvm_arg_addr->getType();
	int addr_space = llvm_type->getPointerAddressSpace();
	llvm_type = llvm_arg_data->getType();
	switch (addr_space)
	{
	// Global memory address
	case 1:
	{
		// Address must be a symbol with UAV
		if (!addr_symbol || !addr_symbol->isAddress())
			throw Error("No UAV for symbol: " + addr_symbol->getName());

		// Get UAV
		FunctionUAV *uav = function->getUAV(addr_symbol->getUAVIndex());
		if (!uav)
			throw Error(misc::fmt("Invalid UAV index (%d)",
					addr_symbol->getUAVIndex()));

		if (llvm_type->isIntegerTy(32) || llvm_type->isFloatTy())
		{
			// Emit memory write.
			//
			// tbuffer_store_format_x v[value_symbol->vreg], s[pointer_symbol->vreg],
			// 	s[sreg_uav,sreg_uav+3], 0 offen format:[BUF_DATA_FORMAT_32,
			// 	BUF_NUM_FORMAT_FLOAT]
			//
			Instruction *instruction = addInstruction(SI::Instruction::Opcode_TBUFFER_STORE_FORMAT_X);
			instruction->addArgument(std::move(arg_data));
			instruction->addArgument(std::move(arg_addr));
			instruction->addScalarRegisterSeries(uav->getSReg(), uav->getSReg() + 3);
			instruction->addMemoryAddress(new ArgLiteral(0),
					new ArgMaddrQual(true, false, 0),
					SI::Instruction::BufDataFormat32,
					SI::Instruction::BufNumFormatFloat);
			assert(instruction->hasValidArguments());

			instruction = addInstruction(SI::Instruction::Opcode_S_WAITCNT);
			instruction->addWaitCounter(ArgWaitCounter::CounterTypeExpCnt);
			assert(instruction->hasValidArguments());		
		}
		else if (llvm_type->isVectorTy())
		{
			llvm::Type *elem_type = llvm_type->getVectorElementType();

			if (elem_type->isIntegerTy(32) || elem_type->isFloatTy())
			{
				int num_elems = llvm_type->getVectorNumElements();

				// Need several vGRPs to store the values
				int num_vregs = num_elems;

				// If # of elements is greater than 4, need to emit several instructions
				int num_insts = (num_elems + 3) / 4;
				int num_vregs_per_inst = num_vregs / num_insts;
				int num_elems_per_inst = num_vregs_per_inst;
				int offset_width = getLlvmTypeSize(elem_type) * num_elems_per_inst;

				// Get corresponding instruction opcode
				SI::Instruction::Opcode inst_op;
				switch (num_elems_per_inst)
				{
				case 1:
					inst_op = SI::Instruction::Opcode_TBUFFER_STORE_FORMAT_X;
					break;
				case 2:
					inst_op = SI::Instruction::Opcode_TBUFFER_STORE_FORMAT_XY;
					break;
				case 3:
					throw misc::Panic("Vector 3 type store not supported");
					break;
				case 4:
					inst_op = SI::Instruction::Opcode_TBUFFER_STORE_FORMAT_XYZW;
					break;
				default:
					throw misc::Panic("Invalid buffer store format");
					break;
				}

				// Get corresponding buffer data format
				enum SI::Instruction::BufDataFormat buf_data_format = getBufDataFormat32(num_elems_per_inst);

				// VGPR info
				ArgVectorRegister *arg_addr_reg = dynamic_cast<ArgVectorRegister *>(arg_addr.get());
				ArgVectorRegister *arg_data_reg = dynamic_cast<ArgVectorRegister *>(arg_data.get());

				int arg_data_vreg = arg_data_reg->getId();
				int arg_addr_vreg = arg_addr_reg->getId();

				// Emit memory store instruction.
				for (int inst_count = 0; inst_count < num_insts; ++inst_count)
				{
					// VGPR series don't overlap
					int vreg_lo = arg_data_vreg + inst_count * num_vregs_per_inst;
					int vreg_hi = vreg_lo + num_vregs_per_inst - 1;

					// Make sure in range
					assert(vreg_lo >= arg_data_vreg);
					assert(vreg_hi <= arg_data_vreg + num_vregs);
					
					// Offset start from 0 if multiple instructions needed
					int offset = inst_count * offset_width;

					Instruction *instruction = addInstruction(inst_op);
					instruction->addVectorRegisterSeries(vreg_lo, vreg_hi);	
					instruction->addVectorRegister(arg_addr_vreg);
					instruction->addScalarRegisterSeries(uav->getSReg(), uav->getSReg() + 3);
					instruction->addMemoryAddress(new ArgLiteral(offset),
							new ArgMaddrQual(true, false, 0),
							buf_data_format,
							SI::Instruction::BufNumFormatFloat);
					assert(instruction->hasValidArguments());

					instruction = addInstruction(SI::Instruction::Opcode_S_WAITCNT);
					instruction->addWaitCounter(ArgWaitCounter::CounterTypeVmCnt);
					assert(instruction->hasValidArguments());
				}
			}
			else
				throw misc::Panic("Only 4-byte int/float vector types supported");
		}
		else
		{
			llvm_type->dump();
			throw misc::Panic("Unsupported LLVM type kind");
		}

		break;
	}
	// Local memory address
	case 3:
	{
		if (llvm_type->isIntegerTy(32) || llvm_type->isFloatTy())
		{
			// Emit LDS store instruction.
			//
			// ds_store arg_addr_vreg, arg_data_vreg
			Instruction *instruction = addInstruction(SI::Instruction::Opcode_DS_WRITE_B32);
			instruction->addArgument(std::move(arg_addr));
			instruction->addArgument(std::move(arg_data));
			assert(instruction->hasValidArguments());

			instruction = addInstruction(SI::Instruction::Opcode_S_WAITCNT);
			instruction->addWaitCounter(ArgWaitCounter::CounterTypeVmCnt);
			assert(instruction->hasValidArguments());
		}
		else if (llvm_type->isVectorTy())
		{
			llvm::Type *elem_type = llvm_type->getVectorElementType();

			if (elem_type->isIntegerTy(32) || elem_type->isFloatTy())
			{
				int num_elems = llvm_type->getVectorNumElements();

				// 
				ArgVectorRegister *arg_addr_reg = dynamic_cast<ArgVectorRegister *>(arg_addr.get());
				ArgVectorRegister *arg_data_reg = dynamic_cast<ArgVectorRegister *>(arg_data.get());
				int arg_addr_vreg = arg_addr_reg->getId();
				int arg_data_vreg = arg_data_reg->getId();

				assert(addr_symbol->getNumRegisters() == data_symbol->getNumRegisters());

				// Emit instuctions to store to LDS 
				Instruction *instruction;
				for (int inst_count = 0; inst_count < num_elems; ++inst_count)
				{
					int dst_vreg = arg_addr_vreg + inst_count;
					int src_vreg = arg_data_vreg + inst_count;
					
					// Emit LDS load instruction.
					//
					// ds_read dst_vreg, arg_addr
					instruction = addInstruction(SI::Instruction::Opcode_DS_WRITE_B32);
					instruction->addVectorRegister(dst_vreg);
					instruction->addVectorRegister(src_vreg);
					assert(instruction->hasValidArguments());
				}

				instruction = addInstruction(SI::Instruction::Opcode_S_WAITCNT);
				instruction->addWaitCounter(ArgWaitCounter::CounterTypeLgkmCnt);
				assert(instruction->hasValidArguments());
			}
			else
				throw misc::Panic("Only 4-byte int/float vector types supported");
		}
		else
		{
			llvm_type->dump();
			throw misc::Panic("Store Local: Unsupported LLVM type kind");
		}
		break;
	}

	default:
		throw misc::Panic(misc::fmt("Only address space 1 or 3 supported, "
				"%d found", addr_space));
	}


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

	bool emit_scalar_arg1 = arg1->hasValidType(Argument::TypeScalarRegister,
		Argument::TypeLiteral,
		Argument::TypeLiteralReduced,
		Argument::TypeLiteralFloat,
		Argument::TypeLiteralFloatReduced);

	bool emit_scalar_arg2 = arg2->hasValidType(Argument::TypeScalarRegister,
		Argument::TypeLiteral,
		Argument::TypeLiteralReduced,
		Argument::TypeLiteralFloat,
		Argument::TypeLiteralFloatReduced);

	bool emit_scalar = emit_scalar_arg1 && emit_scalar_arg2;
	if (emit_scalar)
	{
		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		int ret_sreg = function->AllocSReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeScalarRegister, ret_sreg);

		// Emit subtraction.
		//
		// s_sub_i32 ret_sreg, arg_op1, arg_op2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_S_SUB_I32);
		instruction->addScalarRegister(ret_sreg);
		instruction->addArgument(std::move(arg1));
		instruction->addArgument(std::move(arg2));
		assert(instruction->hasValidArguments());
	}
	else
	{
		// Arg1 can be vector/scalar/literals, Arg2 has to be vector
		ArgScalarToVector(arg2, llvm_arg2);
		ArgLiteralToVector(arg2);

		// Allocate scalar register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Emit subtraction.
		//
		// v_sub_i32 ret_vreg, vcc, arg_op1, arg_op2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_SUB_I32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
		instruction->addArgument(std::move(arg1));
		instruction->addArgument(std::move(arg2));
		assert(instruction->hasValidArguments());
	}
}


void BasicBlock::EmitFAdd(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands (vreg, literal)
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	Symbol *arg1_symbol;
	Symbol *arg2_symbol;
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1, 
		arg1_symbol);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2,
		arg2_symbol);

	// operand 1 and 2 must have same number of registers
	assert(arg1_symbol->getNumRegisters() == arg2_symbol->getNumRegisters());

	// Second operand cannot be a constant
	arg2 = std::move(function->ConstToVReg(this, std::move(arg2)));
	arg1->ValidTypes(Argument::TypeVectorRegister,
			Argument::TypeLiteral,
			Argument::TypeLiteralReduced,
			Argument::TypeLiteralFloat,
			Argument::TypeLiteralFloatReduced);
	arg2->ValidTypes(Argument::TypeVectorRegister);

	// Literal to FP format
	ArgLiteralToFPFormat(arg1);

	// Emit instructions based on instruction type
	llvm::Type *llvm_type = llvm_inst->getType();
	if (llvm_type->isFloatTy())
	{
		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Emit addition.
		//
		// v_add_f32 ret_vreg, arg_op1, arg_op2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_ADD_F32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addArgument(std::move(arg1));
		instruction->addArgument(std::move(arg2));
		assert(instruction->hasValidArguments());
	}
	else if (llvm_type->isVectorTy())
	{
		// Only support 32bit integer type
		llvm::Type *elem_type = llvm_type->getVectorElementType();

		if (elem_type->isFloatTy())
		{
			int num_elems = llvm_type->getVectorNumElements();

			// Sanity check
			assert(num_elems == arg1_symbol->getNumRegisters());
			assert(num_elems == arg2_symbol->getNumRegisters());

			// Allocate vector registers and create symbol for return value
			int ret_vreg = function->AllocVReg(num_elems, 1);
			Symbol *ret_symbol = function->addSymbol(llvm_inst->getName());
			ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

			// Get associated register
			ArgVectorRegister *arg1_reg = dynamic_cast<ArgVectorRegister *>(arg1.get());
			ArgVectorRegister *arg2_reg = dynamic_cast<ArgVectorRegister *>(arg2.get());
			int arg1_vreg_base = arg1_reg->getId();
			int arg2_vreg_base = arg2_reg->getId();

			for (int inst_count = 0; inst_count < num_elems; ++inst_count)
			{
				// Emit addition.
				// v_add_f32 ret_vreg, arg_op1, arg_op2
				Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_ADD_F32);
				instruction->addVectorRegister(ret_vreg + inst_count);
				instruction->addVectorRegister(arg1_vreg_base + inst_count);
				instruction->addVectorRegister(arg2_vreg_base + inst_count);
				assert(instruction->hasValidArguments());
			}
		}
		else 
			throw Error("Only support int vector type");
	}
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

	// Literal to FP format
	ArgLiteralToFPFormat(arg1);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Emit addition.
	//
	// v_sub_f32 ret_vreg, arg_op1, arg_op2
	//
	Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_SUB_F32);
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

	// Literals need to be convert to hex format
	ArgLiteralToFPFormat(arg1);

	// Emit effective address calculation.
	//
	// v_mul_f32 ret_vreg, arg_op1, arg_op2
	//
	Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MUL_F32);
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
	//              v_mov_b32     v2, 0x2f800000
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

	ArgLiteralToFPFormat(arg1);

	int arg2_rcp_id = function->AllocVReg();

	Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_RCP_F32);
	instruction->addVectorRegister(arg2_rcp_id);
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());

	instruction = addInstruction(SI::Instruction::Opcode_V_MUL_F32);
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

	// Get operands
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// If arg1 is a scalar register convert it to a vector register
	if (arg1->getType() == Argument::TypeScalarRegister)
	{	
		ArgScalarRegister *arg1_scalar = 
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
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(arg1_scalar->getId());
		assert(instruction->hasValidArguments());

		arg1 = misc::new_unique<ArgVectorRegister>(ret_vreg);
	}

	// If arg2 is a scalar register convert it to a vector register
	if (arg2->getType() == Argument::TypeScalarRegister)
	{	
		ArgScalarRegister *arg2_scalar = 
				dynamic_cast<ArgScalarRegister *>(arg2.get());
		
		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_arg2->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Emit instruction
		//
		// v_mov_b32 ret_vreg, arg2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MOV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addScalarRegister(arg2_scalar->getId());
		assert(instruction->hasValidArguments());

		arg2 = misc::new_unique<ArgVectorRegister>(ret_vreg);
	}

	// Only the first operand can be a constant, so swap them if there is
	// a constant in the second.
	if (arg2->getType() != Argument::TypeVectorRegister)
		arg1.swap(arg2);

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
	Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_AND_B32);
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
	Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_OR_B32);
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
	Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_XOR_B32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addArgument(std::move(arg1));
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());
}

void BasicBlock::EmitURem(llvm::BinaryOperator *llvm_inst)
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

	// Make sure in vector register
	ArgScalarToVector(arg1, llvm_arg1);
	ArgScalarToVector(arg2, llvm_arg2);
	ArgLiteralToVector(arg1);
	ArgLiteralToVector(arg2);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	ArgVectorRegister *arg1_vector = 
		dynamic_cast<ArgVectorRegister *>(arg1.get());
	ArgVectorRegister *arg2_vector = 
		dynamic_cast<ArgVectorRegister *>(arg2.get());
	int numerator_vreg = arg1_vector->getId();
	int divisor_vreg = arg2_vector->getId();

	// LLVM IR is shown below to calcuate x % y.
	// 
	// x % y = x - floor(x/y) * y
	// 
	// v_cvt_f32_u32 v2, v0 	// v2 = float(v0), convert i32 %x to float
	// v_cvt_f32_u32 v3, v1 	// v3 = float(v1), convert i32 %y to float
	// v_rcp_f32 v4, v1		// v4 = 1 / y
	// v_mul_f32 v6, v2, v4		// v6 = x * 1 / y
	// v_floor_f32 v7, v6		// v7 = floor(v6)
	// v_cvt_u32_f32 v8, v7		// v8 = uint(v7)
	// v_mul_i32 v9, v8, v1		// v9 = v8 * y = floor(x/y) * y
	// v_sub_i32 v10, vcc, v0, v9	// v10 = x - v9 = x - floor(x/y) * y

	Instruction *instruction;
	int numerator_to_float_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CVT_F32_U32);
	instruction->addVectorRegister(numerator_to_float_vreg);
	instruction->addArgument(std::move(arg1));
	assert(instruction->hasValidArguments());

	int divisor_to_float_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CVT_F32_U32);
	instruction->addVectorRegister(divisor_to_float_vreg);
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());

	int rcp_divisor_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_RCP_F32);
	instruction->addVectorRegister(rcp_divisor_vreg);
	instruction->addVectorRegister(divisor_to_float_vreg);
	assert(instruction->hasValidArguments());

	int numerator_mul_rcp_divisor_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_MUL_F32);
	instruction->addVectorRegister(numerator_mul_rcp_divisor_vreg);
	instruction->addVectorRegister(numerator_to_float_vreg);
	instruction->addVectorRegister(rcp_divisor_vreg);
	assert(instruction->hasValidArguments());

	int floor_x_div_y_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_FLOOR_F32);
	instruction->addVectorRegister(floor_x_div_y_vreg);
	instruction->addVectorRegister(numerator_mul_rcp_divisor_vreg);
	assert(instruction->hasValidArguments());

	int uint_floor_x_div_y_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CVT_U32_F32);
	instruction->addVectorRegister(uint_floor_x_div_y_vreg);
	instruction->addVectorRegister(floor_x_div_y_vreg);
	assert(instruction->hasValidArguments());

	int floor_mul_divisor_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_MUL_I32_I24);
	instruction->addVectorRegister(floor_mul_divisor_vreg);
	instruction->addVectorRegister(uint_floor_x_div_y_vreg);
	instruction->addVectorRegister(divisor_vreg);
	assert(instruction->hasValidArguments());

	instruction = addInstruction(SI::Instruction::Opcode_V_SUB_I32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	instruction->addVectorRegister(numerator_vreg);
	instruction->addVectorRegister(floor_mul_divisor_vreg);
	assert(instruction->hasValidArguments());

	// FIXME: more robust algorithm
#if 0
	// fdivisor = UINT_TO_FLT(Divisor)
	Instruction *instruction;
	int fdivisor_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CVT_F32_U32);
	instruction->addVectorRegister(fdivisor_vreg);
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());

	// approx_frecip = RECIP(fdivisor)
	int approx_frecip_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_RCP_F32);
	instruction->addVectorRegister(approx_frecip_vreg);
	instruction->addVectorRegister(fdivisor_vreg);
	assert(instruction->hasValidArguments());

	// scaled_frecip = MUL_V(approx_frecip, 2**32-1)
	int scaled_frecip_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_MUL_F32);
	instruction->addVectorRegister(scaled_frecip_vreg);
	instruction->addLiteral(0x4f800000);
	instruction->addVectorRegister(approx_frecip_vreg);
	assert(instruction->hasValidArguments());

	// approx_recip = FLT_TO_UINT(scaled_frecip)
	int approx_recip_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CVT_U32_F32);
	instruction->addVectorRegister(approx_recip_vreg);
	instruction->addVectorRegister(scaled_frecip_vreg);
	assert(instruction->hasValidArguments());

	// approx_one = MULLO_UINT(approx_recip,divisor);
	int approx_one_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_MUL_LO_U32);
	instruction->addVectorRegister(approx_one_vreg);
	instruction->addVectorRegister(divisor_vreg);
	instruction->addVectorRegister(approx_recip_vreg);
	assert(instruction->hasValidArguments());

	// approx_one_hi = MULHI_UINT(approx_recip,divisor);
	int approx_one_hi_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_MUL_HI_U32);
	instruction->addVectorRegister(approx_one_hi_vreg);
	instruction->addVectorRegister(divisor_vreg);
	instruction->addVectorRegister(approx_recip_vreg);
	assert(instruction->hasValidArguments());

	// error = ~approx_one
	int init_error_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_SUB_I32);
	instruction->addVectorRegister(init_error_vreg);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	instruction->addLiteral(0);
	instruction->addVectorRegister(approx_one_vreg);
	assert(instruction->hasValidArguments());

	// if (approx_one_hi == 0) error = error else error = approx_one
	int cond_approx_one_hi_eq_zero_sreg = function->AllocSReg(2, 2);
	instruction = addInstruction(SI::Instruction::Opcode_V_CMP_NE_I32_VOP3a);
	instruction->addScalarRegisterSeries(cond_approx_one_hi_eq_zero_sreg, 
					     cond_approx_one_hi_eq_zero_sreg + 1);
	instruction->addLiteral(0);
	instruction->addVectorRegister(approx_one_hi_vreg);
	assert(instruction->hasValidArguments());

	// Notice the condition is NE in previous instruction, 
	// need to swap src0 and src1
	int error_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CNDMASK_B32_VOP3a);
	instruction->addVectorRegister(error_vreg);
	instruction->addVectorRegister(init_error_vreg);
	instruction->addVectorRegister(approx_one_vreg);
	instruction->addScalarRegisterSeries(cond_approx_one_hi_eq_zero_sreg, 
		cond_approx_one_hi_eq_zero_sreg + 1);
	assert(instruction->hasValidArguments());

	// recip_corrector = MULHI_UINT(error,approx_recip)
	int recip_corrector_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_MUL_HI_U32);
	instruction->addVectorRegister(recip_corrector_vreg);
	instruction->addVectorRegister(error_vreg);
	instruction->addVectorRegister(approx_recip_vreg);
	assert(instruction->hasValidArguments());

	// better_recip_1 = approx_recip - recip_corrector;
  	// if approx_recip is higher, need to subtract recip_corrector
  	int better_recip_1_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_SUB_I32);
	instruction->addVectorRegister(better_recip_1_vreg);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	instruction->addVectorRegister(approx_recip_vreg);
	instruction->addVectorRegister(recip_corrector_vreg);
	assert(instruction->hasValidArguments());

	// better_recip_2 = approx_recip + recip_corrector;
  	// if approx_recip is lower, need to add recip_corrector
  	int better_recip_2_vreg = function->AllocVReg();
  	instruction = addInstruction(SI::Instruction::Opcode_V_ADD_I32);
  	instruction->addVectorRegister(better_recip_2_vreg);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
  	instruction->addVectorRegister(approx_recip_vreg);
	instruction->addVectorRegister(recip_corrector_vreg);
	assert(instruction->hasValidArguments());
	
	//if (approx_one_hi == 0) better_recip = better_recip_2
	//	else better_recip = better_recip_1
	// Notice the condition is NE in previous instruction, 
	// need to swap src0 and src1
	int better_recip_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CNDMASK_B32_VOP3a);
	instruction->addVectorRegister(better_recip_vreg);
	instruction->addVectorRegister(better_recip_2_vreg);
	instruction->addVectorRegister(better_recip_1_vreg);
	instruction->addScalarRegisterSeries(cond_approx_one_hi_eq_zero_sreg, 
					cond_approx_one_hi_eq_zero_sreg + 1);
	assert(instruction->hasValidArguments());

	// approx_Quotient = MULHI_UINT (Numerator,better_recip)
	int approx_quotient_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_MUL_HI_U32);
	instruction->addVectorRegister(approx_quotient_vreg);
	instruction->addVectorRegister(better_recip_vreg);
	instruction->addVectorRegister(numerator_vreg);
	assert(instruction->hasValidArguments());

	// approx_numerator =  MULLO_UINT(approx_Quotient,Divisor);
	int approx_numerator_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_MUL_LO_U32);
	instruction->addVectorRegister(approx_numerator_vreg);
	instruction->addVectorRegister(approx_quotient_vreg);
	instruction->addVectorRegister(divisor_vreg);
	assert(instruction->hasValidArguments());

	// approx_remainder = Numerator - approx_numerator;
	int approx_remainder_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_SUB_I32);
	instruction->addVectorRegister(approx_remainder_vreg);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	instruction->addVectorRegister(numerator_vreg);
	instruction->addVectorRegister(approx_numerator_vreg);
	assert(instruction->hasValidArguments());

	// rem_gtr_approx_rem = Numerator >= approx_numerator
	int cond_numberator_ge_approx_numberator_sregs = 
		function->AllocSReg(2, 2);	
	instruction = addInstruction(SI::Instruction::Opcode_V_CMP_GE_U32_VOP3a);
	instruction->addScalarRegisterSeries(
		cond_numberator_ge_approx_numberator_sregs,
		cond_numberator_ge_approx_numberator_sregs + 1);
	instruction->addVectorRegister(numerator_vreg);
	instruction->addVectorRegister(approx_numerator_vreg);
	assert(instruction->hasValidArguments());

	// remainder_decrement = approx_remainder - Divisor;
	// int remainder_decrement_vreg = function->AllocVReg();
	// instruction = addInstruction(SI::Instruction::Opcode_V_SUB_I32);
	// instruction->addVectorRegister(remainder_decrement_vreg);
	// instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	// instruction->addVectorRegister(approx_remainder_vreg);
	// instruction->addVectorRegister(divisor_vreg);
	// assert(instruction->hasValidArguments());

	// approx_remainder >= divisor
	int cond_approx_remainder_ge_divisor_sregs = 
		function->AllocSReg(2, 2);	
	instruction = addInstruction(SI::Instruction::Opcode_V_CMP_GE_U32_VOP3a);
	instruction->addScalarRegisterSeries(
		cond_approx_remainder_ge_divisor_sregs,
		cond_approx_remainder_ge_divisor_sregs + 1);
	instruction->addVectorRegister(approx_remainder_vreg);
	instruction->addVectorRegister(divisor_vreg);
	assert(instruction->hasValidArguments());

	// approx_quotient +1
	int approx_quotient_plus_one_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_ADD_I32);
	instruction->addVectorRegister(approx_quotient_plus_one_vreg);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	instruction->addLiteral(1);
	instruction->addVectorRegister(approx_quotient_vreg);
	assert(instruction->hasValidArguments());

	// cond = aprox_remainder >= divisor && numerator >= approx numerator
	int combined_cond_sregs = function->AllocSReg(2, 2);
	instruction = addInstruction(SI::Instruction::Opcode_S_AND_B64);
	instruction->addScalarRegisterSeries(
		combined_cond_sregs,
		combined_cond_sregs + 1);
	instruction->addScalarRegisterSeries(
		cond_numberator_ge_approx_numberator_sregs,
		cond_numberator_ge_approx_numberator_sregs + 1);
	instruction->addScalarRegisterSeries(
		cond_approx_remainder_ge_divisor_sregs,
		cond_approx_remainder_ge_divisor_sregs + 1);
	assert(instruction->hasValidArguments());

	// approx_quotient -1
	int approx_quotient_plus_negone_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_ADD_I32);
	instruction->addVectorRegister(approx_quotient_plus_negone_vreg);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	instruction->addLiteral(-1);
	instruction->addVectorRegister(approx_quotient_vreg);
	assert(instruction->hasValidArguments());

	// If (approx_remainder >= divisor && numerator >= approx numerator)
	//   Remainder = remainder_decrement;
	// else Remainder = approx_remainder;
	// int almost_remainder_vreg = function->AllocVReg();
	// instruction = addInstruction(SI::Instruction::Opcode_V_CNDMASK_B32_VOP3a);
	// instruction->addVectorRegister(almost_remainder_vreg);
	// instruction->addVectorRegister(remainder_decrement_vreg);
	// instruction->addVectorRegister(approx_remainder_vreg);
	// instruction->addScalarRegisterSeries(combined_cond_sregs, 
	// 				combined_cond_sregs + 1);
	// assert(instruction->hasValidArguments());

	// If (approx_remainder >= divisor && numerator >= approx numerator)
	//   approx_quotient = approx_quotient
	// else approx_quotient = approx_quotient + 1
	int cnd1_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CNDMASK_B32_VOP3a);
	instruction->addVectorRegister(cnd1_vreg);
	instruction->addVectorRegister(approx_quotient_vreg);
	instruction->addVectorRegister(approx_quotient_plus_one_vreg);
	instruction->addScalarRegisterSeries(combined_cond_sregs,
		combined_cond_sregs + 1);
	assert(instruction->hasValidArguments());

	// If numerator >= approx_numerator
	//   approx_quotient = approx_quotient -1
	// else approx_quotient = approx_quotient
	int cnd2_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CNDMASK_B32_VOP3a);
	instruction->addVectorRegister(cnd2_vreg);
	instruction->addVectorRegister(approx_quotient_plus_negone_vreg);
	instruction->addVectorRegister(cnd1_vreg);
	instruction->addScalarRegisterSeries(
		cond_numberator_ge_approx_numberator_sregs,
		cond_numberator_ge_approx_numberator_sregs + 1);
	assert(instruction->hasValidArguments());

	//If (divisor == 0) Remainder = 0xffffffff; else Remainder = Remainder
	instruction = addInstruction(SI::Instruction::Opcode_V_CMP_NE_I32);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	instruction->addLiteral(0);
	instruction->addVectorRegister(divisor_vreg);
	assert(instruction->hasValidArguments());

	// instruction = addInstruction(SI::Instruction::Opcode_V_CNDMASK_B32_VOP3a);
	// instruction->addVectorRegister(ret_vreg);
	// instruction->addLiteral(-1);
	// instruction->addVectorRegister(almost_remainder_vreg);
	// instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	// assert(instruction->hasValidArguments());


	// 
	int update_numerator_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CNDMASK_B32_VOP3a);
	instruction->addVectorRegister(update_numerator_vreg);
	instruction->addLiteral(-1);
	instruction->addVectorRegister(cnd2_vreg);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	assert(instruction->hasValidArguments());

	// mullo numerator
	int mul_lo_numerator_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_MUL_LO_I32);
	instruction->addVectorRegister(mul_lo_numerator_vreg);
	instruction->addVectorRegister(update_numerator_vreg);
	instruction->addVectorRegister(numerator_vreg);
	assert(instruction->hasValidArguments());

	// Remainder sub
	instruction = addInstruction(SI::Instruction::Opcode_V_SUB_I32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	instruction->addVectorRegister(divisor_vreg);
	instruction->addVectorRegister(mul_lo_numerator_vreg);
	assert(instruction->hasValidArguments());



	// // Assign remainder
	// int remainder_vreg = function->AllocVReg();
	// instruction = addInstruction(SI::Instruction::Opcode_V_CNDMASK_B32_VOP3a);
	// instruction->addVectorRegister(remainder_vreg);
	// instruction->addVectorRegister(remainder_decrement_vreg);
	// instruction->addVectorRegister(approx_remainder_vreg);
	// instruction->addScalarRegisterSeries(combined_cond_sregs, 
	// 	combined_cond_sregs + 1);
	// assert(instruction->hasValidArguments());

	// // If (numerator < approx_numerator) Remainder += Divisor
	// int remainder_plus_divisor_vreg = function->AllocVReg();
	// instruction = addInstruction(SI::Instruction::Opcode_V_ADD_I32);
	// instruction->addVectorRegister(remainder_plus_divisor_vreg);
	// instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	// instruction->addVectorRegister(remainder_vreg);
	// instruction->addVectorRegister(divisor_vreg);
	// assert(instruction->hasValidArguments());

	// int cond_numberator_lt_approx_numberator_sregs = function->AllocSReg(2, 2);
	// instruction = addInstruction(SI::Instruction::Opcode_V_CMP_LT_I32_VOP3a);
	// instruction->addScalarRegisterSeries(cond_numberator_lt_approx_numberator_sregs,
	// 	cond_numberator_lt_approx_numberator_sregs + 1);
	// instruction->addVectorRegister(numerator_vreg);
	// instruction->addVectorRegister(approx_numerator_vreg);
	// assert(instruction->hasValidArguments());


	// Return remainder
	// instruction = addInstruction(SI::Instruction::Opcode_V_CNDMASK_B32_VOP3a);
	// instruction->addVectorRegister(ret_vreg);
	// instruction->addVectorRegister(remainder_plus_divisor_vreg);
	// instruction->addVectorRegister(remainder_vreg);
	// instruction->addScalarRegisterSeries(
	//	cond_numberator_lt_approx_numberator_sregs, 
	// 	cond_numberator_lt_approx_numberator_sregs + 1);
	// assert(instruction->hasValidArguments());
#endif

}


void BasicBlock::EmitUDiv(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Only supported for 32-bit integers
	llvm::Type *type = llvm_inst->getType();
	if (!type->isIntegerTy())
		throw misc::Panic("Only supported for int type arguments");

	// Get operands
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Make sure in vector registers
	ArgScalarToVector(arg1, llvm_arg1);
	ArgScalarToVector(arg2, llvm_arg2);
	ArgLiteralToVector(arg1);
	ArgLiteralToVector(arg2);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// TODO: use better algorithm 
	Instruction *instruction;
	int numerator_to_float_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CVT_F32_U32);
	instruction->addVectorRegister(numerator_to_float_vreg);
	instruction->addArgument(std::move(arg1));
	assert(instruction->hasValidArguments());

	int divisor_to_float_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CVT_F32_U32);
	instruction->addVectorRegister(divisor_to_float_vreg);
	instruction->addArgument(std::move(arg2));
	assert(instruction->hasValidArguments());

	int rcp_divisor_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_RCP_F32);
	instruction->addVectorRegister(rcp_divisor_vreg);
	instruction->addVectorRegister(divisor_to_float_vreg);
	assert(instruction->hasValidArguments());

	int numerator_mul_rcp_divisor_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_MUL_F32);
	instruction->addVectorRegister(numerator_mul_rcp_divisor_vreg);
	instruction->addVectorRegister(numerator_to_float_vreg);
	instruction->addVectorRegister(rcp_divisor_vreg);
	assert(instruction->hasValidArguments());

	instruction = addInstruction(SI::Instruction::Opcode_V_CVT_U32_F32);
	instruction->addVectorRegister(ret_vreg);
	instruction->addVectorRegister(numerator_mul_rcp_divisor_vreg);
	assert(instruction->hasValidArguments());

}


void BasicBlock::EmitSDiv(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Only supported for 32-bit integers
	llvm::Type *type = llvm_inst->getType();
	if (!type->isIntegerTy())
		throw misc::Panic("Only supported for int type arguments");

	// Get operands
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Convert scalar register to vector registor when neccesary
	ArgScalarToVector(arg1, llvm_arg1);
	ArgScalarToVector(arg2, llvm_arg2);
	ArgLiteralToVector(arg1);
	ArgLiteralToVector(arg2);

	// Allocate vector register and create symbol for return value
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = function->addSymbol(ret_name);
	ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

	// Get ID of vector registers
	ArgVectorRegister *arg1_vector = 
		dynamic_cast<ArgVectorRegister *>(arg1.get());
	ArgVectorRegister *arg2_vector = 
		dynamic_cast<ArgVectorRegister *>(arg2.get());
	
	int numerator_vreg = arg1_vector->getId();
	int divisor_vreg = arg2_vector->getId();

	Instruction *instruction;

	// Convert to positive numerator/divisor
	int numerator_lt_zero_sregs = function->AllocSReg(2, 2);
	instruction = addInstruction(SI::Instruction::Opcode_V_CMP_LT_I32_VOP3a);
	instruction->addScalarRegisterSeries(numerator_lt_zero_sregs,
		numerator_lt_zero_sregs + 1);
	instruction->addVectorRegister(numerator_vreg);
	instruction->addLiteral(0);
	assert(instruction->hasValidArguments());

	int zero_sub_numerator_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_SUB_I32);
	instruction->addVectorRegister(zero_sub_numerator_vreg);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	instruction->addLiteral(0);
	instruction->addVectorRegister(numerator_vreg);
	assert(instruction->hasValidArguments());

	instruction = addInstruction(SI::Instruction::Opcode_V_MAX_I32);
	instruction->addVectorRegister(numerator_vreg);
	instruction->addVectorRegister(zero_sub_numerator_vreg);
	instruction->addVectorRegister(numerator_vreg);
	assert(instruction->hasValidArguments());

        int divisor_lt_zero_sregs = function->AllocSReg(2, 2);
        instruction = addInstruction(SI::Instruction::Opcode_V_CMP_LT_I32_VOP3a);
        instruction->addScalarRegisterSeries(divisor_lt_zero_sregs,
                divisor_lt_zero_sregs + 1);
        instruction->addVectorRegister(divisor_vreg);
        instruction->addLiteral(0);
        assert(instruction->hasValidArguments());

        int zero_sub_divisor_vreg = function->AllocVReg();
        instruction = addInstruction(SI::Instruction::Opcode_V_SUB_I32);
        instruction->addVectorRegister(zero_sub_divisor_vreg);
        instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
        instruction->addLiteral(0);
        instruction->addVectorRegister(divisor_vreg);
        assert(instruction->hasValidArguments());

        instruction = addInstruction(SI::Instruction::Opcode_V_MAX_I32);
        instruction->addVectorRegister(divisor_vreg);
        instruction->addVectorRegister(zero_sub_divisor_vreg);
        instruction->addVectorRegister(divisor_vreg);
        assert(instruction->hasValidArguments());

	// Same as UDiv
	// TODO: use better algorithm 
	int numerator_to_float_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CVT_F32_U32);
	instruction->addVectorRegister(numerator_to_float_vreg);
	instruction->addVectorRegister(numerator_vreg);
	assert(instruction->hasValidArguments());

	int divisor_to_float_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CVT_F32_U32);
	instruction->addVectorRegister(divisor_to_float_vreg);
	instruction->addVectorRegister(divisor_vreg);
	assert(instruction->hasValidArguments());

	int rcp_divisor_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_RCP_F32);
	instruction->addVectorRegister(rcp_divisor_vreg);
	instruction->addVectorRegister(divisor_to_float_vreg);
	assert(instruction->hasValidArguments());

	int numerator_mul_rcp_divisor_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_MUL_F32);
	instruction->addVectorRegister(numerator_mul_rcp_divisor_vreg);
	instruction->addVectorRegister(numerator_to_float_vreg);
	instruction->addVectorRegister(rcp_divisor_vreg);
	assert(instruction->hasValidArguments());

	int pre_ret_vreg = function->AllocVReg();
	instruction = addInstruction(SI::Instruction::Opcode_V_CVT_U32_F32);
	instruction->addVectorRegister(pre_ret_vreg);
	instruction->addVectorRegister(numerator_mul_rcp_divisor_vreg);
	assert(instruction->hasValidArguments());

  	// If numerator/divisor is less than 0, the result is negative
  	int combined_cond_sregs = function->AllocSReg(2, 2);
  	instruction = addInstruction(SI::Instruction::Opcode_S_XOR_B64);
  	instruction->addScalarRegisterSeries(combined_cond_sregs, 
  		combined_cond_sregs + 1);
  	instruction->addScalarRegisterSeries(numerator_lt_zero_sregs,
  		numerator_lt_zero_sregs + 1);
  	instruction->addScalarRegisterSeries(divisor_lt_zero_sregs,
  		divisor_lt_zero_sregs + 1);
  	assert(instruction->hasValidArguments());

  	int zero_sub_pre_ret_vreg = function->AllocVReg();
  	instruction = addInstruction(SI::Instruction::Opcode_V_SUB_I32);
  	instruction->addVectorRegister(zero_sub_pre_ret_vreg);
	instruction->addSpecialRegister(SI::Instruction::SpecialRegVcc);
	instruction->addLiteral(0);
	instruction->addVectorRegister(pre_ret_vreg);
	assert(instruction->hasValidArguments());

	instruction = addInstruction(SI::Instruction::Opcode_V_CNDMASK_B32_VOP3a);
	instruction->addVectorRegister(ret_vreg);
	instruction->addVectorRegister(pre_ret_vreg);
	instruction->addVectorRegister(zero_sub_pre_ret_vreg);
	instruction->addScalarRegisterSeries(combined_cond_sregs,
		combined_cond_sregs + 1);
	assert(instruction->hasValidArguments());

}


void BasicBlock::EmitShl(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 32-bit integers
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32))
		throw misc::Panic("Only supported for 32-bit integers");

	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Emit scalar instruction when:
	//	1. arg1 and arg2 are both scalar
	//	2. one is scalar and the other is literal
	bool emit_scalar_arg1 = arg1->hasValidType(Argument::TypeScalarRegister,
		Argument::TypeLiteral,
		Argument::TypeLiteralReduced);
	bool emit_scalar_arg2 = arg2->hasValidType(Argument::TypeScalarRegister,
		Argument::TypeLiteral,
		Argument::TypeLiteralReduced);
	bool emit_scalar_args_diff_type = arg1->getType() != arg2->getType();
	bool emit_scalar = emit_scalar_arg1 && 
		emit_scalar_arg2 && emit_scalar_args_diff_type;

	// Emit instruction
	if (emit_scalar)
	{		
		// Allocate scalar register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		int ret_sreg = function->AllocSReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeScalarRegister, ret_sreg);

		// Emit left shift.
		//
		// s_lshl_b32 ret_sreg, arg_op1, arg_op2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_S_LSHL_B32);
		instruction->addVectorRegister(ret_sreg);
		instruction->addArgument(std::move(arg1));
		instruction->addArgument(std::move(arg2));
		assert(instruction->hasValidArguments());
	}
	else
	{
		// Arg1 can be vector/scalar/literals, Arg2 has to be vector
		ArgScalarToVector(arg2, llvm_arg2);
		ArgLiteralToVector(arg2);

		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Emit left shift.
		//
		// v_lshl_b32 ret_vreg, arg_op1, arg_op2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_LSHL_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addArgument(std::move(arg1));
		instruction->addArgument(std::move(arg2));
		assert(instruction->hasValidArguments());
	}
}


void BasicBlock::EmitLShr(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 32-bit integers
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32))
		throw misc::Panic("Only supported for 32-bit integers");

	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Emit scalar instruction when:
	//	1. arg1 and arg2 are both scalar
	//	2. one is scalar and the other is literal
	bool emit_scalar_arg1 = arg1->hasValidType(Argument::TypeScalarRegister,
		Argument::TypeLiteral,
		Argument::TypeLiteralReduced);
	bool emit_scalar_arg2 = arg2->hasValidType(Argument::TypeScalarRegister,
		Argument::TypeLiteral,
		Argument::TypeLiteralReduced);
	bool emit_scalar_args_diff_type = arg1->getType() != arg2->getType();
	bool emit_scalar = emit_scalar_arg1 && 
		emit_scalar_arg2 && emit_scalar_args_diff_type;

	// Emit instruction
	if (emit_scalar)
	{		
		// Allocate scalar register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		int ret_sreg = function->AllocSReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeScalarRegister, ret_sreg);

		// Emit logical right shift.
		//
		// s_lshr_b32 ret_sreg, arg_op1, arg_op2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_S_LSHR_B32);
		instruction->addVectorRegister(ret_sreg);
		instruction->addArgument(std::move(arg2));
		instruction->addArgument(std::move(arg1));
		assert(instruction->hasValidArguments());		
	}
	else
	{
		// Arg1 can be vector/scalar/literals, Arg2 has to be vector
		ArgScalarToVector(arg2, llvm_arg2);
		ArgLiteralToVector(arg2);

		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Emit logical right shift.
		//
		// v_lshr_b32 ret_vreg, arg_op1, arg_op2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_LSHRREV_B32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addArgument(std::move(arg2));
		instruction->addArgument(std::move(arg1));
		assert(instruction->hasValidArguments());
	}
}


void BasicBlock::EmitAShr(llvm::BinaryOperator *llvm_inst)
{
	// Only supported for 32-bit integers
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32))
		throw misc::Panic("Only supported for 32-bit integers");

	// Only supported for 2 operands (op1, op2)
	if (llvm_inst->getNumOperands() != 2)
		throw misc::Panic(misc::fmt("2 operands supported, %d found",
				llvm_inst->getNumOperands()));

	// Get operands
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Emit scalar instruction when:
	//	1. arg1 and arg2 are both scalar
	//	2. one is scalar and the other is literal
	bool emit_scalar_arg1 = arg1->hasValidType(Argument::TypeScalarRegister,
		Argument::TypeLiteral,
		Argument::TypeLiteralReduced);
	bool emit_scalar_arg2 = arg2->hasValidType(Argument::TypeScalarRegister,
		Argument::TypeLiteral,
		Argument::TypeLiteralReduced);
	bool emit_scalar_args_diff_type = arg1->getType() != arg2->getType();
	bool emit_scalar = emit_scalar_arg1 && 
		emit_scalar_arg2 && emit_scalar_args_diff_type;

	// Emit instruction
	if (emit_scalar)
	{		
		// Allocate scalar register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		int ret_sreg = function->AllocSReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeScalarRegister, ret_sreg);

		// Emit arithmetic right shift.
		//
		// s_ashr_i32 ret_sreg, arg_op1, arg_op2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_S_ASHR_I32);
		instruction->addVectorRegister(ret_sreg);
		instruction->addArgument(std::move(arg2));
		instruction->addArgument(std::move(arg1));
		assert(instruction->hasValidArguments());		
	}
	else
	{
		// Arg1 can be vector/scalar/literals, Arg2 has to be vector
		ArgScalarToVector(arg2, llvm_arg2);
		ArgLiteralToVector(arg2);

		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, ret_vreg);

		// Emit arithmetic right shift.
		//
		// v_ashr_i32 ret_vreg, arg_op1, arg_op2
		//
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_ASHRREV_I32);
		instruction->addVectorRegister(ret_vreg);
		instruction->addArgument(std::move(arg2));
		instruction->addArgument(std::move(arg1));
		assert(instruction->hasValidArguments());
	}
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
	Symbol *arg1_symbol;
	std::unique_ptr<Argument> arg1 = function->TranslateValue(llvm_arg1, arg1_symbol);
	std::unique_ptr<Argument> arg2 = function->TranslateValue(llvm_arg2);

	// Emit instructions based on instruction type
	llvm::Type *llvm_type = llvm_arg1->getType();
	if (llvm_type->isVectorTy())
	{
		// First argument must be a vector register and the second must be the
		// index.
		arg1->ValidTypes(Argument::TypeVectorRegister);
		arg2->ValidTypes(Argument::TypeLiteral,
				Argument::TypeLiteralReduced);

		// Obtain the index
		ArgLiteral *arg2_literal = dynamic_cast<ArgLiteral *>(arg2.get());
		assert(arg2_literal);
		int elem_idx = arg2_literal->getValue();

		int arg1_symbol_num_regs = arg1_symbol->getNumRegisters();
		int num_elems = llvm_type->getVectorNumElements();
		
		// Sanity check
		assert(elem_idx <= arg1_symbol_num_regs - 1);
		assert( num_elems == arg1_symbol_num_regs);
		
		int vreg = arg1_symbol->getId() + elem_idx;

		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, vreg);		

	}	
	else
	{
		// First argument must be a scalar register and the second must be the
		// offset.
		arg1->ValidTypes(Argument::TypeScalarRegister);
		arg2->ValidTypes(Argument::TypeLiteral,
				Argument::TypeLiteralReduced);

		// Obtain actual scalar register
		ArgScalarRegister *arg1_scalar = 
			dynamic_cast<ArgScalarRegister *>(arg1.get());
		ArgLiteral *arg2_literal = 
			dynamic_cast<ArgLiteral *>(arg2.get());
		assert(arg1_scalar);
		assert(arg2_literal);
		arg1_scalar->setId(arg1_scalar->getId() + 
			arg2_literal->getValue());

		// Allocate vector register and create symbol for return value
		std::string ret_name = llvm_inst->getName();
		//int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeScalarRegister, 
			arg1_scalar->getId());
	}
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

	// insertelement <4 x float> undef, float %op1, i32 0
	if (!llvm_arg1->hasName())
	{
		llvm::Type *llvm_type = llvm_arg1->getType();
		assert(llvm_type->isVectorTy());

		std::unique_ptr<Argument> arg2 = 
			function->TranslateValue(llvm_arg2);
		std::unique_ptr<Argument> arg3 = 
			function->TranslateValue(llvm_arg3);

		int num_elems = llvm_type->getVectorNumElements();
		int ret_vreg = function->AllocVReg(num_elems, 1);

		std::string ret_name = llvm_inst->getName();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, 
			ret_vreg, ret_vreg + num_elems - 1);

		ArgVectorRegister *arg2_reg = 
			dynamic_cast<ArgVectorRegister *>(arg2.get());
		ArgLiteral *arg3_literal = 
			dynamic_cast<ArgLiteral *>(arg3.get());
		assert(arg3_literal);
		assert(arg3_literal->getValue() < num_elems);

		int src_vreg = arg2_reg->getId();
		int dst_vreg = ret_vreg + arg3_literal->getValue();

		// Emit instruction
		//
		// v_mov_b32 dst_vreg, src_vreg
		///
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MOV_B32);
		instruction->addVectorRegister(dst_vreg);
		instruction->addVectorRegister(src_vreg);
		assert(instruction->hasValidArguments());
	}
	else if (llvm_arg1->hasName() && llvm_arg1->getType()->isVectorTy())
	{
		llvm::Type *llvm_type = llvm_arg1->getType();
		assert(llvm_type->isVectorTy());

		Symbol *arg1_symbol;
		std::unique_ptr<Argument> arg1 = 
			function->TranslateValue(llvm_arg1, arg1_symbol);
		std::unique_ptr<Argument> arg2 = 
			function->TranslateValue(llvm_arg2);
		std::unique_ptr<Argument> arg3 = 
			function->TranslateValue(llvm_arg3);

		int num_elems = llvm_type->getVectorNumElements();
		int num_vregs = arg1_symbol->getNumRegisters();
		assert(num_elems == num_vregs);

		ArgVectorRegister *arg1_reg = 
			dynamic_cast<ArgVectorRegister *>(arg1.get());
		ArgVectorRegister *arg2_reg = 
			dynamic_cast<ArgVectorRegister *>(arg2.get());
		ArgLiteral *arg3_literal = 
			dynamic_cast<ArgLiteral *>(arg3.get());
		assert(arg3_literal);
		assert(arg3_literal->getValue() < num_elems);

		int ret_vreg = arg1_reg->getId();

		std::string ret_name = llvm_inst->getName();
		Symbol *ret_symbol = function->addSymbol(ret_name);
		ret_symbol->setRegister(Symbol::TypeVectorRegister, 
			ret_vreg, ret_vreg + num_elems - 1);

		int src_vreg = arg2_reg->getId();
		int dst_vreg = ret_vreg + arg3_literal->getValue();

		// Emit instruction
		//
		// v_mov_b32 dst_vreg, src_vreg
		///
		Instruction *instruction = addInstruction(SI::Instruction::Opcode_V_MOV_B32);
		instruction->addVectorRegister(dst_vreg);
		instruction->addVectorRegister(src_vreg);
		assert(instruction->hasValidArguments());		
	}
	else
	{
		std::unique_ptr<Argument> arg1 = 
			function->TranslateValue(llvm_arg1);
		std::unique_ptr<Argument> arg2 = 
			function->TranslateValue(llvm_arg2);
		std::unique_ptr<Argument> arg3 = 
			function->TranslateValue(llvm_arg3);

		// First argument must be a scalar register and the second must
		// be the offset.
		arg1->ValidTypes(Argument::TypeScalarRegister,
				Argument::TypeVectorRegister);
		arg2->ValidTypes(Argument::TypeScalarRegister,
				Argument::TypeVectorRegister);
		arg3->ValidTypes(Argument::TypeLiteral,
				Argument::TypeLiteralReduced);

		// Rule out case where dest is vgpr and src is sgpr
		assert((arg1->getType() != Argument::TypeScalarRegister) && 
			(arg2->getType() != Argument::TypeVectorRegister));

		ArgLiteral *arg3_literal = 
			dynamic_cast<ArgLiteral *>(arg3.get());
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
			Instruction *instruction = 
				addInstruction(SI::Instruction::Opcode_S_MOV_B32);
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
			Instruction *instruction = 
				addInstruction(SI::Instruction::Opcode_V_MOV_B32);
			instruction->addArgument(std::move(arg2));
			instruction->addArgument(std::move(arg1));
			assert(instruction->hasValidArguments());
		}		
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

		case llvm::Instruction::UIToFP:

			EmitUitofp(misc::cast<llvm::CastInst *>(&llvm_inst));
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

		case llvm::Instruction::URem:

			EmitURem(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::UDiv:

			EmitUDiv(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::SDiv:

			EmitSDiv(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::Shl:

			EmitShl(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::LShr:

			EmitLShr(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::AShr:

			EmitAShr(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
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
				"(%d) %s", llvm_inst.getOpcode(), 
				llvm_inst.getOpcodeName(llvm_inst.getOpcode())));
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

