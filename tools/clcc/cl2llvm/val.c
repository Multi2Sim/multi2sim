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

#include <stdlib.h>
#include <stdio.h>

#include <lib/mhandle/mhandle.h>

#include "val.h"
#include "type.h"


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


