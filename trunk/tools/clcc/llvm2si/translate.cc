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

/* C++ includes */
#include <iostream>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/Instructions.h>
#include <llvm/LLVMContext.h>
#include <llvm/Pass.h>
#include <llvm/PassManager.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/system_error.h>
#include <llvm/Type.h>


/* C includes */
extern "C"
{

#include <stdlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <clcc/llvm2si/basic-block.h>
#include <clcc/llvm2si/function.h>
#include <clcc/si2bin/arg.h>
#include <clcc/si2bin/inst.h>

#include "symbol.h"
#include "symbol-table.h"
#include "translate.h"
}


using namespace llvm;
using namespace std;

/* Private variables */
static FILE *llvm2si_outf;



/*
 * Private Functions
 */

static enum si_arg_data_type_t llvm2si_translate_arg_type(Type *llvm_arg_type)
{
	int bit_width;

	if (llvm_arg_type->isIntegerTy())
	{
		bit_width = llvm_arg_type->getIntegerBitWidth();
		switch (bit_width)
		{
		case 1: return si_arg_i1;
		case 8: return si_arg_i8;
		case 16: return si_arg_i16;
		case 32: return si_arg_i32;
		case 64: return si_arg_i64;

		default:
			fatal("%s: invalid argument bit width (%d)",
				__FUNCTION__, bit_width);
			return si_arg_data_type_invalid;
		}
	}
	else
	{
		errs() << __FUNCTION__ << ": unsupported argument type: "
				<< *llvm_arg_type << "\n";
		return si_arg_data_type_invalid;
	}
}


static void llvm2si_translate_arg(Argument *llvm_arg,
		struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block)
{
	char arg_name[MAX_STRING_SIZE];

	struct llvm2si_function_arg_t *function_arg;
	struct si_arg_t *arg;

	Type *llvm_arg_type;

	/* Argument name */
	snprintf(arg_name, sizeof arg_name, "arg%d", function->arg_list->count);

	/* Check argument type */
	llvm_arg_type = llvm_arg->getType();
	if (llvm_arg_type->isPointerTy())
	{
		llvm_arg_type = llvm_arg_type->getPointerElementType();
		arg = si_arg_create(si_arg_pointer, arg_name);
		arg->pointer.data_type = llvm2si_translate_arg_type(llvm_arg_type);
	}
	else
	{
		arg = si_arg_create(si_arg_value, arg_name);
		arg->value.data_type = llvm2si_translate_arg_type(llvm_arg_type);
	}

	/* Function argument */
	function_arg = llvm2si_function_arg_create(arg);
	llvm2si_function_add_arg(function, function_arg, basic_block);
}


/* Create a Southern Islands instruction argument from an LLVM value. The type
 * of argument created depends on the LLVM value as follows:
 *   - If the LLVM value is an integer constant, the Southern Islands argument
 *     will be of type integer literal.
 *   - If the LLVM value is an LLVM identifier, the Southern Islands argument
 *     will be the vector register associated with that symbol.
 *   - If the LLVM value is a function argument, the Southern Islands argument
 *     will be the scalar register pointing to that argument.
 */
static struct si2bin_arg_t *llvm2si_translate_value(Value *value,
		struct llvm2si_function_t *function)
{
	Type *type;
	ConstantInt *constant_int_value;
	Argument *func_arg;

	/* Get value type */
	type = value->getType();

	/* LLVM identifier */
	if (value->hasName())
	{
		const char *name;
		struct llvm2si_symbol_t *symbol;

		/* Look up symbol */
		name = value->getName().data();
		symbol = llvm2si_symbol_table_lookup(function->symbol_table, name);
		if (!symbol)
			fatal("%s: %s: symbol not found", __FUNCTION__, name);

		/* Create argument */
		return si2bin_arg_create_vector_register(symbol->vreg);
	}

