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


#include <lib/mhandle/mhandle.h>

#include "function.h"
#include "val.h"
#include "cl2llvm.h"
extern int temp_var_count;
extern char temp_var_name[50];

extern LLVMBuilderRef cl2llvm_builder;
extern struct cl2llvm_function_t *cl2llvm_current_function;
extern LLVMBasicBlockRef current_basic_block;

struct cl2llvm_val_t *cl2llvm_val_create(void)
{
	int i;
	struct cl2llvm_val_t *cl2llvm_val;
	cl2llvm_val = xcalloc(1, sizeof(struct cl2llvm_val_t));

	struct cl2llvmTypeWrap *cl2llvm_type;
	cl2llvm_type = cl2llvmTypeWrapCreate(LLVMInt32Type(), 1);
	
	cl2llvm_val->type = cl2llvm_type;
	cl2llvm_val->vector_indices = xmalloc(sizeof(struct cl2llvm_val_t *) * 17);
	
	/* Initialize vector indices to NULL */
	for (i = 0; i < 17; i++)
		cl2llvm_val->vector_indices[i] = NULL;

	return cl2llvm_val;
}

struct cl2llvm_val_t *cl2llvm_val_create_w_init(LLVMValueRef val, int sign)
{
	struct cl2llvm_val_t *cl2llvm_val = cl2llvm_val_create();
	
	cl2llvm_val->val = val;
	cl2llvmTypeWrapSetLlvmType(cl2llvm_val->type, LLVMTypeOf(val));
	cl2llvmTypeWrapSetSign(cl2llvm_val->type, sign);

	return cl2llvm_val;
}

void cl2llvm_val_free(struct cl2llvm_val_t *cl2llvm_val)
{
	int i;

	cl2llvmTypeWrapFree(cl2llvm_val->type);
	for(i = 0; i < 16; i++)
	{
		if (cl2llvm_val->vector_indices[i])
			cl2llvm_val_free(cl2llvm_val->vector_indices[i]);
	}
	free(cl2llvm_val->vector_indices);
	free(cl2llvm_val);
}

struct cl2llvm_val_t *llvm_type_cast(struct cl2llvm_val_t * original_val, 
	struct cl2llvmTypeWrap *totype_w_sign)
{
	struct cl2llvm_val_t *llvm_val = cl2llvm_val_create();

	int i;
	struct cl2llvmTypeWrap *elem_type;
	struct cl2llvm_val_t *cast_original_val;
	LLVMValueRef index;
	LLVMValueRef vector_addr;
	LLVMValueRef vector;
	LLVMValueRef const_elems[16];
	LLVMTypeRef fromtype = cl2llvmTypeWrapGetLlvmType(original_val->type);
	LLVMTypeRef totype = cl2llvmTypeWrapGetLlvmType(totype_w_sign);
	int fromsign = cl2llvmTypeWrapGetSign(original_val->type);
	int tosign = cl2llvmTypeWrapGetSign(totype_w_sign);

	/*By default the return value is the same as the original_val*/
	llvm_val->val = original_val->val;
	cl2llvmTypeWrapSetLlvmType(llvm_val->type, cl2llvmTypeWrapGetLlvmType(original_val->type));
	cl2llvmTypeWrapSetSign(llvm_val->type, cl2llvmTypeWrapGetSign(original_val->type));
	
	snprintf(temp_var_name, sizeof temp_var_name,
		"tmp_%d", temp_var_count++);
		
	/* Check that fromtype is not a vector, unless both types are identical. */
	if (LLVMGetTypeKind(fromtype) == LLVMVectorTypeKind)
	{
		if ((LLVMGetVectorSize(fromtype) != LLVMGetVectorSize(totype) 
			|| LLVMGetElementType(fromtype) 
			!= LLVMGetElementType(totype)) 
			|| fromsign != tosign)
		{
			if (LLVMGetTypeKind(totype) == LLVMVectorTypeKind)
				cl2llvm_yyerror("Casts between vector types are forbidden");
			cl2llvm_yyerror("A vector may not be cast to any other type.");
		}
	}

	/* If totype is a vector, create a vector whose components are equal to 
	original_val */

	if (LLVMGetTypeKind(totype) == LLVMVectorTypeKind
		&& LLVMGetTypeKind(fromtype) != LLVMVectorTypeKind)
	{
		/*Go to entry block and declare vector*/
		LLVMPositionBuilder(cl2llvm_builder, cl2llvm_current_function->entry_block,
			cl2llvm_current_function->branch_instr);
		
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
			
		vector_addr = LLVMBuildAlloca(cl2llvm_builder, 
			totype, temp_var_name);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, current_basic_block);

		/* Load vector */
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
	
		vector = LLVMBuildLoad(cl2llvm_builder, vector_addr, temp_var_name);
		
