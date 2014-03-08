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

#ifndef ARCH_X86_EMU_FILE_TABLE_H
#define ARCH_X86_EMU_FILE_TABLE_H

#include <iostream>
#include <memory>
#include <vector>

#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>


namespace x86
{

extern const misc::StringMap file_desc_type_map;
enum FileDescType
{
	FileDescInvalid = 0,
	FileDescRegular,		/// Regular file
	FileDescStd,			/// Standard input/output
	FileDescPipe,			/// Pipe
	FileDescVirtual,		/// Virtual file with artificial content
	FileDescGPU,			/// GPU device
	FileDescSocket			/// Network socket
};


// File descriptor
class FileDesc
{
	// File type
	FileDescType type;

	// Guest and host file descriptors
	int guest_index;
	int host_index;

	// O_xxx flags used in 'open' system call
	int flags;

	// Associated path, if applicable
	std::string path;

public:

	/// Constructor
	FileDesc(FileDescType type, int guest_index, int host_index, int flags,
			const std::string &path)
			: type(type), guest_index(guest_index),
			host_index(host_index), flags(flags), path(path) { }
	
	/// Dump to output stream, or \c std::cout if \a os is omitted
	void Dump(std::ostream &os = std::cout) const;

	/// Equivalent to Dump()
	friend std::ostream &operator<<(std::ostream &os,
			const FileDesc &desc) {
		desc.Dump(os);
		return os;
	}

	/// Return type of file descriptor
	FileDescType getType() const { return type; }

	/// Return guest file descriptor index
	int getGuestIndex() const { return guest_index; }

	/// Return host file descriptor index
	int getHostIndex() const { return host_index; }

	/// Return the flags with which this file was opened
	int getFlags() const { return flags; }
	
	/// Update the file descriptor flags
	void setFlags(int flags) { this->flags = flags; }

	/// Return associated path, if any
	const std::string &getPath() const { return path; }
};


/// File descriptor table. The same table can be pointed to by multiple
/// contexts. The last context pointing to it is responsible for freeing it.
class FileTable
{
	// List of file descriptors
	std::vector<std::unique_ptr<FileDesc>> file_descs;

public:

	/// Constructor
	FileTable();
	
	/// Dump table into output stream, or \c std::cout if none given.
	void Dump(std::ostream &os = std::cout) const;

	/// Equivalent to Dump()
	friend std::ostream &operator<<(std::ostream &os,
			const FileTable &table) {
		table.Dump(os);
		return os;
	}

	/// Return file descriptor \a index, or \c nullptr is no file descriptor
	/// exists with that identifier.
	FileDesc *getFileDesc(int index) const {
		return misc::inRange(index, 0, (int) file_descs.size() - 1) ?
				file_descs[index].get() : nullptr;
	}

	/// Create a new file descriptor with a specific guest identifier. If
	/// the value in \a guest_index is set to -1, the first free guest file
	/// descriptor will be allocated.
	FileDesc *newFileDesc(FileDescType type, int guest_index, int host_index,
			const std::string &path, int flags);
	
	/// Create a new file descriptor, assigning the first available guest
	/// identifier.
	FileDesc *newFileDesc(FileDescType type, int host_index,
			const std::string &path, int flags) {
		return newFileDesc(type, -1, host_index, path, flags);
	}

	/// Free file descriptor with identifier \a index. If \a index is out of
	/// range, the call will ignore it silently.
	void freeFileDesc(int index);

	/// Return the host file descriptor associated with the guest file
	/// descriptor given in \a guest_index, or -1 if \a index is not a
	/// valid guest descriptor.
	int getHostIndex(int guest_index) const;

	/// Return the guest file descriptor associated with a host file
	/// descriptor given in \a host_index, or -1 if invalid.
	int getGuestIndex(int host_index) const;
};


}  // namespace x86

#endif