	/* Integer constant */
	constant_int_value = dynamic_cast<ConstantInt*>(value);
	if (constant_int_value)
	{
		IntegerType *constant_int_type;
		int bit_width;
		int value;

		/* Only 32-bit constants supported for now. We need to figure
		 * out what to do with the sign extension otherwise. */
		constant_int_type = reinterpret_cast<IntegerType*>(type);
		bit_width = constant_int_type->getBitWidth();
		if (bit_width != 32)
			fatal("%s: only 32-bit type supported (%d-bit found)",
				__FUNCTION__, bit_width);

		/* Create argument */
		value = constant_int_value->getZExtValue();
		return si2bin_arg_create_literal(value);
	}

	/* Function argument */
	func_arg = dynamic_cast<Argument*>(value);
	if (func_arg)
	{
		int index;

		/* Create Southern Islands argument as a scalar register equal
		 * to the baseline register used for arguments 'sreg_arg' plus
		 * the index of the requested function argument. */
		index = func_arg->getArgNo();
		return si2bin_arg_create_scalar_register(function->sreg_arg + index);
	}

	/* Type not supported */
	fatal("%s: value type not supported", __FUNCTION__);
	return NULL;
}


static void llvm2si_translate_add_inst(BinaryOperator *add_inst,
		struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block)
{
	Value *op1;
	Value *op2;

	struct si2bin_arg_t *arg_op1;
	struct si2bin_arg_t *arg_op2;
	struct list_t *arg_list;
	struct llvm2si_symbol_t *symbol;
	struct si2bin_inst_t *inst;

	/* Get operands */
	assert(add_inst->getNumOperands() == 2);
	op1 = add_inst->getOperand(0);
	op2 = add_inst->getOperand(1);

	/* Create SI arguments for operands */
	arg_op1 = llvm2si_translate_value(op1, function);
	arg_op2 = llvm2si_translate_value(op2, function);

	/* Check argument types */
	enum si2bin_arg_type_t arg_types[3] = { si2bin_arg_vector_register,
			si2bin_arg_scalar_register, si2bin_arg_literal };
	si2bin_arg_valid_types(arg_op1, arg_types, 3, __FUNCTION__);
	si2bin_arg_valid_types(arg_op2, arg_types, 3, __FUNCTION__);

	/* Combination const-const not allowed */
	if (arg_op1->type == si2bin_arg_literal &&
			arg_op2->type == si2bin_arg_literal)
		fatal("%s: two literals not allowed", __FUNCTION__);
	
	/* Create symbol and add to function symbol table */
	assert(alloca_inst->hasName());
	symbol = llvm2si_symbol_create(add_inst->getName().data());
	llvm2si_symbol_table_add_symbol(function->symbol_table, symbol);

	/* Allocate a new vector register for the result */
	symbol->vreg = function->num_vregs;
	function->num_vregs++;

	/* Combinations sreg-vreg, vreg-sreg, vreg-vreg, vreg-const, const-vreg
	 * generate a vector instruction. These are the combinations that have
	 * at least one operand being a vector register. Combination vreg-sreg
	 * needs to be swapped first for proper encoding. */
	if (arg_op1->type == si2bin_arg_vector_register &&
			arg_op2->type == si2bin_arg_scalar_register)
		si2bin_arg_swap(&arg_op1, &arg_op2);
	if (arg_op1->type == si2bin_arg_vector_register ||
			arg_op2->type == si2bin_arg_vector_register)
	{
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(symbol->vreg));
		list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
		list_add(arg_list, arg_op1);
		list_add(arg_list, arg_op2);
		inst = si2bin_inst_create(SI_INST_V_ADD_I32, arg_list);
		llvm2si_basic_block_add_inst(basic_block, inst);
		return;
	}

	/* Combinations not supported */
	fatal("%s: argument types not supported (%s, %s)",
			__FUNCTION__, str_map_value(&si2bin_arg_type_map, arg_op1->type),
			str_map_value(&si2bin_arg_type_map, arg_op2->type));
}


