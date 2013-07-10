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

#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>

#include "enum-types.h"
#include "arg.h"
#include "cl2llvm.h"
#include "declarator-list.h"
#include "function.h"
#include "parser.h"
#include "symbol.h"
#include "type.h"
#include "built-in-funcs.h"

extern void cl2llvm_set_lineno(int);
extern int cl2llvm_col_num;
extern int temp_var_count;
extern int block_count;

/* Global variables */
LLVMBuilderRef cl2llvm_builder;
LLVMModuleRef cl2llvm_module;

/* Built-in function table */
struct hash_table_t *cl2llvm_built_in_func_table;

/* Built in constants table */
struct hash_table_t *cl2llvm_built_in_const_table;

/* Global variable_symbol_table */
struct hash_table_t *cl2llvm_global_symbol_table;

/* Current file being compiled */
char *cl2llvm_file_name;

/* Preprocessor file list */
struct list_t *cl2llvm_preprcr_file_list;


void cl2llvm_yyerror(char *s)
{
	printf("%s:%d:%d: error: %s\n", cl2llvm_file_name,
		cl2llvm_yyget_lineno(), cl2llvm_get_col_num(), s);
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

void cl2llvm_warning(char *s)
{
	printf("%s:%d:%d: warning: %s\n", cl2llvm_file_name,
		cl2llvm_yyget_lineno(), cl2llvm_get_col_num(), s);
}

void cl2llvm_init(void)
{
	/* Initialize built in function table */
	cl2llvm_built_in_func_table = built_in_func_table_create();

	/* Initialize enumerated type table */
	cl2llvm_built_in_const_table = cl2llvm_built_in_const_table_create();
}



void cl2llvm_init_global_vars(void)
{
	LLVMLinkInJIT();
	LLVMInitializeNativeTarget();

	cl2llvm_set_lineno(1);
	cl2llvm_col_num = 0;
	temp_var_count = 0;
	block_count = 0;

	/* Initialize LLVM */
	cl2llvm_builder = LLVMCreateBuilder();
	cl2llvm_module = LLVMModuleCreateWithName("module");

	/* Initialize preprocessor file list */
	cl2llvm_preprcr_file_list = list_create();

	/* Initialize global function table */
	cl2llvm_symbol_table = hash_table_create(10, 1);

	/* Initialize global symbol table */
	cl2llvm_global_symbol_table = hash_table_create(10, 1);

}


void cl2llvm_done(void)
{
	char *name;
	int *intptr;

	/* Free built-in function table. */
	HASH_TABLE_FOR_EACH(cl2llvm_built_in_func_table, name, intptr)
		free(intptr);
	hash_table_free(cl2llvm_built_in_func_table);
	
	/* Free enumerated type table */
	hash_table_free(cl2llvm_built_in_const_table);

}

void cl2llvm_erase_global_vars(void)
{
	char *file_name;
	char *name;
	int index;
	struct cl2llvm_function_t *function;
	struct cl2llvm_symbol_t *symbol;

	/* Free global function table */
	HASH_TABLE_FOR_EACH(cl2llvm_symbol_table, name, function)
		cl2llvm_function_free(function);
	hash_table_free(cl2llvm_symbol_table);

	/* Free preprocessor file list */
	LIST_FOR_EACH(cl2llvm_preprcr_file_list, index)
	{
		file_name = list_get(cl2llvm_preprcr_file_list, index);
		free(file_name);
	}
	list_free(cl2llvm_preprcr_file_list);

	/* Free global symbol table */
	HASH_TABLE_FOR_EACH(cl2llvm_global_symbol_table, name, symbol)
		cl2llvm_symbol_free(symbol);
	hash_table_free(cl2llvm_global_symbol_table);

}


void cl2llvm_compile(struct list_t *source_file_list, struct list_t *llvm_file_list,
		int opt_level)
{
	int index;
	char *error = NULL;
	char *llvm_file_name;

	/* This code activates debug information for bison */
	/*	
	 * extern int cl2llvm_yydebug;
	 * cl2llvm_yydebug = 1;
	 */

	/* Process all files */
	LIST_FOR_EACH(source_file_list, index)
	{

		/* Initialize */
		cl2llvm_init_global_vars();

		/* Open file */
		cl2llvm_file_name = list_get(source_file_list, index);
		cl2llvm_yyin = fopen(cl2llvm_file_name, "rb");
		if (!cl2llvm_yyin)
			fatal("%s: cannot open file", cl2llvm_file_name);

		/* Compile */
		cl2llvm_yyparse();
	
		/* Verify module and optimize */
		LLVMVerifyModule(cl2llvm_module, LLVMAbortProcessAction, &error);
		LLVMPassManagerRef pm = LLVMCreatePassManager();

		switch (opt_level)
		{
		case 2:
		case 1:
			LLVMAddCFGSimplificationPass(pm);
			LLVMAddPromoteMemoryToRegisterPass(pm);
		case 0:
			break;
		default:
			panic("invalid optimization level (%d)", opt_level);
		}
		LLVMRunPassManager(pm, cl2llvm_module);

		/* Dump bit code */
		llvm_file_name = list_get(llvm_file_list, index);
		LLVMWriteBitcodeToFile(cl2llvm_module, llvm_file_name);
		LLVMDisposePassManager(pm);
		LLVMDisposeMessage(error);

		/* Finalize */
		fclose(cl2llvm_yyin);
		cl2llvm_erase_global_vars();
	}
}

