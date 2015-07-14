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

#ifndef ARCH_HSA_EMULATOR_VARIABLE_H
#define ARCH_HSA_EMULATOR_VARIABLE_H

#include <arch/hsa/disassembler/Brig.h>

namespace HSA
{

class SegmentManager;

/// Information for argument
class Variable
{
	// Name of the variable
	std::string name;

	// Type of the variable
	BrigType type;

	// The segment that this variable is declared in
	BrigSegment segment;

	// Size of the variable
	unsigned size;

	// Dim of the variable
	unsigned long long dim;

	// Beginning address of the variable
	unsigned address;

	// Is input
	bool is_input = false;

	// Index of the argument in the argument
	unsigned int index = 0;

	// If the variable is formal it does not occupies memory space
	bool isFormal;

public:

	/// Constructor
	Variable(const std::string& name, BrigType type,
			unsigned long long dim, unsigned address,
			BrigSegment segment,
			bool isFormal);

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
	BrigType getType() const { return type; }

	/// Set the type of the variable
	void setType(BrigType type) { this->type = type; }

	/// Get the index of the argument in a functions argument list
	unsigned int getIndex() const { return index; }

	/// Set the index of the argument in the argument list
	void setIndex(unsigned int index) { this->index = index; }

	/// Get the dim
	unsigned long long getDim() const { return dim; }

	/// Set the dim
	void setDim(unsigned long long dim) { this->dim = dim; };

	/// Get the segment
	BrigSegment getSegment() const { return segment; }

};

}  // namespace HSA

#endif 

