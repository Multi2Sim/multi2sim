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

#ifndef M2C_CL2LLVM_ARG_H
#define M2C_CL2LLVM_ARG_H

#include "declarator-list.h"

namespace cl2llvm
{

class Arg
{
	string name;
	Type type_spec;
	string access_qual;
	string kernel_t;
	string inline_t;
	string sc_spec;
	int addr_qual;
	string type_qual;

	// Constructors
	Arg() {}
	Arg(DeclaratorList& decl_list, string name):
		name(name), type_spec(decl_list.type_spec),
		access_qual(decl_list.access_qual), kernel_t(decl_list.kernel_t)
		inline_t(decl_list.inline_t), sc_spec(decl_list.sc_spec),
		addr_qual(decl_list.addr_qual), type_qual(decl_list.type_qual) {}

	// Setters
	void setName(string name) { this->name = name; }
	
	// Getters
	string getName() { return name; }
 	Type getTypeSpec() { return type_spec; }
	string getAccessQual() { return access_qual; }
	string getKernelT() { return kernel_t; }
	string getInlineT() { return inline_t; }
	string getScSpec() { return sc_spec; }
	int getAddrQual() { return addr_qual; }
	string getTypeQual() { return type_qual; }
};

}
#endif
