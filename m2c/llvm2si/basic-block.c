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
#include "symbol.h"
#include "symbol-table.h"


/*
 * Some helper functions
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

void llvm2si_basic_block_emit_add(struct llvm2si_basic_block_t *basic_block,
		LLVMValueRef llinst)
{
	LLVMValueRef llarg_op1;
	LLVMValueRef llarg_op2;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	struct llvm2si_function_t *function;
	struct llvm2si_symbol_t *ret_symbol;
	struct si2bin_arg_t *arg_op1;
	struct si2bin_arg_t *arg_op2;
	struct si2bin_inst_t *inst;
	struct list_t *arg_list;

	int num_operands;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = basic_block->function;
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
	arg_op1 = llvm2si_function_translate_value(function, llarg_op1, NULL);
	arg_op2 = llvm2si_function_translate_value(function, llarg_op2, NULL);

	/* Only the first operand can be a constant, so swap them if there is
	 * a constant in the second. */
	if (arg_op2->type != si2bin_arg_vector_register)
		si2bin_arg_swap(&arg_op1, &arg_op2);
	si2bin_arg_valid_types(arg_op1, si2bin_arg_vector_register,
			si2bin_arg_literal, si2bin_arg_literal_float);
	si2bin_arg_valid_types(arg_op2, si2bin_arg_vector_register);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llinst);
	ret_vreg = llvm2si_function_alloc_vreg(function, 1, 1);
	ret_symbol = llvm2si_symbol_create(ret_name,
			llvm2si_symbol_vector_register,
			ret_vreg);
	llvm2si_symbol_table_add_symbol(function->symbol_table, ret_symbol);

	/* Emit effective address calculation.
	 * v_add_i32 ret_vreg, vcc, arg_op1, arg_op2
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(ret_vreg));
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
	list_add(arg_list, arg_op1);
	list_add(arg_list, arg_op2);
	inst = si2bin_inst_create(SI_INST_V_ADD_I32, arg_list);
	llvm2si_basic_block_add_inst(basic_block, inst);
}


void llvm2si_basic_block_emit_call(struct llvm2si_basic_block_t *basic_block,
		LLVMValueRef llinst)
{
	LLVMValueRef llfunction;
	LLVMValueRef llarg;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	struct llvm2si_function_t *function;
	struct llvm2si_symbol_t *ret_symbol;
	struct si2bin_inst_t *inst;
	struct si2bin_arg_t *ret_arg;
	struct list_t *arg_list;

	int ret_vreg;
	int num_args;
	int dim;

	char *var_name;
	char *func_name;

	/* Get function */
	function = basic_block->function;
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
		ret_symbol = llvm2si_symbol_create(var_name,
				llvm2si_symbol_vector_register,
				function->vreg_gid + dim);

		/* Add to symbol table */
		llvm2si_symbol_table_add_symbol(function->symbol_table, ret_symbol);
	}
	else if (!strcmp(func_name, "get_global_size"))
	{
		/* Allocate a new vector register to copy global size. */
		ret_vreg = llvm2si_function_alloc_vreg(function, 1, 1);
		ret_arg = si2bin_arg_create_vector_register(ret_vreg);
		ret_symbol = llvm2si_symbol_create(var_name,
				llvm2si_symbol_vector_register, ret_vreg);
		llvm2si_symbol_table_add_symbol(function->symbol_table, ret_symbol);

		/* Create new vector register containing the global size.
		 * v_mov_b32 vreg, s[gsize+dim]
		 */
		arg_list = list_create();
		list_add(arg_list, ret_arg);
		list_add(arg_list, si2bin_arg_create_scalar_register(
				function->sreg_gsize + dim));
		inst = si2bin_inst_create(SI_INST_V_MOV_B32, arg_list);
		llvm2si_basic_block_add_inst(basic_block, inst);
	}
	else
	{
		fatal("%s: %s: invalid built-in function",
			__FUNCTION__, func_name);
	}
}


