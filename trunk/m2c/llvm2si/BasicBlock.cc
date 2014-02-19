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
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/Instructions.h>
#include <llvm/InstrTypes.h>
#include <llvm/Type.h>

#include "BasicBlock.h"
#include "Function.h"


using namespace misc;
using namespace si2bin;

namespace llvm2si
{

int BasicBlock::GetLlvmTypeSize(llvm::Type *llvm_type)
{
	/* Return size based on type kind */
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

	/* Actions depending on type */
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
		panic("%s: unsupported type kind", __FUNCTION__);
		return 0;
	}
}


int BasicBlock::GetPointedLlvmTypeSize(llvm::Type *llvm_type)
{
	/* Get pointed type */
	assert(llvm_type->isPointerTy());
	llvm_type = llvm_type->getPointerElementType();
	return GetLlvmTypeSize(llvm_type);
}


void BasicBlock::EmitAdd(llvm::BinaryOperator *llvm_inst)
{
	/* Only supported for 32-bit integers */
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32))
		panic("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Only supported for 2 operands (op1, op2) */
	if (llvm_inst->getNumOperands() != 2)
		panic("%s: 2 operands supported, %d found",
			__FUNCTION__, llvm_inst->getNumOperands());

	/* Get operands (vreg, literal) */
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	Arg *arg1 = function->TranslateValue(llvm_arg1);
	Arg *arg2 = function->TranslateValue(llvm_arg2);

	/* Second operand cannot be a constant */
	arg2 = function->ConstToVReg(this, arg2);
	arg1->ValidTypes(ArgTypeVectorRegister,
			ArgTypeLiteral,
			ArgTypeLiteralReduced,
			ArgTypeLiteralFloat,
			ArgTypeLiteralFloatReduced);
	arg2->ValidTypes(ArgTypeVectorRegister);

	/* Allocate vector register and create symbol for return value */
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = new Symbol(llvm_inst->getName(),
			SymbolVectorRegister, ret_vreg);
	function->AddSymbol(ret_symbol);

	/* Emit addition.
	 * v_add_i32 ret_vreg, vcc, arg_op1, arg_op2
	 */
	Inst *inst = new Inst(SI::INST_V_ADD_I32,
			new ArgVectorRegister(ret_vreg),
			new ArgSpecialRegister(SI::InstSpecialRegVcc),
			arg1,
			arg2);
	AddInst(inst);
}


void BasicBlock::EmitCall(llvm::CallInst *llvm_inst)
{
	/* Get called function, found in last operand of the operand list, as
	 * returned by LLVMGetOperand. */
	llvm::Function *llvm_function = llvm_inst->getCalledFunction();
	std::string func_name = llvm_function->getName();

	/* Get return argument name */
	std::string var_name = llvm_inst->getName();
	if (var_name.empty())
		panic("%s: invalid return variable",
				__FUNCTION__);

	/* Number of arguments */
	if (llvm_inst->getNumOperands() - 1 !=
			llvm_function->getArgumentList().size())
		fatal("%s: %d argument expected for '%s', %d found",
			__FUNCTION__,
			llvm_inst->getNumOperands() - 1,
			func_name.c_str(),
			(int) llvm_function->getArgumentList().size());

	/* Get argument and check type */
	llvm::Value *llvm_op = llvm_inst->getOperand(0);
	llvm::Type *llvm_type = llvm_op->getType();
	if (!llvm_type->isIntegerTy() || !llvm::isa<llvm::ConstantInt>(llvm_op))
		fatal("%s: argument should be an integer constant",
				__FUNCTION__);

	/* Get argument value and check bounds */
	llvm::ConstantInt *llvm_const = llvm::cast<llvm::ConstantInt>(llvm_op);
	int dim = llvm_const->getZExtValue();
	if (!inRange(dim, 0, 2))
		fatal("%s: constant in range [0..2] expected",
				__FUNCTION__);

	/* Built-in functions */
	if (func_name == "__get_global_id_u32")
	{
		/* Create new symbol associating it with the vector register
		 * containing the global ID in the given dimension. */
		Symbol *ret_symbol = new Symbol(var_name, SymbolVectorRegister,
				function->getVRegGid() + dim);
		function->AddSymbol(ret_symbol);
	}
	else if (func_name == "__get_local_id_u32")
	{
		/* Create new symbol associating it with the vector register
		 * containing the global ID in the given dimension. */
		Symbol *ret_symbol = new Symbol(var_name, SymbolVectorRegister,
				function->getVRegLid() + dim);
		function->AddSymbol(ret_symbol);
	}
	else if (func_name == "get_global_size")
	{
		/* Allocate a new vector register to copy global size. */
		int ret_vreg = function->AllocVReg();
		auto ret_arg = new ArgVectorRegister(ret_vreg);
		auto ret_symbol = new Symbol(var_name, SymbolVectorRegister,
				ret_vreg);
		function->AddSymbol(ret_symbol);

		/* Create new vector register containing the global size.
		 * v_mov_b32 vreg, s[gsize+dim]
		 */
		Inst *inst = new Inst(SI::INST_V_MOV_B32,
				ret_arg,
				new ArgScalarRegister(function->getSRegGSize() + dim));
		AddInst(inst);
	}
	else
	{
		fatal("%s: %s: invalid built-in function",
			__FUNCTION__, func_name.c_str());
	}
}


