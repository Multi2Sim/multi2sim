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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/hash-table.h>

#include "symbol.h"
#include "symbol-table.h"


struct llvm2si_symbol_table_t *llvm2si_symbol_table_create(void)
{
	struct llvm2si_symbol_table_t *table;

	/* Initialize */
	table = xcalloc(1, sizeof(struct llvm2si_symbol_table_t));
	table->table = hash_table_create(30, TRUE);

	/* Return */
	return table;
}


void llvm2si_symbol_table_free(struct llvm2si_symbol_table_t *table)
{
	char *key;
	struct llvm2si_symbol_t *symbol;

	/* Free symbol hash table */
	HASH_TABLE_FOR_EACH(table->table, key, symbol)
		llvm2si_symbol_free(symbol);
	hash_table_free(table->table);

	/* Rest */
	free(table);
}


void llvm2si_symbol_table_dump(struct llvm2si_symbol_table_t *table, FILE *f)
{
	char *key;
	struct llvm2si_symbol_t *symbol;

	fprintf(f, "Symbol table:\n");
	HASH_TABLE_FOR_EACH(table->table, key, symbol)
		llvm2si_symbol_dump(symbol, f);
	fprintf(f, "\n");
}


void llvm2si_symbol_table_add_symbol(struct llvm2si_symbol_table_t *table,
		struct llvm2si_symbol_t *symbol)
{
	int ok;

	ok = hash_table_insert(table->table, symbol->name, symbol);
	if (!ok)
		fatal("%s: %s: duplicate symbol",
			__FUNCTION__, symbol->name);
}


struct llvm2si_symbol_t *llvm2si_symbol_table_lookup(struct llvm2si_symbol_table_t *table,
		const char *name)
{
	return hash_table_get(table->table, (char *) name);
}
