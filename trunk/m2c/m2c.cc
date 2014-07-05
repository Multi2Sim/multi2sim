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
#include <m2c/common/Context.h>
#include <m2c/si2bin/Context.h>

#include "Wrapper.h"


int main(int argc, char **argv)
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
	si2bin::Context::RegisterOptions();

	// Process command line
	command_line->Process(argc, argv);

	// Process command line options
	comm::Context::ProcessOptions();
	si2bin::Context::ProcessOptions();

	// Rest of the arguments contain sources
	comm::Context *context = comm::Context::getInstance();
	for (int i = 0; i < command_line->getNumArguments(); i++)
		context->AddSourceFile(command_line->getArgument(i));
	
	// Southern Islands assembler
	if (si2bin::Context::isActive())
	{
		// List of files
		si2bin::Context *si2bin_context = si2bin::Context::getInstance();
		const std::vector<std::string> &source_files = context->getSourceFiles();
		const std::vector<std::string> &bin_files = context->getSourceFiles();

		// Compile
		for (unsigned i = 0; i < source_files.size(); i++)
			si2bin_context->Compile(source_files[i], bin_files[i]);

		// Success
		return 0;
	}

	// End
	return 0;
}

