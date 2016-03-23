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

#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_map>

#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>


namespace mem
{

/// A 32-bit virtual memory space
class Memory
{
public:

	/// Log base 2 of the page size
	static const unsigned LogPageSize = 12;

	/// Size of a memory page
	static const unsigned PageSize = 1u << LogPageSize;

	/// Mask to apply on a byte address to discard the page offset
	static const unsigned PageMask = ~(PageSize - 1);

	/// Class representing a runtime error in a memory object
	class Error : public misc::Error
	{
	public:

		/// Constructor
		Error(const std::string &message) : misc::Error(message)
		{
			// Set module prefix
			AppendPrefix("Memory");
		}
	};

	/// Types of memory accesses
	enum AccessType
	{
		AccessInvalid = 0,
		AccessRead = 1 << 0,
		AccessWrite = 1 << 1,
		AccessExec = 1 << 2,
		AccessInit = 1 << 3,
		AccessModified = 1 << 4
	};

	/// A 4KB page of memory
	class Page
	{
		// Page tag, equal to the address of the first byte contained
		// in the page.
		unsigned tag;

		// Page permissions
		unsigned perm;

		// The page data
		std::unique_ptr<char[]> data;
	
	public:

		/// Constructor
		Page(unsigned tag, unsigned perm) :
				tag(tag),
				perm(perm)
		{
			assert((tag & (PageSize - 1)) == 0);
		}

		/// Return the page tag, equal to the address of the first byte
		/// contained in the page.
		unsigned getTag() const { return tag; }

		/// Return the page permissions. This is a bitmap of flags
		/// contained in AccessType.
		unsigned getPerm() const { return perm; }

		/// Return a pointer to the page data, or `nullptr` if the data
		/// was not allocated.
		char *getData() { return data.get(); }

		/// Allocate the page data. If the data buffer was allocated
		/// before, this call is ignored.
		void AllocateData()
		{
			if (data == nullptr)
				data = misc::new_unique_array<char>(PageSize);
		}

		/// Set the page permissions, given as a bitmap of flags of
		/// type AccessType.
		void setPerm(unsigned perm) { this->perm = perm; }

		/// Add a flag to the page permissions, given as a bitmap of
		/// flags of type AccessType.
		void addPerm(unsigned perm) { this->perm |= perm; }
	};

private:

	// Configuration option indicating whether memory objects should use
	// safe mode.
	static bool safe_mode;

	/// Hash table of memory pages, indexed by the page tag.
	std::unordered_map<unsigned, std::unique_ptr<Page>> pages;

	/// Safe mode
	bool safe;

	/// Heap break for CPU contexts
	unsigned heap_break = 0;

	/// Last accessed address
	unsigned last_address = 0;

	/// Create a new page and add it to the page table. The value given in
	/// \a perm is an *or*'ed bitmap of AccessType flags.
	Page *newPage(unsigned address, unsigned perm);

	// Access memory without exceeding page boundaries
	void AccessAtPageBoundary(unsigned address, unsigned size, char *buffer,
			AccessType access);

public:

	/// Constructor
	Memory();

	/// Copy constructor
	Memory(const Memory &memory);

	/// Set the safe mode. A memory in safe mode will crash with a fatal
	/// error message when a memory address is accessed that was not
	/// allocated before witn a call to Map(). In unsafe mode, all memory
	/// addresses can be accessed without prior allocation.
	void setSafe(bool safe) { this->safe = safe; }

	/// Set safe mode to its original global default value
	void setSafeDefault() { safe = safe_mode; }

	/// Return whether the safe mode is on
	bool getSafe() const { return safe; }

	/// Clear content of memory
	void Clear() { pages.clear(); }

	/// Return the memory page corresponding to an address, or `nullptr` if
	/// there is currently no page allocated for that address.
	Page *getPage(unsigned address);

