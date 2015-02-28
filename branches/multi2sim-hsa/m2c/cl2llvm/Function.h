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

#ifndef M2C_CL2LLVM_FUNCTION_H
#define M2C_CL2LLVM_FUNCTION_H

#include <llvm-c/Core.h>
#include <lib/util/list.h>

namespace cl2llvm
{

class Function
{
	llvm::Value *func;
	llvm::Type *func_type;
	bool sign;
	string name;
	unordered_map<string, Symbol> symbol_table;
	vector<Type> arg_list;
	int arg_count;
	llvm::Value *entry_block;
	llvm::Value *branch_instr;
public:
	// Constructors
	Function(string name, vector<Type> arg_list):
		name(name), arg_list(arg_list) { }
	Function(Function& function): func(function.func),
		func_type(function.func_type), sign(function.sign),
		name(function.name), symbol_table(function.symbol_table),
		arg_list(function.arg_list), arg_count(arg_count),
		entry_block(function.entry_block), 
		branch_instr(function.branch_instr) { }
	// Getters
	llvm::Value *getLlvmFunction() { return func; }
	llvm::Type *getLlvmFunctionType() { return func_type; }
	bool getSign() { return sign; }
	string getName() { return name; }
	vector<Type> getArgList() { return arg_list; }
	int getArgCount() { return arg_count; }
	llvm::Value *getEntryBlock() { return entry_block; }
	llvm::Value *getBranchInstr() { return branch_instr; }

	// Setters
	void setLlvmFunction(llvm::Value *func) {
		this->func = func; }
	void setLlvmFunctionType(llvm::Type *func_type) {
		this->func_type = func_type; }
	void setSign(bool sign) { this->sign = sign; }
	void setName(string name) { this->name = name; }
	void AddArg(Arg);
	void setEntryBlock(llvm::Value *entry_block) {
		this->entry_block = entry_block; }
	void setBranchInstr(llvm::Value *branch_instr) {
		this->branch_instr = branch_instr; }
};

}
#endif