void BasicBlock::EmitGetElementPtr(llvm::GetElementPtrInst *llvm_inst)
{
	/* Only supported for 2 operands (pointer + 1 index) */
	if (llvm_inst->getNumOperands() != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, llvm_inst->getNumOperands());

	/* Get pointer operand (vreg) */
	llvm::Value *llvm_arg_ptr = llvm_inst->getPointerOperand();
	Symbol *ptr_symbol;
	Arg *arg_ptr = function->TranslateValue(llvm_arg_ptr, ptr_symbol);
	arg_ptr->ValidTypes(ArgTypeVectorRegister, ArgTypeScalarRegister);

	/* If arg_ptr is a scalar register convert it to a vector register */
	if (arg_ptr->getType() == ArgTypeScalarRegister)
	{	
		ArgScalarRegister *arg_scalar = 
				dynamic_cast<ArgScalarRegister *>(arg_ptr);
		
		/* Allocate vector register and create symbol for return value */
		std::string ret_name = llvm_arg_ptr->getName();
		int ret_vreg = function->AllocVReg();
		ptr_symbol->setType(SymbolVectorRegister);
		ptr_symbol->setReg(ret_vreg);

		/* Emit instruction
		 * v_mov_b32 ret_vreg, arg1
		 */

		 auto inst = new Inst(SI::INST_V_MOV_B32,
			 new ArgVectorRegister(ret_vreg), 
			 new ArgScalarRegister(arg_scalar->getId()));
		 AddInst(inst);

		 delete arg_ptr;

		 arg_ptr = new ArgVectorRegister(ret_vreg);
	}


	/* Address must be a symbol with UAV */
	assert(ptr_symbol && "symbol not found");
	assert(ptr_symbol->isAddress() && "no UAV for symbol");
	
	/* Get size of pointed value */
	llvm::Type *llvm_type_ptr = llvm_arg_ptr->getType();
	int ptr_size = GetPointedLlvmTypeSize(llvm_type_ptr);

	/* Get index operand (vreg, literal) */
	llvm::Value *llvm_arg_index = llvm_inst->getOperand(1);
	Arg *arg_index = function->TranslateValue(llvm_arg_index);
	arg_index->ValidTypes(ArgTypeVectorRegister,
			ArgTypeLiteral,
			ArgTypeLiteralReduced);

	/* Allocate vector register and create symbol for return value */
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = new Symbol(ret_name, SymbolVectorRegister, ret_vreg);
	ret_symbol->SetUAVIndex(ptr_symbol->getUAVIndex());
	function->AddSymbol(ret_symbol);

	/* Calculate offset as the multiplication between 'arg_index' and the
	 * size of the pointed element ('ptr_size'). If 'arg_index' is a
	 * literal, we can pre-calculate it here. If 'arg_index' is a vector
	 * register, we need to emit an instruction. */
	Arg *arg_offset;
	if (arg_index->getType() == ArgTypeLiteral ||
			arg_index->getType() == ArgTypeLiteralReduced)
	{
		/* Argument 'arg_offset' is just a modification of
		 * 'arg_index'. */
		auto *arg_offset_literal = dynamic_cast<ArgLiteral *>(arg_index);
		assert(arg_offset_literal);
		arg_offset_literal->setValue(arg_offset_literal->getValue() * ptr_size);
		arg_offset = arg_offset_literal;
	}
	else
	{
		/* Allocate one register and create 'arg_offset' with it */
		int tmp_vreg = function->AllocVReg();
		auto arg_offset_vreg = new ArgVectorRegister(tmp_vreg);
		arg_offset = arg_offset_vreg;

		/* Emit calculation of offset as the multiplication between the
		 * index argument and the pointed element size.
		 * v_mul_i32_i24 tmp_vreg, ptr_size, arg_index
		 */
		auto inst = new Inst(SI::INST_V_MUL_I32_I24,
				new ArgVectorRegister(tmp_vreg),
				new ArgLiteral(ptr_size),
				arg_index);
		AddInst(inst);
	}

	/* Emit effective address calculation as the addition between the
	 * original pointer and the offset.
	 * v_add_i32 ret_vreg, vcc, arg_offset, arg_pointer
	 */
	Inst *inst = new Inst(SI::INST_V_ADD_I32,
			new ArgVectorRegister(ret_vreg),
			new ArgSpecialRegister(SI::InstSpecialRegVcc),
			arg_offset,
			arg_ptr);
	AddInst(inst);
}


