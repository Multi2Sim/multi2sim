/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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


#include "Symbol.h"

namespace frm2bin
{

void Symbol::Dump(std::ostream &os)
{
	if (this->GetDefined())
		os << "name='" << this->name << "', value=" << this->val;
	else
		os << "name='" << this->name << "', value=?";
}

}

/*
 * C Wrapper
 */

#include <lib/mhandle/mhandle.h>
#include <lib/util/hash-table.h>
#include <lib/util/string.h>
#include "Symbol.h"

using namespace frm2bin;

//struct hash_table_t *frm_symbol_table;

struct frm_symbol_t *frm_symbol_create(char *name)
{
	return (struct frm_symbol_t *) new Symbol(name);
}

void frm_symbol_free(struct frm_symbol_t *symbol)
{
	delete((Symbol*)symbol);
}

void frm_symbol_dump(struct frm_symbol_t *symbol, FILE *f)
{
	printf("C wrapper for frm_symbol_dump with argumet FILE *f is not "
			"supported yet!\n");
}




/*
 * Global Functions
 */

/* symbol table is not used yet, but
 * it needs to be implemented later */

//void frm_symbol_table_init(void)
//{
//	frm_symbol_table = hash_table_create(5, 1);
//}
//
//
//void frm_symbol_table_done(void)
//{
//	struct frm_symbol_t *symbol;
//	char *name;
//
//	/* Free all symbols */
//	HASH_TABLE_FOR_EACH(frm_symbol_table, name, symbol)
//		frm_symbol_free(symbol);
//
//	/* Free symbol table */
//	hash_table_free(frm_symbol_table);
//}
//
//void frm_symbol_table_dump(FILE *f)
//{
//	struct frm_symbol_t *symbol;
//	char *name;
//
//	fprintf(f, "Symbol Table:\n");
//	HASH_TABLE_FOR_EACH(frm_symbol_table, name, symbol)
//	{
//		fprintf(f, "\t");
//		frm_symbol_dump(symbol, f);
//		fprintf(f, "\n");
//	}
//}
