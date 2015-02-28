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

#include <lib/cpp/CommandLine.h>

#include "Context.h"
#include "Wrapper.h"


namespace cl2llvm
{

std::unique_ptr<Context> Context::instance;

bool Context::active;

int Context::optimization_level;


Context *Context::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create Instance
	instance.reset(new Context());
	return instance.get();
}


void Context::Parse(const std::string &in, const std::string &out)
{
	Cl2llvmContext *context = Cl2llvmContextCreate();
	Cl2llvmContextParse(context, in.c_str(), out.c_str(),
			optimization_level);
	Cl2llvmContextDestroy(context);
}


void Context::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("OpenCL front-end");

	// Option to activate stand-alone OpenCL front-end
	command_line->RegisterBool("--cl2llvm", active,
			"Interpret the source files as OpenCL C source files "
			"(.cl) and produce an LLVM intermediate representation "
			"(.llvm) using the OpenCL front-end.");
	
	// Optimizations
	command_line->RegisterInt32("-O {0, 1, 2}", optimization_level,
			"Optimization level in LLVM code. The default value is "
			"-O2.");
}


void Context::ProcessOptions()
{
	// Check valid optimization level
	if (!misc::inRange(optimization_level, 0, 2))
		throw Error(misc::fmt("Invalid optimization level (-O%d)",
				optimization_level));
}


}  // namespace cl2llvm
