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

#ifndef M2C_SI2BIN_SYMBOL_H
#define M2C_SI2BIN_SYMBOL_H

#ifdef __cplusplus

#include <string>

namespace S2B
{

class Symbol 
{
	std::string name;
	int value;

	/* True if the symbol definition has been found already. False when
	 * the symbol has been found as a forward declaration. */
	bool defined;

public:

	/* Constructor */
	Symbol(std::string name);

	/* Getters */
	std::string GetName() { return name; }
	int GetValue() { return value; }
	bool GetDefined() { return defined; }

	/* Setters */
	void SetValue(int value) { this->value = value; }
	void SetDefined(bool defined) { this->defined = defined; }

	/* Dump Function */
	void Dump(std::ostream& os);
};

} /* Namespace S2B */

#endif /* __cplusplus */


/*
 * C Wrapper
 */

#ifdef __cplusplus
extern "C" {
#endif

struct S2BSymbolWrap;

struct S2BSymbolWrap *S2BSymbolWrapCreate(char *name);
void S2BSymbolWrapFree(struct S2BSymbolWrap *self);

void S2BSymbolWrapDump(struct S2BSymbolWrap *self, FILE *f);

/* Getters */
const char *S2BSymbolWrapGetName(struct S2BSymbolWrap *self);
int S2BSymbolWrapGetValue(struct S2BSymbolWrap *self);
int S2BSymbolWrapGetDefined(struct S2BSymbolWrap *self);

/* Setters */
void S2BSymbolWrapSetValue(struct S2BSymbolWrap, int val);
void S2BSymbolWrapSetDefined(struct S2BSymbolWrap, int def);

#ifdef __cplusplus
}
#endif

#endif /* M2C_SI2BIN_SYMBOL_H */
