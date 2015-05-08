/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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
#include <unistd.h>

#include <arch/hsa/asm/Brig.h>
#include <arch/hsa/asm/AsmService.h>
#include <arch/hsa/asm/BrigOperandEntry.h>

#include "Emu.h"
#include "AQLQueue.h"
#include "ProgramLoader.h"

namespace HSA
{

// Singleton instance
std::unique_ptr<ProgramLoader> ProgramLoader::instance;

void ProgramLoader::LoadProgram(const std::vector<std::string> &args,
			const std::vector<std::string> &env = { },
			const std::string &cwd = "",
			const std::string &stdin_file_name = "",
			const std::string &stdout_file_name = "")
{
	instance.reset(new ProgramLoader());
	instance->exe = misc::getFullPath(args[0], cwd);
	instance->args = args;
	instance->cwd = cwd.empty() ? misc::getCwd() : cwd;
	instance->stdin_file_name = misc::getFullPath(stdin_file_name, cwd);
	instance->stdout_file_name = misc::getFullPath(stdout_file_name, cwd);

	instance->file_table.reset(new comm::FileTable());

	instance->LoadBinary();
}


ProgramLoader *ProgramLoader::getInstance()
{
	if (instance.get())
		return instance.get();
	throw misc::Panic("Program not loaded!");
}


void ProgramLoader::LoadBinary()
{
	// Alternative stdin
	if (!stdin_file_name.empty())
	{
		// Open new stdin
		int f = open(stdin_file_name.c_str(), O_RDONLY);
		if (f < 0)
			throw Error(stdin_file_name +
					": Cannot open standard input");

		// Replace file descriptor 0
		file_table->freeFileDescriptor(0);
		file_table->newFileDescriptor(
				comm::FileDescriptor::TypeStandard,
				0,
				f,
				stdin_file_name,
				O_RDONLY);
	}

	// Alternative stdout/stderr
	if (!stdout_file_name.empty())
	{
		// Open new stdout
		int f = open(stdout_file_name.c_str(),
				O_CREAT | O_APPEND | O_TRUNC | O_WRONLY,
				0660);
		if (f < 0)
			throw Error(stdout_file_name +
					": Cannot open standard output");

		// Replace file descriptors 1 and 2
		file_table->freeFileDescriptor(1);
		file_table->freeFileDescriptor(2);
		file_table->newFileDescriptor(
				comm::FileDescriptor::TypeStandard,
				1,
				f,
				stdout_file_name,
				O_WRONLY);
		file_table->newFileDescriptor(
				comm::FileDescriptor::TypeStandard,
				2,
				f,
				stdout_file_name,
				O_WRONLY);
	}
}

}  // namespace HSA

