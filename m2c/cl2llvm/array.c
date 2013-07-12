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

#include <stdlib.h>
#include <stdio.h>

#include <lib/util/list.h>
#include <llvm-c/Core.h>

#include "function.h"
#include "val.h"
#include "type.h"

extern int temp_var_count;
extern char temp_var_name[50];

extern LLVMBuilderRef cl2llvm_builder;
extern LLVMBasicBlockRef current_basic_block;
struct cl2llvm_function_t *cl2llvm_current_function;

void cl2llvm_array_alloca(struct list_t *array_deref_list, LLVMTypeRef type, LLVMValueRef ptr)
{
	LLVMValueRef index[1];
	LLVMValueRef array_ptr;
	struct cl2llvm_val_t *array_length;
	LLVMValueRef array;
	struct list_t *new_array_deref_list;
	struct cl2llvm_val_t *value;
	struct cl2llvm_val_t *value_dup;
	int i;

	/* Allocate array */
	LLVMPositionBuilder(cl2llvm_builder, cl2llvm_current_function->entry_block,
		cl2llvm_current_function->branch_instr);

	array_length = list_get(array_deref_list, 0);
	
	snprintf(temp_var_name, sizeof(temp_var_name),
		"tmp__%d", temp_var_count++);

	array = LLVMBuildArrayAlloca(cl2llvm_builder, type, 
		array_length->val, temp_var_name);
	
	LLVMPositionBuilderAtEnd(cl2llvm_builder, current_basic_block);

	/* Store Array */
	LLVMBuildStore(cl2llvm_builder, array, ptr);

	/* If there are still more array dereferences, continue allocating arrays */
	if (list_count(array_deref_list) > 1)
	{
		/* Create type of next array */
		type = LLVMGetElementType(type);

		/* Duplicate array deref list and remove first element */
		new_array_deref_list = list_create();
		LIST_FOR_EACH(array_deref_list, i)
		{
			value = list_get(array_deref_list, i);
			value_dup = cl2llvm_val_create_w_init(value->val, 
				value->type->sign);
			list_add(new_array_deref_list, value_dup);
		}
		value = list_get(new_array_deref_list, 0);
		list_remove(new_array_deref_list, value);
		cl2llvm_val_free(value);

		/* Since array size must be a constant, we can use the following
		   method to create a loop conditional */
		i = 0;
		while(LLVMConstInt(LLVMInt1Type() ,1 ,0) == 
			LLVMConstICmp(LLVMIntSLT, 
			LLVMConstInt(LLVMInt32Type(), i, 0), array_length->val))
		{
			index[0] = LLVMConstInt(LLVMInt32Type(), i, 0);
			
			snprintf(temp_var_name, sizeof(temp_var_name),
				"tmp__%d", temp_var_count++);

			array_ptr = LLVMBuildGEP(cl2llvm_builder, array, index, 1,
				temp_var_name);
			
			cl2llvm_array_alloca(new_array_deref_list, type, array_ptr);
			i++;
		}

		/* Free duplicated list */
		LIST_FOR_EACH(new_array_deref_list, i)
		{
			value = list_get(new_array_deref_list, i);
			cl2llvm_val_free(value);
		}
		list_free(new_array_deref_list);
	}
}

