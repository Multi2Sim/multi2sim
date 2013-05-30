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
#include <lib/util/list.h>

#include "init.h"
#include "val.h"


struct cl2llvm_init_t *cl2llvm_init_create(char *name)
{
	struct cl2llvm_init_t *init;

	/* Initialize */
	init = xcalloc(1, sizeof(struct cl2llvm_init_t));
	init->name = xstrdup(name);

	init->cl2llvm_val = cl2llvm_val_create();

	init->array_deref_list = NULL;
	init->cl2llvm_val->val = NULL;

	/* Return */
	return init;
}

struct cl2llvm_init_t *cl2llvm_init_create_w_init(LLVMValueRef val, int sign, char *name)
{
	struct cl2llvm_init_t *init;

	init = cl2llvm_init_create(name);

	init->array_deref_list = NULL;

	init->cl2llvm_val->val = val;
	init->cl2llvm_val->type->llvm_type = LLVMTypeOf(val);
	init->cl2llvm_val->type->sign = sign;

	return init;
}

void cl2llvm_init_free(struct cl2llvm_init_t *init)
{
	/* check for an array_deref_list and free it if necessary*/
	if (init->array_deref_list != NULL)
	{
		int i;
		LIST_FOR_EACH(init->array_deref_list, i)
		{
			cl2llvm_val_free(list_get(init->array_deref_list, i));
		}
		list_free(init->array_deref_list);
	}

	if (init->cl2llvm_val != NULL)
	{
		free(init->cl2llvm_val->type);
		free(init->cl2llvm_val);
	}
	free(init->name);
	free(init);
}

