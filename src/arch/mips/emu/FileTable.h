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
#include <iostream>
#include <memory>
#include <vector>

#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

namespace MIPS
{

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

	/// Return type of file descriptor
	FileDescType getType() const { return type; }

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

	/// Free file descriptor with identifier \a index. If \a index is out of
	/// range, the call will ignore it silently.
	void freeFileDesc(int index);

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

};
}
