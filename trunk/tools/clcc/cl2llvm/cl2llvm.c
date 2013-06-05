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
#include <llvm-c/BitWriter.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>

#include "arg.h"
#include "type.h"
#include "declarator-list.h"
#include "function.h"
#include "symbol.h"
#include "cl2llvm.h"
#include "parser.h"

extern void cl2llvm_set_lineno(int);
extern int cl2llvm_col_num;
extern int temp_var_count;
extern int block_count;

/* Global variables */
LLVMBuilderRef cl2llvm_builder;
LLVMModuleRef cl2llvm_module;

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
}



void cl2llvm_init_global_vars(void)
{
	LLVMLinkInJIT();
	LLVMInitializeNativeTarget();
	
	cl2llvm_set_lineno(0);
	cl2llvm_col_num = 0;
	temp_var_count = 0;
	block_count = 0;
	
	/* Initialize LLVM */
	cl2llvm_builder = LLVMCreateBuilder();
	cl2llvm_module = LLVMModuleCreateWithName("module");

	/* Initialize global symbol table */
	cl2llvm_symbol_table = hash_table_create(10, 1);

	/*Declare get_global_id*/
	
	/*Arguments*/
	LLVMTypeRef args_get_global_id_array[] = {LLVMInt32Type()};
	struct list_t *args_get_global_id = list_create();
	struct cl2llvm_decl_list_t *ggi_dimindex_decl = cl2llvm_decl_list_create();
	ggi_dimindex_decl->type_spec = cl2llvm_type_create_w_init(LLVMInt32Type(), 0);
	struct cl2llvm_arg_t *ggi_dimindex_arg1 = cl2llvm_arg_create(ggi_dimindex_decl, "dimindex");
	list_add(args_get_global_id, ggi_dimindex_arg1);

	/*Function*/
	struct cl2llvm_function_t *cl2llvm_get_global_id = cl2llvm_function_create("get_global_id",
		args_get_global_id);
	
	 cl2llvm_get_global_id->func_type = LLVMFunctionType( LLVMInt32Type(),
	 	args_get_global_id_array, 1, 0);
 	cl2llvm_get_global_id->func = LLVMAddFunction(cl2llvm_module, "get_global_id",
		cl2llvm_get_global_id->func_type);
	cl2llvm_get_global_id->sign = 1;
	LLVMSetFunctionCallConv(cl2llvm_get_global_id->func, LLVMCCallConv);

	/*Insert function in global symbol table*/
	hash_table_insert(cl2llvm_symbol_table, "get_global_id", cl2llvm_get_global_id);

	/*free pointers*/
	cl2llvm_decl_list_struct_free(ggi_dimindex_decl);
}


void cl2llvm_done(void)
{
}

void cl2llvm_erase_global_vars(void)
{
	char *name;
	struct cl2llvm_function_t *function;

	/* Free symbol table */
	HASH_TABLE_FOR_EACH(cl2llvm_symbol_table, name, function)
		cl2llvm_function_free(function);
	hash_table_free(cl2llvm_symbol_table);
}


void cl2llvm_compile(struct list_t *source_file_list, struct list_t *llvm_file_list)
{
	int index;
	char *error = NULL;
	char *llvm_file_name;

	LIST_FOR_EACH(source_file_list, index)
	{
		cl2llvm_init_global_vars();
		/* Open file */
		cl2llvm_file_name = list_get(source_file_list, index);
		cl2llvm_yyin = fopen(cl2llvm_file_name, "rb");
		if (!cl2llvm_yyin)
			fatal("%s: cannot open file", cl2llvm_file_name);

		/* Compile */
		extern int cl2llvm_yydebug;
		/*initialize yydebug to 1 for debug information from bison*/
		cl2llvm_yydebug = 0; ////
		cl2llvm_yyparse();
	
		/*Verify module and dump bit code to file*/
		llvm_file_name = list_get(llvm_file_list, index);
		LLVMVerifyModule(cl2llvm_module, LLVMAbortProcessAction, &error);
		LLVMWriteBitcodeToFile(cl2llvm_module, llvm_file_name);
		LLVMDisposeMessage(error); // Handler == LLVMAbortProcessAction -> No need to check errors


		/* Close */
		fclose(cl2llvm_yyin);

		cl2llvm_erase_global_vars();
	}
	

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

