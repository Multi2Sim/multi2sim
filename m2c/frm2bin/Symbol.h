/*
 * Symbol.h
 *
 *  Created on: Oct 24, 2013
 *      Author: Chulian Zhang
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
