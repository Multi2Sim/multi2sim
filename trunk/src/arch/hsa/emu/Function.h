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

	/// Dump argument related information
	void DumpArgumentInfo(std::ostream &os) const;

	/// Dump register related information
	void DumpRegisterInfo(std::ostream &os) const;




	//
	// Fields related with arguments
	//

	// Information for argument
	struct Argument
	{
		unsigned short type;
		unsigned short size;	// argument size in bytes
		unsigned int offset;
		bool isInput;
	};

	// argument size. When stack frame initialize, allocate the size of
	// memory
	unsigned int arg_size = 0;

	// Map the name with the information of the argument
	std::map<std::string, std::unique_ptr<Argument>> arg_info;




	//
	// Fields related with registers
	//

	// Allocated register size
	unsigned int reg_size = 0;

	// Map the name with the offset of the register
	std::map<std::string, unsigned int> reg_info;

	// Return the size of the register according to its name. It only
	// returns the size of the register from its name, regardless of if
	// if have been allocated. Trying to get the size of an invalid
	// register name will raise panic.
	// "$cx" - 1 returns 1, but its actually only 1 bit
	// "$sx" - 4
	// "$dx" - 8
	// "$qx" - 16
	// Invalid register name - 0
	unsigned int getRegisterSizeByName(const std::string &name) const;



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

	/// Add the register to the register list
	void addRegister(const std::string &name);

	/// Returns the offset of an register. If the register does not exist,
	/// return -1.
	int getRegisterOffset(const std::string &name);

	/// Dump function information for debug propose
	void Dump(std::ostream &os) const;



	/// Operator \c << invoking the function Dump9) on an output stream
	friend std::ostream &operator<<(std::ostream &os,
			const Function &function)
	{
		function.Dump(os);
		return os;
	}

};

}  // namespace HSA

#endif