void BasicBlock::EmitICmp(llvm::ICmpInst *llvm_inst)
{
	/* Only supported for 2 operands (op1, op2) */
	if (llvm_inst->getNumOperands() != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, llvm_inst->getNumOperands());

	/* Get operands (vreg, literal) */
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	Arg *arg1 = function->TranslateValue(llvm_arg1);
	Arg *arg2 = function->TranslateValue(llvm_arg2);

	/* Only supported for 32-bit integers */
	llvm::Type *llvm_type = llvm_arg1->getType();
	if (!llvm_type->isIntegerTy(32))
		panic("%s:%d: only supported for 32-bit integers",
				__FUNCTION__, __LINE__);

	/* Only the first argument can be a literal. If the second argument is
	 * a literal, flip them and invert comparison predicate later. */
	bool invert = false;
	if (arg2->getType() != ArgTypeVectorRegister)
	{
		std::swap(arg1, arg2);
		invert = true;
	}

	/* Valid argument types. Argument 2 cannot be a literal. */
	arg2 = function->ConstToVReg(this, arg2);
	arg1->ValidTypes(ArgTypeVectorRegister,
			ArgTypeLiteral,
			ArgTypeLiteralReduced,
			ArgTypeLiteralFloat,
			ArgTypeLiteralFloatReduced);
	arg2->ValidTypes(ArgTypeVectorRegister);

	/* Allocate vector register and create symbol for return value */
	std::string ret_name = llvm_inst->getName();
	int ret_sreg_series = function->AllocSReg(2, 2);
	Symbol *ret_symbol = new Symbol(ret_name, SymbolScalarRegister,
			ret_sreg_series, ret_sreg_series + 1);
	function->AddSymbol(ret_symbol);

	/* Choose instruction based on predicate */
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

		panic("%s: predicate not considered (%d)",
				__FUNCTION__, llvm_pred);
	}

	/* Emit comparison.
	 * v_cmp_<pred>_<type> vcc, arg_op1, arg_op2
	 */
	Inst *inst = new Inst(opcode,
			new ArgSpecialRegister(SI::InstSpecialRegVcc),
			arg1,
			arg2);
	AddInst(inst);

	/* Store 'vcc' in scalar register
	 * s_mov_b64 ret_sreg_series, vcc
	 */
	inst = new Inst(SI::INST_S_MOV_B64,
			new ArgScalarRegisterSeries(ret_sreg_series, ret_sreg_series + 1),
			new ArgSpecialRegister(SI::InstSpecialRegVcc));
	AddInst(inst);
}


void BasicBlock::EmitLoad(llvm::LoadInst *llvm_inst)
{
	/* Only supported for 1 operand (address) */
	if (llvm_inst->getNumOperands() != 1)
		fatal("%s: 1 operands supported, %d found",
			__FUNCTION__, llvm_inst->getNumOperands());

	/* Get address operand (vreg) */
	llvm::Value *llvm_arg_addr = llvm_inst->getOperand(0);
	Symbol *addr_symbol;
	Arg *arg_addr = function->TranslateValue(llvm_arg_addr, addr_symbol);
	arg_addr->ValidTypes(ArgTypeVectorRegister);

	/* Address must be a symbol with UAV */
	if (!addr_symbol || !addr_symbol->isAddress())
		fatal("%s: no UAV for symbol", __FUNCTION__);

	/* Get UAV */
	FunctionUAV *uav = function->getUAV(addr_symbol->getUAVIndex());
	if (!uav)
		fatal("%s: invalid UAV index (%d)", __FUNCTION__,
				addr_symbol->getUAVIndex());

	/* Get address space - only 1 (global mem.) supported for now */
	llvm::Type *llvm_type = llvm_arg_addr->getType();
	int addr_space = llvm_type->getPointerAddressSpace();
	if (addr_space != 1)
		fatal("%s: address space 1 expected (%d given)",
			__FUNCTION__, addr_space);

	/* Get return type (data) - only support 4-byte types for now */
	llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32) && !llvm_type->isFloatTy())
		fatal("%s: only 4-byte int/float types supported", __FUNCTION__);

	/* Allocate vector register and create symbol for return value */
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = new Symbol(ret_name, SymbolVectorRegister, ret_vreg);
	function->AddSymbol(ret_symbol);

	/* Emit memory load instruction.
	 * tbuffer_load_format_x v[value_symbol->vreg], v[pointer_symbol->vreg],
	 * 	s[sreg_uav,sreg_uav+3], 0 offen format:[BUF_DATA_FORMAT_32,BUF_NUM_FORMAT_FLOAT]
	 */
	Inst *inst = new Inst(SI::INST_TBUFFER_LOAD_FORMAT_X,
			new ArgVectorRegister(ret_vreg),
			arg_addr,
			new ArgScalarRegisterSeries(uav->GetSReg(), uav->GetSReg() + 3),
			new ArgMaddr(
					new ArgLiteral(0),
					new ArgMaddrQual(true, false, 0),
					SI::InstBufDataFormat32,
					SI::InstBufNumFormatFloat)
	);
	AddInst(inst);
	
	inst = new Inst(SI::INST_S_WAITCNT, new ArgWaitCnt(WaitCntTypeVmCnt));
	AddInst(inst);
}


