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
#include <llvm/Type.h>

#include "BasicBlock.h"
#include "Function.h"


using namespace Misc;
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
	if (llvm_type->isIntegerTy(32))
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
	/* Check that there is only one argument. LLVMGetNumOperands returns the
	 * number of arguments plus one (the function being called as a last
	 * argument. */
	if (llvm_inst->getNumOperands() != 1)
		panic("%s: 1 argument expected, %d found",
				__FUNCTION__, llvm_inst->getNumOperands());

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
	if (llvm_function->getNumOperands() != 1)
		fatal("%s: 1 argument expected for '%s', %d found",
			__FUNCTION__, func_name.c_str(),
			llvm_function->getNumOperands());

	/* Get argument and check type */
	llvm::Value *llvm_arg = llvm_inst->getOperand(0);
	llvm::Type *llvm_type = llvm_arg->getType();
	if (!llvm_type->isIntegerTy() || !llvm::isa<llvm::ConstantInt>(llvm_arg))
		fatal("%s: argument should be an integer constant",
				__FUNCTION__);

	/* Get argument value and check bounds */
	llvm::ConstantInt *llvm_const = llvm::cast<llvm::ConstantInt>(llvm_arg);
	int dim = llvm_const->getZExtValue();
	if (!InRange(dim, 0, 2))
		fatal("%s: constant in range [0..2] expected",
				__FUNCTION__);

	/* Built-in functions */
	if (func_name == "__get_global_id_u32")
	{
		/* Create new symbol associating it with the vector register
		 * containing the global ID in the given dimension. */
		Symbol *ret_symbol = new Symbol(var_name, SymbolVectorRegister,
				function->GetVRegGid() + dim);
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
				new ArgScalarRegister(function->GetSRegGSize() + dim));
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
	arg_ptr->ValidTypes(ArgTypeVectorRegister);

	/* Address must be a symbol with UAV */
	if (!ptr_symbol || !ptr_symbol->IsAddress());
		fatal("%s: no UAV for symbol", __FUNCTION__);
	
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
	ret_symbol->SetUAVIndex(ptr_symbol->GetUAVIndex());
	function->AddSymbol(ret_symbol);

	/* Calculate offset as the multiplication between 'arg_index' and the
	 * size of the pointed element ('ptr_size'). If 'arg_index' is a
	 * literal, we can pre-calculate it here. If 'arg_index' is a vector
	 * register, we need to emit an instruction. */
	Arg *arg_offset;
	if (arg_index->GetType() == ArgTypeLiteral ||
			arg_index->GetType() == ArgTypeLiteralReduced)
	{
		/* Argument 'arg_offset' is just a modification of
		 * 'arg_index'. */
		auto *arg_offset_literal = dynamic_cast<ArgLiteral *>(arg_index);
		assert(arg_offset_literal);
		arg_offset_literal->SetValue(arg_offset_literal->GetValue() * ptr_size);
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
		fatal("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Only the first argument can be a literal. If the second argument is
	 * a literal, flip them and invert comparison predicate later. */
	bool invert = false;
	if (arg2->GetType() != ArgTypeVectorRegister)
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


#if 0
static void Llvm2siBasicBlockEmitLoad(Llvm2siBasicBlock *self,
		LLVMValueRef llvm_inst)
{
	LLVMValueRef llvm_arg_address;
	LLVMTypeRef llvm_type;
	LLVMTypeKind llvm_type_kind;

	Llvm2siFunction *function;
	Llvm2siFunctionUAV *uav;
	Llvm2siSymbol *addr_symbol;
	Llvm2siSymbol *ret_symbol;
	Si2binInst *inst;
	Si2binArg *arg_addr;
	Si2binArg *arg_qual;
	Si2binArg *arg_soffset;
	List *arg_list;

	int num_operands;
	int addr_space;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 1 operand (address) */
	num_operands = LLVMGetNumOperands(llvm_inst);
	if (num_operands != 1)
		fatal("%s: 1 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get address operand (vreg) */
	llvm_arg_address = LLVMGetOperand(llvm_inst, 0);
	arg_addr = Llvm2siFunctionTranslateValue(function, llvm_arg_address, &addr_symbol);
	Si2binArgValidTypes(arg_addr, Si2binArgVectorRegister);

	/* Address must be a symbol with UAV */
	if (!addr_symbol || !addr_symbol->address)
		fatal("%s: no UAV for symbol", __FUNCTION__);

	/* Get UAV */
	uav = asLlvm2siFunctionUAV(ArrayGet(function->uav_list,
			addr_symbol->uav_index));
	if (!uav)
		fatal("%s: invalid UAV index (%d)", __FUNCTION__,
				addr_symbol->uav_index);

	/* Get address space - only 1 (global mem.) supported for now */
	llvm_type = LLVMTypeOf(llvm_arg_address);
	addr_space = LLVMGetPointerAddressSpace(llvm_type);
	if (addr_space != 1)
		fatal("%s: address space 1 expected (%d given)",
			__FUNCTION__, addr_space);

	/* Get return type (data) - only support 4-byte types for now */
	llvm_type = LLVMTypeOf(llvm_inst);
	llvm_type_kind = LLVMGetTypeKind(llvm_type);
	if (!(llvm_type_kind == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(llvm_type) == 32) &&
			llvm_type_kind != LLVMFloatTypeKind)
		fatal("%s: only 4-byte int/float types supported", __FUNCTION__);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llvm_inst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Emit memory load instruction.
	 * tbuffer_load_format_x v[value_symbol->vreg], v[pointer_symbol->vreg],
	 * 	s[sreg_uav,sreg_uav+3], 0 offen format:[BUF_DATA_FORMAT_32,BUF_NUM_FORMAT_FLOAT]
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateVectorRegister, ret_vreg)));
	ListAdd(arg_list, asObject(arg_addr));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateScalarRegisterSeries, uav->sreg,
			uav->sreg + 3)));
	arg_soffset = new_ctor(Si2binArg, CreateLiteral, 0);
	arg_qual = new_ctor(Si2binArg, CreateMaddrQual);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateMaddr, arg_soffset, arg_qual,
			SIInstBufDataFormat32, SIInstBufNumFormatFloat)));
	inst = new(Si2binInst, SI::INST_TBUFFER_LOAD_FORMAT_X, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}


