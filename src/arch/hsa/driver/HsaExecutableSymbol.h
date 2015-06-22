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

#ifndef ARCH_HSA_DRIVER_HSAEXECUTABLESYMBOL_H
#define ARCH_HSA_DRIVER_HSAEXECUTABLESYMBOL_H

#include <string>

#include <arch/hsa/emulator/Function.h>

#include "HsaExecutable.h"


namespace HSA
{

class HsaExecutable;
class BrigCodeEntry;

class HsaExecutableSymbol
{

	// The executable that this symbol belongs to
	const HsaExecutable *executable;


	// The directive that declares the symbol
	BrigCodeEntry *directive;

public:

	/// Constructor
	HsaExecutableSymbol(const HsaExecutable *executable,
			BrigCodeEntry *directive):
			executable(executable),
			directive(directive)
	{
	}

	/// Destructor
	~HsaExecutableSymbol()
	{
	}

	/// Get executable
	const HsaExecutable *getExecutable() const
	{
		return executable;
	}

	/// Get directive
	BrigCodeEntry *getDirective() const
	{
		return directive;
	}

	/// Get kernel argument segment size
	unsigned int getKernelArgumentSize() const
	{
		std::string name = directive->getName();
		Function *function = executable->getFunction(name);
		return function->getArgumentSize();
	}

};

}  // namespace HSA

#endif
