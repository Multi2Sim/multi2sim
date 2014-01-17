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


#include <lib/mhandle/mhandle.h>

#include "function.h"
#include "val.h"
#include "cl2llvm.h"

#include "Value.h"

using namespace cl2llvm;
using namespace std;

/*
 * C++ Code
 */


extern int temp_var_count;
extern char temp_var_name[50];

extern LLVMBuilderRef cl2llvm_builder;
extern struct cl2llvm_function_t *cl2llvm_current_function;
extern LLVMBasicBlockRef current_basic_block;

Value Value::TypeCast(Type to_type)
{
	llvm::Value llvm_value;
	int i;
	Type elem_type;
	Value cast_original_val;
	LLVMValueRef index;
	LLVMValueRef vector_addr;
	LLVMValueRef vector;
	LLVMValueRef const_elems[16];

	/*By default the return value is the same as the original_val*/
	llvm_val = this->llvm_value;
	sign = this->type.getSign();

	temp_var_name = TempVarName();

	/* Check that fromtype is not a vector, unless both types are identical. */
	if (this->type.getLlvmType().getTypeID() == VectorTyID)
	{
		if ((LLVMGetVectorSize(this->type.getLlvmType()) != LLVMGetVectorSize(to_type.getLlvmType()) 
			|| LLVMGetElementType(this->type.getLlvmType()) 
			!= LLVMGetElementType(to_type.getLlvmType())) 
			|| this->getSign() != to_type.getSign())
		{
			if (to_type.getLlvmType().getTypeID() == VectorTyID)
				cl2llvm_yyerror("Casts between vector types are forbidden");
			cl2llvm_yyerror("A vector may not be cast to any other type.");
		}
	}

	/* If to_type.getLlvmType() is a vector, create a vector whose components are equal to 
	original_val */

	if (to_type.getLlvmType().getTypeID() == VectorTyID
		&& this->type.getLlvmType().getTypeID() != VectorTyID)
	{
		/*Go to entry block and declare vector*/
		LLVMPositionBuilder(cl2llvm_builder, cl2llvm_current_function->entry_block,
			cl2llvm_current_function->branch_instr);
		
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
			
		vector_addr = LLVMBuildAlloca(cl2llvm_builder, 
			to_type.getLlvmType(), temp_var_name);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, current_basic_block);

		/* Load vector */
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
	
		vector = LLVMBuildLoad(cl2llvm_builder, vector_addr, temp_var_name);
		
		/* Create object to represent element type of totype */
		elem_type = cl2llvmTypeWrapCreate(LLVMGetElementType(to_type.getLlvmType()), to_type.getSign());

		/* If original_val is constant create a constant vector */
		if (LLVMIsConstant(this->llvm_value))
		{
			cast_original_val = llvm_type_cast(original_val, elem_type);
			for (i = 0; i < LLVMGetVectorSize(to_type.getLlvmType()); i++)
				const_elems[i] = cast_original_val->val;

			vector = LLVMConstVector(const_elems, 	
				LLVMGetVectorSize(to_type.getLlvmType()));
			llvm_val = vector;

			cl2llvm_val_free(cast_original_val);
		}
		/* If original value is not constant insert elements */
		else
		{
			for (i = 0; i < LLVMGetVectorSize(to_type.getLlvmType()); i++)
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
		llvm_val = vector;
	}


	if (this->type.getLlvmType() == LLVMInt64Type())
	{
		if (to_type.getLlvmType() == LLVMDoubleType())
		{
			if (this->getSign())
			{
				llvm_val =
						LLVMBuildSIToFP(cl2llvm_builder,
						  this->llvm_value, LLVMDoubleType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMDoubleType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMFloatType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMFloatType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMFloatType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMHalfType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMHalfType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMHalfType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMInt64Type())
		{
			if (to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
			temp_var_count--;
		}
		else if (to_type.getLlvmType() == LLVMInt32Type())
		{
			llvm_val = LLVMBuildTrunc(cl2llvm_builder,
				  this->llvm_value, LLVMInt32Type(), temp_var_name);
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt16Type())
		{
			llvm_val = LLVMBuildTrunc(cl2llvm_builder,
				  this->llvm_value, LLVMInt16Type(), temp_var_name);
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt8Type())
		{
			llvm_val = LLVMBuildTrunc(cl2llvm_builder,
				  this->llvm_value, LLVMInt8Type(), temp_var_name);
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt1Type())
		{
			llvm_val = LLVMBuildTrunc(cl2llvm_builder,
				  this->llvm_value, LLVMInt1Type(), temp_var_name);
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
			
	}
	else if (this->type.getLlvmType() == LLVMInt32Type())
	{
		if (to_type.getLlvmType() == LLVMDoubleType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMDoubleType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMDoubleType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMFloatType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMFloatType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMFloatType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMHalfType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMHalfType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMHalfType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMInt64Type())
		{
			if (this->getSign())
			{
				llvm_val = LLVMBuildSExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt64Type(),
					temp_var_name);
			}
			else
			{
				llvm_val = LLVMBuildZExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt64Type(),
					temp_var_name);
			}
			if (to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt32Type())
		{
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
			temp_var_count--;
		}
		else if (to_type.getLlvmType() == LLVMInt16Type())
		{
			llvm_val = LLVMBuildTrunc(cl2llvm_builder,
				  this->llvm_value, LLVMInt16Type(), temp_var_name);
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt8Type())
		{
			llvm_val = LLVMBuildTrunc(cl2llvm_builder,
				 this->llvm_value, LLVMInt8Type(), temp_var_name);
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt1Type())
		{
			llvm_val = LLVMBuildTrunc(cl2llvm_builder,
				  this->llvm_value, LLVMInt1Type(), temp_var_name);
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
			
	}
	else if (this->type.getLlvmType() == LLVMInt16Type())
	{
		if (to_type.getLlvmType() == LLVMDoubleType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMDoubleType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMDoubleType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMFloatType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMFloatType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMFloatType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMHalfType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMHalfType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMHalfType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMInt64Type())
		{
			if (this->getSign())
			{
				llvm_val = LLVMBuildSExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt64Type(),
					temp_var_name);
			}
			else
			{
				llvm_val = LLVMBuildZExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt64Type(),
					temp_var_name);
			}
			if (to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt32Type())
		{
			if (this->getSign())
			{
				llvm_val = LLVMBuildSExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt32Type(),
					temp_var_name);
			}
			else
			{
				llvm_val = LLVMBuildZExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt32Type(),
					temp_var_name);
			}
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt16Type())
		{
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
			temp_var_count--;
		}
		else if (to_type.getLlvmType() == LLVMInt8Type())
		{
			llvm_val = LLVMBuildTrunc(cl2llvm_builder,
				  this->llvm_value, LLVMInt8Type(), temp_var_name);
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt1Type())
		{
			llvm_val = LLVMBuildTrunc(cl2llvm_builder,
				  this->llvm_value, LLVMInt1Type(), temp_var_name);
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
			
	}
	else if (this->type.getLlvmType() == LLVMInt8Type())
	{
		if (to_type.getLlvmType() == LLVMDoubleType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMDoubleType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMDoubleType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMFloatType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMFloatType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMFloatType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMHalfType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMHalfType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMHalfType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMInt64Type())
		{
			if (this->getSign())
			{
				llvm_val = LLVMBuildSExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt64Type(),
					temp_var_name);
			}
			else
			{
				llvm_val = LLVMBuildZExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt64Type(),
					temp_var_name);
			}
			if (to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt32Type())
		{
			if (this->getSign())
			{
				llvm_val = LLVMBuildSExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt32Type(),
					temp_var_name);
			}
			else
			{
				llvm_val = LLVMBuildZExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt32Type(),
					temp_var_name);
			}
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt16Type())
		{
			if (this->getSign())
			{
				llvm_val = LLVMBuildSExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt16Type(),
					temp_var_name);
			}
			else
			{
				llvm_val = LLVMBuildZExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt16Type(),
					temp_var_name);
			}
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt8Type())
		{
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
			temp_var_count--;
		}
		else if (to_type.getLlvmType() == LLVMInt1Type())
		{
			llvm_val = LLVMBuildTrunc(cl2llvm_builder,
				  this->llvm_value, LLVMInt1Type(), temp_var_name);
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
			
	}
	else if (this->type.getLlvmType() == LLVMInt1Type())
	{
		if (to_type.getLlvmType() == LLVMDoubleType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMDoubleType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMDoubleType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMFloatType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMFloatType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMFloatType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMHalfType())
		{
			if (this->getSign())
			{
				llvm_val =
					LLVMBuildSIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMHalfType(),
					temp_var_name);
			}
			else
			{
				llvm_val =
					LLVMBuildUIToFP(cl2llvm_builder,
					  this->llvm_value, LLVMHalfType(),
					temp_var_name);
			}
			llvm_val->type.setSign( 1);
		}
		else if (to_type.getLlvmType() == LLVMInt64Type())
		{
			if (this->getSign())
			{
				llvm_val = LLVMBuildSExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt64Type(),
					temp_var_name);
			}
			else
			{
				llvm_val = LLVMBuildZExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt64Type(),
					temp_var_name);
			}
			if (to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt32Type())
		{
			if (this->getSign())
			{
				llvm_val = LLVMBuildSExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt32Type(),
					temp_var_name);
			}
			else
			{
				llvm_val = LLVMBuildZExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt32Type(),
					temp_var_name);
			}
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt16Type())
		{
			if (this->getSign())
			{
				llvm_val = LLVMBuildSExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt16Type(),
					temp_var_name);
			}
			else
			{
				llvm_val = LLVMBuildZExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt16Type(),
					temp_var_name);
			}
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt8Type())
		{
			if (this->getSign())
			{
				llvm_val = LLVMBuildSExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt8Type(),
					temp_var_name);
			}
			else
			{
				llvm_val = LLVMBuildZExt(cl2llvm_builder,
					  this->llvm_value, LLVMInt8Type(),
					temp_var_name);
			}
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
		}
		else if (to_type.getLlvmType() == LLVMInt1Type())
		{
			if(to_type.getSign())
				llvm_val->type.setSign( 1);
			else
				llvm_val->type.setSign( 0);
			temp_var_count--;
		}			
	}

	/*We now know that from type must be a floating point.*/

	/*Floating point to signed integer conversions*/
	else if (to_type.getSign() && to_type.getLlvmType().getTypeID() == 8)
	{
		if (to_type.getLlvmType() == LLVMInt64Type())
		{
			llvm_val = LLVMBuildFPToSI(cl2llvm_builder, 
				  this->llvm_value, LLVMInt64Type(), temp_var_name);
		}
		else if (to_type.getLlvmType() == LLVMInt32Type())
		{
			llvm_val = LLVMBuildFPToSI(cl2llvm_builder, 
				  this->llvm_value, LLVMInt32Type(), temp_var_name);
		}
		else if (to_type.getLlvmType() == LLVMInt16Type())
		{
			llvm_val = LLVMBuildFPToSI(cl2llvm_builder, 
				  this->llvm_value, LLVMInt16Type(), temp_var_name);
		}
		else if (to_type.getLlvmType() == LLVMInt8Type())
		{
			llvm_val = LLVMBuildFPToSI(cl2llvm_builder, 
				  this->llvm_value, LLVMInt8Type(), temp_var_name);
		}
		else if (to_type.getLlvmType() == LLVMInt1Type())
		{
			llvm_val = LLVMBuildFPToSI(cl2llvm_builder, 
				  this->llvm_value, LLVMInt1Type(), temp_var_name);
		}
		llvm_val->type.setSign( 1);
	}
	/*Floating point to unsigned integer conversions*/
	else if (!to_type.getSign())
	{
		if (to_type.getLlvmType() == LLVMInt64Type())
		{
			llvm_val = LLVMBuildFPToUI(cl2llvm_builder, 
				  this->llvm_value, LLVMInt64Type(), temp_var_name);
		}
		else if (to_type.getLlvmType() == LLVMInt32Type())
		{
			llvm_val = LLVMBuildFPToUI(cl2llvm_builder, 
				  this->llvm_value, LLVMInt32Type(), temp_var_name);
		}
		else if (to_type.getLlvmType() == LLVMInt16Type())
		{
			llvm_val = LLVMBuildFPToUI(cl2llvm_builder, 
				  this->llvm_value, LLVMInt16Type(), temp_var_name);
		}
		else if (to_type.getLlvmType() == LLVMInt8Type())
		{
			llvm_val = LLVMBuildFPToUI(cl2llvm_builder, 
				  this->llvm_value, LLVMInt8Type(), temp_var_name);
		}
		else if (to_type.getLlvmType() == LLVMInt1Type())
		{
			llvm_val = LLVMBuildFPToUI(cl2llvm_builder, 
				  this->llvm_value, LLVMInt1Type(), temp_var_name);
		}
		llvm_val->type.setSign( 0);
	}
	else if (to_type.getLlvmType() == LLVMDoubleType())
	{
		llvm_val = LLVMBuildFPExt(cl2llvm_builder, 
			  this->llvm_value, LLVMDoubleType(), temp_var_name);
		llvm_val->type.setSign( 1);
	}
	else if (to_type.getLlvmType() == LLVMFloatType())
	{
		if (this->type.getLlvmType() == LLVMDoubleType())
		{
			llvm_val = LLVMBuildFPTrunc(cl2llvm_builder, 
				  this->llvm_value, LLVMFloatType(), temp_var_name);
		}
		else if (this->type.getLlvmType() == LLVMHalfType())
		{
			llvm_val = LLVMBuildFPExt(cl2llvm_builder, 
				  this->llvm_value, LLVMFloatType(), temp_var_name);
		}
		llvm_val->type.setSign( 1);
	}
	else if (to_type.getLlvmType() == LLVMHalfType())
	{
		llvm_val = LLVMBuildFPTrunc(cl2llvm_builder, 
			  this->llvm_value, LLVMHalfType(), temp_var_name);
		llvm_val->type.setSign( 1);
	}
	cl2llvmTypeWrapSetLlvmType(llvm_val->type, to_type.getLlvmType());
	cl2llvmTypeWrapSetSign(llvm_val->type, to_type.getSign());
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