static void Llvm2siBasicBlockEmitMul(Llvm2siBasicBlock *self,
		LLVMValueRef llvm_inst)
{
	LLVMValueRef llvm_arg_op1;
	LLVMValueRef llvm_arg_op2;
	LLVMTypeRef llvm_type;
	LLVMTypeKind llvm_type_kind;

	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	Si2binArg *arg_op1;
	Si2binArg *arg_op2;
	Si2binInst *inst;
	List *arg_list;

	int num_operands;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 2 operands (op1, op2) */
	num_operands = LLVMGetNumOperands(llvm_inst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Only supported for 32-bit integers */
	llvm_type = LLVMTypeOf(llvm_inst);
	llvm_type_kind = LLVMGetTypeKind(llvm_type);
	if (llvm_type_kind != LLVMIntegerTypeKind ||
			LLVMGetIntTypeWidth(llvm_type) != 32)
		fatal("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Get operands (vreg, literal) */
	llvm_arg_op1 = LLVMGetOperand(llvm_inst, 0);
	llvm_arg_op2 = LLVMGetOperand(llvm_inst, 1);
	arg_op1 = Llvm2siFunctionTranslateValue(function, llvm_arg_op1, NULL);
	arg_op2 = Llvm2siFunctionTranslateValue(function, llvm_arg_op2, NULL);

	/* Only the first operand can be a constant, so swap them if there is
	 * a constant in the second. */
	if (arg_op2->type != Si2binArgVectorRegister)
		Si2binArgSwap(&arg_op1, &arg_op2);
	Si2binArgValidTypes(arg_op1, Si2binArgVectorRegister,
			Si2binArgLiteral, Si2binArgLiteralReduced,
			Si2binArgLiteralFloat, Si2binArgLiteralFloatReduced);
	Si2binArgValidTypes(arg_op2, Si2binArgVectorRegister);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llvm_inst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Emit effective address calculation.
	 * v_mul_lo_i32 ret_vreg, arg_op1, arg_op2
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateVectorRegister, ret_vreg)));
	ListAdd(arg_list, asObject(arg_op1));
	ListAdd(arg_list, asObject(arg_op2));
	inst = new(Si2binInst, SI::INST_V_MUL_LO_U32, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}


