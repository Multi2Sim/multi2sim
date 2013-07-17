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


/*
 * Class 'Llvm2siSymbolTable'
 */

CLASS_IMPLEMENTATION(Llvm2siSymbolTable);

void Llvm2siSymbolTableCreate(Llvm2siSymbolTable *self)
{
	self->table = hash_table_create(30, TRUE);
}


void Llvm2siSymbolTableDestroy(Llvm2siSymbolTable *self)
{
	char *key;
	Llvm2siSymbol *symbol;

	/* Free symbol hash table */
	HASH_TABLE_FOR_EACH(self->table, key, symbol)
		delete(symbol);
	hash_table_free(self->table);
}


void Llvm2siSymbolTableDump(Object *self, FILE *f)
{
	Llvm2siSymbolTable *symbol_table;
	Llvm2siSymbol *symbol;

	char *key;

	symbol_table = asLlvm2siSymbolTable(self);
	fprintf(f, "Symbol table:\n");
	HASH_TABLE_FOR_EACH(symbol_table->table, key, symbol)
		Llvm2siSymbolDump(asObject(symbol), f);
	fprintf(f, "\n");
}


void Llvm2siSymbolTableAddSymbol(Llvm2siSymbolTable *self,
		Llvm2siSymbol *symbol)
{
	int ok;

	ok = hash_table_insert(self->table, symbol->name, symbol);
	if (!ok)
		fatal("%s: %s: duplicate symbol",
			__FUNCTION__, symbol->name);
}


Llvm2siSymbol *Llvm2siSymbolTableLookup(Llvm2siSymbolTable *self,
		const char *name)
{
	return hash_table_get(self->table, (char *) name);
}
