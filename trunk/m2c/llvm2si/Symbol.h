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

#ifndef M2C_LLVM2SI_SYMBOL_H
#define M2C_LLVM2SI_SYMBOL_H

#include <iostream>
#include <memory>
#include <unordered_map>

#include <lib/cpp/String.h>


namespace llvm2si
{

extern misc::StringMap symbol_type_map;
enum SymbolType
{
	SymbolTypeInvalid = 0,
	SymbolVectorRegister,
	SymbolScalarRegister
};


class Symbol
{
	friend class SymbolTable;

	std::string name;
	SymbolType type;

	/* Register ID and number of registers */
	int reg;
	int num_regs;

	/* Flag indicating whether the symbol contains a global memory
	 * address. */
	bool address;

	/* If the symbol represents a global memory address (flag 'address'
	 * is set to true), UAV identifier (0=uav10, 1=uav11, ...). */
	int uav_index;

public:

	/* Constructor */
	Symbol(const std::string &name, SymbolType type, int reg) :
			name(name), type(type), reg(reg), num_regs(1),
			address(false), uav_index(0) { }
	Symbol(const std::string &name, SymbolType type, int low, int high) :
			name(name), type(type), reg(low),
			num_regs(high - low + 1), address(false),
			uav_index(0) { }

	/* Getters */
	const std::string &getName() { return name; }
	SymbolType getType() { return type; }
	int getReg() { return reg; }
	int getNumRegs() { return num_regs; }
	bool isAddress() { return address; }
	int getUAVIndex() { return uav_index; }

	/* Dump */
	void Dump(std::ostream &os);
	friend std::ostream &operator<<(std::ostream &os, Symbol &symbol) {
			symbol.Dump(os); return os; }

	/* Set the symbol type to an address to global memory and associate it
	 * with a UAV as specified in 'uav_index' (0=uav10, 1=uav11, ...). */
	void SetUAVIndex(int uav_index) { address = true;
			this->uav_index = uav_index; }
	/* Set Symbol Type */
	void setType(SymbolType type) { this->type = type; }
	void setReg(int reg) { this->reg = reg; }
};


class SymbolTable
{
	std::unordered_map<std::string, std::unique_ptr<Symbol>> table;
public:
	/* Create new symbol and add it to the list */
	void AddSymbol(Symbol *symbol) {
		std::unique_ptr<Symbol> symbol_ptr(symbol);
		table[symbol_ptr->getName()] = std::move(symbol_ptr);
	}

	/* Look up symbol by name and return it, or return null if symbol is
	 * not found. */
	Symbol *Lookup(const std::string &name) {
		auto it = table.find(name);
		return it == table.end() ? nullptr : it->second.get();
	}
};


}  /* namespace llvm2si */

#endif