void BasicBlock::EmitMul(llvm::BinaryOperator *llvm_inst)
{
	/* Only supported for 2 operands (op1, op2) */
	if (llvm_inst->getNumOperands() != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, llvm_inst->getNumOperands());

	/* Only supported for 32-bit integers */
	llvm::Type *type = llvm_inst->getType();
	if (!type->isIntegerTy(32))
		fatal("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Get operands (vreg, literal) */
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	Arg *arg1 = function->TranslateValue(llvm_arg1);
	Arg *arg2 = function->TranslateValue(llvm_arg2);
	
	/* If arg1 is a scalar register convert it to a vector register */
	if (arg1->getType() == ArgTypeScalarRegister)
	{	
		ArgScalarRegister *arg_scalar = 
				dynamic_cast<ArgScalarRegister *>(arg1);
		
		/* Allocate vector register and create symbol for return value */
		std::string ret_name = llvm_arg1->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = new Symbol(ret_name, SymbolVectorRegister, ret_vreg);
		function->AddSymbol(ret_symbol);


		/* Emit instruction
		 * v_mov_b32 ret_vreg, arg1
		 */

		 auto inst = new Inst(SI::INST_V_MOV_B32,
			 new ArgVectorRegister(ret_vreg), 
			 new ArgScalarRegister(arg_scalar->getId()));
		 AddInst(inst);

		 delete arg1;

		 arg1 = new ArgVectorRegister(ret_vreg);
	}
	
	/* If arg2 is a scalar register convert it to a vector register */
	if (arg2->getType() == ArgTypeScalarRegister)
	{	
		ArgScalarRegister *arg_scalar = 
				dynamic_cast<ArgScalarRegister *>(arg2);
		
		/* Allocate vector register and create symbol for return value */
		std::string ret_name = llvm_arg2->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = new Symbol(ret_name, SymbolVectorRegister, ret_vreg);
		function->AddSymbol(ret_symbol);


		/* Emit instruction
		 * v_mov_b32 ret_vreg, arg1
		 */

		 auto inst = new Inst(SI::INST_V_MOV_B32,
			 new ArgVectorRegister(ret_vreg), 
			 new ArgScalarRegister(arg_scalar->getId()));
		 AddInst(inst);

		 delete arg2;

		 arg2 = new ArgVectorRegister(ret_vreg);
	}

	/* Only the first operand can be a constant, so swap them if there is
	 * a constant in the second. */
	if (arg2->getType() != ArgTypeVectorRegister)
		std::swap(arg1, arg2);
	arg1->ValidTypes(ArgTypeVectorRegister,
			ArgTypeLiteral,
			ArgTypeLiteralReduced,
			ArgTypeLiteralFloat,
			ArgTypeLiteralFloatReduced);
	arg2->ValidTypes(ArgTypeVectorRegister);

	/* Allocate vector register and create symbol for return value */
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = new Symbol(ret_name, SymbolVectorRegister, ret_vreg);
	function->AddSymbol(ret_symbol);

	/* Emit effective address calculation.
	 * v_mul_lo_i32 ret_vreg, arg_op1, arg_op2
	 */
	Inst *inst = new Inst(SI::INST_V_MUL_LO_U32,
			new ArgVectorRegister(ret_vreg),
			arg1,
			arg2);
	AddInst(inst);
}


void BasicBlock::EmitPhi(llvm::PHINode *llvm_inst)
{
	/* Only supported for 32-bit integers */
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32))
		fatal("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Allocate vector register and create symbol for return value */
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = new Symbol(ret_name, SymbolVectorRegister, ret_vreg);
	function->AddSymbol(ret_symbol);

	/* Process arguments */
	for (unsigned i = 0; i < llvm_inst->getNumIncomingValues(); i++)
	{
		/* Get item */
		llvm::BasicBlock *llvm_basic_block = llvm_inst->getIncomingBlock(i);
		llvm::Value *value = llvm_inst->getIncomingValue(i);

		/* Find node */
		std::string name = llvm_basic_block->getName();
		Common::Tree *tree = function->getTree();
		Common::LeafNode *node = tree->GetLeafNode(name);
		if (!node)
			panic("%s: cannot find node '%s'",
					__FUNCTION__, name.c_str());

		/* Create destination argument */
		Arg *arg = new ArgVectorRegister(ret_vreg);

		/* Create 'phi' element and add it. */
		Phi *phi = new Phi(node, value, arg);
		function->AddPhi(phi);
	}
}