void type_unify(Value val1, Value val2, Value *new_val1, Value *new_val2)
{
	LLVMTypeRef type1 = val1->val.getType();
	LLVMTypeRef type2 = val2->val.getType();

	/* By default, new values returned are the same as the original
	 * values. */
	*new_val1 = val1;
	*new_val2 = val2;

	/* If types match, no type cast needed */
	if (type1 == type2 && val1->type.getSign() == val2->type.getSign())
		return;
	
	/* If the types do not match and both are vector types, return error */
	if (type1.getTypeID() == VectorTyID 
		&& type2.getTypeID() == VectorTyID)
	{
		cl2llvm_yyerror("Type mis-match. (Type of both operands of a vector operator must be an exact match.)");
	}

	/* Obtain dominant type */
	Type type = cl2llvmTypeWrapCompare(val1->type, val2->type);

	/* Whatever operand differs from the dominant type will be typecast
	 * to it. */
	if (type.getType() != type1 || type.getSign() != val1->type.getSign())
	{
		*new_val1 = llvm_type_cast(val1, type);
	}
	else
	{
		*new_val2 = llvm_type_cast(val2, type);
	}

	/* Free pointers */
	cl2llvmTypeWrapFree(type);
}

/* This function returns an i1 1 if the value is not equal to 0 and 
   an i1 0 if the value is equal to 0. */
