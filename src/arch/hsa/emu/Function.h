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

#ifndef ARCH_HSA_EMU_FUNCTION_H
#define ARCH_HSA_EMU_FUNCTION_H

#include <map>
#include <memory>

namespace HSA
{

/// Abstraction of an HSA function
class Function
{

	// name of the function
	std::string name;

	// Entry point of the function, helps the work item to set its init pc
	char *entry_point;




	//
	// Fields related with arguments
	//

	// Information for argument
	struct Argument
	{
		unsigned short type;
		unsigned short size;	// argument size in bytes
		unsigned short offset;
		bool isInput;
	};

	// argument size. When stack frame initialize, allocate the size of
	// memory
	unsigned short arg_size = 0;

	// Map the name with the information of the argument
	std::map<std::string, std::unique_ptr<Argument>> arg_info;


public:

	/// Constructor
	Function(const std::string &name, char *entry_point);

	/// Destructor
	~Function(){};

	/// Returns function name
	std::string getName() const{return name;}

	/// Sets the entry point
	void setEntryPoint(char *entry_point){this->entry_point = entry_point;} 

	/// Returns pointer to entry point
	char *getEntryPoint() const{return entry_point;}

	/// Adds an argument information in argument table
	///
	/// \Param name
	/// 	Name of the argument
	///
	/// \Param isInput
	/// 	If true, the argument is an input arguement
	///
	/// \Param type
	///	type of the argument, as defined in BrigDef.h
	void addArgument(const std::string &name, bool isInput,
			unsigned short type);

};

}  // namespace HSA

#endif
