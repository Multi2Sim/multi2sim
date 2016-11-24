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
 
#include <unistd.h>

#include "FileTable.h"

namespace comm
{

const misc::StringMap FileDescriptor::TypeTypeMap =
{
	{ "Regular", TypeRegular },
	{ "Standard", TypeStandard },
	{ "Pipe", TypePipe },
	{ "Virtual", TypeVirtual },
	{ "Device", TypeDevice },
	{ "Socket", TypeSocket }
};


void FileDescriptor::Dump(std::ostream &os) const
{
	os << "type = " << TypeTypeMap[type];
	os << ", guest_index = " << guest_index;
	os << ", host_index = " << host_index;
	os << misc::fmt(", flags = 0x%x", flags);
	os << ", path = '" << path << "'";
}


FileTable::FileTable()
{
	// Add stdin
	FileDescriptor *desc = new FileDescriptor(
			FileDescriptor::TypeStandard,
			0, 0, 0, "");
	descriptors.emplace_back(desc);

	// Add stdout
	desc = new FileDescriptor(
			FileDescriptor::TypeStandard,
			1, 1, 0, "");
	descriptors.emplace_back(desc);

	// Add stderr
	desc = new FileDescriptor(
			FileDescriptor::TypeStandard,
			2, 2, 0, "");
	descriptors.emplace_back(desc);
}


void FileTable::Dump(std::ostream &os) const
{
	int occupied = 0;
	os << "File descriptor table:\n";
	for (unsigned i = 0; i < descriptors.size(); i++)
	{
		FileDescriptor *desc = descriptors[i].get();
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
	os << "\t" << descriptors.size() << " entries, " <<
			occupied << " occupied\n\n";
}


int FileTable::getHostIndex(int guest_index) const
{
	// Invalid index
	if (!misc::inRange(guest_index, 0, (int) descriptors.size()))
		return -1;

	// Return
	FileDescriptor *desc = descriptors[guest_index].get();
	return desc ? desc->getHostIndex() : -1;
}


int FileTable::getGuestIndex(int host_index) const
{
	int guest_index = 0;
	for (auto &desc : descriptors)
	{
		if (desc.get() && desc->getHostIndex() == host_index)
			return guest_index;
		guest_index++;
	}

	// Not found
	return -1;
}


FileDescriptor *FileTable::newFileDescriptor(
		FileDescriptor::Type type,
		int guest_index,
		int host_index,
		const std::string &path,
		int flags)
{
	// Look for a free entry
	for (int i = 0; i < (int) descriptors.size() && guest_index < 0; i++)
		if (!descriptors[i].get())
			guest_index = i;
	
	// If no free entry was found, add new entry.
	if (guest_index < 0)
	{
		guest_index = descriptors.size();
		descriptors.emplace_back(nullptr);
	}

	// Specified guest_index may still be too large
	for (int i = descriptors.size(); i <= guest_index; ++i)
		descriptors.emplace_back(nullptr);

	// Create guest file descriptor and return.
	FileDescriptor *desc = new FileDescriptor(type, guest_index, host_index,
			flags, path);
	descriptors[guest_index].reset(desc);

	// Return
	return desc;
}


void FileTable::freeFileDescriptor(int index)
{
	// Out of range
	if (!misc::inRange(index, 0, (int) descriptors.size() - 1))
		return;

	// Get file descriptor. If it is empty, exit
	FileDescriptor *desc = descriptors[index].get();
	if (!desc)
		return;
	
	// If it is a virtual file, delete the temporary host path.
	if (desc->getType() == FileDescriptor::TypeVirtual)
	{
		if (unlink(desc->getPath().c_str()))
			misc::Warning("%s: temporary host virtual file could not "
					"be deleted", desc->getPath().c_str());
	}

	// Free file descriptor and remove entry in table.
	descriptors[index] = nullptr;
}


}  // namespace comm

