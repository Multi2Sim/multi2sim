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

#include <stdlib.h>
#include <stdio.h>

#include <llvm-c/Core.h>
#include <lib/mhandle/mhandle.h>

#include "val.h"
#include "type.h"


struct cl2llvm_type_t *cl2llvm_type_create(void)
{
	struct cl2llvm_type_t *cl2llvm_type;

	cl2llvm_type = xcalloc(1, sizeof(struct cl2llvm_type_t));

	return cl2llvm_type;
}

struct cl2llvm_type_t *cl2llvm_type_create_w_init(LLVMTypeRef llvm_type, int sign) 
{
	struct cl2llvm_type_t *cl2llvm_type = cl2llvm_type_create();
	cl2llvm_type->llvm_type = llvm_type;
	cl2llvm_type->sign = sign;

	return cl2llvm_type;
}

void cl2llvm_type_free(struct cl2llvm_type_t *cl2llvm_type)
{
	free(cl2llvm_type);
}
/*
 * This function takes two cl2llvm_value_t's and returns the type of the   
 * dominant operand. The return value points to a newly created cl2llvm_type_t 
 * which must be freed using cl2llvm_type_free().
 */
#define type_cmp_num_types  31

struct cl2llvm_type_t *type_cmp(struct cl2llvm_val_t *type1_w_sign, struct cl2llvm_val_t *type2_w_sign)
{
	struct cl2llvm_type_t *dom_type = cl2llvm_type_create();

	LLVMTypeRef type1_type = LLVMTypeOf(type1_w_sign->val);
	LLVMTypeRef type2_type = LLVMTypeOf(type2_w_sign->val);
	int type1_sign = type1_w_sign->type->sign;
	int type2_sign = type1_w_sign->type->sign;

	dom_type->llvm_type = type1_type;
	dom_type->sign = type1_sign;

	struct llvm_type_const
	{
		LLVMTypeRef type;
		int sign;
	};

	struct llvm_type_table 
	{
		struct llvm_type_const type1;
		struct llvm_type_const type2;
	};
	struct llvm_type_table table[type_cmp_num_types] = 
	{
		{ {LLVMDoubleType(), 1}, {LLVMInt64Type(), 1} },
		{ {LLVMDoubleType(), 1}, {LLVMInt32Type(), 1} },
		{ {LLVMDoubleType(), 1}, {LLVMInt16Type(), 1} },
		{ {LLVMDoubleType(), 1}, {LLVMInt8Type(), 1} },
		{ {LLVMDoubleType(), 1}, {LLVMInt1Type(), 1} },
		{ {LLVMFloatType(), 1}, {LLVMInt64Type(), 1} },
		{ {LLVMFloatType(), 1}, {LLVMInt32Type(), 1} },
		{ {LLVMFloatType(), 1}, {LLVMInt16Type(), 1} },
		{ {LLVMFloatType(), 1}, {LLVMInt8Type(), 1} },
		{ {LLVMFloatType(), 1}, {LLVMInt1Type(), 1} },
		{ {LLVMInt64Type(), 0}, {LLVMInt32Type(), 0} },
		{ {LLVMInt64Type(), 0}, {LLVMInt16Type(), 0} },
		{ {LLVMInt64Type(), 0}, {LLVMInt8Type(), 0} },
		{ {LLVMInt64Type(), 0}, {LLVMInt1Type(), 0} },
		{ {LLVMInt64Type(), 0}, {LLVMInt32Type(), 1} },
		{ {LLVMInt64Type(), 0}, {LLVMInt16Type(), 1} },
		{ {LLVMInt64Type(), 0}, {LLVMInt8Type(), 1} },
		{ {LLVMInt64Type(), 0}, {LLVMInt1Type(), 1} },
		{ {LLVMInt64Type(), 1}, {LLVMInt32Type(), 1} },
		{ {LLVMInt64Type(), 1}, {LLVMInt16Type(), 1} },
		{ {LLVMInt64Type(), 1}, {LLVMInt8Type(), 1} },
		{ {LLVMInt64Type(), 1}, {LLVMInt1Type(), 1} },
		{ {LLVMInt32Type(), 0}, {LLVMInt8Type(), 1} },
		{ {LLVMInt32Type(), 0}, {LLVMInt16Type(), 1} },
		{ {LLVMInt32Type(), 0}, {LLVMInt1Type(), 1} },
		{ {LLVMInt32Type(), 0}, {LLVMInt8Type(), 0} },
		{ {LLVMInt32Type(), 0}, {LLVMInt16Type(), 0} },
		{ {LLVMInt32Type(), 0}, {LLVMInt1Type(), 0} },
		{ {LLVMInt32Type(), 1}, {LLVMInt8Type(), 1} },
		{ {LLVMInt32Type(), 1}, {LLVMInt16Type(), 1} },
		{ {LLVMInt32Type(), 1}, {LLVMInt1Type(), 1} }

	};
	int i;

	for (i = 0; i < type_cmp_num_types; i++)
	{
		if ((type1_type == table[i].type1.type 
				&& type1_sign == table[i].type1.sign
				&& type2_type == table[i].type2.type 
				&& type2_sign == table[i].type2.sign)
			|| (type2_type == table[i].type1.type 
				&& type2_sign == table[i].type1.sign 
				&& type1_type == table[i].type2.type 
				&& type1_sign == table[i].type2.sign))
		{
			dom_type->llvm_type = table[i].type1.type;
			dom_type->sign = table[i].type1.sign;
		}
	}
	return dom_type;
}