	/// Return the memory page following \a address in the current memory
	/// map. This function is useful to reconstruct consecutive ranges of
	/// mapped pages.
	Page *getNextPage(unsigned address);

 	/// Allocate, if not already allocated, all necessary memory pages to
	/// access \a size bytes after base address \a address. These fields
	/// have no alignment restrictions.
	/// 
	/// Each new page will be allocated with the permissions specified in \a
	/// perm, a bitmap of constants of type AccessType. If any page already
	/// existed, the permissions in \a perm will be added to it.
	void Map(unsigned address, unsigned size, unsigned perm);

 	/// Deallocate memory. If a page in the specified range was not
	/// allocated, it is silently skipped.
	///
	/// \param address
	///	Address aligned to page boundary.
	///
	/// \param size
	///	Number of bytes, multiple of page size.
	void Unmap(unsigned address, unsigned size);

	/// Allocate memory.
	///
	/// \param address
	///	Address to start looking for free memory. If the end of the
	///	memory space is reached, the function will continue circularly
	///	with the lowest memory addresses. The address must be aligned to
	///	the page boundary.
	///
	/// \param size
	///	Number of bytes to allocate. Must be a multiple of the page
	///	size.
	///
	/// \param perm
	///	Bitmap of constants of type AccessType containing the permission
	///	assigned to the allocated pages.
	///
	/// \return
	///	The function returns the base address of the allocated memory
	///	region, or (unsigned) -1 if no free region was found with the
	///	requested size.
	unsigned MapSpace(unsigned address, unsigned size);
	
	/// Allocate memory downward.
	///
	/// \param address
	///	Address to start checking for available free memory. If not
	///	available, the function will keep trying to look for free
	///	regions circularly in a decreasing order. The address must be
	///	align to the page boundary.
	///
	/// \param size
	///	Number of bytes to allocate. Thie value must be a multiple of
	///	the page size.
	///
	/// \param perm
	///	Bitmap of constants of type AccessType containing the permission
	///	assigned to the allocated pages.
	///
	/// \returns
	///	The base address of the allocated memory region, or `(unsigned)
	///	-1` if no free space was found with \a size bytes.
	unsigned MapSpaceDown(unsigned address, unsigned size);
	
	/// Assign protection attributes to pages. If a page in the range is not
	/// allocated, it is silently skipped.
	///
	/// \param address
	///	Address aligned to page boundary.
	///
	/// \param size
	///	Number of bytes, multiple of page size.
	///
	/// \param perm
	///	Bitmap of constants of type AccessType specifying the new
	///	permissions of the pages in the range.
	void Protect(unsigned address, unsigned size, unsigned perm);

	/// Copy a region of memory. All arguments must be multiples of the page
	/// size. In safe mode, the source region must have read access, and the
	/// destination region must have write access.
	///
	/// \param dest
	///	Destination address
	///
	/// \param src
	///	Source address
	///
	/// \param size
	///	Number of bytes to copy
	///
	/// \throw
	///	This function throws a Memory::Error in safe mode if the
	///	source region does not have read permissions, or the destination
	///	region does not have write permissions.
	void Copy(unsigned dest, unsigned src, unsigned size);

 	/// Access memory at any address and size, without page boundary
	/// restrictions.
	///
 	/// \param address
	///	Memory address
	///
	/// \param size
	///	Number of bytes
	///
	/// \param buffer
	///	Buffer to read data from, or write data into.
	///
	/// \param access
	///	Type of access
	///
	/// \throw
	///	A Memory::Error is thrown in safe mode is the written pages
	///	are not allocated, or do not have the permissions requested in
	///	argument \a access.
	void Access(unsigned address, unsigned size, char *buffer,
			AccessType access);

	/// Read from memory, with no alignment or size restrictions.
	///
	/// \param address
	///	Memory address
	///
	/// \param size
	///	Number of bytes
	///
	/// \param buffer
	///	Output buffer to write data
	///
	/// \throw
	///	A Memory::Error is thrown in safe mode is the written pages
	///	are not allocated, or do not have read permissions.
	void Read(unsigned address, unsigned size, char *buffer)
	{
		Access(address, size, buffer, AccessRead);
	}

