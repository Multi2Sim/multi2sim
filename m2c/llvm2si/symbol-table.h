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

#ifndef M2C_LLVM2SI_SYMBOL_TABLE_H
#define M2C_LLVM2SI_SYMBOL_TABLE_H

#include <stdio.h>


/* Forward declarations */
struct hash_table_t;
struct llvm2si_symbol_t;



/*
 * Symbol Table Object
 */

struct llvm2si_symbol_table_t
{
	struct hash_table_t *table;
};

struct llvm2si_symbol_table_t *llvm2si_symbol_table_create(void);
void llvm2si_symbol_table_free(struct llvm2si_symbol_table_t *table);
void llvm2si_symbol_table_dump(struct llvm2si_symbol_table_t *table, FILE *f);

void llvm2si_symbol_table_add_symbol(struct llvm2si_symbol_table_t *table,
		struct llvm2si_symbol_t *symbol);
struct llvm2si_symbol_t *llvm2si_symbol_table_lookup(struct llvm2si_symbol_table_t *table,
		const char *name);

#endif

