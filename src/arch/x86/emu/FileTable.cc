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

#include "FileTable.h"
#include <unistd.h>

namespace x86
{

const misc::StringMap file_desc_type_map =
{
	{ "Regular", FileDescRegular },
	{ "Standard", FileDescStd },
	{ "Pipe", FileDescPipe },
	{ "Virtual", FileDescVirtual },
	{ "GPU", FileDescGPU },
	{ "Socket", FileDescSocket }
};


void FileDesc::Dump(std::ostream &os) const
{
	os << "type = " << file_desc_type_map.MapValue(type);
	os << ", guest_index = " << guest_index;
	os << ", host_index = " << host_index;
	os << misc::fmt(", flags = 0x%x", flags);
	os << ", path = '" << path << "'";
}


FileTable::FileTable()
{
	// Add stdin
	FileDesc *desc = new FileDesc(FileDescStd, 0, 0, 0, "");
	file_descs.emplace_back(desc);

	// Add stdout
	desc = new FileDesc(FileDescStd, 1, 1, 0, "");
	file_descs.emplace_back(desc);

	// Add stderr
	desc = new FileDesc(FileDescStd, 2, 2, 0, "");
	file_descs.emplace_back(desc);
}


void FileTable::Dump(std::ostream &os) const
{
	int occupied = 0;
	os << "File descriptor table:\n";
	for (unsigned i = 0; i < file_descs.size(); i++)
	{
		FileDesc *desc = file_descs[i].get();
		os << i << ". ";
		if (!desc)
		{
			os << "-\n";
			continue;
		}

		desc->Dump(os);
		os << '\n';
		occupied++;
	}
	os << "\t" << file_descs.size() << " entries, " <<
			occupied << " occupied\n\n";
}


int FileTable::getHostIndex(int guest_index) const
{
	// Invalid index
	if (!misc::inRange(guest_index, 0, (int) file_descs.size()))
		return -1;

	// Return
	FileDesc *desc = file_descs[guest_index].get();
	return desc ? desc->getHostIndex() : -1;
}


int FileTable::getGuestIndex(int host_index) const
{
	int guest_index = 0;
	for (auto &desc : file_descs)
	{
		if (desc.get() && desc->getHostIndex() == host_index)
			return guest_index;
		guest_index++;
	}

	// Not found
	return -1;
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


}  // namespace x86

