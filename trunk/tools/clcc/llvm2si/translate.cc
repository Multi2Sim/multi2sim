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
#include <llvm/Function.h>
#include <llvm/LLVMContext.h>
#include <llvm/Pass.h>
#include <llvm/PassManager.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/system_error.h>


/* C includes */
extern "C"
{

#include <stdlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <clcc/llvm2si/basic-block.h>
#include <clcc/llvm2si/function.h>
#include <clcc/si2bin/arg.h>
#include <clcc/si2bin/inst.h>

#include "translate.h"
}


using namespace llvm;
using namespace std;

/* Private variables */
static FILE *llvm2si_outf;




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
	function = llvm2si_function_create(f.getName().data());

	/* Create a basic block and generate header code in it */
	basic_block = llvm2si_basic_block_create("entry");
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
		outs() << "\tBasic block '" << bb->getName() << "'\n";
		for (BasicBlock::iterator inst = bb->begin(), inst_end = bb->end();
				inst != inst_end; inst++)
		{
			outs() << "\t\t" << *inst << "\n";
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
