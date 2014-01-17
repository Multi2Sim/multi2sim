/*
 *  Multi2Sim
 *  Copyright (C) 2013  Chris Barton (barton.ch@husky.neu.edu)
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
#ifndef M2C_CL2LLVM_BUILT_IN_FUNCS_H
#define M2C_CL2LLVM_BUILT_IN_FUNCS_H

#include <llvm-c/Core.h>
#include <lib/util/list.h>

#include "Type.h"

struct cl2llvm_built_in_func_t
{
	int arg_count;

	struct list_t* format_list;
};

struct cl2llvm_built_in_func_llvm_name_t
{
	char* llvm_name;

	/* Arguments */
	int arg_count;
	struct cl2llvmTypeWrap** arg_list;

	/* Return type */
	struct cl2llvmTypeWrap* ret_type;
};

struct hash_table_t *built_in_func_table_create(void);

void cl2llvm_built_in_func_table_free(struct hash_table_t *built_in_func_table);

struct cl2llvm_built_in_func_t *cl2llvm_built_in_func_create(int, char*, char*);

void cl2llvm_built_in_func_free(struct cl2llvm_built_in_func_t* built_in_Func);

struct cl2llvm_built_in_func_llvm_name_t* cl2llvm_built_in_func_llvm_name_create(void);

void cl2llvm_built_in_func_llvm_name_free(struct cl2llvm_built_in_func_llvm_name_t*);

void cl2llvm_built_in_func_analyze(char* name, struct list_t *param_list);

void func_declare(int arg_count, struct cl2llvmTypeWrap** arg_list, struct cl2llvmTypeWrap *ret_type, 
	char* name, char* param_spec_name);

int *intptr(int num);

struct cl2llvmTypeWrap *string_to_type(char*);

/* This function creates an error message for argument type mismatches based
   on and arg_info string and a list of the attempted argument types. */
char *cl2llvm_error_built_in_func_arg_mismatch(struct list_t *param_list,
	struct cl2llvm_built_in_func_t *func_info,  char *func_name, 
	char *error_message);

#endif
