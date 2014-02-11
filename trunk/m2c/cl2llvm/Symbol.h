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

#ifndef M2C_CL2LLVM_SYMBOL_H
#define M2C_CL2LLVM_SYMBOL_H

#include <llvm-c/Core.h>

namespace cl2llvm {

class Symbol
{
	string name;
	Value value;

public:

	// Constructors
	Symbol() {}
	Symbol(string name, Value value):
		name(name), value(value) {}
	
	// Setters
	void setName(string name) { this->name = name; }
	void setValue(Value value) { this->value = value; }

	// Getters
	string getName() { return name; }
	Value getValue() { return value; }

};

}
#endif