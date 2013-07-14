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

#include <assert.h>

#include <m2c/common/basic-block.h>
#include <m2c/common/node.h>
#include <m2c/common/ctree.h>
#include <m2c/si2bin/arg.h>
#include <m2c/si2bin/inst.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "basic-block.h"
#include "function.h"
#include "phi.h"
#include "symbol.h"
#include "symbol-table.h"


/*
 * Non-Class Functions
 */

static int llvm2si_get_lltype_size(LLVMTypeRef lltype)
{
	LLVMTypeKind lltype_kind;
	int bit_width;

	/* Return size based on type kind */
	lltype_kind = LLVMGetTypeKind(lltype);
	switch (lltype_kind)
	{

	case LLVMIntegerTypeKind:
		
		bit_width = LLVMGetIntTypeWidth(lltype);
		return (bit_width + 7) / 8;

	case LLVMPointerTypeKind:

		/* Memory address is 4 bytes */
		return 4;
	
	default:

		fatal("%s: unsupported type kind (%d)",
			__FUNCTION__, lltype_kind);
		return 0;
	}
}


static int llvm2si_get_pointed_lltype_size(LLVMTypeRef lltype)
{
	LLVMTypeKind lltype_kind;

	/* Type must be a pointer */
	lltype_kind = LLVMGetTypeKind(lltype);
	if (lltype_kind != LLVMPointerTypeKind)
		fatal("%s: type not a pointer", __FUNCTION__);

	/* Get pointed type */
	lltype = LLVMGetElementType(lltype);
	return llvm2si_get_lltype_size(lltype);
}



/*
 * Private Functions
 */

