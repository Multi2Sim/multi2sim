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
#ifndef M2C_CL2LLVM_BUILT_IN_FUNCS_H
#define M2C_CL2LLVM_BUILT_IN_FUNCS_H

#include <llvm-c/Core.h>
#include <lib/util/list.h>

#include "Type.h"

namespace cl2llvm
{

class BuiltInFunction
{
	int arg_count;
	vector<BuiltInFunctionInst> format_list;

public:

	BuiltInFunction(int, string, string);
};

class BuiltInFunctionInst
{
	// Name of function as it appears in llvm
	string llvm_name;

	/* Arguments */
	int arg_count;
	vector<Type> arg_list;

	/* Return type */
	Type ret_type;

public:

	BuiltInFunctionInst();
};

hash_table<BuiltInFunction> BuiltInFunctionTableCreate(void);

void BuiltInFunctionAnalyze(string name, vector<Value> param_list);

void FunctionDeclare(int arg_count, vector<Type> arg_list, Type ret_type, 
	string name, string param_spec_name);

Type StringToType(string);

/* This function creates an error message for argument type mismatches based
   on an arg_info string and a list of the attempted argument types. */
string ErrorBuiltInFunctionArgMismatch(vector<Value> param_list,
	BuiltInFunction func_info,  string func_name, 
	string error_message);

} // cl2llvm
#endif
