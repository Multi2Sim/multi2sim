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
#define ARHC_HSA_EMU_FUNCTION_H

namespace HSA
{

/// Abstraction of an HSA function
class Function
{

	// name of the function
	std::string name;

	// Entry point of the function, helps the work item to set its init pc
	char *entry_point;

public:

	/// Constructor
	Function(const std::string &name);

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

};

}  // namespace HSA

#endif
