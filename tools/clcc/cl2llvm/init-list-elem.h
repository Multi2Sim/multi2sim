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

#ifndef TOOLS_CLCC_CL2LLVM_INIT_LIST_ELEM_H
#define TOOLS_CLCC_CL2LLVM_INIT_LIST_ELEM_H

#include <llvm-c/Core.h>

struct cl2llvm_init_list_elem_t
{
	char *elem_name;
	LLVMValueRef val;
	int is_signed;
};

struct cl2llvm_init_list_elem_t *cl2llvm_create_init_list_elem(char *name);
void cl2llvm_init_list_elem_free(struct cl2llvm_init_list_elem_t *elem);

#endif
