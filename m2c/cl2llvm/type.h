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

#ifndef TOOLS_CLCC_CL2LLVM_TYPE_H
#define TOOLS_CLCC_CL2LLVM_TYPE_H

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

struct cl2llvm_val_t;

struct cl2llvm_type_t
{
	LLVMTypeRef llvm_type;
	int sign;
};

/*
 * Creates a new cl2llvm_value_t.
 */

struct cl2llvm_type_t *cl2llvm_type_create(void);

/*
 * Creates a new cl2llvm_type_t with the llvm_type and sign specified by the 
 * first and second arguments respectively.
 */
struct cl2llvm_type_t *cl2llvm_type_create_w_init(LLVMTypeRef llvm_type, int sign);

/*
 * Frees the cl2llvm_type_t specified by the argument.
 */

void cl2llvm_type_free(struct cl2llvm_type_t*);

/*
 * This function takes two cl2llvm_value_t's and returns the type of the   
 * dominant operand. The return value points to a newly created cl2llvm_type_t 
 * which must be freed using cl2llvm_type_free().
 */

struct cl2llvm_type_t *type_cmp(struct cl2llvm_val_t *type1_w_sign, struct cl2llvm_val_t *type2_w_sign);

#endif

