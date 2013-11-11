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

#include <lib/util/hash-table.h>
#include <llvm-c/Core.h>
extern struct hash_table_t *cl2llvm_global_symbol_table;
extern struct hash_table_t *cl2llvm_built_in_const_table;

char *cl2llvm_mem_fence_flags[2][22] = {
	{"CLK_LOCAL_MEM_FENCE\00 "},
	{"CLK_GLOBAL_MEM_FENCE\00"}
};

struct hash_table_t *cl2llvm_built_in_const_table_create()
{
	int i;
	struct hash_table_t *enum_table = hash_table_create(10, 1);

	for (i = 0; i < 2; i++)
	{
		hash_table_insert(enum_table, cl2llvm_mem_fence_flags[i][0], LLVMConstInt(LLVMInt32Type(), i, 0));
	}

	return enum_table;
}
