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
#include "Config.h"

#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Misc.h>

using namespace misc;

namespace si2bin
{

void Config::Register(CommandLine &command_line)
{
	// Option --si2bin <file>
	command_line.RegisterString("--si2bin", source_file,
			"Creates an AMD Southern Islands GPU compliant ELF "
			"from the assembly file provided in <arg> using the "
			"internal Southern Islands Assembler.");
}

void Config::Process()
{
	// Run Southern Islands Assembler
	if (!source_file.empty())
	{
		output_file = "output.bin";

			
		std::string err_note =
			"\tThe Southern Islands assembler could not be compiled due to missing\n"
			"\tlibraries on your system, as reported by the ./configure script.\n"
			"\tPlease install these libraries and recompile.\n";
			
		misc::fatal("Southern Islands assembler not available.\n%s", err_note.c_str());

		exit(0);
	}
}


Config config;



}  // namespace si2bin