static void Llvm2siBasicBlockEmitAdd(Llvm2siBasicBlock *self,
		LLVMValueRef llinst)
{
	LLVMValueRef llarg_op1;
	LLVMValueRef llarg_op2;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	struct si2bin_arg_t *arg_op1;
	struct si2bin_arg_t *arg_op2;
	struct si2bin_inst_t *inst;
	struct list_t *arg_list;

	int num_operands;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 32-bit integers */
	lltype = LLVMTypeOf(llinst);
	lltype_kind = LLVMGetTypeKind(lltype);
	if (lltype_kind != LLVMIntegerTypeKind ||
			LLVMGetIntTypeWidth(lltype) != 32)
		fatal("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Only supported for 2 operands (op1, op2) */
	num_operands = LLVMGetNumOperands(llinst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get operands (vreg, literal) */
	llarg_op1 = LLVMGetOperand(llinst, 0);
	llarg_op2 = LLVMGetOperand(llinst, 1);
	arg_op1 = Llvm2siFunctionTranslateValue(function, llarg_op1, NULL);
	arg_op2 = Llvm2siFunctionTranslateValue(function, llarg_op2, NULL);

	/* Second operand cannot be a constant */
	arg_op2 = Llvm2siFunctionConstToVReg(function, self, arg_op2);
	si2bin_arg_valid_types(arg_op1, si2bin_arg_vector_register,
			si2bin_arg_literal, si2bin_arg_literal_reduced,
			si2bin_arg_literal_float, si2bin_arg_literal_float_reduced);
	si2bin_arg_valid_types(arg_op2, si2bin_arg_vector_register);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llinst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Emit addition.
	 * v_add_i32 ret_vreg, vcc, arg_op1, arg_op2
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(ret_vreg));
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
	list_add(arg_list, arg_op1);
	list_add(arg_list, arg_op2);
	inst = si2bin_inst_create(SI_INST_V_ADD_I32, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}


static void Llvm2siBasicBlockEmitCall(Llvm2siBasicBlock *self,
		LLVMValueRef llinst)
{
	LLVMValueRef llfunction;
	LLVMValueRef llarg;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	struct si2bin_inst_t *inst;
	struct si2bin_arg_t *ret_arg;
	struct list_t *arg_list;

	int ret_vreg;
	int num_args;
	int dim;

	char *var_name;
	char *func_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Check that there is only one argument. LLVMGetNumOperands returns the
	 * number of arguments plus one (the function being called as a last
	 * argument. */
	num_args = LLVMGetNumOperands(llinst) - 1;
	if (num_args != 1)
		fatal("%s: 1 argument expected, %d found",
				__FUNCTION__, num_args);

	/* Get called function, found in last operand of the operand list, as
	 * returned by LLVMGetOperand. */
	llfunction = LLVMGetOperand(llinst, num_args);
	func_name = (char *) LLVMGetValueName(llfunction);

	/* Get return argument name */
	var_name = (char *) LLVMGetValueName(llinst);
	if (!*var_name)
		fatal("%s: invalid return variable",
				__FUNCTION__);

	/* Number of arguments */
	num_args = LLVMCountParams(llfunction);
	if (num_args != 1)
		fatal("%s: 1 argument expected for '%s', %d found",
			__FUNCTION__, func_name, num_args);

	/* Get argument and check type */
	llarg = LLVMGetOperand(llinst, 0);
	lltype = LLVMTypeOf(llarg);
	lltype_kind = LLVMGetTypeKind(lltype);
	if (lltype_kind != LLVMIntegerTypeKind || !LLVMIsConstant(llarg))
		fatal("%s: argument should be an integer constant",
				__FUNCTION__);

	/* Get argument value and check bounds */
	dim = LLVMConstIntGetZExtValue(llarg);
	if (!IN_RANGE(dim, 0, 2))
		fatal("%s: constant in range [0..2] expected",
				__FUNCTION__);

	/* Built-in functions */
	if (!strcmp(func_name, "get_global_id"))
	{
		/* Create new symbol associating it with the vector register
		 * containing the global ID in the given dimension. */
		ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, var_name,
				function->vreg_gid + dim);
		Llvm2siSymbolTableAddSymbol(function->symbol_table,
				ret_symbol);
	}
	else if (!strcmp(func_name, "get_global_size"))
	{
		/* Allocate a new vector register to copy global size. */
		ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
		ret_arg = si2bin_arg_create_vector_register(ret_vreg);
		ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, var_name, ret_vreg);
		Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

		/* Create new vector register containing the global size.
		 * v_mov_b32 vreg, s[gsize+dim]
		 */
		arg_list = list_create();
		list_add(arg_list, ret_arg);
		list_add(arg_list, si2bin_arg_create_scalar_register(
				function->sreg_gsize + dim));
		inst = si2bin_inst_create(SI_INST_V_MOV_B32, arg_list);
		Llvm2siBasicBlockAddInst(self, inst);
	}
	else
	{
		fatal("%s: %s: invalid built-in function",
			__FUNCTION__, func_name);
	}
}


static void Llvm2siBasicBlockEmitGetelementptr(Llvm2siBasicBlock *self,
		LLVMValueRef llinst)
{
	LLVMValueRef llarg_ptr;
	LLVMValueRef llarg_index;
	LLVMTypeRef lltype_ptr;

	Llvm2siFunction *function;
	Llvm2siSymbol *ptr_symbol;
	Llvm2siSymbol *ret_symbol;
	struct si2bin_arg_t *arg_ptr;
	struct si2bin_arg_t *arg_index;
	struct si2bin_arg_t *arg_offset;
	struct si2bin_inst_t *inst;
	struct list_t *arg_list;

	int num_operands;
	int tmp_vreg;
	int ret_vreg;
	int ptr_size;

	char *ret_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 2 operands (pointer + 1 index) */
	num_operands = LLVMGetNumOperands(llinst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get pointer operand (vreg) */
	llarg_ptr = LLVMGetOperand(llinst, 0);
	arg_ptr = Llvm2siFunctionTranslateValue(function, llarg_ptr, &ptr_symbol);
	si2bin_arg_valid_types(arg_ptr, si2bin_arg_vector_register);

	/* Address must be a symbol with UAV */
	if (!ptr_symbol || !ptr_symbol->address)
		fatal("%s: no UAV for symbol", __FUNCTION__);
	
	/* Get size of pointed value */
	lltype_ptr = LLVMTypeOf(llarg_ptr);
	ptr_size = llvm2si_get_pointed_lltype_size(lltype_ptr);

	/* Get index operand (vreg, literal) */
	llarg_index = LLVMGetOperand(llinst, 1);
	arg_index = Llvm2siFunctionTranslateValue(function, llarg_index, NULL);
	si2bin_arg_valid_types(arg_index, si2bin_arg_vector_register,
			si2bin_arg_literal, si2bin_arg_literal_reduced);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llinst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolSetUAVIndex(ret_symbol, ptr_symbol->uav_index);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Calculate offset as the multiplication between 'arg_index' and the
	 * size of the pointed element ('ptr_size'). If 'arg_index' is a
	 * literal, we can pre-calculate it here. If 'arg_index' is a vector
	 * register, we need to emit an instruction. */
	if (arg_index->type == si2bin_arg_literal ||
			arg_index->type == si2bin_arg_literal_reduced)
	{
		/* Argument 'arg_offset' is just a modification of
		 * 'arg_index'. */
		arg_offset = arg_index;
		arg_offset->value.literal.val *= ptr_size;
	}
	else
	{
		/* Allocate one register and create 'arg_offset' with it */
		tmp_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
		arg_offset = si2bin_arg_create_vector_register(tmp_vreg);

		/* Emit calculation of offset as the multiplication between the
		 * index argument and the pointed element size.
		 * v_mul_i32_i24 tmp_vreg, ptr_size, arg_index
		 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(tmp_vreg));
		list_add(arg_list, si2bin_arg_create_literal(ptr_size));
		list_add(arg_list, arg_index);
		inst = si2bin_inst_create(SI_INST_V_MUL_I32_I24, arg_list);
		Llvm2siBasicBlockAddInst(self, inst);
	}

	/* Emit effective address calculation as the addition between the
	 * original pointer and the offset.
	 * v_add_i32 ret_vreg, vcc, arg_offset, arg_pointer
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(ret_vreg));
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
	list_add(arg_list, arg_offset);
	list_add(arg_list, arg_ptr);
	inst = si2bin_inst_create(SI_INST_V_ADD_I32, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}


static void Llvm2siBasicBlockEmitIcmp(Llvm2siBasicBlock *basic_block,
		LLVMValueRef llinst)
{
	LLVMValueRef llarg_op1;
	LLVMValueRef llarg_op2;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;
	LLVMIntPredicate llpred;

	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	struct si2bin_arg_t *arg_op1;
	struct si2bin_arg_t *arg_op2;
	struct si2bin_inst_t *inst;
	struct list_t *arg_list;

	enum si_inst_opcode_t opcode;

	int num_operands;
	int ret_sreg_series;
	int invert;

	char *ret_name;

	/* Get function */
	function = basic_block->function;
	assert(function);

	/* Only supported for 2 operands (op1, op2) */
	num_operands = LLVMGetNumOperands(llinst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get operands (vreg, literal) */
	llarg_op1 = LLVMGetOperand(llinst, 0);
	llarg_op2 = LLVMGetOperand(llinst, 1);
	arg_op1 = Llvm2siFunctionTranslateValue(function, llarg_op1, NULL);
	arg_op2 = Llvm2siFunctionTranslateValue(function, llarg_op2, NULL);

	/* Only supported for 32-bit integers */
	lltype = LLVMTypeOf(llarg_op1);
	lltype_kind = LLVMGetTypeKind(lltype);
	if (lltype_kind != LLVMIntegerTypeKind ||
			LLVMGetIntTypeWidth(lltype) != 32)
		fatal("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Only the first argument can be a literal. If the second argument is
	 * a literal, flip them and invert comparison predicate later. */
	invert = 0;
	if (arg_op2->type != si2bin_arg_vector_register)
	{
		si2bin_arg_swap(&arg_op1, &arg_op2);
		invert = 1;
	}

	/* Valid argument types. Argument 2 cannot be a literal. */
	arg_op2 = Llvm2siFunctionConstToVReg(function, basic_block, arg_op2);
	si2bin_arg_valid_types(arg_op1, si2bin_arg_vector_register,
			si2bin_arg_literal, si2bin_arg_literal_reduced,
			si2bin_arg_literal_float, si2bin_arg_literal_float_reduced);
	si2bin_arg_valid_types(arg_op2, si2bin_arg_vector_register);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llinst);
	ret_sreg_series = Llvm2siFunctionAllocSReg(function, 2, 2);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateSRegSeries, ret_name,
			ret_sreg_series, ret_sreg_series + 1);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Choose instruction based on predicate */
	opcode = 0;
	llpred = LLVMGetICmpPredicate(llinst);
	switch (llpred)
	{

	case LLVMIntEQ:

		opcode = SI_INST_V_CMP_EQ_I32;
		break;

	case LLVMIntNE:

		opcode = SI_INST_V_CMP_NE_I32;
		break;

	case LLVMIntUGT:

		opcode = SI_INST_V_CMP_GT_U32;
		if (invert)
			opcode = SI_INST_V_CMP_LT_U32;
		break;

	case LLVMIntUGE:

		opcode = SI_INST_V_CMP_GE_U32;
		if (invert)
			opcode = SI_INST_V_CMP_LE_U32;
		break;

	case LLVMIntULT:

		opcode = SI_INST_V_CMP_LT_U32;
		if (invert)
			opcode = SI_INST_V_CMP_GT_U32;
		break;

	case LLVMIntULE:

		opcode = SI_INST_V_CMP_LE_U32;
		if (invert)
			opcode = SI_INST_V_CMP_GE_U32;
		break;

	case LLVMIntSGT:

		opcode = SI_INST_V_CMP_GT_I32;
		if (invert)
			opcode = SI_INST_V_CMP_LT_I32;
		break;

	case LLVMIntSGE:

		opcode = SI_INST_V_CMP_GE_I32;
		if (invert)
			opcode = SI_INST_V_CMP_LE_I32;
		break;

	case LLVMIntSLT:

		opcode = SI_INST_V_CMP_LT_I32;
		if (invert)
			opcode = SI_INST_V_CMP_GT_I32;
		break;

	case LLVMIntSLE:

		opcode = SI_INST_V_CMP_LE_I32;
		if (invert)
			opcode = SI_INST_V_CMP_GE_I32;
		break;
	}

	/* Emit comparison.
	 * v_cmp_<pred>_<type> vcc, arg_op1, arg_op2
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
	list_add(arg_list, arg_op1);
	list_add(arg_list, arg_op2);
	inst = si2bin_inst_create(opcode, arg_list);
	Llvm2siBasicBlockAddInst(basic_block, inst);

	/* Store 'vcc' in scalar register
	 * s_mov_b64 ret_sreg_series, vcc
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register_series(ret_sreg_series,
			ret_sreg_series + 1));
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
	inst = si2bin_inst_create(SI_INST_S_MOV_B64, arg_list);
	Llvm2siBasicBlockAddInst(basic_block, inst);
}


static void Llvm2siBasicBlockEmitLoad(Llvm2siBasicBlock *self,
		LLVMValueRef llinst)
{
	LLVMValueRef llarg_address;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	Llvm2siFunction *function;
	Llvm2siFunctionUAV *uav;
	Llvm2siSymbol *addr_symbol;
	Llvm2siSymbol *ret_symbol;
	struct si2bin_inst_t *inst;
	struct si2bin_arg_t *arg_addr;
	struct si2bin_arg_t *arg_qual;
	struct si2bin_arg_t *arg_soffset;
	struct list_t *arg_list;

	int num_operands;
	int addr_space;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 1 operand (address) */
	num_operands = LLVMGetNumOperands(llinst);
	if (num_operands != 1)
		fatal("%s: 1 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get address operand (vreg) */
	llarg_address = LLVMGetOperand(llinst, 0);
	arg_addr = Llvm2siFunctionTranslateValue(function, llarg_address, &addr_symbol);
	si2bin_arg_valid_types(arg_addr, si2bin_arg_vector_register);

	/* Address must be a symbol with UAV */
	if (!addr_symbol || !addr_symbol->address)
		fatal("%s: no UAV for symbol", __FUNCTION__);

	/* Get UAV */
	uav = list_get(function->uav_list, addr_symbol->uav_index);
	if (!uav)
		fatal("%s: invalid UAV index (%d)", __FUNCTION__,
				addr_symbol->uav_index);

	/* Get address space - only 1 (global mem.) supported for now */
	lltype = LLVMTypeOf(llarg_address);
	addr_space = LLVMGetPointerAddressSpace(lltype);
	if (addr_space != 1)
		fatal("%s: address space 1 expected (%d given)",
			__FUNCTION__, addr_space);

	/* Get return type (data) - only support 4-byte types for now */
	lltype = LLVMTypeOf(llinst);
	lltype_kind = LLVMGetTypeKind(lltype);
	if (!(lltype_kind == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(lltype) == 32) &&
			lltype_kind != LLVMFloatTypeKind)
		fatal("%s: only 4-byte int/float types supported", __FUNCTION__);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llinst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Emit memory load instruction.
	 * tbuffer_load_format_x v[value_symbol->vreg], v[pointer_symbol->vreg],
	 * 	s[sreg_uav,sreg_uav+3], 0 offen format:[BUF_DATA_FORMAT_32,BUF_NUM_FORMAT_FLOAT]
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(ret_vreg));
	list_add(arg_list, arg_addr);
	list_add(arg_list, si2bin_arg_create_scalar_register_series(uav->sreg,
			uav->sreg + 3));
	arg_soffset = si2bin_arg_create_literal(0);
	arg_qual = si2bin_arg_create_maddr_qual();
	list_add(arg_list, si2bin_arg_create_maddr(arg_soffset, arg_qual,
			si_inst_buf_data_format_32, si_inst_buf_num_format_float));
	inst = si2bin_inst_create(SI_INST_TBUFFER_LOAD_FORMAT_X, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}


static void Llvm2siBasicBlockEmitMul(Llvm2siBasicBlock *self,
		LLVMValueRef llinst)
{
	LLVMValueRef llarg_op1;
	LLVMValueRef llarg_op2;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	struct si2bin_arg_t *arg_op1;
	struct si2bin_arg_t *arg_op2;
	struct si2bin_inst_t *inst;
	struct list_t *arg_list;

	int num_operands;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 2 operands (op1, op2) */
	num_operands = LLVMGetNumOperands(llinst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Only supported for 32-bit integers */
	lltype = LLVMTypeOf(llinst);
	lltype_kind = LLVMGetTypeKind(lltype);
	if (lltype_kind != LLVMIntegerTypeKind ||
			LLVMGetIntTypeWidth(lltype) != 32)
		fatal("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Get operands (vreg, literal) */
	llarg_op1 = LLVMGetOperand(llinst, 0);
	llarg_op2 = LLVMGetOperand(llinst, 1);
	arg_op1 = Llvm2siFunctionTranslateValue(function, llarg_op1, NULL);
	arg_op2 = Llvm2siFunctionTranslateValue(function, llarg_op2, NULL);

	/* Only the first operand can be a constant, so swap them if there is
	 * a constant in the second. */
	if (arg_op2->type != si2bin_arg_vector_register)
		si2bin_arg_swap(&arg_op1, &arg_op2);
	si2bin_arg_valid_types(arg_op1, si2bin_arg_vector_register,
			si2bin_arg_literal, si2bin_arg_literal_reduced,
			si2bin_arg_literal_float, si2bin_arg_literal_float_reduced);
	si2bin_arg_valid_types(arg_op2, si2bin_arg_vector_register);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llinst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Emit effective address calculation.
	 * v_mul_lo_i32 ret_vreg, arg_op1, arg_op2
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(ret_vreg));
	list_add(arg_list, arg_op1);
	list_add(arg_list, arg_op2);
	inst = si2bin_inst_create(SI_INST_V_MUL_LO_U32, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}


static void Llvm2siBasicBlockEmitPhi(Llvm2siBasicBlock *self,
		LLVMValueRef llinst)
{
	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	Node *node;
	struct si2bin_arg_t *arg;
	Llvm2siPhi *phi;

	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;
	LLVMValueRef llvalue;
	LLVMValueRef llbb_value;
	LLVMBasicBlockRef llbb;

	char *ret_name;
	char *llbb_name;

	int ret_vreg;
	int num_operands;
	int i;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 32-bit integers */
	lltype = LLVMTypeOf(llinst);
	lltype_kind = LLVMGetTypeKind(lltype);
	if (lltype_kind != LLVMIntegerTypeKind ||
			LLVMGetIntTypeWidth(lltype) != 32)
		fatal("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llinst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Process arguments */
	num_operands = LLVMCountIncoming(llinst);
	for (i = 0; i < num_operands; i++)
	{
		/* Get source basic block */
		llbb = LLVMGetIncomingBlock(llinst, i);
		llbb_value = LLVMBasicBlockAsValue(llbb);
		llbb_name = (char *) LLVMGetValueName(llbb_value);
		node = CTreeGetNode(function->ctree, llbb_name);
		if (!node)
			panic("%s: cannot find node '%s'",
					__FUNCTION__, llbb_name);

		/* Get source value */
		llvalue = LLVMGetIncomingValue(llinst, i);

		/* Create destination argument */
		arg = si2bin_arg_create_vector_register(ret_vreg);

		/* Create 'phi' element and add it. */
		phi = new(Llvm2siPhi, node, llvalue, arg);
		linked_list_add(function->phi_list, phi);
	}
}


static void Llvm2siBasicBlockEmitRet(Llvm2siBasicBlock *self,
		LLVMValueRef llinst)
{
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;

	/* Emit program end instruction.
	 * s_endpgm
	 */
	arg_list = list_create();
	inst = si2bin_inst_create(SI_INST_S_ENDPGM, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}


static void Llvm2siBasicBlockEmitStore(Llvm2siBasicBlock *self,
		LLVMValueRef llinst)
{
	LLVMValueRef llarg_data;
	LLVMValueRef llarg_addr;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	Llvm2siFunction *function;
	Llvm2siFunctionUAV *uav;
	Llvm2siSymbol *addr_symbol;
	struct si2bin_inst_t *inst;
	struct si2bin_arg_t *arg_data;
	struct si2bin_arg_t *arg_addr;
	struct si2bin_arg_t *arg_qual;
	struct si2bin_arg_t *arg_soffset;
	struct list_t *arg_list;

	int num_operands;
	int addr_space;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 2 operand (address, data) */
	num_operands = LLVMGetNumOperands(llinst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get data operand (vreg) */
	llarg_data = LLVMGetOperand(llinst, 0);
	arg_data = Llvm2siFunctionTranslateValue(function, llarg_data, NULL);
	arg_data = Llvm2siFunctionConstToVReg(function, self, arg_data);
	si2bin_arg_valid_types(arg_data, si2bin_arg_vector_register);

	/* Get address operand (vreg) */
	llarg_addr = LLVMGetOperand(llinst, 1);
	arg_addr = Llvm2siFunctionTranslateValue(function, llarg_addr, &addr_symbol);
	si2bin_arg_valid_types(arg_addr, si2bin_arg_vector_register);

	/* Address must be a symbol with UAV */
	if (!addr_symbol || !addr_symbol->address)
		fatal("%s: no UAV for symbol", __FUNCTION__);

	/* Get UAV */
	uav = list_get(function->uav_list, addr_symbol->uav_index);
	if (!uav)
		fatal("%s: invalid UAV index (%d)", __FUNCTION__,
				addr_symbol->uav_index);

	/* Get address space - only 1 (global mem.) supported for now */
	lltype = LLVMTypeOf(llarg_addr);
	addr_space = LLVMGetPointerAddressSpace(lltype);
	if (addr_space != 1)
		fatal("%s: address space 1 expected (%d given)",
			__FUNCTION__, addr_space);

	/* Get type of data - only support 4-byte types for now */
	lltype = LLVMTypeOf(llarg_data);
	lltype_kind = LLVMGetTypeKind(lltype);
	if (!(lltype_kind == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(lltype) == 32) &&
			lltype_kind != LLVMFloatTypeKind)
		fatal("%s: only 4-byte int/float types supported", __FUNCTION__);

	/* Emit memory write.
	 * tbuffer_store_format_x v[value_symbol->vreg], s[pointer_symbol->vreg],
	 * 	s[sreg_uav,sreg_uav+3], 0 offen format:[BUF_DATA_FORMAT_32,
	 * 	BUF_NUM_FORMAT_FLOAT]
	 */
	arg_list = list_create();
	list_add(arg_list, arg_data);
	list_add(arg_list, arg_addr);
	list_add(arg_list, si2bin_arg_create_scalar_register_series(uav->sreg,
			uav->sreg + 3));
	arg_soffset = si2bin_arg_create_literal(0);
	arg_qual = si2bin_arg_create_maddr_qual();
	list_add(arg_list, si2bin_arg_create_maddr(arg_soffset, arg_qual,
			si_inst_buf_data_format_32, si_inst_buf_num_format_float));
	inst = si2bin_inst_create(SI_INST_TBUFFER_STORE_FORMAT_X, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}


static void Llvm2siBasicBlockEmitSub(Llvm2siBasicBlock *self,
		LLVMValueRef llinst)
{
	LLVMValueRef llarg_op1;
	LLVMValueRef llarg_op2;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	Llvm2siFunction *function;
	Llvm2siSymbol *ret_symbol;
	struct si2bin_arg_t *arg_op1;
	struct si2bin_arg_t *arg_op2;
	struct si2bin_inst_t *inst;
	struct list_t *arg_list;

	int num_operands;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = self->function;
	assert(function);

	/* Only supported for 32-bit integers */
	lltype = LLVMTypeOf(llinst);
	lltype_kind = LLVMGetTypeKind(lltype);
	if (lltype_kind != LLVMIntegerTypeKind ||
			LLVMGetIntTypeWidth(lltype) != 32)
		fatal("%s: only supported for 32-bit integers",
				__FUNCTION__);

	/* Only supported for 2 operands (op1, op2) */
	num_operands = LLVMGetNumOperands(llinst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get operands (vreg, literal) */
	llarg_op1 = LLVMGetOperand(llinst, 0);
	llarg_op2 = LLVMGetOperand(llinst, 1);
	arg_op1 = Llvm2siFunctionTranslateValue(function, llarg_op1, NULL);
	arg_op2 = Llvm2siFunctionTranslateValue(function, llarg_op2, NULL);

	/* Operand 2 cannot be a constant */
	arg_op2 = Llvm2siFunctionConstToVReg(function, self, arg_op2);
	si2bin_arg_valid_types(arg_op1, si2bin_arg_vector_register,
			si2bin_arg_literal, si2bin_arg_literal_reduced,
			si2bin_arg_literal_float, si2bin_arg_literal_float_reduced);
	si2bin_arg_valid_types(arg_op2, si2bin_arg_vector_register);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llinst);
	ret_vreg = Llvm2siFunctionAllocVReg(function, 1, 1);
	ret_symbol = new_ctor(Llvm2siSymbol, CreateVReg, ret_name, ret_vreg);
	Llvm2siSymbolTableAddSymbol(function->symbol_table, ret_symbol);

	/* Emit subtraction.
	 * v_sub_i32 ret_vreg, vcc, arg_op1, arg_op2
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(ret_vreg));
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
	list_add(arg_list, arg_op1);
	list_add(arg_list, arg_op2);
	inst = si2bin_inst_create(SI_INST_V_SUB_I32, arg_list);
	Llvm2siBasicBlockAddInst(self, inst);
}




/*
 * Public Functions
 */

CLASS_IMPLEMENTATION(Llvm2siBasicBlock);

void Llvm2siBasicBlockCreate(Llvm2siBasicBlock *self,
		Llvm2siFunction *function, LeafNode *node)
{
	/* Parent */
	BasicBlockCreate(asBasicBlock(self), node);

	/* Initialize */
	self->function = function;
	self->inst_list = linked_list_create();

	/* Virtual functions */
	asObject(self)->Dump = Llvm2siBasicBlockDump;
}


void Llvm2siBasicBlockDestroy(Llvm2siBasicBlock *self)
{
	/* Free list of instructions */
	LINKED_LIST_FOR_EACH(self->inst_list)
		si2bin_inst_free(linked_list_get(self->inst_list));
	linked_list_free(self->inst_list);

	/* Rest */
	self->comment = str_free(self->comment);
}


void Llvm2siBasicBlockDump(Object *self, FILE *f)
{
	Llvm2siBasicBlock *basic_block;
	LeafNode *node;
	struct si2bin_inst_t *inst;

	/* Nothing if basic block is empty */
	basic_block = asLlvm2siBasicBlock(self);
	if (!basic_block->inst_list->count)
		return;

	/* Label with node's name */
	node = asBasicBlock(basic_block)->node;
	fprintf(f, "\n%s:\n", asNode(node)->name);

	/* Print list of instructions */
	LINKED_LIST_FOR_EACH(basic_block->inst_list)
	{
		inst = linked_list_get(basic_block->inst_list);
		si2bin_inst_dump_assembly(inst, f);
	}
}


void Llvm2siBasicBlockAddInst(Llvm2siBasicBlock *self,
		struct si2bin_inst_t *inst)
{
	/* Check that the instruction does not belong to any other basic
	 * block already. */
	if (inst->basic_block)
		panic("%s: instruction already added to basic block",
				__FUNCTION__);

	/* Add instruction */
	linked_list_add(self->inst_list, inst);
	inst->basic_block = self;

	/* If there was a comment added to the basic block, attach it to
	 * the instruction being added now. */
	if (self->comment)
	{
		si2bin_inst_add_comment(inst, self->comment);
		self->comment = str_free(self->comment);
	}
}


void Llvm2siBasicBlockAddComment(Llvm2siBasicBlock *self, char *comment)
{
	self->comment = str_set(self->comment, comment);
}


void Llvm2siBasicBlockEmit(Llvm2siBasicBlock *self, LLVMBasicBlockRef llbb)
{
	LLVMValueRef llinst;
	LLVMOpcode llopcode;

	/* Iterate over LLVM instructions */
	assert(llbb);
	for (llinst = LLVMGetFirstInstruction(llbb); llinst;
			llinst = LLVMGetNextInstruction(llinst))
	{
		llopcode = LLVMGetInstructionOpcode(llinst);
		switch (llopcode)
		{

		case LLVMAdd:

			Llvm2siBasicBlockEmitAdd(self, llinst);
			break;

		case LLVMBr:

			/* Ignore branches here */
			break;

		case LLVMCall:

			Llvm2siBasicBlockEmitCall(self, llinst);
			break;

		case LLVMGetElementPtr:

			Llvm2siBasicBlockEmitGetelementptr(self, llinst);
			break;

		case LLVMICmp:

			Llvm2siBasicBlockEmitIcmp(self, llinst);
			break;

		case LLVMLoad:

			Llvm2siBasicBlockEmitLoad(self, llinst);
			break;

		case LLVMMul:

			Llvm2siBasicBlockEmitMul(self, llinst);
			break;

		case LLVMPHI:

			Llvm2siBasicBlockEmitPhi(self, llinst);
			break;

		case LLVMRet:

			Llvm2siBasicBlockEmitRet(self, llinst);
			break;

		case LLVMStore:

			Llvm2siBasicBlockEmitStore(self, llinst);
			break;

		case LLVMSub:

			Llvm2siBasicBlockEmitSub(self, llinst);
			break;

		default:

			fatal("%s: LLVM opcode not supported (%d)",
					__FUNCTION__, llopcode);
		}
	}
}