void BasicBlock::EmitRet(llvm::ReturnInst *llvm_inst)
{
	/* Emit program end instruction.
	 * s_endpgm
	 */
	Inst *inst = new Inst(SI::INST_S_ENDPGM);
	AddInst(inst);
}


void BasicBlock::EmitStore(llvm::StoreInst *llvm_inst)
{
	/* Only supported for 2 operand (address, data) */
	if (llvm_inst->getNumOperands() != 2)
		panic("%s: 2 operands supported, %d found",
			__FUNCTION__, llvm_inst->getNumOperands());

	/* Get data operand (vreg) */
	llvm::Value *llvm_arg_data = llvm_inst->getOperand(0);
	Arg *arg_data = function->TranslateValue(llvm_arg_data);
	arg_data = function->ConstToVReg(this, arg_data);
	arg_data->ValidTypes(ArgTypeVectorRegister, ArgTypeScalarRegister);

	/* If arg_data is a scalar register convert it to a vector register */
	if (arg_data->getType() == ArgTypeScalarRegister)
	{	
		ArgScalarRegister *arg_scalar = 
				dynamic_cast<ArgScalarRegister *>(arg_data);
		
		/* Allocate vector register and create symbol for return value */
		std::string ret_name = llvm_arg_data->getName();
		int ret_vreg = function->AllocVReg();
		Symbol *ret_symbol = new Symbol(ret_name, SymbolVectorRegister, ret_vreg);
		function->AddSymbol(ret_symbol);


		/* Emit instruction
		 * v_mov_b32 ret_vreg, arg1
		 */

		 auto inst = new Inst(SI::INST_V_MOV_B32,
			 new ArgVectorRegister(ret_vreg), 
			 new ArgScalarRegister(arg_scalar->getId()));
		 AddInst(inst);

		 delete arg_data;

		 arg_data = new ArgVectorRegister(ret_vreg);
	}

	/* Get address operand (vreg) */
	llvm::Value *llvm_arg_addr = llvm_inst->getOperand(1);
	Symbol *addr_symbol;
	Arg *arg_addr = function->TranslateValue(llvm_arg_addr, addr_symbol);
	arg_addr->ValidTypes(ArgTypeVectorRegister);

	/* Address must be a symbol with UAV */
	if (!addr_symbol || !addr_symbol->isAddress())
		panic("%s: no UAV for symbol", __FUNCTION__);

	/* Get UAV */
	FunctionUAV *uav = function->getUAV(addr_symbol->getUAVIndex());
	if (!uav)
		panic("%s: invalid UAV index (%d)", __FUNCTION__,
				addr_symbol->getUAVIndex());

	/* Get address space - only 1 (global mem.) supported for now */
	llvm::Type *llvm_type = llvm_arg_addr->getType();
	int addr_space = llvm_type->getPointerAddressSpace();
	if (addr_space != 1)
		panic("%s: only address space 1 supported (%d given)",
			__FUNCTION__, addr_space);

	/* Get type of data - only support 4-byte types for now */
	llvm_type = llvm_arg_data->getType();
	if (!llvm_type->isIntegerTy(32) && !llvm_type->isFloatTy())
		panic("%s: only 4-byte int/float types supported", __FUNCTION__);

	/* Emit memory write.
	 * tbuffer_store_format_x v[value_symbol->vreg], s[pointer_symbol->vreg],
	 * 	s[sreg_uav,sreg_uav+3], 0 offen format:[BUF_DATA_FORMAT_32,
	 * 	BUF_NUM_FORMAT_FLOAT]
	 */

	Inst *inst = new Inst(SI::INST_TBUFFER_STORE_FORMAT_X,
			arg_data,
			arg_addr,
			new ArgScalarRegisterSeries(uav->GetSReg(), uav->GetSReg() + 3),
			new ArgMaddr(
					new ArgLiteral(0),
					new ArgMaddrQual(true, false, 0),
					SI::InstBufDataFormat32,
					SI::InstBufNumFormatFloat)
	);
	AddInst(inst);

	inst = new Inst(SI::INST_S_WAITCNT, new ArgWaitCnt(WaitCntTypeExpCnt));
	AddInst(inst);
}


