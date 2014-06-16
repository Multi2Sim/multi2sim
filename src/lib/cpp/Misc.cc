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
#include <stdexcept>
#include <unistd.h>

#include "Misc.h"
#include "String.h"


namespace misc
{


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
	std::string msg = "fatal: ";
	std::cerr << '\n' << StringParagraph(msg + buf, 7) << '\n';

	// Exit with error code 1, or abort in debug mode in order to allow for
	// backtrace in gdb.
#ifdef NDEBUG
	exit(1);
#else
	abort();
#endif
}


void panic(const char *fmt, ...)
{
	char buf[4096];  // 4KB
	va_list va;

	// Construct message
	va_start(va, fmt);
	vsnprintf(buf, sizeof buf, fmt, va);

	// Print in clean paragraphs
	std::string msg = "panic: ";
	std::cerr << '\n' << StringParagraph(msg + buf, 7) << '\n';

	// Abort program
	abort();
}


void warning(const char *fmt, ...)
{
	char buf[4096];  // 4KB
	va_list va;

	// Construct message
	va_start(va, fmt);
	vsnprintf(buf, sizeof buf, fmt, va);

	// Print in clean paragraphs
	std::string msg = "warning: ";
	std::cerr << '\n' << StringParagraph(msg + buf, 9) << '\n';
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
	while (StringPrefix(path_local, "./"))
		path_local.erase(0, 2);

	// File name is empty
	if (path_local.empty())
		return path_local;

	// File name is given as an absolute path
	if (path_local[0] == '/')
		return path_local;
	
	// Default value for base directory
	std::string cwd_local = cwd.empty() ? getCwd() : cwd;

	// Add '/' suffix if not present
	if (cwd_local.back() != '/')
		cwd_local += '/';

	// Return absolute path
	return cwd_local + path_local;
}


std::string getExtension(const std::string &filename)
{
	return filename.substr(filename.find_last_of('.')+1);

}

}  // namespace Misc

