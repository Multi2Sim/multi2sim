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

#include <lib/util/hash-table.h>
#include <lib/mhandle/mhandle.h>

#include "arg.h"
#include "symbol.h"
#include "function.h"
#include "Type.h"
#include "declarator-list.h"
#include "arg.h"


struct cl2llvm_function_t *cl2llvm_function_create(char *name, struct list_t *arg_list)
{
	struct cl2llvm_function_t *function;
	function = xcalloc(1, sizeof(struct cl2llvm_function_t));

	function->symbol_table = hash_table_create(10, 1);

	function->arg_list = arg_list;

	if (list_get(arg_list, 0))
		function->arg_count = list_count(arg_list);
	else
		function->arg_count = 0;

	function->name = xstrdup(name);

	return function;
}

struct cl2llvm_function_t *cl2llvm_func_cpy(struct cl2llvm_function_t *src_func)
{
	int i;
	struct cl2llvm_function_t *new_func;
	struct cl2llvm_decl_list_t *new_arg_decl;
	struct cl2llvm_arg_t *src_arg;
	struct cl2llvm_arg_t *new_arg;
	
	new_func = xcalloc(1, sizeof(struct cl2llvm_function_t));

	new_func->arg_list = list_create();

	new_func->symbol_table = hash_table_create(10, 1);

	LIST_FOR_EACH(src_func->arg_list, i)
	{
		src_arg = list_get(src_func->arg_list, i);
		
		new_arg_decl = cl2llvm_decl_list_create();
		new_arg_decl->type_spec = cl2llvmTypeWrapCreate(
			cl2llvmTypeWrapGetLlvmType(src_arg->type_spec), 
			cl2llvmTypeWrapGetSign(src_arg->type_spec));
		new_arg_decl->access_qual = src_arg->access_qual;

		new_arg = cl2llvm_arg_create(new_arg_decl, "arg");

		list_add(new_func->arg_list, new_arg);

		cl2llvm_decl_list_struct_free(new_arg_decl);
	}

	new_func->arg_count = src_func->arg_count;
	new_func->func = src_func->func;
	new_func->func_type = src_func->func_type;
	new_func->sign = src_func->sign;
	new_func->name = xstrdup(src_func->name);

	return new_func;
}

void cl2llvm_function_free(struct cl2llvm_function_t *function)
{
	char *symbol_name;
	struct cl2llvm_symbol_t *symbol;

	free(function->name);	
	/* Free symbol table */
	HASH_TABLE_FOR_EACH(function->symbol_table, symbol_name, symbol)
		cl2llvm_symbol_free(symbol);
	hash_table_free(function->symbol_table);
	
	int i;
	LIST_FOR_EACH(function->arg_list, i)
	{
		cl2llvm_arg_free(list_get(function->arg_list, i));
	}
	list_free(function->arg_list);

	free(function);
}
	