static void Llvm2siBasicBlockEmitPhi(Llvm2siBasicBlock *self,
		LLVMValueRef llvm_inst)
{
	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	Node *node;
	Si2binArg *arg;
	Llvm2siPhi *phi;

	LLVMTypeRef llvm_type;
	LLVMTypeKind llvm_type_kind;
	LLVMValueRef llvalue;
	LLVMValueRef llbb_value;
	LLVMBasicBlockRef llvm_basic_block;

	char *ret_name;
	char *llbb_name;

	int ret_vreg;
	int num_operands;
	int i;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 32-bit integers */
	llvm_type = LLVMTypeOf(llvm_inst);
	llvm_type_kind = LLVMGetTypeKind(llvm_type);
	if (llvm_type_kind != LLVMIntegerTypeKind ||
			LLVMGetIntTypeWidth(llvm_type) != 32)
		fatal("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llvm_inst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Process arguments */
	num_operands = LLVMCountIncoming(llvm_inst);
	for (i = 0; i < num_operands; i++)
	{
		/* Get source basic block */
		llvm_basic_block = LLVMGetIncomingBlock(llvm_inst, i);
		llbb_value = LLVMBasicBlockAsValue(llvm_basic_block);
		llbb_name = (char *) LLVMGetValueName(llbb_value);
		node = CTreeGetNode(function->ctree, llbb_name);
		if (!node)
			panic("%s: cannot find node '%s'",
					__FUNCTION__, llbb_name);

		/* Get source value */
		llvalue = LLVMGetIncomingValue(llvm_inst, i);

		/* Create destination argument */
		arg = new_ctor(Si2binArg, CreateVectorRegister, ret_vreg);

		/* Create 'phi' element and add it. */
		phi = new(Llvm2siPhi, node, llvalue, arg);
		ListAdd(function->phi_list, asObject(phi));
	}
}


static void Llvm2siBasicBlockEmitRet(Llvm2siBasicBlock *self,
		LLVMValueRef llvm_inst)
{
	List *arg_list;
	Si2binInst *inst;

	/* Emit program end instruction.
	 * s_endpgm
	 */
	arg_list = new(List);
	inst = new(Si2binInst, SI::INST_S_ENDPGM, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}


static void Llvm2siBasicBlockEmitStore(Llvm2siBasicBlock *self,
		LLVMValueRef llvm_inst)
{
	LLVMValueRef llvm_arg_data;
	LLVMValueRef llvm_arg_addr;
	LLVMTypeRef llvm_type;
	LLVMTypeKind llvm_type_kind;

	Llvm2siFunction *function;
	Llvm2siFunctionUAV *uav;
	Llvm2siSymbol *addr_symbol;
	Si2binInst *inst;
	Si2binArg *arg_data;
	Si2binArg *arg_addr;
	Si2binArg *arg_qual;
	Si2binArg *arg_soffset;
	List *arg_list;

	int num_operands;
	int addr_space;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 2 operand (address, data) */
	num_operands = LLVMGetNumOperands(llvm_inst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get data operand (vreg) */
	llvm_arg_data = LLVMGetOperand(llvm_inst, 0);
	arg_data = Llvm2siFunctionTranslateValue(function, llvm_arg_data, NULL);
	arg_data = Llvm2siFunctionConstToVReg(function, self, arg_data);
	Si2binArgValidTypes(arg_data, Si2binArgVectorRegister);

	/* Get address operand (vreg) */
	llvm_arg_addr = LLVMGetOperand(llvm_inst, 1);
	arg_addr = Llvm2siFunctionTranslateValue(function, llvm_arg_addr, &addr_symbol);
	Si2binArgValidTypes(arg_addr, Si2binArgVectorRegister);

	/* Address must be a symbol with UAV */
	if (!addr_symbol || !addr_symbol->address)
		fatal("%s: no UAV for symbol", __FUNCTION__);

	/* Get UAV */
	uav = asLlvm2siFunctionUAV(ArrayGet(function->uav_list,
			addr_symbol->uav_index));
	if (!uav)
		fatal("%s: invalid UAV index (%d)", __FUNCTION__,
				addr_symbol->uav_index);

	/* Get address space - only 1 (global mem.) supported for now */
	llvm_type = LLVMTypeOf(llvm_arg_addr);
	addr_space = LLVMGetPointerAddressSpace(llvm_type);
	if (addr_space != 1)
		fatal("%s: address space 1 expected (%d given)",
			__FUNCTION__, addr_space);

	/* Get type of data - only support 4-byte types for now */
	llvm_type = LLVMTypeOf(llvm_arg_data);
	llvm_type_kind = LLVMGetTypeKind(llvm_type);
	if (!(llvm_type_kind == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(llvm_type) == 32) &&
			llvm_type_kind != LLVMFloatTypeKind)
		fatal("%s: only 4-byte int/float types supported", __FUNCTION__);

	/* Emit memory write.
	 * tbuffer_store_format_x v[value_symbol->vreg], s[pointer_symbol->vreg],
	 * 	s[sreg_uav,sreg_uav+3], 0 offen format:[BUF_DATA_FORMAT_32,
	 * 	BUF_NUM_FORMAT_FLOAT]
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(arg_data));
	ListAdd(arg_list, asObject(arg_addr));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateScalarRegisterSeries, uav->sreg,
			uav->sreg + 3)));
	arg_soffset = new_ctor(Si2binArg, CreateLiteral, 0);
	arg_qual = new_ctor(Si2binArg, CreateMaddrQual);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateMaddr, arg_soffset, arg_qual,
			SIInstBufDataFormat32, SIInstBufNumFormatFloat)));
	inst = new(Si2binInst, SI::INST_TBUFFER_STORE_FORMAT_X, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}


static void Llvm2siBasicBlockEmitSub(Llvm2siBasicBlock *self,
		LLVMValueRef llvm_inst)
{
	LLVMValueRef llvm_arg_op1;
	LLVMValueRef llvm_arg_op2;
	LLVMTypeRef llvm_type;
	LLVMTypeKind llvm_type_kind;

	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	Si2binArg *arg_op1;
	Si2binArg *arg_op2;
	Si2binInst *inst;
	List *arg_list;

	int num_operands;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 32-bit integers */
	llvm_type = LLVMTypeOf(llvm_inst);
	llvm_type_kind = LLVMGetTypeKind(llvm_type);
	if (llvm_type_kind != LLVMIntegerTypeKind ||
			LLVMGetIntTypeWidth(llvm_type) != 32)
		fatal("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Only supported for 2 operands (op1, op2) */
	num_operands = LLVMGetNumOperands(llvm_inst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get operands (vreg, literal) */
	llvm_arg_op1 = LLVMGetOperand(llvm_inst, 0);
	llvm_arg_op2 = LLVMGetOperand(llvm_inst, 1);
	arg_op1 = Llvm2siFunctionTranslateValue(function, llvm_arg_op1, NULL);
	arg_op2 = Llvm2siFunctionTranslateValue(function, llvm_arg_op2, NULL);

	/* Operand 2 cannot be a constant */
	arg_op2 = Llvm2siFunctionConstToVReg(function, self, arg_op2);
	Si2binArgValidTypes(arg_op1, Si2binArgVectorRegister,
			Si2binArgLiteral, Si2binArgLiteralReduced,
			Si2binArgLiteralFloat, Si2binArgLiteralFloatReduced);
	Si2binArgValidTypes(arg_op2, Si2binArgVectorRegister);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llvm_inst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Emit subtraction.
	 * v_sub_i32 ret_vreg, vcc, arg_op1, arg_op2
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateVectorRegister, ret_vreg)));
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateSpecialRegister, SIInstSpecialRegVcc)));
	ListAdd(arg_list, asObject(arg_op1));
	ListAdd(arg_list, asObject(arg_op2));
	inst = new(Si2binInst, SI::INST_V_SUB_I32, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}

