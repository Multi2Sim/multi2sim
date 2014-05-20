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
#include "Emu.h"

namespace MIPS
{
static misc::StringMap section_flags_map =
{
	{ "SHF_WRITE", 1 },
	{ "SHF_ALLOC", 2 },
	{ "SHF_EXECINSTR", 4 }
};

void Context::LoadELFSections(ELFReader::File *binary)
{
	Emu::loader_debug << "\nLoading ELF sections\n";
	loader->bottom = 0xffffffff;
	for (auto &section : binary->getSections())
	{
		// Debug
		unsigned perm = mem::MemoryAccessInit | mem::MemoryAccessRead;
		std::string flags_str = section_flags_map.MapFlags(section->getFlags());
		Emu::loader_debug << misc::fmt("  section '%s': offset=0x%x, "
						"addr=0x%x, size=%u, flags=%s\n",
						section->getName().c_str(), section->getOffset(),
						section->getAddr(), section->getSize(),
						flags_str.c_str());

				// Process section
				if (section->getFlags() & SHF_ALLOC)
				{
					// Permissions
					if (section->getFlags() & SHF_WRITE)
						perm |= mem::MemoryAccessWrite;
					if (section->getFlags() & SHF_EXECINSTR)
						perm |= mem::MemoryAccessExec;

					// Load section
					memory->Map(section->getAddr(), section->getSize(), perm);
					memory->growHeapBreak(section->getAddr() + section->getSize());
					loader->bottom = std::min(loader->bottom, section->getAddr());

					// If section type is SHT_NOBITS (sh_type=8), initialize to 0.
					// Otherwise, copy section contents from ELF file.
					if (section->getType() == 8)
					{
						char *zero_buffer = new char[section->getSize()]();
						memory->Init(section->getAddr(), section->getSize(),
								zero_buffer);
						delete zero_buffer;
					}
					else
					{
						memory->Init(section->getAddr(), section->getSize(),
								section->getBuffer());
					}
				}
	}
}

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

		// Replace file descriptors 1 and 2
		file_table->freeFileDesc(1);
		file_table->freeFileDesc(2);
		file_table->newFileDesc(FileDescStd, 1, f,
				stdout_full_path, O_WRONLY);
		file_table->newFileDesc(FileDescStd, 2, f,
				stdout_full_path, O_WRONLY);
	}

	// Load ELF binary
	loader->exe = getFullPath(loader->args[0]);
	loader->binary.reset(new ELFReader::File(loader->exe));

	// Read sections and program entry
	LoadELFSections(loader->binary.get());
	loader->prog_entry = loader->binary->getEntry();

}
} // namespace mips
