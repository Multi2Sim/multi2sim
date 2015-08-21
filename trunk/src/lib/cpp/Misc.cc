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

#include <algorithm>
#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "Error.h"
#include "Misc.h"
#include "String.h"
#include "Terminal.h"


namespace misc
{

//
// Numeric Functions
//


unsigned LogBase2(unsigned value)
{
	// Check that value is a valid power of two
	if ((value & (value - 1)) != 0)
		throw misc::Panic("Value is not a power of 2");
	
	// Check that value is not 0
	if (value == 0)
		throw misc::Panic("Value cannot be 0");

	// Calculate
	unsigned result = 0;
	while ((value & 1) == 0)
	{
		value >>= 1;
		result++;
	}
	return result;
}



//
// Output messages
//

void fatal(const char *fmt, ...)
{
	char buf[4096];  // 4KB
	va_list va;

	// Construct message
	va_start(va, fmt);
	vsnprintf(buf, sizeof buf, fmt, va);

	// Print in clean paragraphs
	StringFormatter formatter;
	formatter << buf;

	// Dump to standard error output
	Terminal::Red(std::cerr);
	std::cerr << '\n' << formatter << "\n\n";
	Terminal::Reset(std::cerr);

	// Finish program
	exit(1);
}


void panic(const char *fmt, ...)
{
	char buf[4096];  // 4KB
	va_list va;

	// Construct message
	va_start(va, fmt);
	vsnprintf(buf, sizeof buf, fmt, va);

	// Print in clean paragraphs
	StringFormatter formatter;
	formatter << "Panic: " << buf;

	// Dump to standard error output
	Terminal::Red(std::cerr);
	std::cerr << '\n' << formatter << "\n\n";
	Terminal::Reset(std::cerr);

	// Abort program
	abort();
}


void Warning(const char *fmt, ...)
{
	char buf[4096];  // 4KB
	va_list va;

	// Construct message
	va_start(va, fmt);
	vsnprintf(buf, sizeof buf, fmt, va);
	
	// Print in clean paragraphs
	StringFormatter formatter;
	formatter << "Warning: " << buf;

	// Dump to standard error output
	Terminal::Blue(std::cerr);
	std::cerr << '\n' << formatter << "\n\n";
	Terminal::Reset(std::cerr);
}




//
// File system
//

std::string getCwd()
{
	char path[FILENAME_MAX];
	if (!getcwd(path, sizeof path))
		panic("%s: cannot store the current working directory in a "
				"buffer of %d bytes", __FUNCTION__,
				(int) sizeof path);
	return path;
}


std::string getFullPath(const std::string &path, const std::string &cwd)
{
	// Remove './' prefix from path
	std::string path_local = path;

	// File name is given as an absolute path
	if (path_local[0] == '/')
		return path_local;

	while (StringPrefix(path_local, "./"))
		path_local.erase(0, 2);

	// File name is empty
	if (path_local.empty())
		return path_local;
	
	// Default value for base directory
	std::string cwd_local = cwd.empty() ? getCwd() : cwd;

	// Add '/' suffix if not present
	if (cwd_local.back() != '/')
		cwd_local += '/';

	// Return absolute path
	return cwd_local + path_local;
}


std::string getExtension(const std::string &path)
{
	// Get last '.' and '/'
	size_t dot_index = path.find_last_of('.');
	size_t slash_index = path.find_last_of('/');

	// No '.' found
	if (dot_index == std::string::npos)
		return "";
	
	// Last '.' comes before last '/'
	if (slash_index != std::string::npos && slash_index > dot_index)
		return "";
	
	// Return extension
	return path.substr(dot_index + 1);
}


std::string getBaseName(const std::string &path)
{
	// Get last '.' and '/'
	size_t dot_index = path.find_last_of('.');
	size_t slash_index = path.find_last_of('/');

	// No '.' found
	if (dot_index == std::string::npos)
		return path;
	
	// Last '.' comes before last '/'
	if (slash_index != std::string::npos && slash_index > dot_index)
		return path;
	
	// Return base name
	return path.substr(0, dot_index);
}

}  // namespace Misc

