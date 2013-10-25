/*
 * Symbol.cc
 *
 *  Created on: Oct 24, 2013
 *      Author: Chulian Zhang
 */


#include "Symbol.h"

using namespace frm2bin;

void Symbol::dump(Symbol sym, FILE *f)
{
	printf("C++ symbol_dump function cannot be implemented"
			"because one argument is FILE which is not "
			"supported in C++");
}



/*
 * C Wrapper
 */

#include <lib/mhandle/mhandle.h>
#include <lib/util/hash-table.h>
#include <lib/util/string.h>
#include "Symbol.h"


struct hash_table_t *frm_symbol_table;

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
	printf("C wrapper for frm_symbol_dump not supportd yet!\n");
	//char buf[MAX_STRING_SIZE];

	//fprintf(f, "name='%s', ", symbol->name);

	//snprintf(buf, sizeof buf, "%d", symbol->value);
	//fprintf(f, "value=%s", symbol->defined ? buf : "?");
}




/*
 * Global Functions
 */

void frm_symbol_table_init(void)
{
	frm_symbol_table = hash_table_create(5, 1);
}


void frm_symbol_table_done(void)
{
	struct frm_symbol_t *symbol;
	char *name;

	/* Free all symbols */
	HASH_TABLE_FOR_EACH(frm_symbol_table, name, symbol)
		frm_symbol_free(symbol);

	/* Free symbol table */
	hash_table_free(frm_symbol_table);
}

void frm_symbol_table_dump(FILE *f)
{
	struct frm_symbol_t *symbol;
	char *name;

	fprintf(f, "Symbol Table:\n");
	HASH_TABLE_FOR_EACH(frm_symbol_table, name, symbol)
	{
		fprintf(f, "\t");
		frm_symbol_dump(symbol, f);
		fprintf(f, "\n");
	}
}