static void llvm2si_translate_alloca_inst(AllocaInst *alloca_inst,
		struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block)
{
	Value *ArraySize;
	ConstantInt *ArraySizeConst;

	Type *AllocatedType;
	IntegerType *AllocatedIntType;

	int num_elem;
	int elem_size;
	int total_size;

	struct llvm2si_symbol_t *symbol;
	struct list_t *arg_list;
	struct si2bin_inst_t *inst;

	/* Get number of elements to allocate */
	ArraySize = alloca_inst->getArraySize();
	ArraySizeConst = dynamic_cast<ConstantInt*>(ArraySize);
	if (!ArraySizeConst)
		fatal("%s: alloca: number of elements must be constant",
			__FUNCTION__);
	num_elem = ArraySizeConst->getZExtValue();

	/* Get size of each element */
	AllocatedType = alloca_inst->getAllocatedType();
	if (AllocatedType->isPointerTy())
	{
		elem_size = 4;
	}
	else if (AllocatedType->isIntegerTy())
	{
		AllocatedIntType = reinterpret_cast<IntegerType*>(AllocatedType);
		elem_size = AllocatedIntType->getBitWidth();
		if (elem_size % 8)
			fatal("%s: alloca: invalid integer size",
				__FUNCTION__);
		elem_size /= 8;
	}
	else
	{
		fatal("%s: alloca: invalid element type",
			__FUNCTION__);
	}

	/* Total size */
	total_size = num_elem * elem_size;
	if (!total_size)
		fatal("%s: alloca: invalid total size",
			__FUNCTION__);

	/* Create symbol and add to function symbol table */
	assert(alloca_inst->hasName());
	symbol = llvm2si_symbol_create(alloca_inst->getName().data());
	llvm2si_symbol_table_add_symbol(function->symbol_table, symbol);

	/* Allocate a new vector register. This register will point to
	 * the current stack top, and will be associated with the symbol
	 * returned by the 'alloca' function. */
	symbol->vreg = function->num_vregs;
	function->num_vregs++;

	/* Emit instruction to store top of the stack.
	 * v_mov_b32 v[vreg], v[sp]
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(symbol->vreg));
	list_add(arg_list, si2bin_arg_create_vector_register(function->vreg_sp));
	inst = si2bin_inst_create(SI_INST_V_MOV_B32, arg_list);
	llvm2si_basic_block_add_inst(basic_block, inst);

	/* Emit instruction to increment stack pointer.
	 * v_add_i32 v[sp], vcc, v[sp], total_size
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(function->vreg_sp));
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
	list_add(arg_list, si2bin_arg_create_vector_register(function->vreg_sp));
	list_add(arg_list, si2bin_arg_create_literal(total_size));
	inst = si2bin_inst_create(SI_INST_V_ADD_I32, arg_list);
	llvm2si_basic_block_add_inst(basic_block, inst);
}


static void llvm2si_translate_call_inst(CallInst *call_inst,
		struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block)
{
	Function *CalledFunction;

	const char *func_name;

	/* Get function name */
	CalledFunction = call_inst->getCalledFunction();
	func_name = CalledFunction->getName().data();

	/* Built-in functions */
	if (!strcmp(func_name, "get_global_id"))
	{
		Value *ArgOperand;
		ConstantInt *ArgOperandConst;

		struct llvm2si_symbol_t *symbol;

		int dim;

		/* Check number of arguments */
		if (call_inst->getNumArgOperands() != 1)
			fatal("%s: %s: invalid number of arguments",
				__FUNCTION__, func_name);

		/* Argument must be an integer constat */
		ArgOperand = call_inst->getArgOperand(0);
		ArgOperandConst = dynamic_cast<ConstantInt*>(ArgOperand);
		if (!ArgOperandConst)
			fatal("%s: %s: argument must be integer constant",
				__FUNCTION__, func_name);

		/* Get dimension */
		dim = ArgOperandConst->getZExtValue();
		if (!IN_RANGE(dim, 0, 2))
			fatal("%s: %s: invalid range for argument",
				__FUNCTION__, func_name);

		/* Create new symbol */
		assert(call_inst->hasName());
		symbol = llvm2si_symbol_create(call_inst->getName().data());
		llvm2si_symbol_table_add_symbol(function->symbol_table, symbol);

		/* Associate symbol with vector register containing the global
		 * ID in the specified dimension. */
		symbol->vreg = function->vreg_gid + dim;
	}
	else
	{
		fatal("%s: %s: invalid built-in function",
			__FUNCTION__, func_name);
	}
}