		/* Create object to represent element type of totype */
		elem_type = cl2llvmTypeWrapCreate(LLVMGetElementType(totype), tosign);

		/* If original_val is constant create a constant vector */
		if (LLVMIsConstant(original_val->val))
		{
			cast_original_val = llvm_type_cast(original_val, elem_type);
			for (i = 0; i < LLVMGetVectorSize(totype); i++)
				const_elems[i] = cast_original_val->val;

			vector = LLVMConstVector(const_elems, 	
				LLVMGetVectorSize(totype));
			llvm_val->val = vector;

			cl2llvm_val_free(cast_original_val);
		}
		/* If original value is not constant insert elements */
		else
		{
			for (i = 0; i < LLVMGetVectorSize(totype); i++)
			{
				index = LLVMConstInt(LLVMInt32Type(), i, 0);
				cast_original_val = llvm_type_cast(original_val, elem_type);
				snprintf(temp_var_name, sizeof temp_var_name,
					"tmp_%d", temp_var_count++);
	
				vector = LLVMBuildInsertElement(cl2llvm_builder, 
					vector, cast_original_val->val, index, temp_var_name);
				cl2llvm_val_free(cast_original_val);
			}
		}
		cl2llvmTypeWrapFree(elem_type);
		llvm_val->val = vector;
	}


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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
		}
		else if (totype == LLVMInt64Type())
		{
			if (tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
			temp_var_count--;
		}
		else if (totype == LLVMInt32Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt32Type(), temp_var_name);
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
		}
		else if (totype == LLVMInt16Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt16Type(), temp_var_name);
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
		}
		else if (totype == LLVMInt8Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt8Type(), temp_var_name);
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
		}
		else if (totype == LLVMInt1Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt1Type(), temp_var_name);
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
		}
		else if (totype == LLVMInt32Type())
		{
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
			temp_var_count--;
		}
		else if (totype == LLVMInt16Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt16Type(), temp_var_name);
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
		}
		else if (totype == LLVMInt8Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				 original_val->val, LLVMInt8Type(), temp_var_name);
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
		}
		else if (totype == LLVMInt1Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt1Type(), temp_var_name);
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
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
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
		}
		else if (totype == LLVMInt16Type())
		{
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
			temp_var_count--;
		}
		else if (totype == LLVMInt8Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt8Type(), temp_var_name);
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
		}
		else if (totype == LLVMInt1Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt1Type(), temp_var_name);
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
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
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
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
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
		}
		else if (totype == LLVMInt8Type())
		{
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
			temp_var_count--;
		}
		else if (totype == LLVMInt1Type())
		{
			llvm_val->val = LLVMBuildTrunc(cl2llvm_builder,
				  original_val->val, LLVMInt1Type(), temp_var_name);
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
			cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
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
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
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
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
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
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
		}
		else if (totype == LLVMInt1Type())
		{
			if(tosign)
				cl2llvmTypeWrapSetSign(llvm_val->type, 1);
			else
				cl2llvmTypeWrapSetSign(llvm_val->type, 0);
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
		cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
		cl2llvmTypeWrapSetSign(llvm_val->type, 0);
	}
	else if (totype == LLVMDoubleType())
	{
		llvm_val->val = LLVMBuildFPExt(cl2llvm_builder, 
			  original_val->val, LLVMDoubleType(), temp_var_name);
		cl2llvmTypeWrapSetSign(llvm_val->type, 1);
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
		cl2llvmTypeWrapSetSign(llvm_val->type, 1);
	}
	else if (totype == LLVMHalfType())
	{
		llvm_val->val = LLVMBuildFPTrunc(cl2llvm_builder, 
			  original_val->val, LLVMHalfType(), temp_var_name);
		cl2llvmTypeWrapSetSign(llvm_val->type, 1);
	}
	cl2llvmTypeWrapSetLlvmType(llvm_val->type, totype);
	cl2llvmTypeWrapSetSign(llvm_val->type, tosign);
	
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
	if (type1 == type2 && cl2llvmTypeWrapGetSign(val1->type) == cl2llvmTypeWrapGetSign(val2->type))
		return;
	
	/* If the types do not match and both are vector types, return error */
	if (LLVMGetTypeKind(type1) == LLVMVectorTypeKind 
		&& LLVMGetTypeKind(type2) == LLVMVectorTypeKind)
	{
		cl2llvm_yyerror("Type mis-match. (Type of both operands of a vector operator must be an exact match.)");
	}

	/* Obtain dominant type */
	struct cl2llvmTypeWrap *type = cl2llvmTypeWrapCompare(val1->type, val2->type);

	/* Whatever operand differs from the dominant type will be typecast
	 * to it. */
	if (cl2llvmTypeWrapGetLlvmType(type) != type1 || cl2llvmTypeWrapGetSign(type) != cl2llvmTypeWrapGetSign(val1->type))
	{
		*new_val1 = llvm_type_cast(val1, type);
	}
	else
	{
		*new_val2 = llvm_type_cast(val2, type);
	}

}

