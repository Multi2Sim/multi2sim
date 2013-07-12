/*
 *  Multi2Sim
 *  Copyright (C) 2013  Chris Barton (barton-ch@husky.neu.edu)
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
#include <string.h>

#include <lib/util/list.h>
#include <llvm-c/Core.h>

#include "vector-ops.h"
#include "val.h"

extern LLVMBuilderRef cl2llvm_builder;
extern char temp_var_name[50];
extern int temp_var_count;

extern void cl2llvm_yyerror(char *);

void expand_vectors(struct list_t *elem_list)
{
	int index;
	int vec_index;
	struct cl2llvm_val_t *cl2llvm_index;
	struct cl2llvm_val_t *current_vec_elem;
	struct cl2llvm_val_t *current_elem;
	
	LIST_FOR_EACH(elem_list, index)
	{
		current_elem = list_get(elem_list, index);
		if (LLVMGetTypeKind(current_elem->type->llvm_type) == LLVMVectorTypeKind)
		{

			for(vec_index = 0; vec_index < LLVMGetVectorSize(current_elem->type->llvm_type); vec_index++)
			{
				cl2llvm_index = cl2llvm_val_create_w_init( LLVMConstInt(
					LLVMInt32Type(), vec_index, 0), 1);


				snprintf(temp_var_name, sizeof(temp_var_name),
					"tmp_%d", temp_var_count++);

				current_vec_elem = cl2llvm_val_create_w_init( LLVMBuildExtractElement(cl2llvm_builder, current_elem->val, cl2llvm_index->val, temp_var_name), current_elem->type->sign);
				list_insert(elem_list, index + vec_index, current_vec_elem);
			cl2llvm_val_free(cl2llvm_index);
			}
			cl2llvm_val_free(current_elem);
			list_remove(elem_list, current_elem);
		}
	}
}

void cl2llvm_get_vector_indices(struct cl2llvm_val_t *value, char *string)
{
	int i;
	int boundary_error = 0;
	int vector_size;
	int s_prefix = 0;
	int leng = strlen(string);
	char error_message[50];
	struct cl2llvm_val_t *index;

	vector_size = LLVMGetVectorSize(LLVMGetElementType(value->type->llvm_type));
	i = 0;

	if (string[0] == 's' || string[0] == 'S')
	{
		s_prefix = 1;
		i++;
	}


	for (; i < leng; i++)
	{
		/* Check that number of specified components does not exceed 16 */
		if (i > 16 && !s_prefix)
			cl2llvm_yyerror("Too many components for vector type");
		if (i > 17)
			cl2llvm_yyerror("Too many components for vector type");

		index = cl2llvm_val_create();
		index->type->llvm_type = LLVMInt32Type();
		index->type->sign = 1;

		switch (string[i])
		{
			case 'x':
				if (!s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 0, 0);
					value->vector_indices[i] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case 'y':
				if (!s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 1, 0);
					value->vector_indices[i] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;


			case 'z':
				if (!s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 2, 0);
					value->vector_indices[i] = index;
					if(vector_size < 3)
						boundary_error = 1;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case 'w':
				if (!s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 3, 0);
					value->vector_indices[i] = index;
					if(vector_size < 4)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;
			
			case '0':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 0, 0);
					value->vector_indices[i-1] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '1':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 1, 0);
					value->vector_indices[i-1] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;
		
			case '2':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 2, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 3)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;
		
			case '3':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 3, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 4)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '4':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 4, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 5)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;
			case '5':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 5, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 6)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '6':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 6, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 7)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '7':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 7, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 8)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '8':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 8, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 9)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '9':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 9, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 10)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case 'a':
			case 'A':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 10, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 11)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;
	
			case 'b':
			case 'B':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 11, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 12)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index");
				break;

			case 'c':
			case 'C':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 12, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 13)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case 'd':
			case 'D':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 13, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 14)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case 'e':
			case 'E':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 14, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 15)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case 'f':
			case 'F':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 15, 0);
					value->vector_indices[i-1] = index;
					if(vector_size < 16)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;
			default:
				snprintf(error_message, sizeof(error_message),
					"Invalid character '%c' in vector index", string[i]);

				cl2llvm_yyerror(error_message);
				
		}
	}
	if (boundary_error)
		cl2llvm_yyerror("Component reference is outside boundary of vector");
	if (!s_prefix)
		value->vector_indices[i] = NULL;
	else
		value->vector_indices[i-1] = NULL;
}

struct cl2llvm_val_t *cl2llvm_build_component_wise_assignment(struct cl2llvm_val_t *lvalue_addr, 
	struct cl2llvm_val_t *rvalue)
{
	int component_count = 0;
	int i;
	struct cl2llvm_type_t *component_type;
	struct cl2llvm_val_t *lvalue;
	struct cl2llvm_val_t *new_lvalue;
	struct cl2llvm_val_t *cast_rvalue;
	LLVMValueRef llvm_index;
	LLVMValueRef component;
	LLVMValueRef new_lvalue_val;
	
	snprintf(temp_var_name, sizeof(temp_var_name),
		"tmp_%d", temp_var_count++);

