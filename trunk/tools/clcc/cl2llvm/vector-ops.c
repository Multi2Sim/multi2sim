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

#include <lib/util/list.h>
#include <llvm-c/Core.h>

#include "vector-ops.h"
#include "val.h"

extern LLVMBuilderRef cl2llvm_builder;
extern char temp_var_name[50];
extern int temp_var_count;

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
		printf("run1, %d\n",LLVMGetTypeKind(current_elem->type->llvm_type));
		if (LLVMGetTypeKind(current_elem->type->llvm_type) == LLVMVectorTypeKind)
		{
			printf("run2\n");

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
