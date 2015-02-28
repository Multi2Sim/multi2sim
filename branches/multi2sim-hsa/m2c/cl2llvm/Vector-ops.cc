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

#include <stdio.h>
#include <string.h>
#include <sstream>

#include <lib/util/list.h>

#include "Vector-ops.h"

extern LLVMBuilderRef cl2llvm_builder;
extern char temp_var_name[50];
extern int temp_var_count;

extern void cl2llvm_yyerror(char *);

void ExpandVectors(list<Value>& elem_list)
{
	int index;
	int vec_index;
	Value cl2llvm_index;
	Value current_vec_elem;
	Value current_elem;
	
	for(list<Value>::iterator current_elem = elem_list.begin();
		current_elem != elem_list.end(); ++current_elem)
	{
		if (current_elem->getLlvmType().getTypeID() == VectorTyID)
		{

			for(vec_index = 0; vec_index < current_elem.getLlvmType().getNumElements(); vec_index++)
			{
				cl2llvm_index = Value(llvm::Constant::get(llvm::Type::getInt32Ty(context), vec_index, 0), 1);

				current_vec_elem = Value( cl2llvm_builder.CreateExtractElement(current_elem->getLlvmValue(), cl2llvm_index->getLlvmValue(), TempVarName()), current_elem.getSign());
				
				elem_list.insert(index, current_vec_elem);
			}
			index = elem_list.erase(index);
			--index;
		}
	}
}


void Value::GetVectorIndices(string idx_str);
{
	int i;
	int boundary_error = 0;
	int vector_size;
	int s_prefix = 0;
	Value index;

	if (!this->getLlvmType().isVectorTy())
		return;

	vector_size = this->getLlvmType().getNumElements();
	i = 0;

	if (idx_str.c_str()[0] == 's' || idx_str.c_str()[0] == 'S')
	{
		s_prefix = 1;
		i++;
	}


	for (; i < idx_str.size(); i++)
	{
		/* Check that number of specified components does not exceed 16 */
		if (i > 16 && !s_prefix)
			cl2llvm_yyerror("Too many components for vector type");
		if (i > 17)
			cl2llvm_yyerror("Too many components for vector type");

		Value index;
		index.setLlvmType(getInt32Ty(context));
		index.setSign(1);

		switch (idx_str.c_str()[i])
		{
			case 'x':
				if (!s_prefix)
				{
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 0, 0);
					this->vector_indices[i] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case 'y':
				if (!s_prefix)
				{
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 1, 0);
					this->vector_indices[i] = index;
			
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;


			case 'z':
				if (!s_prefix)
				{
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 2, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 3, 0);
					this->vector_indices[i] = index;
					
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 0, 0);
					this->vector_indices[i] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;

			case '1':
				if(s_prefix)
				{
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 1, 0);
					this->vector_indices[i] = index;
				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;
		
			case '2':
				if(s_prefix)
				{
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 2, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 3, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 4, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 5, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 6, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 7, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 8, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 9, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 10, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 11, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 12, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 13, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 14, 0);
					this->vector_indices[i] = index;
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
					index.setLlvmValue(llvm::Constant::get(llvm::Type::getInt32Ty(context), 15, 0);
					this->vector_indices[i] = index;
					if(vector_size < 16)
						boundary_error = 1;

				}
				else
					cl2llvm_yyerror("Invalid syntax for vector index (may not combine"
						"'x', 'y', 'z'... with '0', '1', '2'...)");
				break;
			default:
				stringstream ss;
				ss << "Invalid character '" << idx_str.c_str()[i]
				<< "' in vector index";
				cl2llvm_yyerror(ss.str());
		}
	}
	if (boundary_error)
		cl2llvm_yyerror("Component reference is outside boundary of vector");
}

