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
#include <lib/util/hash-table.h>
#include <lib/util/list.h>

#include "declarator-list.h"
#include "built-in-funcs.h"
#include "type.h"
#include "arg.h"
#include "function.h"

extern LLVMModuleRef cl2llvm_module;
extern struct hash_table_t *cl2llvm_symbol_table;

struct hash_table_t *built_in_func_table_create(void)
{
	/* Create hash table */
	struct hash_table_t *built_in_func_table = hash_table_create(200, 1);
	
	/* Insert function names and id numbers into hash table. */
	hash_table_insert(built_in_func_table, "get_global_size", intptr(0));
	hash_table_insert(built_in_func_table, "get_global_id", intptr(1));

	return built_in_func_table;
}

void func_declare(int *func_id)
{
	LLVMTypeRef args_array[50];
	struct list_t *args;
	struct cl2llvm_decl_list_t *arg_decl1;
	struct cl2llvm_arg_t *arg1;
	struct cl2llvm_function_t *function;

	switch (*func_id)
	{
		case 0:
			break;

		case 1:
		/*Declare get_global_id*/
	
		/* Arguments */
		args_array[0] = LLVMInt32Type();
		args = list_create();
		arg_decl1 = cl2llvm_decl_list_create();
		arg_decl1->type_spec = 
			cl2llvm_type_create_w_init(LLVMInt32Type(), 0);
		arg1 = cl2llvm_arg_create(arg_decl1, "dimindex");
		list_add(args, arg1);

		/* Function */
		function = cl2llvm_function_create("get_global_id", args);
	
		 function->func_type = LLVMFunctionType( LLVMInt32Type(), 
		 	args_array, 1, 0);
	 	function->func = LLVMAddFunction(cl2llvm_module, 
			"get_global_id", function->func_type);
		function->sign = 1;
		LLVMSetFunctionCallConv(function->func, LLVMCCallConv);
		LLVMAddFunctionAttr(function->func, 1 << 5);

		/* Insert function in global symbol table */
		hash_table_insert(cl2llvm_symbol_table, "get_global_id", 
			function);

		/* Free pointers */
		cl2llvm_decl_list_struct_free(arg_decl1);
		
		break;
	}
}

int *intptr(int num)
{
	int *ptr = malloc(sizeof(int));
	*ptr = num;
	return ptr;
}