static void Llvm2siBasicBlockEmitFAdd(Llvm2siBasicBlock *self,
		LLVMValueRef llvm_inst)
{
	LLVMValueRef llvm_arg_op1;
	LLVMValueRef llvm_arg_op2;
	LLVMTypeRef llvm_type;
	LLVMTypeKind llvm_type_kind;

	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	Si2binArg *arg_op1;
	Si2binArg *arg_op2;
	Si2binInst *inst;
	List *arg_list;

	int num_operands;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 32-bit integers */
	llvm_type = LLVMTypeOf(llvm_inst);
	llvm_type_kind = LLVMGetTypeKind(llvm_type);
	if (llvm_type_kind != LLVMFloatTypeKind)
		fatal("%s: only supported for float type arguments",
				__FUNCTION__);

	/* Only supported for 2 operands (op1, op2) */
	num_operands = LLVMGetNumOperands(llvm_inst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get operands (vreg, literal) */
	llvm_arg_op1 = LLVMGetOperand(llvm_inst, 0);
	llvm_arg_op2 = LLVMGetOperand(llvm_inst, 1);
	arg_op1 = Llvm2siFunctionTranslateValue(function, llvm_arg_op1, NULL);
	arg_op2 = Llvm2siFunctionTranslateValue(function, llvm_arg_op2, NULL);

	/* Second operand cannot be a constant */
	arg_op2 = Llvm2siFunctionConstToVReg(function, self, arg_op2);
	Si2binArgValidTypes(arg_op1, Si2binArgVectorRegister,
			Si2binArgLiteral, Si2binArgLiteralReduced,
			Si2binArgLiteralFloat, Si2binArgLiteralFloatReduced);
	Si2binArgValidTypes(arg_op2, Si2binArgVectorRegister);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llvm_inst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Emit addition.
	 * v_add_f32 ret_vreg, arg_op1, arg_op2
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateVectorRegister, ret_vreg)));
	ListAdd(arg_list, asObject(arg_op1));
	ListAdd(arg_list, asObject(arg_op2));
	inst = new(Si2binInst, SI::INST_V_ADD_F32, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}

