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

#include <cstdlib>
#include <iostream>

#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Misc.h>
#include <m2c/cl2llvm/Context.h>
#include <m2c/common/Context.h>
#include <m2c/llvm2si/Context.h>
#include <m2c/preprocessor/Context.h>
#include <m2c/si2bin/Context.h>

#include "Wrapper.h"


int MainProgram(int argc, char **argv)
{
	// Read command line
	misc::CommandLine *command_line = misc::CommandLine::getInstance();
	command_line->setErrorMessage("Please type 'm2c --help' for a list of "
			"valid Multi2C command-line options.\n");

	// Set help message
	command_line->setHelp("Syntax:"
		"\n\n"
		"$ m2c [<options>] [<sources>]"
		"\n\n"
		"Multi2Sim's command line can take a program executable <exe> "
		"as an argument, given as a binary file in any of the "
		"supported CPU architectures, and optionally followed by its "
		"arguments <args>. The following list of command-line options "
		"can be used for <options>:");

	
	// Register command-line options
	comm::Context::RegisterOptions();
	cl2llvm::Context::RegisterOptions();
	llvm2si::Context::RegisterOptions();
	si2bin::Context::RegisterOptions();
	preprocessor::Context::RegisterOptions();

	// Process command line
	command_line->Process(argc, argv);

	// Process command line options
	comm::Context::ProcessOptions();
	cl2llvm::Context::ProcessOptions();
	llvm2si::Context::ProcessOptions();
	si2bin::Context::ProcessOptions();
	preprocessor::Context::ProcessOptions();

	// Rest of the arguments contain sources
	comm::Context *context = comm::Context::getInstance();
	for (int i = 0; i < command_line->getNumArguments(); i++)
		context->AddSourceFile(command_line->getArgument(i));



	// Stand-alone OpenCL front-end
	if (cl2llvm::Context::isActive())
	{
		// Preprocessor
		preprocessor::Context *preprocessor_context = 
				preprocessor::Context::getInstance();
		for (int i = 0; i < context->getNumSourceFiles(); i++)
		{
			std::string in = context->getSourceFile(i);
			std::string out = context->getSourceFile(i, ".clp");
			preprocessor_context->Parse(in, out);
		}

		// Compile all source files
		cl2llvm::Context *cl2llvm_context = cl2llvm::Context::getInstance();
		for (int i = 0; i < context->getNumSourceFiles(); i++)
		{
			std::string in = context->getSourceFile(i, ".clp");
			std::string out = context->getSourceFile(i, ".llvm");
			cl2llvm_context->Parse(in, out);
			if (!preprocessor::Context::DumpOutput())
			{
				//Command string
				std::string cmd;
	
				// Initialize command
				cmd = "rm " + in;

				//Run command
				system(cmd.c_str());
			}
		}
	}
	
	// Stand-alone Southern Islands back-end
	if (llvm2si::Context::isActive())
	{
		// Compile all source files
		llvm2si::Context *llvm2si_context = llvm2si::Context::getInstance();
		for (int i = 0; i < context->getNumSourceFiles(); i++)
		{
			std::string in = context->getSourceFile(i);
			std::string out = context->getSourceFile(i, ".s");
			llvm2si_context->Parse(in, out);
		}

		// Success
		return 0;
	}

	// Stand-alone Southern Islands assembler
	if (si2bin::Context::isActive())
	{
		// Compile all source files
		si2bin::Context *si2bin_context = si2bin::Context::getInstance();
		for (int i = 0; i < context->getNumSourceFiles(); i++)
		{
			std::string in = context->getSourceFile(i);
			std::string out = context->getSourceFile(i, ".bin");
			si2bin_context->Parse(in, out);
		}

		// Success
		return 0;
	}

	// End
	return 0;
}


int main(int argc, char **argv)
{
	// Main exception handler
	try
	{
		// Run main program
		return MainProgram(argc, argv);
	}
	catch (misc::Exception &e)
	{
		e.Dump();
		return 1;
	}
}

