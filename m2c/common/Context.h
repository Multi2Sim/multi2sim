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

#ifndef M2C_COMMON_CONTEXT_H
#define M2C_COMMON_CONTEXT_H

#include <vector>

#include <lib/cpp/Debug.h>


namespace comm
{

/// Singleton containing global information for the M2C compiler.
class Context
{

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

	/// Obtain instance of singleton
	static Context *getInstance();

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Add source file. The new source file has a fixed index in an
	/// internal source file list. The new file is added to the list of
	/// .clp/.llvm/.s/.bin files with the same index.
	void AddSourceFile(const std::string &path);

	/// Return the number of source files
	int getNumSourceFiles() const { return source_files.size(); }

	/// Return the name of a registered source file.
	///
	/// \param index
	///	Index of the registered source file.
	///
	/// \param extension
	///	Optional argument indicating a replacement for the source file
	///	extension. If none specified, the original source file is
	///	returned.
	std::string getSourceFile(int index, const std::string &extension = "");

	/// Obtain list of source file
	const std::vector<std::string> &getSourceFiles()
	{
		return source_files;
	}
};


}  // namespace comm

#endif

