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

#ifndef M2C_CL2LLVM_DECLARATOR_LIST_H
#define M2C_CL2LLVM_DECLARATOR_LIST_H

namespace cl2llvm
{

class DeclaratorList
{
	Type type_spec;
	int addr_qual;
	string kernel_t;
	string inline_t;
	string sc_spec;
	string access_qual;
	string type_qual;

public:
	// Constructors
	DeclaratorList() {}

	// Setters
	void setName(string name) { this->name = name; }
	void setTypeSpec(Type type_spec) { return type_spec; }
	void setAddrQual(int addr_qual) { return addr_qual; }

	// Getters
	string getName() { return name; }
 	Type getTypeSpec() { return type_spec; }
	string getAccessQual() { return access_qual; }
	string getKernelT() { return kernel_t; }
	string getInlineT() { return inline_t; }
	string getScSpec() { return sc_spec; }
	int getAddrQual() { return addr_qual; }
	string getTypeQual() { return type_qual; }

	// Other Member Functions
	void Merge(DeclaratorList&);
};


/*frees just the struct pointer and not its fields*/
void cl2llvm_decl_list_struct_free(struct cl2llvm_decl_list_t *declarator_list);


void cl2llvm_attach_decl_to_list(struct cl2llvm_decl_list_t *declarator, struct cl2llvm_decl_list_t *declarator_list);

#endif
