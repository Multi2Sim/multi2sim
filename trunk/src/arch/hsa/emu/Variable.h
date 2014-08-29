/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#ifndef ARCH_HSA_EMU_VARIABLE_H
#define ARCH_HSA_EMU_VARIABLE_H

namespace HSA
{

/// Information for argument
class Variable
{
	// Name of the variable
	std::string name;

	// Type of the variable
	unsigned short type;

	// Size of the variable
	unsigned size;

	// Beginning address of the variable
	unsigned address;

	// Is input
	bool is_input = false;

	// Index of the argument in the argument
	unsigned int index = 0;

public:

	/// Constructor
	Variable(const std::string& name, unsigned short type, unsigned size,
			unsigned address);

	/// Destructor
	~Variable();

	/// Dump variable information
	void Dump(std::ostream &os = std::cout, unsigned int indent = 0,
			bool is_simple_format = false) const;

	// Output variable value
	template<typename T>
	void DumpValue(std::ostream &os = std::cout) const;

	/// Operator \c << invoking the function Dump) on an output stream
	friend std::ostream &operator<<(std::ostream &os,
			const Variable &variable)
	{
		variable.Dump(os);
		return os;
	}

	/// Get variable address
	unsigned getAddress() const { return address; }

	/// Set variable address
	void setAddress(unsigned address) { this->address = address; }

	/// Determine if is input
	bool isInput() const { return is_input; }

	/// Set input
	void setInput(bool is_input) { this->is_input = is_input; }

	/// Get name of the variable
	const std::string& getName() const { return name; }

	/// Set the name of the variable
	void setName(const std::string& name) { this->name = name; }

	/// Return the size of variable
	unsigned getSize() const { return size; }

	/// Set the size of the variable
	void setSize(unsigned size) { this->size = size; }

	/// Get the type of the variable
	unsigned short getType() const { return type; }

	/// Set the type of the variable
	void setType(unsigned short type) { this->type = type; }

	/// Get the index of the argument in a functions argument list
	unsigned int getIndex() const { return index; }

	/// Set the index of the argument in the argument list
	void setIndex(unsigned int index) { this->index = index; }

};

}  // namespace HSA

#endif 

