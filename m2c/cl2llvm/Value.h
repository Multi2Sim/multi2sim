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

#ifndef M2C_CL2LLVM_VALUE_H
#define M2C_CL2LLVM_VALUE_H


/*
 * C++ code
 */

#ifdef __cplusplus

#include "Type.h"
#include <llvm::Value.h>

namespace cl2llvm
{

class Value
{
	llvm::Value *llvm_value;
	Type *type;

public:

	Value(llvm::Value *llvm_value, bool sign) { this->value = llvm_value;
		this->type->llvm_type = llvm_value->getType(); this->type->sign = sign; }

	Value *GetLlvmValue() { return llvm_value; }
	void SetLlvmValue(llvm::Value *llvm_value) { this->llvm_value = llvm_value; }
	Type *GetType() { return type; }
	void SetType(Type *type) { this->type = type; }

};


}  /* namespace cl2llvm */

#endif  /* __cplusplus */



/*
 * C Wrapper
 */

#ifdef __cplusplus
extern "C"
{
#endif

struct cl2llvmValueWrap;

struct cl2llvmValueWrap *cl2llvmValueWrapCreate(void *llvm_value,
		int sign);
void cl2llvmValueWrapFree(struct cl2llvmValueWrap *value);
void* cl2llvmValueWrapGetLlvmValue(struct cl2llvmValueWrap *value);
void cl2llvmValueWrapSetLlvmValue(struct cl2llvmValueWrap* value, void *llvm_value);
void* cl2llvmValueWrapGetType(struct cl2llvmValueWrap *value);
void cl2llvmValueWrapSetType(struct cl2llvmValueWrap* value, Type *type);

#ifdef __cplusplus
}
#endif

#endif