static void Llvm2siBasicBlockEmitFSub(Llvm2siBasicBlock *self,
		LLVMValueRef llvm_inst)
{
	LLVMValueRef llvm_arg_op1;
	LLVMValueRef llvm_arg_op2;
	LLVMTypeRef llvm_type;
	LLVMTypeKind llvm_type_kind;

	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	Si2binArg *arg_op1;
	Si2binArg *arg_op2;
	Si2binInst *inst;
	List *arg_list;

	int num_operands;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 32-bit integers */
	llvm_type = LLVMTypeOf(llvm_inst);
	llvm_type_kind = LLVMGetTypeKind(llvm_type);
	if (llvm_type_kind != LLVMFloatTypeKind)
		fatal("%s: only supported for float type arguments",
				__FUNCTION__);

	/* Only supported for 2 operands (op1, op2) */
	num_operands = LLVMGetNumOperands(llvm_inst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get operands (vreg, literal) */
	llvm_arg_op1 = LLVMGetOperand(llvm_inst, 0);
	llvm_arg_op2 = LLVMGetOperand(llvm_inst, 1);
	arg_op1 = Llvm2siFunctionTranslateValue(function, llvm_arg_op1, NULL);
	arg_op2 = Llvm2siFunctionTranslateValue(function, llvm_arg_op2, NULL);

	/* Second operand cannot be a constant */
	arg_op2 = Llvm2siFunctionConstToVReg(function, self, arg_op2);
	Si2binArgValidTypes(arg_op1, Si2binArgVectorRegister,
			Si2binArgLiteral, Si2binArgLiteralReduced,
			Si2binArgLiteralFloat, Si2binArgLiteralFloatReduced);
	Si2binArgValidTypes(arg_op2, Si2binArgVectorRegister);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llvm_inst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Emit addition.
	 * v_sub_f32 ret_vreg, arg_op1, arg_op2
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateVectorRegister, ret_vreg)));
	ListAdd(arg_list, asObject(arg_op1));
	ListAdd(arg_list, asObject(arg_op2));
	inst = new(Si2binInst, SI::INST_V_SUB_F32, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}