void BasicBlock::EmitSub(llvm::BinaryOperator *llvm_inst)
{
	/* Only supported for 32-bit integers */
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isIntegerTy(32))
		fatal("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Only supported for 2 operands (op1, op2) */
	if (llvm_inst->getNumOperands() != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, llvm_inst->getNumOperands());

	/* Get operands (vreg, literal) */
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	Arg *arg1 = function->TranslateValue(llvm_arg1);
	Arg *arg2 = function->TranslateValue(llvm_arg2);

	/* Operand 2 cannot be a constant */
	arg2 = function->ConstToVReg(this, arg2);
	arg1->ValidTypes(ArgTypeVectorRegister,
			ArgTypeLiteral,
			ArgTypeLiteralReduced,
			ArgTypeLiteralFloat,
			ArgTypeLiteralFloatReduced);
	arg2->ValidTypes(ArgTypeVectorRegister);

	/* Allocate vector register and create symbol for return value */
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = new Symbol(ret_name, SymbolVectorRegister, ret_vreg);
	function->AddSymbol(ret_symbol);

	/* Emit subtraction.
	 * v_sub_i32 ret_vreg, vcc, arg_op1, arg_op2
	 */
	Inst *inst = new Inst(SI::INST_V_SUB_I32,
			new ArgVectorRegister(ret_vreg),
			new ArgSpecialRegister(SI::InstSpecialRegVcc),
			arg1,
			arg2);
	AddInst(inst);
}


void BasicBlock::EmitFAdd(llvm::BinaryOperator *llvm_inst)
{
	/* Only supported for floats */
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isFloatTy())
		panic("%s: only supported for float type arguments",
				__FUNCTION__);

	/* Only supported for 2 operands (op1, op2) */
	if (llvm_inst->getNumOperands() != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, llvm_inst->getNumOperands());

	/* Get operands (vreg, literal) */
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	Arg *arg1 = function->TranslateValue(llvm_arg1);
	Arg *arg2 = function->TranslateValue(llvm_arg2);

	/* Second operand cannot be a constant */
	arg2 = function->ConstToVReg(this, arg2);
	arg1->ValidTypes(ArgTypeVectorRegister,
			ArgTypeLiteral,
			ArgTypeLiteralReduced,
			ArgTypeLiteralFloat,
			ArgTypeLiteralFloatReduced);
	arg2->ValidTypes(ArgTypeVectorRegister);

	/* Allocate vector register and create symbol for return value */
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = new Symbol(ret_name, SymbolVectorRegister, ret_vreg);
	function->AddSymbol(ret_symbol);

	/* Emit addition.
	 * v_add_f32 ret_vreg, arg_op1, arg_op2
	 */
	Inst *inst = new Inst(SI::INST_V_ADD_F32,
			new ArgVectorRegister(ret_vreg),
			arg1,
			arg2);
	AddInst(inst);
}


void BasicBlock::EmitFSub(llvm::BinaryOperator *llvm_inst)
{
	/* Only supported for floats */
	llvm::Type *llvm_type = llvm_inst->getType();
	if (!llvm_type->isFloatTy())
		panic("%s: only supported for float type arguments",
				__FUNCTION__);

	/* Only supported for 2 operands (op1, op2) */
	if (llvm_inst->getNumOperands() != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, llvm_inst->getNumOperands());

	/* Get operands (vreg, literal) */
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	Arg *arg1 = function->TranslateValue(llvm_arg1);
	Arg *arg2 = function->TranslateValue(llvm_arg2);

	/* Second operand cannot be a constant */
	arg2 = function->ConstToVReg(this, arg2);
	arg1->ValidTypes(ArgTypeVectorRegister,
			ArgTypeLiteral,
			ArgTypeLiteralReduced,
			ArgTypeLiteralFloat,
			ArgTypeLiteralFloatReduced);
	arg2->ValidTypes(ArgTypeVectorRegister);

	/* Allocate vector register and create symbol for return value */
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = new Symbol(ret_name, SymbolVectorRegister, ret_vreg);
	function->AddSymbol(ret_symbol);

	/* Emit addition.
	 * v_sub_f32 ret_vreg, arg_op1, arg_op2
	 */
	Inst *inst = new Inst(SI::INST_V_SUB_F32,
			new ArgVectorRegister(ret_vreg),
			arg1,
			arg2);
	AddInst(inst);
}


