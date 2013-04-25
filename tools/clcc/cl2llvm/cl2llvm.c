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

#include <stdio.h>
#include <stdlib.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>

#include "cl2llvm.h"
#include "parser.h"


/* Global variables */
LLVMBuilderRef cl2llvm_builder;
LLVMModuleRef cl2llvm_module;
LLVMValueRef cl2llvm_function;
LLVMBasicBlockRef cl2llvm_basic_block;


void cl2llvm_init(void)
{
	LLVMLinkInJIT();
	LLVMInitializeNativeTarget();
	
	/* Initialize LLVM */
	cl2llvm_builder = LLVMCreateBuilder();
	cl2llvm_module = LLVMModuleCreateWithName("module");

	/* Initialize symbol table */
	cl2llvm_symbol_table = hash_table_create(10, 1);

}


void cl2llvm_done(void)
{
	char *error = NULL;

	/* Free symbol table */
	hash_table_free(cl2llvm_symbol_table);

	LLVMDumpModule(cl2llvm_module);
	LLVMVerifyModule(cl2llvm_module, LLVMAbortProcessAction, &error);
	LLVMDisposeMessage(error); // Handler == LLVMAbortProcessAction -> No need to check errors


	LLVMExecutionEngineRef engine;
	LLVMModuleProviderRef provider = LLVMCreateModuleProviderForExistingModule(cl2llvm_module);
	error = NULL;
	if (LLVMCreateJITCompiler(&engine, provider, 2, &error) != 0)
	{
		fprintf(stderr, "%s\n", error);
		LLVMDisposeMessage(error);
		abort();
	}

	LLVMPassManagerRef pass = LLVMCreatePassManager();
	LLVMAddTargetData(LLVMGetExecutionEngineTargetData(engine), pass);

	LLVMDisposePassManager(pass);
	LLVMDisposeBuilder(cl2llvm_builder);
	LLVMDisposeExecutionEngine(engine);
}


void cl2llvm_compile(struct list_t *source_file_list, struct list_t *llvm_file_list)
{
	char *file_name;
	int index;

	LIST_FOR_EACH(source_file_list, index)
	{
		/* Open file */
		file_name = list_get(source_file_list, index);
		yyin = fopen(file_name, "rb");
		if (!yyin)
			fatal("%s: cannot open file", file_name);

		/* Compile */
		yyparse();

		/* Close */
		fclose(yyin);
	}
}