/* This function returns an i1 1 if the value is not equal to 0 and 
   an i1 0 if the value is equal to 0. */
struct cl2llvm_val_t *cl2llvm_to_bool_ne_0(struct cl2llvm_val_t *value)
{
	LLVMValueRef const_zero;
	LLVMValueRef zero_vec[16];
	LLVMTypeRef switch_type;
	int i;
	int veclength;

	struct cl2llvm_val_t *bool_val = cl2llvm_val_create_w_init(value->val, cl2llvmTypeWrapGetSign(value->type));
	
	/* if value is i1 no conversion necessary */
	if (LLVMTypeOf(value->val) == LLVMInt1Type())
		return bool_val;

	/* If value is a vector create a vector of constant zeros, else
	   create a scalar 0. */
	if (LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(value->type)) == LLVMVectorTypeKind)
	{
		switch_type = LLVMGetElementType(cl2llvmTypeWrapGetLlvmType(value->type));

		veclength = LLVMGetVectorSize(cl2llvmTypeWrapGetLlvmType(value->type));
		switch (LLVMGetTypeKind(LLVMGetElementType(cl2llvmTypeWrapGetLlvmType(value->type))))
		{
		case LLVMIntegerTypeKind:
		
			/* Create zero vector */
			for (i = 0; i < veclength; i++)
				zero_vec[i] = LLVMConstInt(switch_type, 0, 0);
			break;
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:
		case LLVMHalfTypeKind:
			
			/* Create zero vector */
			for (i = 0; i < veclength; i++)
				zero_vec[i] = LLVMConstReal(switch_type, 0);
			break;
		default:
			cl2llvm_yyerror("unreachable code reached");
		}
		const_zero = LLVMConstVector(zero_vec, veclength);
	}
	else if (LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(value->type)) == LLVMIntegerTypeKind)
	{
		const_zero = LLVMConstInt(cl2llvmTypeWrapGetLlvmType(value->type), 0, 0);
		switch_type = cl2llvmTypeWrapGetLlvmType(value->type);
	}
	else if (LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(value->type)) == LLVMFloatTypeKind
		|| LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(value->type)) == LLVMDoubleTypeKind
		|| LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(value->type)) == LLVMHalfTypeKind)
	{
		const_zero = LLVMConstReal(cl2llvmTypeWrapGetLlvmType(value->type), 0);
		switch_type = cl2llvmTypeWrapGetLlvmType(value->type);
	}
	/* Create comparison */
	snprintf(temp_var_name, sizeof temp_var_name,
		"tmp_%d", temp_var_count++);

	switch (LLVMGetTypeKind(switch_type))
	{
	case LLVMFloatTypeKind:
	case LLVMDoubleTypeKind:
	case LLVMHalfTypeKind:

		bool_val->val = LLVMBuildFCmp(cl2llvm_builder, LLVMRealONE, 
			value->val, const_zero, temp_var_name);
		break;
	case LLVMIntegerTypeKind:

		bool_val->val = LLVMBuildICmp(cl2llvm_builder, LLVMIntNE, 
			value->val, const_zero, temp_var_name);
		break;
	default:
		cl2llvm_yyerror("unreachable code reached");
		break;
	}
	cl2llvmTypeWrapSetLlvmType(bool_val->type, LLVMInt1Type());
	cl2llvmTypeWrapSetSign(bool_val->type, 0);

	return bool_val;
}

/* This function returns an i1 1 if the value is equal to 0 and 
   an i1 0 if the value is not equal to 0. */
struct cl2llvm_val_t *cl2llvm_to_bool_eq_0(struct cl2llvm_val_t *value)
{
	LLVMValueRef const_zero;
	LLVMValueRef zero_vec[16];
	int i;
	int veclength;
	LLVMTypeRef switch_type;
	struct cl2llvm_val_t *bool_val = cl2llvm_val_create_w_init(value->val, cl2llvmTypeWrapGetSign(value->type));
	
	/* if value is i1 no conversion necessary */
	if (LLVMTypeOf(value->val) == LLVMInt1Type())
		return bool_val;