void BasicBlock::EmitFMul(llvm::BinaryOperator *llvm_inst)
{
	/* Only supported for 2 operands (op1, op2) */
	if (llvm_inst->getNumOperands() != 2)
		panic("%s: 2 operands supported, %d found",
			__FUNCTION__, llvm_inst->getNumOperands());

	/* Only supported for 32-bit integers */
	llvm::Type *type = llvm_inst->getType();
	if (!type->isFloatTy())
		panic("%s: only supported for float type arguments",
				__FUNCTION__);

	/* Get operands (vreg, literal) */
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	Arg *arg1 = function->TranslateValue(llvm_arg1);
	Arg *arg2 = function->TranslateValue(llvm_arg2);

	/* Only the first operand can be a constant, so swap them if there is
	 * a constant in the second. */
	if (arg2->getType() != ArgTypeVectorRegister)
		std::swap(arg1, arg2);
	arg1->ValidTypes(ArgTypeVectorRegister,
			ArgTypeLiteral,
			ArgTypeLiteralReduced,
			ArgTypeLiteralFloat,
			ArgTypeLiteralFloatReduced);
	arg2->ValidTypes(ArgTypeVectorRegister);

	/* Allocate vector register and create symbol for return value */
	std::string ret_name = llvm_inst->getName();
	int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = new Symbol(ret_name, SymbolVectorRegister, ret_vreg);
	function->AddSymbol(ret_symbol);

	/* Emit effective address calculation.
	 * v_mul_f32 ret_vreg, arg_op1, arg_op2
	 */
	Inst *inst = new Inst(SI::INST_V_MUL_F32,
			new ArgVectorRegister(ret_vreg),
			arg1,
			arg2);
	AddInst(inst);
}


void BasicBlock::EmitExtractElement(llvm::ExtractElementInst *llvm_inst)
{
	/* Only supported for 2 operands (op1, op2) */
	if (llvm_inst->getNumOperands() != 2)
		panic("%s: 2 operands supported, %d found",
			__FUNCTION__, llvm_inst->getNumOperands());

	/* Get operands (vreg, literal) */
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	Arg *arg1 = function->TranslateValue(llvm_arg1);
	Arg *arg2 = function->TranslateValue(llvm_arg2);

	/* First argument must be a scalar register 
	 * and the second must be the offset */
	arg1->ValidTypes(ArgTypeScalarRegister);
	arg2->ValidTypes(ArgTypeLiteral,
			ArgTypeLiteralReduced);

	/* Obtain actual scalar register */
	ArgScalarRegister *arg1_scalar = dynamic_cast<ArgScalarRegister *>(arg1);
	ArgLiteral *arg2_literal = dynamic_cast<ArgLiteral *>(arg2);
	assert(arg1_scalar);
	assert(arg2_literal);
	arg1_scalar->setId(arg1_scalar->getId() + arg2_literal->getValue());
	delete arg2;

	/* Allocate vector register and create symbol for return value */
	std::string ret_name = llvm_inst->getName();
	//int ret_vreg = function->AllocVReg();
	Symbol *ret_symbol = new Symbol(ret_name, SymbolScalarRegister, 
			arg1_scalar->getId());
	function->AddSymbol(ret_symbol);

	
	/* Emit instruction
	 * v_mov_b32 ret_vreg, arg1
	 
	Inst *inst = new Inst(SI::INST_V_MOV_B32,
			new ArgVectorRegister(ret_vreg),
			arg1);
:q
AddInst(inst);*/
}

void BasicBlock::EmitInsertElement(llvm::InsertElementInst *llvm_inst)
{
	/* Only supported for 2 operands (op1, op2) */
	if (llvm_inst->getNumOperands() != 3)
		panic("%s: 3 operands supported, %d found",
			__FUNCTION__, llvm_inst->getNumOperands());

	/* Get operands (vreg, literal) */
	llvm::Value *llvm_arg1 = llvm_inst->getOperand(0);
	llvm::Value *llvm_arg2 = llvm_inst->getOperand(1);
	llvm::Value *llvm_arg3 = llvm_inst->getOperand(2);
	Arg *arg1 = function->TranslateValue(llvm_arg1);
	Arg *arg2 = function->TranslateValue(llvm_arg2);
	Arg *arg3 = function->TranslateValue(llvm_arg3);

	/* First argument must be a scalar register 
	 * and the second must be the offset */
	arg1->ValidTypes(ArgTypeScalarRegister, ArgTypeVectorRegister);
	arg2->ValidTypes(ArgTypeScalarRegister, ArgTypeVectorRegister);
	arg3->ValidTypes(ArgTypeLiteral,
			ArgTypeLiteralReduced);

	/* Rule out case where dest is vgpr and src is sgpr */
	assert((arg1->getType() != ArgTypeScalarRegister) && 
			(arg2->getType() != ArgTypeVectorRegister));

	ArgLiteral *arg3_literal = dynamic_cast<ArgLiteral *>(arg3);
	assert(arg3_literal);

	/* Allocate vector register and create symbol for return value */
	std::string ret_name = llvm_arg2->getName();
	
	if (arg1->getType() == ArgTypeScalarRegister && 
			arg2->getType() == ArgTypeScalarRegister)
	{
		ArgScalarRegister *arg1_scalar = dynamic_cast<ArgScalarRegister *>(arg1);
		arg1_scalar->setId(arg1_scalar->getId() + arg3_literal->getValue());
		
		Symbol *ret_symbol = new Symbol(ret_name, SymbolScalarRegister, 
			arg1_scalar->getId());
		function->AddSymbol(ret_symbol);
		
		/* Emit instruction
		 * s_mov_b32 arg1, arg2
		 */

		 Inst *inst = new Inst(SI::INST_S_MOV_B32, arg1, arg2);
		 AddInst(inst);
	}
	else
	{
		ArgVectorRegister *arg1_vector = dynamic_cast<ArgVectorRegister *>(arg1);
		arg1_vector->setId(arg1_vector->getId() + arg3_literal->getValue());
		
		Symbol *ret_symbol = new Symbol(ret_name, SymbolVectorRegister, 
			arg1_vector->getId());
		function->AddSymbol(ret_symbol);
		
		/* Emit instruction
		 * v_mov_b32 arg2, arg1
		 */

		 Inst *inst = new Inst(SI::INST_V_MOV_B32, arg2, arg1);
		 AddInst(inst);

	}
	
	delete arg3;
	
	
	}


