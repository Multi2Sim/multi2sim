/*
 *  Multi2Sim
 *  Copyright (C) 2014  Alejandro Villegas (avillegas@uma.es)
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

namespace preprocessor
{


std::unique_ptr<Context> Context::instance;

misc::Debug Context::debug;

std::string Context::debug_file;

bool Context::dump_output;

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
	
	//Error returned
	int ret = 0;

	//Command string
	std::string cmd;
	
	// Initialize command
	cmd = "cpp " + in + " -o " + out;

	// Add defines
	//TODO: add defines

	//Run command
	ret = system(cmd.c_str());
	
	//Quit if error returned by ret
	if(ret)
		exit(ret);
}

void Context::ProcessOptions()
{
	// Debug file
	if (!debug_file.empty())
		debug.setPath(debug_file);
}


void Context::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("Preprocessor");

	// Option to activate stand-alone OpenCL front-end
	command_line->RegisterBool("--dump-preprocessor", dump_output,
			"Dump the output of the preprocessor after processing "
			"(.cl) files into (.clp) files. Must be used together "
			"with the --cl2llvm option.");

}

}  // namespace preprocessor