	/* Load vector */
	lvalue = cl2llvm_val_create_w_init(LLVMBuildLoad(cl2llvm_builder, lvalue_addr->val,
		temp_var_name), lvalue_addr->type->sign);
	
	new_lvalue_val = lvalue->val;
	
	/* Create object to  represent component type of lvalue. */
	component_type = cl2llvm_type_create_w_init(LLVMGetElementType(lvalue->type->llvm_type), 
		lvalue->type->sign);
	
	/* Get number of components referenced by lvalue. */
	while(lvalue_addr->vector_indices[component_count])
		component_count++;
	
	
	/* Check that none of the vector's components are referenced twice */
	cl2llvm_no_repeated_component_references(lvalue_addr);

	/* If rvalue is a vector */
	if (LLVMGetTypeKind(rvalue->type->llvm_type) == LLVMVectorTypeKind)
	{
		/* Check that element type of rvalue vector matches element type of 
		lvalue vector */
		if (LLVMGetElementType(rvalue->type->llvm_type) != LLVMGetElementType(lvalue->type->llvm_type))
			cl2llvm_yyerror("Type mis-match. (casts between vector types are forbidden)");

		/* Check that size of vector matches number of components specified 
		in lvalue. */
		if (LLVMGetVectorSize(rvalue->type->llvm_type) != component_count)
			cl2llvm_yyerror("Size of vector does not match number of components specified in lvalue.");

		/* Extract each component from rvalue and assign it to the specified
		component of the lvalue. */

		for (i = 0; i < component_count; i++)
		{
			snprintf(temp_var_name, sizeof(temp_var_name),
				"tmp_%d", temp_var_count++);

			llvm_index = LLVMConstInt(LLVMInt32Type(), i, 0);

			/* Extract component from rvalue */
			component = LLVMBuildExtractElement(cl2llvm_builder, rvalue->val, llvm_index, temp_var_name);

			snprintf(temp_var_name, sizeof(temp_var_name),
				"tmp_%d", temp_var_count++);

			/* Insert component into lvalue */
			new_lvalue_val = LLVMBuildInsertElement(cl2llvm_builder, new_lvalue_val, 
				component, lvalue_addr->vector_indices[i]->val, temp_var_name);
		}
	}

	/* If rvalue is a scalar, assign this value to every specified component of the lavlue */
	else
	{
		cast_rvalue = llvm_type_cast(rvalue, component_type);
	
		for (i = 0; i < component_count; i++)
		{
			snprintf(temp_var_name, sizeof(temp_var_name),
				"tmp_%d", temp_var_count++);

			/* Insert component into lvalue */
			new_lvalue_val = LLVMBuildInsertElement(cl2llvm_builder, new_lvalue_val, 
				cast_rvalue->val, lvalue_addr->vector_indices[i]->val, temp_var_name);
			
		}
		cl2llvm_val_free(cast_rvalue);
	}

	new_lvalue = cl2llvm_val_create_w_init(new_lvalue_val, component_type->sign);

	/* Free pointers */
	cl2llvm_type_free(component_type);
	cl2llvm_val_free(lvalue);

	return new_lvalue;
}


void cl2llvm_no_repeated_component_references(struct cl2llvm_val_t *lvalue)
{
	int index_0 = 0, index_1 = 0, index_2 = 0, index_3 = 0, index_4 = 0, 
		index_5 = 0, index_6 = 0,  index_7 = 0, index_8 = 0, index_9 = 0, 
		index_10 = 0, index_11 = 0, index_12 = 0, index_13 = 0, 
		index_14 = 0, index_15 = 0;
	int error = 0;
	int i = 0;

	while(lvalue->vector_indices[i])
	{
		if(lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 0, 0))
		{
			if(!index_0)
				index_0 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 1, 0))
		{
			if(!index_1)
				index_1 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 2, 0))
		{
			if(!index_2)
				index_2 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 3, 0))
		{
			if(!index_3)
				index_3 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 4, 0))
		{
			if(!index_4)
				index_4 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 5, 0))
		{
			if(!index_5)
				index_5 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 6, 0))
		{
			if(!index_6)
				index_6 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 7, 0))
		{
			if(!index_7)
				index_7 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 8, 0))
		{
			if(!index_8)
				index_8 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 9, 0))
		{
			if(!index_9)
				index_9 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 10, 0))
		{
			if(!index_10)
				index_10 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 11, 0))
		{
			if(!index_11)
				index_11 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 12, 0))
		{
			if(!index_12)
				index_12 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 13, 0))
		{
			if(!index_13)
				index_13 = 1;
			else
				error = 1;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 14, 0))
		{
			if(!index_14)
				index_14 = 1;
			else
				error = 14;
		}
		else if (lvalue->vector_indices[i]->val == LLVMConstInt(LLVMInt32Type(), 15, 0))
		{
			if(!index_15)
				index_15 = 1;
			else
				error = 1;
		}
		i++;
		if (error)
			cl2llvm_yyerror("Invalid lvalue. (May not reference the same vector component twice in an lvalue)");
	}
}
