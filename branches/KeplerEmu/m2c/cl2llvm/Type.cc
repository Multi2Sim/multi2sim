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

#include "Type.h"

using namespace cl2llvm;
using namespace std;

/*
 * C++ Code
 */

/*
 * This function takes two cl2llvm_value_t's and returns the type of the   
 * dominant operand. The return value points to a newly created cl2llvm_type_t 
 * which must be freed using cl2llvm_type_free().
 */
const int TYPE_CMP_TABLE_SIZE  = 31;

Type *Type::Compare(Type *type2)
{

	llvm::Type *type1_type = llvm_type;
	llvm::Type *type2_type = type2->llvm_type;
	bool type1_sign = sign;
	bool type2_sign = type2->sign;

	/* Set return value equal to type1 */
	Type* dom_type = this;

	struct llvm_type_const
	{
		llvm::Type* llvm_type;
		bool sign;
	};

	struct llvm_type_table 
	{
		struct llvm_type_const type1;
		struct llvm_type_const type2;
	};
		
	struct llvm_type_table table[TYPE_CMP_TABLE_SIZE] = 
	{
		{ {llvm::Type::getDoubleTy(context), 1}, {llvm::Type::getInt64Ty(context), 1} },
		{ {llvm::Type::getDoubleTy(context), 1}, {llvm::Type::getInt32Ty(context), 1} },
		{ {llvm::Type::getDoubleTy(context), 1}, {llvm::Type::getInt16Ty(context), 1} },
		{ {llvm::Type::getDoubleTy(context), 1}, {llvm::Type::getInt8Ty(context), 1} },
		{ {llvm::Type::getDoubleTy(context), 1}, {llvm::Type::getInt1Ty(context), 1} },
		{ {llvm::Type::getFloatTy(context), 1}, {llvm::Type::getInt64Ty(context), 1} },
		{ {llvm::Type::getFloatTy(context), 1}, {llvm::Type::getInt32Ty(context), 1} },
		{ {llvm::Type::getFloatTy(context), 1}, {llvm::Type::getInt16Ty(context), 1} },
		{ {llvm::Type::getFloatTy(context), 1}, {llvm::Type::getInt8Ty(context), 1} },
		{ {llvm::Type::getFloatTy(context), 1}, {llvm::Type::getInt1Ty(context), 1} },
		{ {llvm::Type::getInt64Ty(context), 0}, {llvm::Type::getInt32Ty(context), 0} },
		{ {llvm::Type::getInt64Ty(context), 0}, {llvm::Type::getInt16Ty(context), 0} },
		{ {llvm::Type::getInt64Ty(context), 0}, {llvm::Type::getInt8Ty(context), 0} },
		{ {llvm::Type::getInt64Ty(context), 0}, {llvm::Type::getInt1Ty(context), 0} },
		{ {llvm::Type::getInt64Ty(context), 0}, {llvm::Type::getInt32Ty(context), 1} },
		{ {llvm::Type::getInt64Ty(context), 0}, {llvm::Type::getInt16Ty(context), 1} },
		{ {llvm::Type::getInt64Ty(context), 0}, {llvm::Type::getInt8Ty(context), 1} },
		{ {llvm::Type::getInt64Ty(context), 0}, {llvm::Type::getInt1Ty(context), 1} },
		{ {llvm::Type::getInt32Ty(context), 0}, {llvm::Type::getInt8Ty(context), 1} },
		{ {llvm::Type::getInt32Ty(context), 0}, {llvm::Type::getInt16Ty(context), 1} },	
		{ {llvm::Type::getInt32Ty(context), 0}, {llvm::Type::getInt1Ty(context), 1} },
		{ {llvm::Type::getInt32Ty(context), 0}, {llvm::Type::getInt8Ty(context), 0} },
		{ {llvm::Type::getInt32Ty(context), 0}, {llvm::Type::getInt16Ty(context), 0} },
		{ {llvm::Type::getInt32Ty(context), 0}, {llvm::Type::getInt1Ty(context), 0} },
		{ {llvm::Type::getInt32Ty(context), 1}, {llvm::Type::getInt8Ty(context), 1} },
		{ {llvm::Type::getInt32Ty(context), 1}, {llvm::Type::getInt16Ty(context), 1} },
		{ {llvm::Type::getInt32Ty(context), 1}, {llvm::Type::getInt1Ty(context), 1} }

	};
	int i;

	for (i = 0; i < TYPE_CMP_TABLE_SIZE; i++)
	{
		if (type1_type == table[i].type1.llvm_type 
			&& type1_sign == table[i].type1.sign
			&& type2_type == table[i].type2.llvm_type 
			&& type2_sign == table[i].type2.sign)
		{
			dom_type = this;
		}
		else if (type2_type == table[i].type1.llvm_type 
			&& type2_sign == table[i].type1.sign 
			&& type1_type == table[i].type2.llvm_type 
			&& type1_sign == table[i].type2.sign)
		{
			dom_type = type2;
		}
	}

	/* If one type is of vector type, then return it as dominant type */
	if (type1_type->isVectorTy())
	{
		dom_type = this;
	}
	else if (type2_type->isVectorTy())
	{
		dom_type = type2;
	}
	return dom_type;
}



/*
 * C Wrapper
 */

struct cl2llvmTypeWrap *cl2llvmTypeWrapCreate(void *llvm_type,
		int sign)
{
	Type *type = new Type((llvm::Type *) llvm_type, sign);
	return (cl2llvmTypeWrap *) type;
}


void cl2llvmTypeWrapFree(struct cl2llvmTypeWrap *self)
{
	delete (Type *) self;
}


int cl2llvmTypeWrapGetSign(struct cl2llvmTypeWrap *self)
{
	Type *type = (Type *) self;
	return type->GetSign();
}

void* cl2llvmTypeWrapGetLlvmType(struct cl2llvmTypeWrap *self)
{
	Type *type = (Type *) self;
	return type->GetLlvmType();
}

void cl2llvmTypeWrapSetLlvmType(struct cl2llvmTypeWrap *self, void *llvm_type)
{
	Type *type = (Type *) self;
	type->SetLlvmType((llvm::Type *) llvm_type);
}

void cl2llvmTypeWrapSetSign(struct cl2llvmTypeWrap *self, int sign)
{
	Type *type = (Type *) self;
	type->SetSign((bool) sign);
}

struct cl2llvmTypeWrap* cl2llvmTypeWrapCompare(struct cl2llvmTypeWrap* type1, struct cl2llvmTypeWrap* type2)
{
	Type* type = (Type *) type1;
	Type* dom_type = type->Compare((Type *) type2);
	return (cl2llvmTypeWrap *) dom_type;
}