static void llvm2si_translate_getelementptr_inst(GetElementPtrInst *getelementptr_inst,
		struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block)
{
	Value *PointerOperand;
	Value *IndexOperand;

	struct si2bin_arg_t *pointer_arg;
	struct si2bin_arg_t *index_arg;
	struct si2bin_inst_t *inst;
	struct llvm2si_symbol_t *effaddr_symbol;
	struct list_t *arg_list;

	const char *effaddr_name;

	/* Get pointer operand */
	PointerOperand = getelementptr_inst->getPointerOperand();
	pointer_arg = llvm2si_translate_value(PointerOperand, function);

	/* Check valid type of pointer argument */
	enum si2bin_arg_type_t pointer_arg_types[3] = { si2bin_arg_vector_register,
			si2bin_arg_scalar_register, si2bin_arg_literal };
	si2bin_arg_valid_types(pointer_arg, pointer_arg_types, 3, __FUNCTION__);

	/* Check that there is only one index */
	if (getelementptr_inst->getNumIndices() != 1)
		fatal("%s: only supported for 1 index", __FUNCTION__);

	/* Get index operand */
	IndexOperand = *getelementptr_inst->idx_begin();
	index_arg = llvm2si_translate_value(IndexOperand, function);

	/* Check valid types of index argument */
	enum si2bin_arg_type_t index_arg_types[1] = { si2bin_arg_vector_register };
	si2bin_arg_valid_types(index_arg, index_arg_types, 1, __FUNCTION__);

	/* Create symbol for effective address */
	effaddr_name = getelementptr_inst->getName().data();
	effaddr_symbol = llvm2si_symbol_create(effaddr_name);
	llvm2si_symbol_table_add_symbol(function->symbol_table, effaddr_symbol);

	/* Allocate a vector register for effective address */
	effaddr_symbol->vreg = function->num_vregs;
	function->num_vregs++;

	/* Emit effective address calculation (addition) */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(effaddr_symbol->vreg));
	list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
	list_add(arg_list, pointer_arg);
	list_add(arg_list, index_arg);
	inst = si2bin_inst_create(SI_INST_V_ADD_I32, arg_list);
	llvm2si_basic_block_add_inst(basic_block, inst);
}