	/// Write to memory, with no alignment of size restrictions.
	///
	/// \param address
	///	Memory address
	///
	/// \param size
	///	Number of bytes
	///
	/// \param buffer
	///	Input buffer to read data from
	///
	/// \throw
	///	A Memory::Error is thrown in safe mode is the written pages
	///	are not allocated, or do not have write permissions.
	void Write(unsigned address, unsigned size, const char *buffer)
	{
		Access(address, size, const_cast<char *>(buffer), AccessWrite);
	}

	/// Initialize memory with no alignment of size restrictions. The
	/// operation is equivalent to writing, but with different permissions.
	///
	/// \param address
	///	Memory address
	///
	/// \param size
	///	Number of bytes
	///
	/// \param buffer
	///	Input buffer to read data from
	///
	/// \throw
	///	This function throws a Memory::Error in safe mode if the
	///	destination page does not have initialization permissions.
	void Init(unsigned address, unsigned size, const char *buffer)
	{
		Access(address, size, const_cast<char *>(buffer), AccessInit);
	}

	/// Read a string from memory.
	///
	/// \param address
	///	Memory address to read string from
	///
	/// \param max_length (optional, default = 1024)
	///	Maximum length of the read string.
	///
	/// \throw
	///	This function throws a Memory::Error in safe mode if the
	///	source page does not have read permissions.
	///
	/// \throw
	///	A misc::Panic exception occurs if a null terminator is not
	///	found within the first \a max_length characters.
	///
	std::string ReadString(unsigned address, int max_length = 1024);

	/// Write a string into memory.
	///
	/// \param address
	///	Memory address
	///
	/// \param s
	///	String to write
	///
	/// \throw
	///	This function throws a Memory::Error in safe mode if the
	///	destination page does not have write permissions.
	void WriteString(unsigned address, const std::string &s);
	
	/// Zero-out \a size bytes of memory starting at address \a address.
	void Zero(unsigned address, unsigned size);

	/// Obtain a buffer to memory content.
	///
	/// \param address
	///	Memory address
	///
	/// \param size
	///	Number of bytes requested
	///
	/// \param access
	///	Type of access requested
	///
	/// \return
	///	Return a pointer to the memory content. If the requested exceeds
	///	page boundaries, the function returns null. This function is
	///	useful to read content from memory directly with zero-copy
	///	operations.
	///
	/// \throw
	///	This function will throw a Memory::Error if the memory is on
	///	safe mode and the page does not have the permissions requested
	///	in argument \a access.
	char *getBuffer(unsigned address, unsigned size, AccessType access);

	/// Save a subset of the memory space into a file
	///
	/// \param path
	///	Destination file
	///
	/// \param start
	///	Start saving memory from this address.
	///
	/// \param end
	///	Last address to save
	///
	/// \throw
	///	A Memory::Error is thrown if file \a path cannot be accessed.
	void Save(const std::string &path, unsigned start, unsigned end);

	/// Load a region of the memory space from a file into address \a start
	///
	/// \throw
	///	A Memory::Error is thrown if file \a path cannot be accessed.
	void Load(const std::string &path, unsigned start);

	/// Set a new value for the heap break.
	void setHeapBreak(unsigned heap_break) { this->heap_break = heap_break; }

	/// Set the heap break to the value given in \a heap_break if this is
	/// a higher value than the current heap break.
	void growHeapBreak(unsigned heap_break)
	{
		if (this->heap_break < heap_break)
			this->heap_break = heap_break;
	}

	/// Get current heap break.
	unsigned getHeapBreak() { return heap_break; }

	/// Copy the content and attributes from another memory object
	void Clone(const Memory &memory);

};


}  // namespace mem

#endif