static void Llvm2siBasicBlockEmitFMul(Llvm2siBasicBlock *self,
		LLVMValueRef llvm_inst)
{
	LLVMValueRef llvm_arg_op1;
	LLVMValueRef llvm_arg_op2;
	LLVMTypeRef llvm_type;
	LLVMTypeKind llvm_type_kind;

	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	Si2binArg *arg_op1;
	Si2binArg *arg_op2;
	Si2binInst *inst;
	List *arg_list;

	int num_operands;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 2 operands (op1, op2) */
	num_operands = LLVMGetNumOperands(llvm_inst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Only supported for 32-bit integers */
	llvm_type = LLVMTypeOf(llvm_inst);
	llvm_type_kind = LLVMGetTypeKind(llvm_type);
	if (llvm_type_kind != LLVMFloatTypeKind)
		fatal("%s: only supported for float type arguments",
				__FUNCTION__);

	/* Get operands (vreg, literal) */
	llvm_arg_op1 = LLVMGetOperand(llvm_inst, 0);
	llvm_arg_op2 = LLVMGetOperand(llvm_inst, 1);
	arg_op1 = Llvm2siFunctionTranslateValue(function, llvm_arg_op1, NULL);
	arg_op2 = Llvm2siFunctionTranslateValue(function, llvm_arg_op2, NULL);

	/* Only the first operand can be a constant, so swap them if there is
	 * a constant in the second. */
	if (arg_op2->type != Si2binArgVectorRegister)
		Si2binArgSwap(&arg_op1, &arg_op2);
	Si2binArgValidTypes(arg_op1, Si2binArgVectorRegister,
			Si2binArgLiteral, Si2binArgLiteralReduced,
			Si2binArgLiteralFloat, Si2binArgLiteralFloatReduced);
	Si2binArgValidTypes(arg_op2, Si2binArgVectorRegister);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llvm_inst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Emit effective address calculation.
	 * v_mul_f32 ret_vreg, arg_op1, arg_op2
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateVectorRegister, ret_vreg)));
	ListAdd(arg_list, asObject(arg_op1));
	ListAdd(arg_list, asObject(arg_op2));
	inst = new(Si2binInst, SI::INST_V_MUL_F32, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}

