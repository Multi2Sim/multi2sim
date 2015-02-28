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

#ifndef FRM2BIN_SYMBOL_H
#define FRM2BIN_SYMBOL_H


#ifdef __cplusplus

#include <string>
#include <iostream>

namespace frm2bin
{
/* Class definition for symbol */
class Symbol
{
	std::string name;
	int val;

	/* True if the symbol definition has been found already. False when
	 * the symbol has been found as a forward declaration. */
	int defined;

public:
	/* Constructor */
	Symbol(const std::string &name) : name(name), val(0), defined(false) { }

	/* Getters */
	const std::string &GetName() { return name; }
	int GetValue() { return val; }
	bool GetDefined() { return defined; }

	/* Setters */
	void SetValue(int value) { this->val = value; }
	void SetDefined(bool defined) { this->defined = defined; }

	/* Dump Function */
	void Dump(std::ostream& os);
	friend std::ostream &operator<<(std::ostream &os, Symbol &symbol)
			{ symbol.Dump(os); return os; }
};

} /* namespace frm2bin */

#endif  /* __cplusplus */


/*
 * C Wrapper
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

//extern struct hash_table_t *frm_symbol_table;
//
//void frm_symbol_table_init(void);
//void frm_symbol_table_done(void);
//void frm_symbol_table_dump(FILE *f);


/*
 * Symbol
 */

struct frm_symbol_t
{
	char *name;
	int value;

	/* True if the symbol definition has been found already. False when
	 * the symbol has been found as a forward declaration. */
	int defined;
};

struct frm_symbol_t *frm_symbol_create(char *name);
void frm_symbol_free(struct frm_symbol_t *symbol);

void frm_symbol_dump(struct frm_symbol_t *symbol, FILE *f);

#ifdef __cplusplus
}
#endif


#endif /* FRM2BIN_SYMBOL_H_ */
