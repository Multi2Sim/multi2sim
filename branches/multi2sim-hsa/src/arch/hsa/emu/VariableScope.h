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

#ifndef ARCH_HSA_EMU_ARGSCOPE_H
#define ARCH_HSA_EMU_ARGSCOPE_H

#include <map>
#include <memory>

#include <memory/Manager.h>

#include "Variable.h"

namespace HSA
{

class SegmentManager;

class VariableScope
{

	// Argument information
	std::map<std::string, std::unique_ptr<Variable>> variable_info;

public:

	/// Constructor
	VariableScope();

	/// Destructor
	~VariableScope();

	/// Add and variable into variable list
	///
	/// \param name
	/// 	name of the variable,
	///
	/// \param size
	/// 	size of the memory this variable needs, in byte
	///
	/// \param type
	/// 	Type of variable, defined in Brig.h
	///
	/// \return
	///	The address of the newly created variable
	///
	unsigned DeclearVariable(const std::string &name, BrigTypeX type,
			unsigned long long dim, SegmentManager *segment);

	/// Get the address of a certain variable, return 0 if the variable is
	/// not defined
	unsigned getAddress(const std::string &name) const;

	/// Get the buffer pointer to a certain variable, return nullptr if
	/// the variable is not defined
	char *getBuffer(const std::string &name);

	/// Dump information in the variable scope
	void Dump(std::ostream &os = std::cout, unsigned int indent = 0) const;

	/// Dump information in the variable scope in one line
	void DumpInLine(std::ostream &os = std::cout) const;

	/// Operator \c << invoking the function Dump) on an output stream
	friend std::ostream &operator<<(std::ostream &os,
			const VariableScope &variable_scope)
	{
		variable_scope.Dump(os);
		return os;
	}

};

}  // namespace HSA

#endif 
