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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>

#include "symbol.h"
#include "cl2llvm.h"
#include "parser.h"


/* Global variables */
LLVMBuilderRef cl2llvm_builder;
LLVMModuleRef cl2llvm_module;
LLVMValueRef cl2llvm_function;
LLVMBasicBlockRef cl2llvm_basic_block;

/* Current file being compiled */
char *cl2llvm_file_name;


void cl2llvm_yyerror(char *s)
{
	printf("%s:%d:%d: error: %s\n", cl2llvm_file_name, cl2llvm_yyget_lineno(),
			cl2llvm_get_col_num(), s);
	exit(1);
}


void cl2llvm_yyerror_fmt(char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "%s:%d:%d: error: ", cl2llvm_file_name,
			cl2llvm_yyget_lineno(), cl2llvm_get_col_num());
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	fflush(NULL);
	exit(1);
}






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
	char *name;
	struct cl2llvm_symbol_t *symbol;

	/* Free symbol table */
	HASH_TABLE_FOR_EACH(cl2llvm_symbol_table, name, symbol)
		cl2llvm_symbol_free(symbol);
	hash_table_free(cl2llvm_symbol_table);
}


void cl2llvm_compile(struct list_t *source_file_list, struct list_t *llvm_file_list)
{
	int index;
	char *error = NULL;

	LIST_FOR_EACH(source_file_list, index)
	{
		/* Open file */
		cl2llvm_file_name = list_get(source_file_list, index);
		cl2llvm_yyin = fopen(cl2llvm_file_name, "rb");
		if (!cl2llvm_yyin)
			fatal("%s: cannot open file", cl2llvm_file_name);

		/* Compile */
		cl2llvm_yyparse();

		/* Close */
		fclose(cl2llvm_yyin);
	}
	
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