static void Llvm2siBasicBlockEmitExtractElement(Llvm2siBasicBlock *self,
		LLVMValueRef llvm_inst)
{
	LLVMValueRef llvm_arg_op1;
	LLVMValueRef llvm_arg_op2;

	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	Si2binArg *arg_op1;
	Si2binArg *arg_op2;
	Si2binInst *inst;
	List *arg_list;

	int num_operands;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 2 operands (op1, op2) */
	num_operands = LLVMGetNumOperands(llvm_inst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get operands (vreg, literal) */
	llvm_arg_op1 = LLVMGetOperand(llvm_inst, 0);
	llvm_arg_op2 = LLVMGetOperand(llvm_inst, 1);

	arg_op1 = Llvm2siFunctionTranslateValue(function, llvm_arg_op1, NULL);
	arg_op2 = Llvm2siFunctionTranslateValue(function, llvm_arg_op2, NULL);

	/* First argument must be a scalar register 
	 * and the second must be the offset */
	Si2binArgValidTypes(arg_op1, Si2binArgScalarRegister);
	Si2binArgValidTypes(arg_op2, Si2binArgLiteral, Si2binArgLiteralReduced);

	arg_op1->value.scalar_register.id += arg_op2->value.literal.val;

	delete(arg_op2);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llvm_inst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Emit effective address calculation.
	 * v_mov_b32 ret_vreg, arg_op1, arg_op2
	 */
	arg_list = new(List);
	ListAdd(arg_list, asObject(new_ctor(Si2binArg, CreateVectorRegister, ret_vreg)));
	ListAdd(arg_list, asObject(arg_op1));
	inst = new(Si2binInst, SI::INST_V_MOV_B32, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}

/*
 * Public Functions
 */

void Llvm2siBasicBlockCreate(Llvm2siBasicBlock *self,
		Llvm2siFunction *function, LeafNode *node)
{
	/* Parent */
	BasicBlockCreate(asBasicBlock(self), node);

	/* Initialize */
	self->comment = new(String, "");
	self->function = function;
	self->inst_list = new(List);

	/* Virtual functions */
	asObject(self)->Dump = Llvm2siBasicBlockDump;
}


void Llvm2siBasicBlockDestroy(Llvm2siBasicBlock *self)
{
	ListDeleteObjects(self->inst_list);
	delete(self->inst_list);
	delete(self->comment);
}


void Llvm2siBasicBlockDump(Object *self, FILE *f)
{
	Llvm2siBasicBlock *basic_block;
	LeafNode *node;
	Si2binInst *inst;

	/* Nothing if basic block is empty */
	basic_block = asLlvm2siBasicBlock(self);
	if (!basic_block->inst_list->count)
		return;

	/* Label with node's name */
	node = asBasicBlock(basic_block)->node;
	fprintf(f, "\n%s:\n", asNode(node)->name);

	/* Print list of instructions */
	ListForEach(basic_block->inst_list, inst, Si2binInst)
		Si2binInstDumpAssembly(inst, f);
}


void Llvm2siBasicBlockAddInst(Llvm2siBasicBlock *self, Si2binInst *inst)
{
	/* Check that the instruction does not belong to any other basic
	 * block already. */
	if (inst->basic_block)
		panic("%s: instruction already added to basic block",
				__FUNCTION__);

	/* Add instruction */
	ListAdd(self->inst_list, asObject(inst));
	inst->basic_block = self;

	/* If there was a comment added to the basic block, attach it to
	 * the instruction being added now. */
	if (self->comment->length)
	{
		Si2binInstAddComment(inst, self->comment->text);
		StringClear(self->comment);
	}
}


void Llvm2siBasicBlockAddComment(Llvm2siBasicBlock *self, char *comment)
{
	StringSet(self->comment, "%s", comment);
}


void Llvm2siBasicBlockEmit(Llvm2siBasicBlock *self, LLVMBasicBlockRef llvm_basic_block)
{
	LLVMValueRef llvm_inst;
	LLVMOpcode llopcode;

	/* Iterate over LLVM instructions */
	assert(llvm_basic_block);
	for (llvm_inst = LLVMGetFirstInstruction(llvm_basic_block); llvm_inst;
			llvm_inst = LLVMGetNextInstruction(llvm_inst))
	{
		llopcode = LLVMGetInstructionOpcode(llvm_inst);
		switch (llopcode)
		{

		case LLVMAdd:

			Llvm2siBasicBlockEmitAdd(self, llvm_inst);
			break;

		case LLVMBr:

			/* Ignore branches here */
			break;

		case LLVMCall:

			Llvm2siBasicBlockEmitCall(self, llvm_inst);
			break;

		case LLVMGetElementPtr:

			Llvm2siBasicBlockEmitGetelementptr(self, llvm_inst);
			break;

		case LLVMICmp:

			Llvm2siBasicBlockEmitIcmp(self, llvm_inst);
			break;

		case LLVMLoad:

			Llvm2siBasicBlockEmitLoad(self, llvm_inst);
			break;

		case LLVMMul:

			Llvm2siBasicBlockEmitMul(self, llvm_inst);
			break;

		case LLVMPHI:

			Llvm2siBasicBlockEmitPhi(self, llvm_inst);
			break;

		case LLVMRet:

			Llvm2siBasicBlockEmitRet(self, llvm_inst);
			break;

		case LLVMStore:

			Llvm2siBasicBlockEmitStore(self, llvm_inst);
			break;

		case LLVMSub:

			Llvm2siBasicBlockEmitSub(self, llvm_inst);
			break;

		case LLVMFAdd:

			Llvm2siBasicBlockEmitFAdd(self, llvm_inst);
			break;

		case LLVMFSub:

			Llvm2siBasicBlockEmitFSub(self, llvm_inst);
			break;
		
		case LLVMFMul:

			Llvm2siBasicBlockEmitFMul(self, llvm_inst);
			break;

		case LLVMExtractElement:
			
			Llvm2siBasicBlockEmitExtractElement(self, llvm_inst);
			break;
		
		default:

			fatal("%s: LLVM opcode not supported (%d)",
					__FUNCTION__, llopcode);
		}
	}
}

#endif


}  /* namespace llvm2si */

