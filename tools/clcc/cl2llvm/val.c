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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <lib/mhandle/mhandle.h>

#include "val.h"
#include "type.h"
#include "cl2llvm.h"

extern int temp_var_count;
extern char temp_var_name[50];

extern LLVMBuilderRef cl2llvm_builder;

struct cl2llvm_val_t *cl2llvm_val_create(void)
{
	struct cl2llvm_val_t *cl2llvm_val;
	cl2llvm_val = xcalloc(1, sizeof(struct cl2llvm_val_t));

	struct cl2llvm_type_t *cl2llvm_type;
	cl2llvm_type = xcalloc(1, sizeof(struct cl2llvm_type_t));
	
	cl2llvm_val->type = cl2llvm_type;

	return cl2llvm_val;
}

struct cl2llvm_val_t *cl2llvm_val_create_w_init(LLVMValueRef val, int sign)
{
	struct cl2llvm_val_t *cl2llvm_val = cl2llvm_val_create();
	
	cl2llvm_val->val = val;
	cl2llvm_val->type->llvm_type = LLVMTypeOf(val);
	cl2llvm_val->type->sign = sign;

	return cl2llvm_val;
}

void cl2llvm_val_free(struct cl2llvm_val_t *cl2llvm_val)
{
	free(cl2llvm_val->type);
	free(cl2llvm_val);
}

struct cl2llvm_val_t *llvm_type_cast(struct cl2llvm_val_t * original_val, struct cl2llvm_type_t *totype_w_sign)
{
	struct cl2llvm_val_t *llvm_val = cl2llvm_val_create();

	LLVMTypeRef fromtype = LLVMTypeOf(original_val->val);
	LLVMTypeRef totype = totype_w_sign->llvm_type;
	int fromsign = original_val->type->sign;
	int tosign = totype_w_sign->sign;

	snprintf(temp_var_name, sizeof temp_var_name,
		"tmp%d", temp_var_count++);

