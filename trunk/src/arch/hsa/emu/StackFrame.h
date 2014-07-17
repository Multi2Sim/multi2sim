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

#ifndef ARCH_HSA_EMU_STACKFRAME_H
#define ARCH_HSA_EMU_STACKFRAME_H

#include "Function.h"

namespace HSA
{

class StackFrame
{

	// The function this stack frame assorciate with
	Function *function;

	// Argument storage
	std::unique<char> argument_storage;

	// Register storage
	std::unique<char> register_storage;

	// Pointer to parent stack frame
	StackFrame *parent;

public:

	/// Constructor
	StackFrame(Function *function, StackFrame *parent);

	/// Destructor
	virtual ~StackFrame();

	/// Copy the argument value
	void copyArgument();

};

}  // namespace HSA

#endif