	/* If value is a vector create a vector of constant zeros, else
	   create a scalar 0. */
	if (LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(value->type)) == LLVMVectorTypeKind)
	{
		veclength = LLVMGetVectorSize(cl2llvmTypeWrapGetLlvmType(value->type));
		switch_type = LLVMGetElementType(cl2llvmTypeWrapGetLlvmType(value->type));

		switch (LLVMGetTypeKind(switch_type))
		{
		case LLVMIntegerTypeKind:
		
			/* Create zero vector */
			for (i = 0; i < veclength; i++)
				zero_vec[i] = LLVMConstInt(switch_type, 0, 0);
			break;
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:
		case LLVMHalfTypeKind:
			
			/* Create zero vector */
			for (i = 0; i < veclength; i++)
				zero_vec[i] = LLVMConstReal(switch_type, 0);
			break;
		default:
			cl2llvm_yyerror("unreachable code reached");
		}
		const_zero = LLVMConstVector(zero_vec, veclength);
	}
	else if (LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(value->type)) == LLVMIntegerTypeKind)
	{
		const_zero = LLVMConstInt(cl2llvmTypeWrapGetLlvmType(value->type), 0, 0);
		switch_type = cl2llvmTypeWrapGetLlvmType(value->type);
	}
	else if (LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(value->type)) == LLVMFloatTypeKind
		|| LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(value->type)) == LLVMDoubleTypeKind
		|| LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(value->type)) == LLVMHalfTypeKind)
	{
		const_zero = LLVMConstReal(cl2llvmTypeWrapGetLlvmType(value->type), 0);
		switch_type =  cl2llvmTypeWrapGetLlvmType(value->type);
	}
	/* Create comparison */
	snprintf(temp_var_name, sizeof temp_var_name,
		"tmp_%d", temp_var_count++);

	switch (LLVMGetTypeKind(switch_type))
	{
	case LLVMFloatTypeKind:
	case LLVMDoubleTypeKind:
	case LLVMHalfTypeKind:

		bool_val->val = LLVMBuildFCmp(cl2llvm_builder, LLVMRealOEQ, 
			value->val, const_zero, temp_var_name);
		break;
	case LLVMIntegerTypeKind:

		bool_val->val = LLVMBuildICmp(cl2llvm_builder, LLVMIntEQ, 
			value->val, const_zero, temp_var_name);
		break;
	default:
		cl2llvm_yyerror("unreachable code reached");
		break;
	}
	cl2llvmTypeWrapSetLlvmType(bool_val->type, LLVMInt1Type());
	cl2llvmTypeWrapSetSign(bool_val->type, 0);

	return bool_val;
}

/* This function will take a bool and sign extend it to a specified bitwidth.
   It will also perform i1 to floating point conversions if necessary. All vector
   components that are equal to 1 will be converted to -1 in accordance with the 
   OpenCL standard. */
struct cl2llvm_val_t *cl2llvm_bool_ext(struct cl2llvm_val_t *bool_val,
	struct cl2llvmTypeWrap *type)
{
	struct cl2llvm_val_t *value;
	struct cl2llvmTypeWrap *switch_type;
	LLVMTypeRef totype;
	int vec_length;

	switch_type = cl2llvmTypeWrapCreate(cl2llvmTypeWrapGetLlvmType(type), cl2llvmTypeWrapGetSign(type));

	if (LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(type)) == LLVMVectorTypeKind)		
		cl2llvmTypeWrapSetLlvmType(switch_type, LLVMGetElementType(cl2llvmTypeWrapGetLlvmType(type)));
		

	if (LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(type)) == LLVMVectorTypeKind)
	{
		vec_length = LLVMGetVectorSize(cl2llvmTypeWrapGetLlvmType(type));
		switch (LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(switch_type)))
		{
		case LLVMIntegerTypeKind:
			totype = cl2llvmTypeWrapGetLlvmType(type);
			break;
		case LLVMFloatTypeKind:
			totype = LLVMVectorType(LLVMInt32Type(), vec_length);
			break;
		case LLVMDoubleTypeKind:
			totype = LLVMVectorType(LLVMInt64Type(), vec_length);
			break;
		case LLVMHalfTypeKind:
			totype = LLVMVectorType(LLVMInt16Type(), vec_length);
			break;
		default:
			cl2llvm_yyerror("unreachable code reached");
			break;
		}
	}
	else
		totype = LLVMInt32Type();
	
	value = cl2llvm_val_create();

	snprintf(temp_var_name, sizeof temp_var_name,
		"tmp_%d", temp_var_count++);

	/* Build sign extension */
	value->val = LLVMBuildSExt(cl2llvm_builder, 
		bool_val->val, totype, temp_var_name);	
	cl2llvmTypeWrapSetLlvmType(value->type, totype);
	cl2llvmTypeWrapSetSign(value->type, 1);

	/* if value is a vector, change 1's to -1's */
	if (LLVMGetTypeKind(cl2llvmTypeWrapGetLlvmType(type)) == LLVMVectorTypeKind)
	{
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);

		value->val = LLVMBuildNeg(cl2llvm_builder, 
			value->val, temp_var_name);
	}

	cl2llvmTypeWrapFree(switch_type);

	return value;
}