Value cl2llvm_to_bool_ne_0(Value value)
{
	LLVMValueRef const_zero;
	LLVMValueRef zero_vec[16];
	LLVMTypeRef switch_type;
	int i;
	int veclength;

	Value bool_val = cl2llvm_val_create_w_init(value->val, value->type.getSign());
	
	/* if value is i1 no conversion necessary */
	if (value->val.getType() == LLVMInt1Type())
		return bool_val;

	/* If value is a vector create a vector of constant zeros, else
	   create a scalar 0. */
	if (value->type.getType().getTypeID() == VectorTyID)
	{
		switch_type = LLVMGetElementType(value->type.getType());

		veclength = LLVMGetVectorSize(value->type.getType());
		switch (LLVMGetElementType(value->type.getType()).getTypeID())
		{
		case IntegerTyID:
		
			/* Create zero vector */
			for (i = 0; i < veclength; i++)
				zero_vec[i] = LLVMConstInt(switch_type, 0, 0);
			break;
		case FloatTyID:
		case DoubleTyID:
		case HalfTyID:
			
			/* Create zero vector */
			for (i = 0; i < veclength; i++)
				zero_vec[i] = LLVMConstReal(switch_type, 0);
			break;
		default:
			cl2llvm_yyerror("unreachable code reached");
		}
		const_zero = LLVMConstVector(zero_vec, veclength);
	}
	else if (value->type.getType().getTypeID() == IntegerTyID)
	{
		const_zero = LLVMConstInt(value->type.getType(), 0, 0);
		switch_type = value->type.getType();
	}
	else if (value->type.getType().getTypeID() == FloatTyID
		|| value->type.getType().getTypeID() == DoubleTyID
		|| value->type.getType().getTypeID() == HalfTyID)
	{
		const_zero = LLVMConstReal(value->type.getType(), 0);
		switch_type = value->type.getType();
	}
	/* Create comparison */
	snprintf(temp_var_name, sizeof temp_var_name,
		"tmp_%d", temp_var_count++);

	switch (switch_type.getTypeID())
	{
	case FloatTyID:
	case DoubleTyID:
	case HalfTyID:

		bool_val->val = LLVMBuildFCmp(cl2llvm_builder, LLVMRealONE, 
			value->val, const_zero, temp_var_name);
		break;
	case IntegerTyID:

		bool_val->val = LLVMBuildICmp(cl2llvm_builder, LLVMIntNE, 
			value->val, const_zero, temp_var_name);
		break;
	default:
		cl2llvm_yyerror("unreachable code reached");
		break;
	}
	cl2llvmTypeWrapSetLlvmType(bool_val->type, LLVMInt1Type());
	bool_val->type.setSign( 0);

	return bool_val;
}

