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

#ifndef TOOLS_CLCC_CL2LLVM_DECLARATOR_LIST_H
#define TOOLS_CLCC_CL2LLVM_DECLARATOR_LIST_H

struct cl2llvm_decl_list_t
{
	struct cl2llvm_type_t *type_spec;
	char *addr_qual;
	char *kernel_t;
	char *inline_t;
	char *sc_spec;
	char *access_qual;
	char *type_qual;
};

struct cl2llvm_decl_list_t *cl2llvm_decl_list_create();

void cl2llvm_decl_list_free(struct cl2llvm_decl_list_t *declarator_list);


/*frees just the struct pointer and not its fields*/
void cl2llvm_decl_list_struct_free(struct cl2llvm_decl_list_t *declarator_list);


void cl2llvm_attach_decl_to_list(struct cl2llvm_decl_list_t *declarator, struct cl2llvm_decl_list_t *declarator_list);

#endif
