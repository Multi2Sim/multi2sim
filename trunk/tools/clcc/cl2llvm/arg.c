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

#include <llvm-c/Core.h>
#include <lib/mhandle/mhandle.h>

#include "declarator-list.h"
#include "type.h"
#include "symbol.h"
#include "arg.h"

struct cl2llvm_arg_t *cl2llvm_arg_create(struct cl2llvm_decl_list_t *decl_list, char *name)
{
	struct cl2llvm_arg_t *arg;
	
	arg = xcalloc(1, sizeof(struct cl2llvm_arg_t));
	arg->name = xstrdup(name);

	/*merge declarators to arg*/
	arg->type_spec = decl_list->type_spec;
	arg->access_qual = decl_list->access_qual;
	arg->kernel_t = decl_list->kernel_t;
	arg->inline_t = decl_list->inline_t;
	arg->sc_spec = decl_list->sc_spec;
	arg->addr_qual = decl_list->addr_qual;
	arg->type_qual = decl_list->type_qual;

	return arg;
}

void cl2llvm_arg_free(struct cl2llvm_arg_t *arg)
{
	if (arg->type_spec != NULL)
		free(arg->type_spec);
	if (arg->access_qual != NULL)
		free(arg->access_qual);
	if (arg->kernel_t != NULL)
		free(arg->kernel_t);
	if (arg->inline_t != NULL)
		free(arg->inline_t);
	if (arg->sc_spec != NULL)
		free(arg->sc_spec);
	if (arg->type_qual != NULL)
		free(arg->type_qual);
	free(arg->name);
	free(arg);
}

