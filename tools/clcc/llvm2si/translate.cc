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

#include <iostream>
#include <stdlib.h>

#include <llvm/ADT/OwningPtr.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Function.h>
#include <llvm/LLVMContext.h>
#include <llvm/Pass.h>
#include <llvm/PassManager.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/system_error.h>

#include "translate.h"


using namespace llvm;
using namespace std;


class si2llvm_translate_pass_t : public FunctionPass
{
public:
	char ID;
	si2llvm_translate_pass_t();
	virtual bool runOnFunction(Function &f);
};


si2llvm_translate_pass_t::si2llvm_translate_pass_t() : FunctionPass(ID)
{
}


bool si2llvm_translate_pass_t::runOnFunction(Function &f)
{
	outs() << "Function '" << f.getName() << "'\n";
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
	return false;
}


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
	
	/* Apply pass */
	PassManager pm;
	pm.add(new si2llvm_translate_pass_t());
	pm.run(*module);
}

