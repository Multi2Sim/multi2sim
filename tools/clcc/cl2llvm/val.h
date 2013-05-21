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

#ifndef TOOLS_CLCC_CL2LLVM_VAL_H
#define TOOLS_CLCC_CL2LLVM_VAL_H

#include <llvm-c/Core.h>

#include "type.h"

struct cl2llvm_type_t;

struct cl2llvm_val_t
{
	LLVMValueRef val;
	struct cl2llvm_type_t *type;
};

/*
 * Creates a new cl2llvm_value_t.
 */

struct cl2llvm_val_t *cl2llvm_val_create(void);

/* 
 * Creates a new cl2llvm_value_t with the llvm_type and val specified by the 
 * first argument and the sign specified by the second argument.
 */
struct cl2llvm_val_t *cl2llvm_val_create_w_init(LLVMValueRef val, int sign);

/*
 * Frees the cl2llvm_value_t specified in the argument.
 */

void cl2llvm_val_free(struct cl2llvm_val_t *cl2llvm_val);

/*
 * This function takes a cl2llvm_value_t and casts it to the type of the 
 * cl2llvm_type_t specified in the second argument. A new cl2llvm_value_t which 
 * contains the casted value is created and returned. This new cl2llvm_value_t 
 * must be freed using cl2llvm_value_free().
 */

struct cl2llvm_val_t *llvm_type_cast(struct cl2llvm_val_t *original_val, struct cl2llvm_type_t *totype_w_sign);

/*
 * This function takes to cl2llvm_value_t's determines the dominant type and  
 * creates a new cl2llvm_value_t which contains the non-dominant operand cast 
 * to the type of the dominant operand. This function will either create one or 
 * zero cl2llvm_value_t's based on whether or not a type cast is needed. It is 
 * up to the user to determine which of the two pointers passed by reference, 
 * if any, points to a new value. The new value must be freed using 
 * cl2llvm_value_free.
 */

void type_unify(struct cl2llvm_val_t *val1, struct cl2llvm_val_t *val2, struct cl2llvm_val_t **new_val1, struct cl2llvm_val_t **new_val2);

#endif