Value BuildComponentWiseAssignment(Value lvalue_addr, 
	Value rvalue);
{
	int component_count = 0;
	int i;
	Type component_type;
	Value lvalue;
	Value new_lvalue;
	Value cast_rvalue;
	llvm::Value *llvm_index;
	llvm::Value *component;
	llvm::Value new_lvalue_val;
	
	/* Load vector */
	Value lvalue(cl2llvm_builder.CreateLoad(lvalue_addr.getLlvmValue(),
		TempVarName()), lvalue_addr.getSign());
	
	new_lvalue_val = lvalue.getLlvmValue();
	
	/* Create object to  represent component type of lvalue. */
	Type component_type(lvalue.getLlvmType().getElementType(), 
		lvalue.getSign());
	
	/* Get number of components referenced by lvalue. */
	component_count = lvalue_addr.getVectorIndices().size();
	
	
	/* Check that none of the vector's components are referenced twice */
	NoRepeatedComponentReferences(lvalue_addr);

	/* If rvalue is a vector */
	if (rvalue.getLlvmType().getTypeID() == VectorTyID)
	{
		/* Check that element type of rvalue vector matches element type of 
		lvalue vector */
		if (rvalue.getLlvmType().getElementType() != lvalue.getLlvmType().getElementType())
			cl2llvm_yyerror("Type mis-match. (casts between vector types are forbidden)");

		/* Check that size of vector matches number of components specified 
		in lvalue. */
		if (rvalue.getLlvmType().getNumElements() != component_count)
			cl2llvm_yyerror("Size of vector does not match number of components specified in lvalue.");

		/* Extract each component from rvalue and assign it to the specified
		component of the lvalue. */

		for (i = 0; i < component_count; i++)
		{
			llvm_index = llvm::Constant::get(llvm::Type::getInt32Ty(context), i, 0);

			/* Extract component from rvalue */
			component = cl2llvm_builder.CreateExtractElement(rvalue.getLlvmValue(), llvm_index, TempVarName());

			/* Insert component into lvalue */
			new_lvalue_val = cl2llvm_builder.CreateInsertElement( new_lvalue_val, 
				component, lvalue_addr.getVectorIndices()[i].getLlvmValue, TempVarName());
		}
	}

	/* If rvalue is a scalar, assign this value to every specified component of the lavlue */
	else
	{
		rvalue.Cast(component_type);
	
		for (i = 0; i < component_count; i++)
		{
			/* Insert component into lvalue */
		
			new_lvalue_val = cl2llvm_builder.CreateInsertElement( new_lvalue_val, 
				rvalue, lvalue_addr.getVectorIndices()[i].getLlvmValue, TempVarName());
		}
	}

	Value new_lvalue(new_lvalue_val, component_type.getSign());

	return new_lvalue;
}


void NoRepeatedComponentReferences(Value lvalue)
{
	int index_0 = 0, index_1 = 0, index_2 = 0, index_3 = 0, index_4 = 0, 
		index_5 = 0, index_6 = 0,  index_7 = 0, index_8 = 0, index_9 = 0, 
		index_10 = 0, index_11 = 0, index_12 = 0, index_13 = 0, 
		index_14 = 0, index_15 = 0;
	int error = 0;

	for(int i = 0; i < lvalue.getVectorIndices().size(); i++)
	{
		if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 0, 0))
		{
			if(!index_0)
				index_0 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 1, 0))
		{
			if(!index_1)
				index_1 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 2, 0))
		{
			if(!index_2)
				index_2 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 3, 0))
		{
			if(!index_3)
				index_3 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 4, 0))
		{
			if(!index_4)
				index_4 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 5, 0))
		{
			if(!index_5)
				index_5 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 6, 0))
		{
			if(!index_6)
				index_6 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 7, 0))
		{
			if(!index_7)
				index_7 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 8, 0))
		{
			if(!index_8)
				index_8 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 9, 0))
		{
			if(!index_9)
				index_9 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 10, 0))
		{
			if(!index_10)
				index_10 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 11, 0))
		{
			if(!index_11)
				index_11 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 12, 0))
		{
			if(!index_12)
				index_12 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 13, 0))
		{
			if(!index_13)
				index_13 = 1;
			else
				error = 1;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 14, 0))
		{
			if(!index_14)
				index_14 = 1;
			else
				error = 14;
		}
		else if(lvalue.getVectorIndices[i].getLlvmValue() == llvm::Constant::get(llvm::Type::getInt32Ty(context), 15, 0))
		{
			if(!index_15)
				index_15 = 1;
			else
				error = 1;
		}
		if (error)
			cl2llvm_yyerror("Invalid lvalue. (May not reference the same vector component twice in an lvalue)");
	}
}
