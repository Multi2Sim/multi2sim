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

#ifndef MEM_SYSTEM_MEMORY_H
#define MEM_SYSTEM_MEMORY_H

#include <iostream>

namespace mem
{

enum MemoryAccess
{
	MemoryAccessInvalid = 0,
	MemoryAccessRead = 1 << 0,
	MemoryAccessWrite = 1 << 1,
	MemoryAccessExec = 1 << 2,
	MemoryAccessInit = 1 << 3,
	MemoryAccessModif = 1 << 4
};

static const unsigned MemoryLogPageSize = 12;
static const unsigned MemoryPageShift = MemoryLogPageSize;
static const unsigned MemoryPageSize = (1u << MemoryLogPageSize);
static const unsigned MemoryPageMask = (~(MemoryPageSize - 1));
static const unsigned MemoryPageCount = 1024;


/// A 4KB page of memory
struct MemoryPage
{	
	unsigned tag;
	unsigned perm;
	MemoryPage *next;
	char *data;
};


/// A 32-bit virtual memory space
class Memory
{
	static bool safe_mode;

	/// Hash table of memory pages
	MemoryPage *page_table[MemoryPageCount];

	/// Safe mode
	bool safe;

	/// Heap break for CPU contexts
	unsigned heap_break;

	/// Last accessed address
	unsigned last_address;

	/// Create a new page and add it to the page table. The value given in
	/// \a perm is an OR'ed bitmap of \a MemoryAccess flags.
	MemoryPage *NewPage(unsigned addr, unsigned perm);

	/// Free page at the given address
	void FreePage(unsigned addr);

	// Access memory without exceeding page boundaries
	void AccessAtPageBoundary(unsigned addr, unsigned size, char *buf,
			MemoryAccess access);

public:

	/// Constructor
	Memory();

	/// Destructor
	~Memory();

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
	bool getSafe() { return safe; }

	/// Clear content of memory
	void Clear();

	/// Return the memory page corresponding to an address
	MemoryPage *getPage(unsigned addr);

	/// Return the memory page following \a addr in the current memory map.
	/// This function is useful to reconstruct consecutive ranges of mapped
	/// pages.
	MemoryPage *getNextPage(unsigned addr);

 	/// Allocate, if not already allocated, all necessary memory pages to
	/// access \a size bytes after base address \a addr. These fields have
	/// no alignment restrictions.
	/// 
	/// Each new page will be allocated with the permissions specified in \a
	/// perm, a bitmap of constants of type \a MemoryAccess. If any page
	/// already existed, the permissions in \a perm will be added to it.
	void Map(unsigned addr, unsigned size, unsigned perm);

 	/// Deallocate memory. If a page in the specified range was not
	/// allocated, it is silently skipped.
	///
	/// \param addr Address aligned to page boundary.
	/// \param size Number of bytes, multiple of page size.
	void Unmap(unsigned addr, unsigned size);

	/// Allocate memory.
	///
	/// \param addr Address to start looking for free memory. If the end of
	///        the memory space is reached, the function will continue
	///        circularly with the lowest memory addresses. The address must
	///        be aligned to the page boundary.
	/// \param size Number of bytes to allocate. Must be a multiple of the
	///        page size.
	/// \param perm Bitmap of constants of type \a MemoryAccess containing
	///        the permission assigned to the allocated pages.
	/// \returns The function returns the base address of the allocated
	///          memory region, or (unsigned) -1 if no free region was found
	///          with the requested size.
	unsigned MapSpace(unsigned addr, unsigned size);
	
	/// Allocate memory downwoard.
	///
	/// \param addr Address to start checking for available free memory. If
	///        not available, the function will keep trying to look for free
	///        regions circularly in a decreasing order. The address must be
	///        align to the page boundary.
	/// \param size Number of bytes to allocate. Thie value must be a
	///        multiple of the page size.
	/// \param perm Bitmap of constants of type \a MemoryAccess containing
	///        the permission assigned to the allocated pages.
	/// \returns The base address of the allocated memory region, or
	///          (unsigned) -1 if no free space was found with \a size
	///          bytes.
	unsigned MapSpaceDown(unsigned addr, unsigned size);
	
	/// Assign protection attributes to pages. If a page in the range is not
	/// allocated, it is silently skipped.
	///
	/// \param addr Address aligned to page boundary.
	/// \param size Number of bytes, multiple of page size.
	/// \param perm Bitmap of constants of type \a MemoryAccess specifying
	///        the new permissions of the pages in the range.
	void Protect(unsigned addr, unsigned size, unsigned perm);

	/// Copy a total of \a size bytes from address \a src into address \a
	/// dest. All parameters must be multiple of the page size. The pages in
	/// the source and destination interval must exist. */
	void Copy(unsigned dest, unsigned src, unsigned size);

 	/// Access memory at any address and size, without page boundary
	/// restrictions.
	///
 	/// \param addr Address
	/// \param size Number of bytes
	/// \param buf Buffer to read data from, or write data into.
	/// \param access Type of access
	void Access(unsigned addr, unsigned size, char *buf,
			MemoryAccess access);

	/// Read from memory, with no alignment or size restrictions.
	///
	/// \param addr Address
	/// \param size Number of bytes
	/// \param buf Output buffer to write data
	void Read(unsigned addr, unsigned size, char *buf) {
		Access(addr, size, buf, MemoryAccessRead);
	}

	/// Write to memory, with no alignment of size restrictions.
	///
	/// \param addr Address
	/// \param size Number of bytes
	/// \param buf Input buffer to read data from
	void Write(unsigned addr, unsigned size, const char *buf) {
		Access(addr, size, const_cast<char *>(buf), MemoryAccessWrite);
	}

	/// Initialize memory with no alignment of size restrictions. The
	/// operation is equivalent to writing, but this operation must have a
	/// different category of permissions grantes.
	void Init(unsigned addr, unsigned size, const char *buf) {
		Access(addr, size, const_cast<char *>(buf), MemoryAccessInit);
	}

	/// Read a string from memory at \a addr, with a maximum length of \a
	/// max_length characters.
	std::string ReadString(unsigned addr, int max_length = 1024);

	/// Write a string into memory at \a addr
	void WriteString(unsigned addr, const std::string &s);
	
	/// Zero-out \a size bytes of memory starting at address \a addr.
	void Zero(unsigned addr, unsigned size);

	/// Obtain a buffer to memory content.
	///
	/// \param addr Address
	/// \param size Number of bytes requested
	/// \param access Type of access requested
	/// \returns Return a pointer to the memory content. If the requested
	///          exceeds page boundaries, the function returns null. This
	///          function is useful to read content from memory directly
	///          with zero-copy operations.
	char *getBuffer(unsigned addr, unsigned size, MemoryAccess access);

	/// Save a subset of the memory space into a file
	void Save(const std::string &path, unsigned start, unsigned end);

	/// Load a region of the memory space from a file into address \a start
	void Load(const std::string &path, unsigned start);

	/// Set a new value for the heap break.
	void setHeapBreak(unsigned heap_break) { this->heap_break = heap_break; }

	/// Set the heap break to the value given in \a heap_break if this is
	/// a higher value than the current heap break.
	void growHeapBreak(unsigned heap_break) {
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