void llvm2si_basic_block_emit_getelementptr(struct llvm2si_basic_block_t *basic_block,
		LLVMValueRef llinst)
{
	LLVMValueRef llarg_ptr;
	LLVMValueRef llarg_index;
	LLVMTypeRef lltype_ptr;

	struct llvm2si_function_t *function;
	struct llvm2si_symbol_t *ptr_symbol;
	struct llvm2si_symbol_t *ret_symbol;
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
	function = basic_block->function;
	assert(function);

	/* Only supported for 2 operands (pointer + 1 index) */
	num_operands = LLVMGetNumOperands(llinst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get pointer operand (vreg) */
	llarg_ptr = LLVMGetOperand(llinst, 0);
	arg_ptr = llvm2si_function_translate_value(function, llarg_ptr, &ptr_symbol);
	si2bin_arg_valid_types(arg_ptr, si2bin_arg_vector_register);

	/* Address must be a symbol with UAV */
	if (!ptr_symbol || !ptr_symbol->address)
		fatal("%s: no UAV for symbol", __FUNCTION__);
	
	/* Get size of pointed value */
	lltype_ptr = LLVMTypeOf(llarg_ptr);
	ptr_size = llvm2si_get_pointed_lltype_size(lltype_ptr);

	/* Get index operand (vreg, literal) */
	llarg_index = LLVMGetOperand(llinst, 1);
	arg_index = llvm2si_function_translate_value(function, llarg_index, NULL);
	si2bin_arg_valid_types(arg_index, si2bin_arg_vector_register,
			si2bin_arg_literal);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llinst);
	ret_vreg = llvm2si_function_alloc_vreg(function, 1, 1);
	ret_symbol = llvm2si_symbol_create(ret_name,
			llvm2si_symbol_vector_register,
			ret_vreg);
	llvm2si_symbol_set_uav_index(ret_symbol, ptr_symbol->uav_index);
	llvm2si_symbol_table_add_symbol(function->symbol_table, ret_symbol);

	/* Calculate offset as the multiplication between 'arg_index' and the
	 * size of the pointed element ('ptr_size'). If 'arg_index' is a
	 * literal, we can pre-calculate it here. If 'arg_index' is a vector
	 * register, we need to emit an instruction. */
	if (arg_index->type == si2bin_arg_literal)
	{
		/* Argument 'arg_offset' is just a modification of
		 * 'arg_index'. */
		arg_offset = arg_index;
		arg_offset->value.literal.val *= ptr_size;
	}
	else
	{
		/* Allocate one register and create 'arg_offset' with it */
		tmp_vreg = llvm2si_function_alloc_vreg(function, 1, 1);
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
		llvm2si_basic_block_add_inst(basic_block, inst);
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
	llvm2si_basic_block_add_inst(basic_block, inst);
}


void llvm2si_basic_block_emit_load(struct llvm2si_basic_block_t *basic_block,
		LLVMValueRef llinst)
{
	LLVMValueRef llarg_address;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	struct llvm2si_function_t *function;
	struct llvm2si_function_uav_t *uav;
	struct llvm2si_symbol_t *addr_symbol;
	struct llvm2si_symbol_t *ret_symbol;
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
	function = basic_block->function;
	assert(function);

	/* Only supported for 1 operand (address) */
	num_operands = LLVMGetNumOperands(llinst);
	if (num_operands != 1)
		fatal("%s: 1 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get address operand (vreg) */
	llarg_address = LLVMGetOperand(llinst, 0);
	arg_addr = llvm2si_function_translate_value(function, llarg_address, &addr_symbol);
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
	ret_vreg = llvm2si_function_alloc_vreg(function, 1, 1);
	ret_symbol = llvm2si_symbol_create(ret_name,
			llvm2si_symbol_vector_register,
			ret_vreg);
	llvm2si_symbol_table_add_symbol(function->symbol_table, ret_symbol);

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
	llvm2si_basic_block_add_inst(basic_block, inst);
}


void llvm2si_basic_block_emit_mul(struct llvm2si_basic_block_t *basic_block,
		LLVMValueRef llinst)
{
	LLVMValueRef llarg_op1;
	LLVMValueRef llarg_op2;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	struct llvm2si_function_t *function;
	struct llvm2si_symbol_t *ret_symbol;
	struct si2bin_arg_t *arg_op1;
	struct si2bin_arg_t *arg_op2;
	struct si2bin_inst_t *inst;
	struct list_t *arg_list;

	int num_operands;
	int ret_vreg;

	char *ret_name;

	/* Get function */
	function = basic_block->function;
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
	arg_op1 = llvm2si_function_translate_value(function, llarg_op1, NULL);
	arg_op2 = llvm2si_function_translate_value(function, llarg_op2, NULL);

	/* Only the first operand can be a constant, so swap them if there is
	 * a constant in the second. */
	if (arg_op2->type != si2bin_arg_vector_register)
		si2bin_arg_swap(&arg_op1, &arg_op2);
	si2bin_arg_valid_types(arg_op1, si2bin_arg_vector_register,
			si2bin_arg_literal, si2bin_arg_literal_float);
	si2bin_arg_valid_types(arg_op2, si2bin_arg_vector_register);

	/* Allocate vector register and create symbol for return value */
	ret_name = (char *) LLVMGetValueName(llinst);
	ret_vreg = llvm2si_function_alloc_vreg(function, 1, 1);
	ret_symbol = llvm2si_symbol_create(ret_name,
			llvm2si_symbol_vector_register,
			ret_vreg);
	llvm2si_symbol_table_add_symbol(function->symbol_table, ret_symbol);

	/* Emit effective address calculation.
	 * v_mul_lo_i32 ret_vreg, arg_op1, arg_op2
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(ret_vreg));
	list_add(arg_list, arg_op1);
	list_add(arg_list, arg_op2);
	inst = si2bin_inst_create(SI_INST_V_MUL_LO_U32, arg_list);
	llvm2si_basic_block_add_inst(basic_block, inst);
}


void llvm2si_basic_block_emit_ret(struct llvm2si_basic_block_t *basic_block,
		LLVMValueRef llinst)
{
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;

	/* Emit program end instruction.
	 * s_endpgm
	 */
	arg_list = list_create();
	inst = si2bin_inst_create(SI_INST_S_ENDPGM, arg_list);
	llvm2si_basic_block_add_inst(basic_block, inst);
}


void llvm2si_basic_block_emit_store(struct llvm2si_basic_block_t *basic_block,
		LLVMValueRef llinst)
{
	LLVMValueRef llarg_data;
	LLVMValueRef llarg_addr;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	struct llvm2si_function_t *function;
	struct llvm2si_function_uav_t *uav;
	struct llvm2si_symbol_t *addr_symbol;
	struct si2bin_inst_t *inst;
	struct si2bin_arg_t *arg_data;
	struct si2bin_arg_t *arg_addr;
	struct si2bin_arg_t *arg_qual;
	struct si2bin_arg_t *arg_soffset;
	struct list_t *arg_list;

	int num_operands;
	int addr_space;

	/* Get function */
	function = basic_block->function;
	assert(function);

	/* Only supported for 2 operand (address, data) */
	num_operands = LLVMGetNumOperands(llinst);
	if (num_operands != 2)
		fatal("%s: 2 operands supported, %d found",
			__FUNCTION__, num_operands);

	/* Get data operand (vreg) */
	llarg_data = LLVMGetOperand(llinst, 0);
	arg_data = llvm2si_function_translate_value(function, llarg_data, NULL);
	si2bin_arg_valid_types(arg_data, si2bin_arg_vector_register);

	/* Get address operand (vreg) */
	llarg_addr = LLVMGetOperand(llinst, 1);
	arg_addr = llvm2si_function_translate_value(function, llarg_addr, &addr_symbol);
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
	llvm2si_basic_block_add_inst(basic_block, inst);
}




/*
 * Public Functions
 */

struct llvm2si_basic_block_t *llvm2si_basic_block_create(LLVMBasicBlockRef llbb)
{
	LLVMValueRef llbb_as_value;
	struct llvm2si_basic_block_t *basic_block;
	char *name;

	/* Initialize */
	basic_block = xcalloc(1, sizeof(struct llvm2si_basic_block_t));
	basic_block->llbb = llbb;
	basic_block->inst_list = linked_list_create();
	basic_block->pred_list = linked_list_create();
	basic_block->succ_list = linked_list_create();

	/* Name */
	basic_block->name = str_set(basic_block->name, "");
	if (llbb)
	{
		/* Get name */
		llbb_as_value = LLVMBasicBlockAsValue(llbb);
		name = (char *) LLVMGetValueName(llbb_as_value);
		basic_block->name = str_set(basic_block->name, name);

		/* Do not allow LLVM basic blocks to be anonymous, since their
		 * name is used for insertion in a hash table. */
		if (!*basic_block->name)
			fatal("%s: anonymous LLVM basic blocks forbidden",
				__FUNCTION__);
	}

	/* Return */
	return basic_block;
}


struct llvm2si_basic_block_t *llvm2si_basic_block_create_with_name(char *name)
{
	struct llvm2si_basic_block_t *basic_block;

	/* Create */
	basic_block = llvm2si_basic_block_create(NULL);
	str_set(basic_block->name, name);
	
	/* Return */
	return basic_block;
}


void llvm2si_basic_block_free(struct llvm2si_basic_block_t *basic_block)
{
	/* Free list of instructions */
	LINKED_LIST_FOR_EACH(basic_block->inst_list)
		si2bin_inst_free(linked_list_get(basic_block->inst_list));
	linked_list_free(basic_block->inst_list);

	/* Free list of predecessors and successors. The elements of this list
	 * are not freed here. They are freen by the function they belong to. */
	linked_list_free(basic_block->pred_list);
	linked_list_free(basic_block->succ_list);
	
	/* Rest */
	str_free(basic_block->name);
	str_free(basic_block->comment);
	free(basic_block);
}


void llvm2si_basic_block_dump(struct llvm2si_basic_block_t *basic_block, FILE *f)
{
	struct si2bin_inst_t *inst;

	/* Nothing if basic block is empty */
	if (!basic_block->inst_list->count)
		return;

	/* Label with basic block name if not empty */
	if (*basic_block->name)
		fprintf(f, "\n%s:\n", basic_block->name);

	/* Print list of instructions */
	LINKED_LIST_FOR_EACH(basic_block->inst_list)
	{
		inst = linked_list_get(basic_block->inst_list);
		si2bin_inst_dump_assembly(inst, f);
	}
}


void llvm2si_basic_block_add_inst(struct llvm2si_basic_block_t *basic_block,
		struct si2bin_inst_t *inst)
{
	/* Check that the instruction does not belong to any other basic
	 * block already. */
	if (inst->basic_block)
		panic("%s: instruction already added to basic block",
				__FUNCTION__);

	/* Add instruction */
	linked_list_add(basic_block->inst_list, inst);
	inst->basic_block = basic_block;

	/* If there was a comment added to the basic block, attach it to
	 * the instruction being added now. */
	if (basic_block->comment)
	{
		si2bin_inst_add_comment(inst, basic_block->comment);
		basic_block->comment = str_free(basic_block->comment);
	}
}


void llvm2si_basic_block_add_comment(struct llvm2si_basic_block_t *basic_block,
		char *comment)
{
	basic_block->comment = str_set(basic_block->comment, comment);
}


void llvm2si_basic_block_emit(struct llvm2si_basic_block_t *basic_block)
{
	LLVMValueRef llinst;
	LLVMBasicBlockRef llbb;
	LLVMOpcode llopcode;

	/* Iterate over LLVM instructions */
	llbb = basic_block->llbb;
	for (llinst = LLVMGetFirstInstruction(llbb); llinst;
			llinst = LLVMGetNextInstruction(llinst))
	{
		llopcode = LLVMGetInstructionOpcode(llinst);
		switch (llopcode)
		{

		case LLVMAdd:

			llvm2si_basic_block_emit_add(basic_block, llinst);
			break;

		case LLVMCall:

			llvm2si_basic_block_emit_call(basic_block, llinst);
			break;

		case LLVMGetElementPtr:

			llvm2si_basic_block_emit_getelementptr(basic_block, llinst);
			break;

		case LLVMLoad:

			llvm2si_basic_block_emit_load(basic_block, llinst);
			break;

		case LLVMMul:

			llvm2si_basic_block_emit_mul(basic_block, llinst);
			break;

		case LLVMRet:

			llvm2si_basic_block_emit_ret(basic_block, llinst);
			break;

		case LLVMStore:

			llvm2si_basic_block_emit_store(basic_block, llinst);
			break;

		default:

			fatal("%s: LLVM opcode not supported (%d)",
					__FUNCTION__, llopcode);
		}
	}
}


void llvm2si_basic_block_connect(struct llvm2si_basic_block_t *basic_block,
		struct llvm2si_basic_block_t *basic_block_dest)
{
	/* Make sure that connection does not exist */
	linked_list_find(basic_block->succ_list, basic_block_dest);
	linked_list_find(basic_block_dest->pred_list, basic_block);
	if (!basic_block->succ_list->error_code ||
			!basic_block_dest->pred_list->error_code)
		panic("%s: redundant connection between basic blocks",
				__FUNCTION__);

	/* Make connection */
	linked_list_add(basic_block->succ_list, basic_block_dest);
	linked_list_add(basic_block_dest->pred_list, basic_block);
}