	if (fromtype == LLVMInt64Type())
	{
		if (totype == LLVMDoubleType())
		{
		if (fromsign)
			{
				llvm_val->val =
						LLVMBuildSIToFP(cl2llvm_builder,
						  original_val->val, LLVMDoubleType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMDoubleType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMFloatType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMFloatType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMFloatType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMHalfType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMHalfType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMHalfType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMInt64Type())
		{
			if (tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
			temp_var_count--;
		}
		else if (totype == LLVMInt32Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt32Type(), temp_var_name);
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt16Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt16Type(), temp_var_name);
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt8Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt8Type(), temp_var_name);
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt1Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt1Type(), temp_var_name);
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
			
	}
	else if (fromtype == LLVMInt32Type())
	{
		if (totype == LLVMDoubleType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMDoubleType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMDoubleType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMFloatType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMFloatType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMFloatType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMHalfType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMHalfType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMHalfType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMInt64Type())
		{
			if (fromsign)
			{
				llvm_val->val = LLVMBuildSExt(cl2llvm_builder,
					  original_val->val, LLVMInt64Type(),
					temp_var_name);
			}
			else
			{
				llvm_val->val = LLVMBuildZExt(cl2llvm_builder,
					  original_val->val, LLVMInt64Type(),
					temp_var_name);
			}
			if (tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt32Type())
		{
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
			temp_var_count--;
		}
		else if (totype == LLVMInt16Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt16Type(), temp_var_name);
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt8Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				 original_val->val, LLVMInt8Type(), temp_var_name);
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt1Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt1Type(), temp_var_name);
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
			
	}
	else if (fromtype == LLVMInt16Type())
	{
		if (totype == LLVMDoubleType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMDoubleType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMDoubleType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMFloatType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMFloatType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMFloatType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMHalfType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMHalfType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMHalfType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMInt64Type())
		{
			if (fromsign)
			{
				llvm_val->val = LLVMBuildSExt(cl2llvm_builder,
					  original_val->val, LLVMInt64Type(),
					temp_var_name);
			}
			else
			{
				llvm_val->val = LLVMBuildZExt(cl2llvm_builder,
					  original_val->val, LLVMInt64Type(),
					temp_var_name);
			}
			if (tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt32Type())
		{
			if (fromsign)
			{
				llvm_val->val = LLVMBuildSExt(cl2llvm_builder,
					  original_val->val, LLVMInt32Type(),
					temp_var_name);
			}
			else
			{
				llvm_val->val = LLVMBuildZExt(cl2llvm_builder,
					  original_val->val, LLVMInt32Type(),
					temp_var_name);
			}
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt16Type())
		{
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
			temp_var_count--;
		}
		else if (totype == LLVMInt8Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt8Type(), temp_var_name);
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt1Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt1Type(), temp_var_name);
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
			
	}
	else if (fromtype == LLVMInt8Type())
	{
		if (totype == LLVMDoubleType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMDoubleType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMDoubleType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMFloatType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMFloatType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMFloatType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMHalfType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMHalfType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMHalfType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMInt64Type())
		{
			if (fromsign)
			{
				llvm_val->val = LLVMBuildSExt(cl2llvm_builder,
					  original_val->val, LLVMInt64Type(),
					temp_var_name);
			}
			else
			{
				llvm_val->val = LLVMBuildZExt(cl2llvm_builder,
					  original_val->val, LLVMInt64Type(),
					temp_var_name);
			}
			if (tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt32Type())
		{
			if (fromsign)
			{
				llvm_val->val = LLVMBuildSExt(cl2llvm_builder,
					  original_val->val, LLVMInt32Type(),
					temp_var_name);
			}
			else
			{
				llvm_val->val = LLVMBuildZExt(cl2llvm_builder,
					  original_val->val, LLVMInt32Type(),
					temp_var_name);
			}
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt16Type())
		{
			if (fromsign)
			{
				llvm_val->val = LLVMBuildSExt(cl2llvm_builder,
					  original_val->val, LLVMInt16Type(),
					temp_var_name);
			}
			else
			{
				llvm_val->val = LLVMBuildZExt(cl2llvm_builder,
					  original_val->val, LLVMInt16Type(),
					temp_var_name);
			}
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt8Type())
		{
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
			temp_var_count--;
		}
		else if (totype == LLVMInt1Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt1Type(), temp_var_name);
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
			
	}
	else if (fromtype == LLVMInt1Type())
	{
		if (totype == LLVMDoubleType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMDoubleType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMDoubleType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMFloatType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMFloatType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMFloatType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMHalfType())
		{
			if (fromsign)
			{
				llvm_val->val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  original_val->val, LLVMHalfType(),
					temp_var_name);
			}
			else
			{
				llvm_val->val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  original_val->val, LLVMHalfType(),
					temp_var_name);
			}
			llvm_val->type->sign = 1;
		}
		else if (totype == LLVMInt64Type())
		{
			if (fromsign)
			{
				llvm_val->val = LLVMBuildSExt(cl2llvm_builder,
					  original_val->val, LLVMInt64Type(),
					temp_var_name);
			}
			else
			{
				llvm_val->val = LLVMBuildZExt(cl2llvm_builder,
					  original_val->val, LLVMInt64Type(),
					temp_var_name);
			}
			if (tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt32Type())
		{
			if (fromsign)
			{
				llvm_val->val = LLVMBuildSExt(cl2llvm_builder,
					  original_val->val, LLVMInt32Type(),
					temp_var_name);
			}
			else
			{
				llvm_val->val = LLVMBuildZExt(cl2llvm_builder,
					  original_val->val, LLVMInt32Type(),
					temp_var_name);
			}
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt16Type())
		{
			if (fromsign)
			{
				llvm_val->val = LLVMBuildSExt(cl2llvm_builder,
					  original_val->val, LLVMInt16Type(),
					temp_var_name);
			}
			else
			{
				llvm_val->val = LLVMBuildZExt(cl2llvm_builder,
					  original_val->val, LLVMInt16Type(),
					temp_var_name);
			}
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt8Type())
		{
			if (fromsign)
			{
				llvm_val->val = LLVMBuildSExt(cl2llvm_builder,
					  original_val->val, LLVMInt8Type(),
					temp_var_name);
			}
			else
			{
				llvm_val->val = LLVMBuildZExt(cl2llvm_builder,
					  original_val->val, LLVMInt8Type(),
					temp_var_name);
			}
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
		}
		else if (totype == LLVMInt1Type())
		{
			if(tosign)
				llvm_val->type->sign = 1;
			else
				llvm_val->type->sign = 0;
			temp_var_count--;
		}			
	}

	/*We now know that from type must be a floating point.*/

	/*Floating point to signed integer conversions*/
	else if (tosign && LLVMGetTypeKind(totype) == 8)
	{
		if (totype == LLVMInt64Type())
		{
			llvm_val->val = LLVMBuildFPToSI(cl2llvm_builder, 
				  original_val->val, LLVMInt64Type(), temp_var_name);
		}
		else if (totype == LLVMInt32Type())
		{
			llvm_val->val = LLVMBuildFPToSI(cl2llvm_builder, 
				  original_val->val, LLVMInt32Type(), temp_var_name);
		}
		else if (totype == LLVMInt16Type())
		{
			llvm_val->val = LLVMBuildFPToSI(cl2llvm_builder, 
				  original_val->val, LLVMInt16Type(), temp_var_name);
		}
		else if (totype == LLVMInt8Type())
		{
			llvm_val->val = LLVMBuildFPToSI(cl2llvm_builder, 
				  original_val->val, LLVMInt8Type(), temp_var_name);
		}
		else if (totype == LLVMInt1Type())
		{
			llvm_val->val = LLVMBuildFPToSI(cl2llvm_builder, 
				  original_val->val, LLVMInt1Type(), temp_var_name);
		}
		llvm_val->type->sign = 1;
	}
	/*Floating point to unsigned integer conversions*/
	else if (!tosign)
	{
		if (totype == LLVMInt64Type())
		{
			llvm_val->val = LLVMBuildFPToUI(cl2llvm_builder, 
				  original_val->val, LLVMInt64Type(), temp_var_name);
		}
		else if (totype == LLVMInt32Type())
		{
			llvm_val->val = LLVMBuildFPToUI(cl2llvm_builder, 
				  original_val->val, LLVMInt32Type(), temp_var_name);
		}
		else if (totype == LLVMInt16Type())
		{
			llvm_val->val = LLVMBuildFPToUI(cl2llvm_builder, 
				  original_val->val, LLVMInt16Type(), temp_var_name);
		}
		else if (totype == LLVMInt8Type())
		{
			llvm_val->val = LLVMBuildFPToUI(cl2llvm_builder, 
				  original_val->val, LLVMInt8Type(), temp_var_name);
		}
		else if (totype == LLVMInt1Type())
		{
			llvm_val->val = LLVMBuildFPToUI(cl2llvm_builder, 
				  original_val->val, LLVMInt1Type(), temp_var_name);
		}
		llvm_val->type->sign = 0;
	}
	else if (totype == LLVMDoubleType())
	{
		llvm_val->val = LLVMBuildFPExt(cl2llvm_builder, 
			  original_val->val, LLVMDoubleType(), temp_var_name);
		llvm_val->type->sign = 1;
	}
	else if (totype == LLVMFloatType())
	{
		if (fromtype == LLVMDoubleType())
		{
			llvm_val->val = LLVMBuildFPTrunc(cl2llvm_builder, 
				  original_val->val, LLVMFloatType(), temp_var_name);
		}
		else if (fromtype == LLVMHalfType())
		{
			llvm_val->val = LLVMBuildFPExt(cl2llvm_builder, 
				  original_val->val, LLVMFloatType(), temp_var_name);
		}
		llvm_val->type->sign = 1;
	}
	else if (totype == LLVMHalfType())
	{
		llvm_val->val = LLVMBuildFPTrunc(cl2llvm_builder, 
			  original_val->val, LLVMHalfType(), temp_var_name);
		llvm_val->type->sign = 1;
	}
	llvm_val->type->llvm_type = totype;
	return llvm_val;
}

