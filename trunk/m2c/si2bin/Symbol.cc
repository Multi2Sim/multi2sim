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

#include <string>
#include <ostream>
#include <ext/stdio_filebuf.h>

#include <lib/class/string.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/hash-table.h>
#include <lib/util/string.h>

#include "Symbol.h"

using namespace S2B;
using namespace std;


Symbol::Symbol(std::string symbol_name)
{
	name = symbol_name;
}

void Symbol::Dump(ostream& os)
{
	if (this->GetDefined())
		os << "name='" << name << "', value=" << value;
	else
		os << "name='" << name << "', value=?";
}



/*
 * Class 'S2BSymbol'
 */

S2BSymbolWrap *S2BSymbolWrapCreate(char *name)
{
	Symbol *symbol = new Symbol(name);
	return (S2BSymbolWrap *) symbol;
}


void S2BSymbolWrapFree(S2BSymbolWrap *self)
{
	Symbol *symbol = (Symbol*) self;
	delete symbol;
}


void S2BSymbolWrapDump(S2BSymbolWrap *self, FILE *f)
{
	Symbol *symbol = (Symbol *) self;
        __gnu_cxx::stdio_filebuf<char> filebuf(fileno(f), std::ios::out);
	ostream os(&filebuf);
        symbol->Dump(os);
}

/* Getters */
const char *S2BSymbolWrapGetName(struct S2BSymbolWrap *self)
{
	Symbol *symbol = (Symbol *) self;
	return symbol->GetName().c_str();
}

int S2BSymbolWrapGetValue(struct S2BSymbolWrap *self)
{
	Symbol *symbol = (Symbol *) self;
	return symbol->GetValue();
}

int S2BSymbolWrapGetDefined(struct S2BSymbolWrap *self)
{
	Symbol *symbol = (Symbol *) self;
	return symbol->GetDefined();
}

/* Setters */
void S2BSymbolWrapSetValue(struct S2BSymbolWrap *self, int val)
{
	Symbol *symbol = (Symbol *) self;
	symbol->SetValue(val);
}

void S2BSymbolWrapSetDefined(struct S2BSymbolWrap *self, int def)
{
	Symbol *symbol = (Symbol *) self;
	symbol->SetDefined(def);
}

