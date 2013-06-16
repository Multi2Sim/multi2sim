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

#include "symbol.h"
#include "val.h"


struct cl2llvm_symbol_t *cl2llvm_symbol_create(char *name)
{
	struct cl2llvm_symbol_t *symbol;

	/* Initialize */
	symbol = xcalloc(1, sizeof(struct cl2llvm_symbol_t));
	symbol->name = xstrdup(name);

	symbol->cl2llvm_val = cl2llvm_val_create();

	/* Return */
	return symbol;
}

struct cl2llvm_symbol_t *cl2llvm_symbol_create_w_init(LLVMValueRef val, int sign, char *name)
{
	struct cl2llvm_symbol_t *symbol;

	symbol = cl2llvm_symbol_create(name);

	symbol->cl2llvm_val->val = val;
	symbol->cl2llvm_val->type->llvm_type = LLVMTypeOf(val);
	symbol->cl2llvm_val->type->sign = sign;

	return symbol;
}

void cl2llvm_symbol_free(struct cl2llvm_symbol_t *symbol)
{
	cl2llvm_val_free(symbol->cl2llvm_val);
	free(symbol->name);
	free(symbol);
}