/*
 * This function takes to cl2llvm_value_t's determines the dominant type and  
 * creates a new cl2llvm_value_t which contains the non-dominant operand cast 
 * to the type of the dominant operand. This function will either create one or 
 * zero cl2llvm_value_t's based on whether or not a type cast is needed. It is 
 * up to the user to determine which of the two pointers passed by reference, 
 * if any, points to a new value. The new value must be freed using 
 * cl2llvm_value_free.
 */

void type_unify(struct cl2llvm_val_t *val1, struct cl2llvm_val_t *val2, struct cl2llvm_val_t **new_val1, struct cl2llvm_val_t **new_val2)
{
	LLVMTypeRef type1 = LLVMTypeOf(val1->val);
	LLVMTypeRef type2 = LLVMTypeOf(val2->val);

	/* By default, new values returned are the same as the original
	 * values. */
	*new_val1 = val1;
	*new_val2 = val2;

	/* If types match, no type cast needed */
	if (type1 == type2 && val1->type->sign == val2->type->sign)
		return;

	/* Obtain dominant type */
	struct cl2llvm_type_t *type = type_cmp(val1, val2);
	assert((type->llvm_type != type1 || type->llvm_type != type2) || ( type->sign != val1->type->sign || type->sign != val2->type->sign));

	/* Whatever operand differs from the dominant type will be typecast
	 * to it. */
	if (type->llvm_type != type1 || type->sign != val1->type->sign)
	{
		*new_val1 = llvm_type_cast(val1, type);
	}
	else
	{
		*new_val2 = llvm_type_cast(val2, type);
	}
}



