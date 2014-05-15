/*
 *  Multi2Sim
 *  Copyright (C) 2013  Sida Gu (gu.sid@husky.neu.edu)
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

#include <fcntl.h>

#include <lib/cpp/String.h>

#include "Context.h"

namespace MIPS
{
std::string Context::getFullPath(const std::string &path)
{
	// Remove './' prefix from 's'
	std::string s = path;
	while (misc::StringPrefix(s, "./"))
		s.erase(0, 2);

	// File name is empty
	if (s.empty())
		return s;

	// File name is given as an absolute path
	if (s[0] == '/')
		return s;

	// Relative path
	return loader->cwd + "/" + s;
}

void Context::LoadBinary()
{
	// Alternative stdin
	std::string stdin_full_path = getFullPath(loader->stdin_file_name);
	if (!stdin_full_path.empty())
	{
		// Open new stdin
		int f = open(stdin_full_path.c_str(), O_RDONLY);
		if (f < 0)
			misc::fatal("%s: cannot open stdin",
					stdin_full_path.c_str());

		// Replace file descriptor 0
		file_table->freeFileDesc(0);
		file_table->newFileDesc(FileDescStd, 0, f,
					stdin_full_path, O_RDONLY);
	}

	// Alternative stdout/stderr
	std::string stdout_full_path = getFullPath(loader->stdout_file_name);
	if (!stdout_full_path.empty())
	{
		// Open new stdout
		int f = open(stdout_full_path.c_str(),
				O_CREAT | O_APPEND | O_TRUNC | O_WRONLY,
				0660);
		if (f < 0)
			misc::fatal("%s: cannot open stdin",
					stdin_full_path.c_str());

		// TODO: Replace file descriptors 1 and 2
		//file_table->freeFileDesc(1);
		//file_table->freeFileDesc(2);
		//file_table->newFileDesc(FileDescStd, 1, f,
		//		stdout_full_path, O_WRONLY);
		//file_table->newFileDesc(FileDescStd, 2, f,
		//		stdout_full_path, O_WRONLY);
	}

}
} // namespace mips