static void llvm2si_translate_load_inst(LoadInst *load_inst,
		struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block)
{
	int address_space;
	int sreg_uav;
	int size;

	Value *PointerOperand;
	Type *PointerOperandType;
	Type *PointedType;
	IntegerType *PointedTypeInt;

	const char *pointer_name;
	const char *value_name;

	struct llvm2si_symbol_t *pointer_symbol;
	struct llvm2si_symbol_t *value_symbol;
	struct si2bin_arg_t *arg_soffset;
	struct si2bin_arg_t *arg_qual;
	struct si2bin_inst_t *inst;
	struct list_t *arg_list;

	/* Get address space */
	address_space = load_inst->getPointerAddressSpace();
	switch (address_space)
	{
	case 0:
		/* Private memory */
		sreg_uav = function->sreg_uav10;
		break;
	case 1:
		/* Global memory */
		sreg_uav = function->sreg_uav11;
		break;
	default:
		fatal("%s: invalid address space (%d)",
			__FUNCTION__, address_space);
	}

	/* Get pointer operand (address) */
	PointerOperand = load_inst->getPointerOperand();
	if (!PointerOperand->hasName())
		fatal("%s: pointer operand not a variable",
			__FUNCTION__);

	/* Get pointer type */
	PointerOperandType = PointerOperand->getType();
	if (!PointerOperandType->isPointerTy())
		fatal("%s: pointer operand not a pointer",
			__FUNCTION__);

	/* Get pointed type */
	if (PointerOperandType->getNumContainedTypes() != 1)
		fatal("%s: multiple sub-types not supported",
			__FUNCTION__);
	PointedType = PointerOperandType->getContainedType(0);

	/* Get size of pointed type */
	if (PointedType->isPointerTy())
	{
		size = 4;
	}
	else if (PointedType->isIntegerTy())
	{
		PointedTypeInt = reinterpret_cast<IntegerType*>(PointedType);
		size = PointedTypeInt->getBitWidth();
		if (size % 8)
			fatal("%s: invalid integer size", __FUNCTION__);

		/* Get size in bytes */
		size /= 8;
		if (size != 4)
			fatal("%s: only size 4 supported", __FUNCTION__);
	}
	else
	{
		size = 0;
		fatal("%s: invalid element type", __FUNCTION__);
	}

	/* Get pointer symbol */
	pointer_name = PointerOperand->getName().data();
	pointer_symbol = llvm2si_symbol_table_lookup(function->symbol_table, pointer_name);
	if (!pointer_symbol)
		fatal("%s: %s: invalid symbol", __FUNCTION__, pointer_name);

	/* Create value symbol */
	value_name = load_inst->getName().data();
	value_symbol = llvm2si_symbol_create(value_name);
	llvm2si_symbol_table_add_symbol(function->symbol_table, value_symbol);

	/* Allocate new vector register */
	value_symbol->vreg = function->num_vregs;
	function->num_vregs++;

	/* Emit memory load instruction.
	 * tbuffer_load_format_x v[value_symbol->vreg], v[pointer_symbol->vreg],
	 * 	s[sreg_uav,sreg_uav+3], 0 offen format:[BUF_DATA_FORMAT_32,BUF_NUM_FORMAT_FLOAT]
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(value_symbol->vreg));
	list_add(arg_list, si2bin_arg_create_vector_register(pointer_symbol->vreg));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(sreg_uav, sreg_uav + 3));
	arg_soffset = si2bin_arg_create_literal(0);
	arg_qual = si2bin_arg_create_maddr_qual();
	list_add(arg_list, si2bin_arg_create_maddr(arg_soffset, arg_qual,
			si_inst_buf_data_format_32, si_inst_buf_num_format_float));
	inst = si2bin_inst_create(SI_INST_TBUFFER_LOAD_FORMAT_X, arg_list);
	llvm2si_basic_block_add_inst(basic_block, inst);
}


static void llvm2si_translate_store_inst(StoreInst *store_inst,
		struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block)
{
	int address_space;
	int sreg_uav;

	Value *ValueOperand;
	Value *PointerOperand;
	Type *ValueOperandType;
	IntegerType *ValueOperandTypeInt;

	const char *value_name;
	const char *pointer_name;

	struct llvm2si_symbol_t *value_symbol;
	struct llvm2si_symbol_t *pointer_symbol;
	struct si2bin_arg_t *arg_soffset;
	struct si2bin_arg_t *arg_qual;
	struct si2bin_inst_t *inst;
	struct list_t *arg_list;

	int size;

	/* Get address space */
	address_space = store_inst->getPointerAddressSpace();
	switch (address_space)
	{
	case 0:
		/* Private memory */
		sreg_uav = function->sreg_uav10;
		break;
	case 1:
		/* Global memory */
		sreg_uav = function->sreg_uav11;
		break;
	default:
		fatal("%s: invalid address space (%d)",
			__FUNCTION__, address_space);
	}

	/* Get value operand */
	ValueOperand = store_inst->getValueOperand();
	if (!ValueOperand->hasName())
		fatal("%s: value operand not a variable",
			__FUNCTION__);

	/* Get pointer operand (address) */
	PointerOperand = store_inst->getPointerOperand();
	if (!PointerOperand->hasName())
		fatal("%s: pointer operand not a variable",
			__FUNCTION__);

	/* Get size of the value */
	ValueOperandType = ValueOperand->getType();
	if (ValueOperandType->isPointerTy())
	{
		size = 4;
	}
	else if (ValueOperandType->isIntegerTy())
	{
		ValueOperandTypeInt = reinterpret_cast<IntegerType*>(ValueOperandType);
		size = ValueOperandTypeInt->getBitWidth();
		if (size % 8)
			fatal("%s: invalid integer size", __FUNCTION__);

		/* Get size in bytes */
		size /= 8;
		if (size != 4)
			fatal("%s: only size 4 supported", __FUNCTION__);
	}
	else
	{
		size = 0;
		fatal("%s: invalid element type", __FUNCTION__);
	}

	/* Get value symbol */
	value_name = ValueOperand->getName().data();
	value_symbol = llvm2si_symbol_table_lookup(function->symbol_table, value_name);
	if (!value_symbol)
		fatal("%s: %s: invalid symbol", __FUNCTION__, value_name);

	/* Get pointer symbol */
	pointer_name = PointerOperand->getName().data();
	pointer_symbol = llvm2si_symbol_table_lookup(function->symbol_table, pointer_name);
	if (!pointer_symbol)
		fatal("%s: %s: invalid symbol", __FUNCTION__, pointer_name);

	/* Emit memory write.
	 * tbuffer_store_format_x v[value_symbol->vreg], s[pointer_symbol->vreg],
	 * 	s[sreg_uav,sreg_uav+3], 0 offen format:[BUF_DATA_FORMAT_32,
	 * 	BUF_NUM_FORMAT_FLOAT]
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_vector_register(value_symbol->vreg));
	list_add(arg_list, si2bin_arg_create_vector_register(pointer_symbol->vreg));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(sreg_uav, sreg_uav + 3));
	arg_soffset = si2bin_arg_create_literal(0);
	arg_qual = si2bin_arg_create_maddr_qual();
	list_add(arg_list, si2bin_arg_create_maddr(arg_soffset, arg_qual,
			si_inst_buf_data_format_32, si_inst_buf_num_format_float));
	inst = si2bin_inst_create(SI_INST_TBUFFER_STORE_FORMAT_X, arg_list);
	llvm2si_basic_block_add_inst(basic_block, inst);
}


static void llvm2si_translate_inst(Instruction *inst,
		struct llvm2si_function_t *function,
		struct llvm2si_basic_block_t *basic_block)
{
	assert(basic_block->function == function);
	switch (inst->getOpcode())
	{

	case Instruction::Alloca:
	{
		AllocaInst *alloca_inst = dynamic_cast<AllocaInst*>(inst);
		assert(alloca_inst);
		llvm2si_translate_alloca_inst(alloca_inst, function, basic_block);
		break;
	}

	case Instruction::Add:
	{
		BinaryOperator *add_inst = dynamic_cast<BinaryOperator*>(inst);
		assert(add_inst);
		llvm2si_translate_add_inst(add_inst, function, basic_block);
		break;
	}

	case Instruction::Call:
	{
		CallInst *call_inst = dynamic_cast<CallInst*>(inst);
		assert(call_inst);
		llvm2si_translate_call_inst(call_inst, function, basic_block);
		break;
	}

	case Instruction::GetElementPtr:
	{
		GetElementPtrInst *getelementptr_inst =
				dynamic_cast<GetElementPtrInst*>(inst);
		assert(getelementptr_inst);
		llvm2si_translate_getelementptr_inst(getelementptr_inst,
				function, basic_block);
		break;
	}

	case Instruction::Load:
	{
		LoadInst *load_inst = dynamic_cast<LoadInst*>(inst);
		assert(load_inst);
		llvm2si_translate_load_inst(load_inst, function, basic_block);
		break;
	}

	case Instruction::Store:
	{
		StoreInst *store_inst = dynamic_cast<StoreInst*>(inst);
		assert(store_inst);
		llvm2si_translate_store_inst(store_inst, function, basic_block);
		break;
	}

	default:
		warning("%s: %s: unsupported LLVM instruction",
			__FUNCTION__, inst->getOpcodeName());
	}
}




/*
 * Class for LLVM Pass
 */

class llvm2si_translate_pass_t : public FunctionPass
{
public:
	char ID;
	llvm2si_translate_pass_t();
	virtual bool runOnFunction(Function &f);
};


llvm2si_translate_pass_t::llvm2si_translate_pass_t() : FunctionPass(ID)
{
}


bool llvm2si_translate_pass_t::runOnFunction(Function &llvm_func)
{
	struct llvm2si_function_t *function;
	struct llvm2si_function_arg_t *function_arg;
	struct llvm2si_basic_block_t *basic_block;

	const char *func_name;

	/* Skip special functions */
	func_name = llvm_func.getName().data();
	if (!strcmp(func_name, "get_work_dim")
			|| !strcmp(func_name, "get_global_size")
			|| !strcmp(func_name, "get_global_id")
			|| !strcmp(func_name, "get_local_size")
			|| !strcmp(func_name, "get_local_id")
			|| !strcmp(func_name, "get_num_groups")
			|| !strcmp(func_name, "get_group_id")
			|| !strcmp(func_name, "get_global_offset"))
		return false;

	/* Create function */
	function = llvm2si_function_create(func_name);

	/* Create a basic block and generate header code in it */
	basic_block = llvm2si_basic_block_create(func_name);
	llvm2si_function_add_basic_block(function, basic_block);
	llvm2si_function_gen_header(function, basic_block);

	/* Add function arguments and generate code to load them */
	for (Function::arg_iterator llvm_arg = llvm_func.arg_begin(), llvm_arg_end = llvm_func.arg_end();
			llvm_arg != llvm_arg_end; llvm_arg++)
		llvm2si_translate_arg(llvm_arg, function, basic_block);

	/* Basic blocks in function */
	for (Function::iterator llvm_basic_block = llvm_func.begin(),
			llvm_basic_block_end = llvm_func.end();
			llvm_basic_block != llvm_basic_block_end; llvm_basic_block++)
	{
		for (BasicBlock::iterator llvm_inst = llvm_basic_block->begin(),
				llvm_inst_end = llvm_basic_block->end();
				llvm_inst != llvm_inst_end; llvm_inst++)
			llvm2si_translate_inst(llvm_inst, function, basic_block);
	}

	/* Free function. This takes care of freeing all basic blocks and
	 * instructions added to the function. */
	llvm2si_function_dump(function, llvm2si_outf);
	llvm2si_function_free(function);

	/* Module not modified, return false */
	return false;
}



/*
 * Public Functions
 */

extern "C" void llvm2si_translate(char *source_file, char *output_file)
{
	OwningPtr<MemoryBuffer> buffer;
	LLVMContext context;
	Module *module;
	string err_msg;
	
	/* Read bitcode in 'source_file' */
	if (MemoryBuffer::getFile(source_file, buffer))
	{
		cerr << "fatal: " << source_file << ": error loading file\n";
		exit(1);
	}

	/* Load module */
	module = ParseBitcodeFile(buffer.get(), context, &err_msg);
	if (!module)
	{
		cerr << "fatal: " << source_file << ": " << err_msg << "\n";
		exit(1);
	}
	
	/* Open output file */
	llvm2si_outf = fopen(output_file, "w");
	if (!llvm2si_outf)
		fatal("%s: cannot open output file", output_file);

	/* Apply pass */
	PassManager pm;
	pm.add(new llvm2si_translate_pass_t());
	pm.run(*module);

	/* Close output file */
	fclose(llvm2si_outf);
}
