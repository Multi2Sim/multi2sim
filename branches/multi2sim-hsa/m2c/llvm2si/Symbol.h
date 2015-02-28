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

class Symbol
{
public:

	/// Symbol types
	enum Type
	{
		TypeInvalid = 0,
		TypeVectorRegister,
		TypeScalarRegister
	};

	/// String map for Type
	static const misc::StringMap TypeMap;


private:

	// FIXME
	friend class SymbolTable;

	// Symbol name
	std::string name;

	// Symbol type
	Type type;

	// Register identifier
	int id;

	// num of registers
	int num_registers = 1;

	// Flag indicating whether the symbol contains a global memory
	// address.
	bool address = false;

	/* If the symbol represents a global memory address (flag 'address'
	 * is set to true), UAV identifier (0=uav10, 1=uav11, ...). */
	int uav_index = 0;

public:

	/// Constructor specifying a single register
	Symbol(const std::string &name, Type type, int id) :
			name(name),
			type(type),
			id(id)
	{
	}

	/// Constructor specifying a register range
	Symbol(const std::string &name, Type type, int low, int high) :
			name(name),
			type(type),
			id(low),
			num_registers(high - low + 1)
	{
	}

	/// Constructor specifying a name
	Symbol(const std::string &name): name(name)
	{
	}

	/// Return the symbol name
	const std::string &getName() const { return name; }

	/// Return the symbol type
	Type getType() const { return type; }

	/// Return the register identifier associated with the symbol. This can
	/// be a vector or scalar register, depending on the symbol type.
	int getId() const { return id; }

	/// Return the number of registers if the symbol contains a register
	/// range.
	int getNumRegisters() const { return num_registers; }

	/// Return true if the symbol represents a memory address
	bool isAddress() const { return address; }

	/// For memory address symbols, return the associated UAV index
	int getUAVIndex() const { return uav_index; }

	/// Dump the symbol
	void Dump(std::ostream &os = std::cout) const;

	/// Alternative syntax for Dump()
	friend std::ostream &operator<<(std::ostream &os, const Symbol &symbol)
	{
		symbol.Dump(os);
		return os;
	}

	/// Set the symbol type to an address to global memory and associate it
	/// with a UAV as specified in \a uav_index (0 = uav10, 1 = uav11, ...).
	void setUAVIndex(int uav_index)
	{
		address = true;
		this->uav_index = uav_index;
	}

	/// Update the symbol type
	void setType(Type type) { this->type = type; }

	/// Update the register identifier associated with the symbol
	void setRegister(int id) { this->id = id; }
	
	/// Update the register type and identifier associate with the symbol
	void setRegister(Type type, int id)
	{
		this->type = type;
		this->id  = id;
		num_registers = 1;
	}

	/// Update the register type and identifier range asscoiate with the
	/// symbol
	void setRegister(Type type, int low, int high)
	{
		this->type = type;
		id  = low;
		num_registers = high - low + 1;
	}
};


class SymbolTable
{
	// Hash table of symbols
	std::unordered_map<std::string, std::unique_ptr<Symbol>> table;

public:

	/// Create new symbol and add it to the list, return a pointer to the
	/// symbol
	Symbol *addSymbol(const std::string &name)
	{
		table[name] = misc::new_unique<Symbol>(name);
		return table[name].get();
	}
	

	/// Look up symbol by name and return it, or return null if symbol is
	/// not found.
	Symbol *Lookup(const std::string &name)
	{
		auto it = table.find(name);
		return it == table.end() ? nullptr : it->second.get();
	}
};


}  // namespace llvm2si

#endif

