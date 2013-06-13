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

#include "type.h"
#include "symbol.h"
#include "declarator-list.h"

struct cl2llvm_decl_list_t *cl2llvm_decl_list_create()
{
	struct cl2llvm_decl_list_t *decl_list;
	
	decl_list = xcalloc(1, sizeof(struct cl2llvm_decl_list_t));

	/*initialize pointers to null*/
	decl_list->type_spec = NULL;
	decl_list->access_qual = NULL;
	decl_list->kernel_t = NULL;
	decl_list->inline_t = NULL;
	decl_list->sc_spec = NULL;
	decl_list->addr_qual = 0;
	decl_list->type_qual = NULL;

	return decl_list;
}

void cl2llvm_decl_list_free(struct cl2llvm_decl_list_t *declarator_list)
{
	if (declarator_list->type_spec != NULL)
		free(declarator_list->type_spec);
	if (declarator_list->access_qual != NULL)
		free(declarator_list->access_qual);
	/*if (declarator_list->kernel_t != NULL)
		free(declarator_list->kernel_t);*/
	if (declarator_list->inline_t != NULL)
		free(declarator_list->inline_t);
	if (declarator_list->sc_spec != NULL)
		free(declarator_list->sc_spec);
	if (declarator_list->type_qual != NULL)
		free(declarator_list->type_qual);
	free(declarator_list);
}

void cl2llvm_decl_list_struct_free(struct cl2llvm_decl_list_t *declarator_list)
{
	free(declarator_list);
}

void cl2llvm_attach_decl_to_list(struct cl2llvm_decl_list_t *declarator, struct cl2llvm_decl_list_t *declarator_list) 
{
	if (declarator->type_spec != NULL)
		declarator_list->type_spec = declarator->type_spec;
	if (declarator->access_qual != NULL)
		declarator_list->access_qual = declarator->access_qual;
	if (declarator->kernel_t != NULL)
		declarator_list->kernel_t = declarator->kernel_t;
	if (declarator->inline_t != NULL)
		declarator_list->inline_t = declarator->inline_t;
	if (declarator->sc_spec != NULL)
		declarator_list->sc_spec = declarator->sc_spec;	
	if (declarator->addr_qual != 0)
		declarator_list->addr_qual = declarator->addr_qual;	
	if (declarator->type_qual != NULL)
		declarator_list->type_qual = declarator->type_qual;
}