/* This function returns an i1 1 if the value is equal to 0 and 
   an i1 0 if the value is not equal to 0. */
Value cl2llvm_to_bool_eq_0(Value value)
{
	LLVMValueRef const_zero;
	LLVMValueRef zero_vec[16];
	int i;
	int veclength;
	LLVMTypeRef switch_type;
	Value bool_val = cl2llvm_val_create_w_init(value->val, value->type.getSign());
	
	/* if value is i1 no conversion necessary */
	if (value->val.getType() == LLVMInt1Type())
		return bool_val;

	/* If value is a vector create a vector of constant zeros, else
	   create a scalar 0. */
	if (value->type.getType().getTypeID() == VectorTyID)
	{
		veclength = LLVMGetVectorSize(value->type.getType());
		switch_type = LLVMGetElementType(value->type.getType());

		switch (switch_type.getTypeID())
		{
		case IntegerTyID:
		
			/* Create zero vector */
			for (i = 0; i < veclength; i++)
				zero_vec[i] = LLVMConstInt(switch_type, 0, 0);
			break;
		case FloatTyID:
		case DoubleTyID:
		case HalfTyID:
			
			/* Create zero vector */
			for (i = 0; i < veclength; i++)
				zero_vec[i] = LLVMConstReal(switch_type, 0);
			break;
		default:
			cl2llvm_yyerror("unreachable code reached");
		}
		const_zero = LLVMConstVector(zero_vec, veclength);
	}
	else if (value->type.getType().getTypeID() == IntegerTyID)
	{
		const_zero = LLVMConstInt(value->type.getType(), 0, 0);
		switch_type = value->type.getType();
	}
	else if (value->type.getType().getTypeID() == FloatTyID
		|| value->type.getType().getTypeID() == DoubleTyID
		|| value->type.getType().getTypeID() == HalfTyID)
	{
		const_zero = LLVMConstReal(value->type.getType(), 0);
		switch_type =  value->type.getType();
	}
	/* Create comparison */
	snprintf(temp_var_name, sizeof temp_var_name,
		"tmp_%d", temp_var_count++);

	switch (switch_type.getTypeID())
	{
	case FloatTyID:
	case DoubleTyID:
	case HalfTyID:

		bool_val->val = LLVMBuildFCmp(cl2llvm_builder, LLVMRealOEQ, 
			value->val, const_zero, temp_var_name);
		break;
	case IntegerTyID:

		bool_val->val = LLVMBuildICmp(cl2llvm_builder, LLVMIntEQ, 
			value->val, const_zero, temp_var_name);
		break;
	default:
		cl2llvm_yyerror("unreachable code reached");
		break;
	}
	cl2llvmTypeWrapSetLlvmType(bool_val->type, LLVMInt1Type());
	bool_val->type.setSign( 0);

	return bool_val;
}

