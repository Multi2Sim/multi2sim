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

#ifndef M2C_PREPROCESSOR_CONTEXT_H
#define M2C_PREPROCESSOR_CONTEXT_H

#include <vector>

#include <lib/cpp/Debug.h>


namespace preprocessor
{

/// Singleton containing global information for the M2C compiler.
class Context
{

	// Dump preprocessor output
	static bool dump_output;

	// Source files obtained from command line
	std::vector<std::string> source_files;

	// Macros defined in command line
	std::vector<std::string> macros;

	// Debug file name
	static std::string debug_file;

	// Intance of singleton
	static std::unique_ptr<Context> instance;

	// Debugger
	static misc::Debug debug;

	// Private constructor for singleton
	Context() { }

public:

	/// Compile one file
	void Parse(const std::string &in, const std::string &out);

	/// Get instance of singleton
	static Context *getInstance();

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Dumps .clp files as preprocessor output	
	static bool DumpOutput() {return dump_output; }
};


}  // namespace comm

#endif

