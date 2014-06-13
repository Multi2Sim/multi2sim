/*
 *  Multi2Sim
 *  Copyright (C) 2014  Sida Gu (gu.sid@husky.neu.edu)
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

#include "FileTable.h"
#include <unistd.h>

namespace MIPS
{
const misc::StringMap file_desc_type_map =
{
	{ "Regular", FileDescRegular },
	{ "Standard", FileDescStd },
	{ "Pipe", FileDescPipe },
	{ "Virtual", FileDescVirtual },
	{ "Device", FileDescDevice },
	{ "Socket", FileDescSocket }
};

void FileTable::freeFileDesc(int index)
{
	// Out of range
	if (!misc::inRange(index, 0, (int) file_descs.size() - 1))
		return;

	// Get file descriptor. If it is empty, exit
	FileDesc *desc = file_descs[index].get();
	if (!desc)
		return;

	// If it is a virtual file, delete the temporary host path.
	if (desc->getType() == FileDescVirtual)
	{
		if (unlink(desc->getPath().c_str()))
			misc::warning("%s: temporary host virtual file could not "
					"be deleted", desc->getPath().c_str());
	}

	// Free file descriptor and remove entry in table.
	file_descs[index] = nullptr;
}

FileDesc *FileTable::newFileDesc(FileDescType type, int guest_index,
		int host_index, const std::string &path, int flags)
{
	// Look for a free entry
	for (int i = 0; i < (int) file_descs.size() && guest_index < 0; i++)
		if (!file_descs[i].get())
			guest_index = i;

	// If no free entry was found, add new entry.
	if (guest_index < 0)
	{
		guest_index = file_descs.size();
		file_descs.emplace_back(nullptr);
	}

	// Specified guest_index may still be too large
	for (int i = file_descs.size(); i <= guest_index; ++i)
		file_descs.emplace_back(nullptr);

	// Create guest file descriptor and return.
	FileDesc *desc = new FileDesc(type, guest_index, host_index,
			flags, path);
	file_descs[guest_index].reset(desc);

	// Return
	return desc;
}
}