/* This function will take a bool and sign extend it to a specified bitwidth.
   It will also perform i1 to floating point conversions if necessary. All vector
   components that are equal to 1 will be converted to -1 in accordance with the 
   OpenCL standard. */
Value cl2llvm_bool_ext(Value bool_val,
	Type type)
{
	Value value;
	Type switch_type;
	LLVMTypeRef totype;
	int vec_length;

	switch_type = cl2llvmTypeWrapCreate(type.getType(), type.getSign());

	if (type.getType().getTypeID() == VectorTyID)		
		cl2llvmTypeWrapSetLlvmType(switch_type, LLVMGetElementType(type.getType()));
		

	if (type.getType().getTypeID() == VectorTyID)
	{
		vec_length = LLVMGetVectorSize(type.getType());
		switch (switch_type.getType().getTypeID())
		{
		case IntegerTyID:
			totype = type.getType();
			break;
		case FloatTyID:
			totype = LLVMVectorType(LLVMInt32Type(), vec_length);
			break;
		case DoubleTyID:
			totype = LLVMVectorType(LLVMInt64Type(), vec_length);
			break;
		case HalfTyID:
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
	value->type.setLlvmType( totype);
	value->type.setSign( 1);

	/* if value is a vector, change 1's to -1's */
	if (type.getType().getTypeID() == VectorTyID)
	{
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);

		value->val = LLVMBuildNeg(cl2llvm_builder, 
			value->val, temp_var_name);
	}

	cl2llvmTypeWrapFree(switch_type);

	return value;
}

struct cl2llvmValueWrap *cl2llvmValueWrapCreate(void *llvm_value,
		int sign);
void cl2llvmValueWrapFree(struct cl2llvmValueWrap *value);
void* cl2llvmValueWrapGetLlvmValue(struct cl2llvmValueWrap *value);
void cl2llvmValueWrapSetLlvmValue(struct cl2llvmValueWrap* value, void *llvm_value);
void* cl2llvmValueWrapGetType(struct cl2llvmValueWrap *value);
void cl2llvmValueWrapSetType(struct cl2llvmValueWrap* value, Type *type);
void cl2llvmValueWrapSetSign(struct cl2llvmValueWrap* value, int sign);
bool cl2llvmValueWrapGetSign(struct cl2llvmValueWrap* value);
