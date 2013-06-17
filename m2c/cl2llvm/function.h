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

#ifndef M2C_CL2LLVM_FUNCTION_H
#define M2C_CL2LLVM_FUNCTION_H

#include <llvm-c/Core.h>
#include <lib/util/list.h>

struct cl2llvm_function_t
{
	LLVMValueRef func;
	LLVMTypeRef func_type;
	int sign;
	char *name;
	struct hash_table_t *symbol_table;
	struct list_t *arg_list;
	int arg_count;
	LLVMBasicBlockRef entry_block;
	LLVMValueRef branch_instr;
};

struct cl2llvm_function_t *cl2llvm_function_create(char *name, struct list_t *arg_list);

void cl2llvm_function_free(struct cl2llvm_function_t *function);
	
#endif
