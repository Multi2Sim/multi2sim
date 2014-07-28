/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef M2C_COMMON_PASS_INFO_H
#define M2C_COMMON_PASS_INFO_H

namespace comm
{

/// Abstract class to be inherited from classes unique to each pass.
/// For example, ModuleDataDependencyPassInfo.
class ModulePassInfo
{

public:

	/// Pure virtual destructor makes this class abstract.
	virtual ~ModulePassInfo() = 0;

};

/// Abstract class to be inherited from classes unique to each pass.
/// For example, FunctionDataDependecyPassInfo.
class FunctionPassInfo
{

public:

	/// Pure virtual destructor makes this class abstract.
	virtual ~FunctionPassInfo() = 0;

};

/// Abstract class to be inherited from classes unique to each pass.
/// For example, BasicBlockDataDependecyPassInfo.
class BasicBlockPassInfo
{

public:

	/// Pure virtual destructor makes this class abstract.
	virtual ~BasicBlockPassInfo() = 0;

};

/// Abstract class to be inherited from classes unique to each pass.
/// For example, InstructionDataDependecyPassInfo.
class InstructionPassInfo
{

public:

	/// Pure virtual destructor makes this class abstract.
	virtual ~InstructionPassInfo() = 0;

};

/// Abstract class to be inherited from classes unique to each pass.
/// For example, ArgumentDataDependecyPassInfo.
class ArgumentPassInfo
{

public:

	/// Pure virtual destructor makes this class abstract.
	virtual ~ArgumentPassInfo() = 0;

};

}  // namespace comm

#endif

