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

#include <clcc/si2bin/arg.h>
#include <clcc/si2bin/inst.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "basic-block.h"
#include "function.h"
#include "symbol.h"
#include "symbol-table.h"



/*
 * Private Functions
 */

void llvm2si_basic_block_emit_add(struct llvm2si_basic_block_t *basic_block,
		LLVMValueRef llinst)
{
	fatal("%s: not supported", __FUNCTION__);
}


void llvm2si_basic_block_emit_call(struct llvm2si_basic_block_t *basic_block,
		LLVMValueRef llinst)
{
	LLVMValueRef llfunction;
	LLVMValueRef llarg;
	LLVMTypeRef lltype;
	LLVMTypeKind lltype_kind;

	struct llvm2si_function_t *function;
	struct llvm2si_symbol_t *symbol;

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
		symbol = llvm2si_symbol_create(var_name,
				llvm2si_symbol_type_vector_register,
				function->vreg_gid + dim);

		/* Add to symbol table */
		llvm2si_symbol_table_add_symbol(function->symbol_table, symbol);
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
	LLVMValueRef llarg_pointer;
	//LLVMValueRef llarg_index;

	struct llvm2si_function_t *function;
	struct si2bin_arg_t *arg_pointer;

	/* Get function */
	function = basic_block->function;
	assert(function);

	/* Get pointer operand */
	llarg_pointer = LLVMGetOperand(llinst, 0);
	arg_pointer = llvm2si_function_translate_value(function, llarg_pointer);

	si2bin_arg_dump(arg_pointer, stdout);
#if 0
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
#endif
}


void llvm2si_basic_block_emit_load(struct llvm2si_basic_block_t *basic_block,
		LLVMValueRef llinst)
{
	fatal("%s: not supported", __FUNCTION__);
}


void llvm2si_basic_block_emit_ret(struct llvm2si_basic_block_t *basic_block,
		LLVMValueRef llinst)
{
	fatal("%s: not supported", __FUNCTION__);
}


void llvm2si_basic_block_emit_store(struct llvm2si_basic_block_t *basic_block,
		LLVMValueRef llinst)
{
	fatal("%s: not supported", __FUNCTION__);
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

	/* Name */
	basic_block->name = str_set(basic_block->name, "");
	if (llbb)
	{
		/* Get name */
		llbb_as_value = LLVMBasicBlockAsValue(llbb);
		name = (char *) LLVMGetValueName(llbb_as_value);
		basic_block->name = str_set(basic_block->name, name);
	}

	/* Return */
	return basic_block;
}


void llvm2si_basic_block_free(struct llvm2si_basic_block_t *basic_block)
{
	/* Free list of instructions */
	LINKED_LIST_FOR_EACH(basic_block->inst_list)
		si2bin_inst_free(linked_list_get(basic_block->inst_list));
	linked_list_free(basic_block->inst_list);
	
	/* Rest */
	str_free(basic_block->name);
	free(basic_block);
}


void llvm2si_basic_block_dump(struct llvm2si_basic_block_t *basic_block, FILE *f)
{
	struct si2bin_inst_t *inst;

	/* Label with basic block name if not empty */
	if (*basic_block->name)
		fprintf(f, "%s:\n", basic_block->name);

	/* Print list of instructions */
	LINKED_LIST_FOR_EACH(basic_block->inst_list)
	{
		inst = linked_list_get(basic_block->inst_list);
		si2bin_inst_dump_assembly(inst, f);
	}

	/* End */
	fprintf(f, "\n");
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
