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
 * C Wrapper
 */

struct cl2llvmTypeWrap *cl2llvmTypeWrapCreate(void *llvm_type,
		int sign)
{
	/* FIXME: check if LLVMTypeRef points to same location as llvm::Type. Is the type cast valid below? */
	/* FIXME: how does the 'int' type cast to 'bool' in C++? */
	Type *type = new Type((llvm::Type *) llvm_type, sign);
	return (cl2llvmTypeWrap *) type;
}


void cl2llvmTypeWrapFree(struct cl2llvmTypeWrap *self)
{
	delete (Type *) self;
}


int cl2llvmTypeWrapGetSign(struct cl2llvmTypeWrap *self)
{
	/* FIXME: check how 'bool' is type cast to 'int'. */
	Type *type = (Type *) self;
	return type->GetSign();
}

