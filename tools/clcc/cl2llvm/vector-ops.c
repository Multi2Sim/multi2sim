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
					"tmp%d", temp_var_count++);

				current_vec_elem = cl2llvm_val_create_w_init( LLVMBuildExtractElement(cl2llvm_builder, current_elem->val, cl2llvm_index->val, temp_var_name), current_elem->type->sign);
				list_insert(elem_list, index + vec_index, current_vec_elem);
			}
			list_remove(elem_list, current_elem);
		}
	}
}

void cl2llvm_get_vector_indices(struct cl2llvm_val_t **indice_array, char *string)
{
	int i;
	int s_prefix = 0;
	int leng = strlen(string);
	char error_message[50];
	struct cl2llvm_val_t *index;

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
					indice_array[i] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case 'y':
				if (!s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 1, 0);
					indice_array[i] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;


			case 'z':
				if (!s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 2, 0);
					indice_array[i] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case 'w':
				if (!s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 3, 0);
					indice_array[i] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;
			
			case '0':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 0, 0);
					indice_array[i-1] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '1':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 1, 0);
					indice_array[i-1] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;
		
			case '2':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 2, 0);
					indice_array[i-1] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;
		
			case '3':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 3, 0);
					indice_array[i-1] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '4':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 4, 0);
					indice_array[i-1] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;
			case '5':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 5, 0);
					indice_array[i-1] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '6':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 6, 0);
					indice_array[i-1] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '7':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 7, 0);
					indice_array[i-1] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '8':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 8, 0);
					indice_array[i-1] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '9':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 9, 0);
					indice_array[i-1] = index;
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
					indice_array[i-1] = index;
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
					indice_array[i-1] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index");
				break;

			case 'c':
			case 'C':
				if(s_prefix)
				{
					index->val = LLVMConstInt(LLVMInt32Type(), 12, 0);
					indice_array[i-1] = index;
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
					indice_array[i-1] = index;
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
					indice_array[i-1] = index;
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
					indice_array[i-1] = index;
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
	if (!s_prefix)
		indice_array[i] = NULL;
	else
		indice_array[i-1] = NULL;
}

/*void cl2llvm_build_component_wise_assignment(struct cl2llvm_val_t *lvalue, 
	struct cl2llvm_val_t *rvalue);
{
	snprintf(temp_var_name, sizeof(temp_var_name),
		"tmp%d", temp_var_count++);

	LLVMBuildLoad(cl2llvm_builder, lvalue->val, temp_var_name);
	
	Get number of components 
	while(lvalue->component_indices)
		component_count++;
	
	 Check that none of the vector's components are referenced twice 
	cl2llvm_no_repeated_component_references(lvalue);

	if (LLVMGetTypeKind(LLVMTypeOf(rvalue)) == LLVMVectorTypeKind)
	{
		
	}
}


void cl2llvm_no_repeated_component_references(struct cl2llvm_val_t *lvalue)
{
	int i = 0;

	while(lvalue->component_indices[i])
	{
		if(component_indices[i]->val == LLVMConstInt(LLVMInt
	}
}*/
