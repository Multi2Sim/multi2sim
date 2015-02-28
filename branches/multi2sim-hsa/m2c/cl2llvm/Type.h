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

#ifndef M2C_CL2LLVM_TYPE_H
#define M2C_CL2LLVM_TYPE_H


/*
 * C++ code
 */

#ifdef __cplusplus

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>

namespace cl2llvm
{

llvm::LLVMContext& context = llvm::getGlobalContext();

class Type
{
	llvm::Type *llvm_type;
	bool sign;  /* true = signed, false = unsigned */

public:

	Type(llvm::Type *type) { this->llvm_type = type; sign = true; }
	Type(llvm::Type *type, bool sign) { this->llvm_type = type;
			this->sign = sign; }

	llvm::Type *GetLlvmType() { return llvm_type; }
	void SetLlvmType(llvm::Type *llvm_type) { this->llvm_type = llvm_type; }
	bool GetSign() { return sign; }
	void SetSign(bool sign) { this->sign = sign; }
	Type* Compare(Type *type2);

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

struct cl2llvmTypeWrap;

struct cl2llvmTypeWrap *cl2llvmTypeWrapCreate(void *llvm_type,
		int sign);
void cl2llvmTypeWrapFree(struct cl2llvmTypeWrap *type);
int cl2llvmTypeWrapGetSign(struct cl2llvmTypeWrap *type);
void cl2llvmTypeWrapSetSign(struct cl2llvmTypeWrap *type, int sign);
void* cl2llvmTypeWrapGetLlvmType(struct cl2llvmTypeWrap *type);
void cl2llvmTypeWrapSetLlvmType(struct cl2llvmTypeWrap* type, void *llvm_type);
struct cl2llvmTypeWrap* cl2llvmTypeWrapCompare(struct cl2llvmTypeWrap* type1, struct cl2llvmTypeWrap* type2);

#ifdef __cplusplus
}
#endif

#endif

