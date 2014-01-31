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

#include "si2bin/Config.h"
#include "Wrapper.h"

using namespace misc;


void main_cpp(int argc, char **argv)
{
	// Read command line
	CommandLine command_line(argc, argv);
	command_line.setErrorMessage("Please type 'm2c --help' for a list of "
			"valid Multi2C command-line options.\n");
	command_line.setHelp("Syntax:"
			"\n\n"
			"$ m2c [<options>] [<sources>]"
			"\n\n"
			"Multi2Sim's command line can take a program "
			"executable <exe> as an argument, given as a binary "
			"file in any of the supported CPU architectures, and "
			"optionally followed by its arguments <args>. The "
			"following list of command-line options can be used "
			"for <options>:");

	
	// Register module configurations
	command_line.AddConfig(si2bin::config);

	// Process command line. Return to C version of Multi2Sim if a
	// command-line option was not recognized.
	if (!command_line.Process(false))
		return;

	// Finish if C++ version of Multi2Sim is not activated
	if (!command_line.getUseCpp())
		return;

	std::cerr << "; Multi2C C++\n";
	std::cerr <<
		"\n"
		"* WARNING: The version of Multi2C released together with Multi2Sim *\n"
		"* 4.2 is aimed to be a preliminary version of an open-source       *\n"
		"* OpenCL compiler generating compatible binaries for real GPUs.    *\n"
		"* Important features of OpenCL C are still missing or not fully    *\n"
		"* supported. To request support or provide contributions, please   *\n"
		"* email development@multi2sim.org.                                 *\n"
		"\n";

	// End
	exit(0);
}

