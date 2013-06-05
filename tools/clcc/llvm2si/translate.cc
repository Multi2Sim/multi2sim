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
		fatal("%s: store: value operand not a variable",
			__FUNCTION__);

	/* Get pointer operand (address) */
	PointerOperand = store_inst->getPointerOperand();
	if (!PointerOperand->hasName())
		fatal("%s: store: pointer operand not a variable",
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
			fatal("%s: store: invalid integer size",
				__FUNCTION__);

		/* Get size in bytes */
		size /= 8;
		if (size != 4)
			fatal("%s: store: only size 4 supported",
				__FUNCTION__);
	}
	else
	{
		size = 0;
		fatal("%s: store: invalid element type",
			__FUNCTION__);
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
	/* FIXME - better representation needed for 'maddr' field */
	list_add(arg_list, si2bin_arg_create_maddr(arg_soffset, arg_qual,
			"BUF_DATA_FORMAT_32", "BUF_NUM_FORMAT_FLOAT"));
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

	case Instruction::Call:
	{
		CallInst *call_inst = dynamic_cast<CallInst*>(inst);
		assert(call_inst);
		llvm2si_translate_call_inst(call_inst, function, basic_block);
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


bool llvm2si_translate_pass_t::runOnFunction(Function &f)
{
	struct llvm2si_function_t *function;
	struct llvm2si_function_arg_t *arg;
	struct llvm2si_basic_block_t *basic_block;
	const char *name;

	/* Skip special functions */
	name = f.getName().data();
	if (!strcmp(name, "get_work_dim")
			|| !strcmp(name, "get_global_size")
			|| !strcmp(name, "get_global_id")
			|| !strcmp(name, "get_local_size")
			|| !strcmp(name, "get_local_id")
			|| !strcmp(name, "get_num_groups")
			|| !strcmp(name, "get_group_id")
			|| !strcmp(name, "get_global_offset"))
		return false;

	/* Create function */
	function = llvm2si_function_create(name);

	/* Create a basic block and generate header code in it */
	basic_block = llvm2si_basic_block_create(name);
	llvm2si_function_add_basic_block(function, basic_block);
	llvm2si_function_gen_header(function, basic_block);

	/* Add function arguments and generate code to load them */
	for (Function::arg_iterator farg = f.arg_begin(), farg_end = f.arg_end();
			farg != farg_end; farg++)
	{
		arg = llvm2si_function_arg_create();
		llvm2si_function_add_arg(function, arg, basic_block);
	}

	/* Basic blocks in function */
	for (Function::iterator bb = f.begin(), bb_end = f.end();
			bb != bb_end; bb++)
	{
		for (BasicBlock::iterator inst = bb->begin(), inst_end = bb->end();
				inst != inst_end; inst++)
		{
			llvm2si_translate_inst(inst, function, basic_block);
		}
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