void BasicBlock::Dump(std::ostream &os)
{
	/* Nothing if basic block is empty */
	if (!inst_list.size())
		return;

	/* Label with node's name */
	Common::LeafNode *node = GetNode();
	os << "\n" << node->getName() << ":\n";

	/* Print list of instructions */
	for (auto &inst : inst_list)
		inst->Dump(os);
}


void BasicBlock::AddInst(Inst *inst)
{
	/* Check that the instruction does not belong to any other basic
	 * block already. */
	if (inst->GetBasicBlock())
		panic("%s: instruction already added to basic block",
				__FUNCTION__);

	/* Add instruction */
	inst_list.emplace_back(inst);
	inst->SetBasicBlock(this);

	/* If there was a comment added to the basic block, attach it to
	 * the instruction being added now. */
	if (!comment.empty())
	{
		inst->SetComment(comment);
		comment.clear();
	}
}


void BasicBlock::Emit(llvm::BasicBlock *llvm_basic_block)
{
	/* Iterate over LLVM instructions */
	assert(llvm_basic_block);
	for (auto &llvm_inst : llvm_basic_block->getInstList())
	{
		switch (llvm_inst.getOpcode())
		{

		case llvm::Instruction::Add:

			EmitAdd(misc::cast<llvm::BinaryOperator *>(&llvm_inst));
			break;

		case llvm::Instruction::Br:

			/* Ignore branches here */
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

		case llvm::Instruction::ExtractElement:
			
			EmitExtractElement(misc::cast<llvm::ExtractElementInst *>
					(&llvm_inst));
			break;
		
		case llvm::Instruction::InsertElement:
			
			EmitInsertElement(misc::cast<llvm::InsertElementInst *>
					(&llvm_inst));
			break;
		
		default:

			fatal("%s: LLVM opcode not supported (%d)",
					__FUNCTION__, llvm_inst.getOpcode());
		}
	}
}

void BasicBlock::LiveRegisterAnalysis()
{
	/* Iterate through list of instructions to populate bitmaps*/
	for (auto &inst : this->getInstList())
	{
		this->def = new Bitmap(this->function->num_vregs);
		this->use = new Bitmap(this->function->num_vregs);

		/* Get each argument in the line */
		for (auto &arg : inst->getArgs())
		{
			/* Currently only deals with scalar registers */
			if (arg->getType() == si2bin::ArgTypeVectorRegister) {

				si2bin::ArgVectorRegister *argReg = dynamic_cast<si2bin::ArgVectorRegister *>(arg.get());
				if (!argReg)
					continue;

				if (arg->getToken()->GetDirection() == si2bin::TokenDirectionDst)
				{
					this->def->Set(argReg->getId(), true);
				}
				else if (arg->getToken()->GetDirection() == si2bin::TokenDirectionSrc)
				{
					/* If register wasn't defined in the same basic block */
					if (this->def->Test(argReg->getId()) != true)
					{
						this->use->Set(argReg->getId(), true);
					}
				}
			}
		}
	}

	assert(out != NULL);

	si2bin::Inst *prev_inst = nullptr;
	for (auto it = inst_list.rbegin(), e = inst_list.rend(); it != e; ++it)
	{
		std::unique_ptr<Inst> &inst = *it;

		/* Sets out bitmap of instruction */
		if (it == inst_list.rbegin())
		{
			inst->out = new Bitmap(*out);
		}
		else
		{
			inst->out = new Bitmap(*prev_inst->in);
		}

		/* Clones out into in so that it can be used to perform calculations */
		inst->in = new Bitmap(*inst->out);
		*inst->in -= *inst->def;
		*inst->in |= *inst->use;

		// Iterate
		prev_inst = inst.get();
	}

	/* Makes sure first instruction's in bitmap matches basic blocks in bitmap */
	//assert(!BitmapCompare(prev_inst->in, basic_block->in));
}


}  /* namespace llvm2si */

