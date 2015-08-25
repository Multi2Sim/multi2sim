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

#ifndef MEMORY_MMU_H
#define MEMORY_MMU_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <lib/cpp/Debug.h>


namespace mem
{


/// Memory management unit. This class represents a 32-bit physical memory
/// space and provides virtual-to-physical memory translations. The physical
/// memory space supports creation of multiple virtual memory spaces.
class Mmu
{
public:

	// Forward declaration
	class Space;

	/// Log base 2 of the page size
	static const unsigned LogPageSize = 12;

	/// Size of a memory page
	static const unsigned PageSize = 1u << LogPageSize;

	/// Mask to apply on a byte address to discard the page offset
	static const unsigned PageMask = ~(PageSize - 1);

	/// Access types to memory pages
	enum AccessType
	{
		AccessInvalid,
		AccessRead,
		AccessWrite,
		AccessExecute
	};

	/// One page in the MMU
	class Page
	{
		// Virtual address space that the page belongs to
		Space *space;

		// The page virtual address
		unsigned virtual_address;

		// The page physical address
		unsigned physical_address;

		// Statistics
		long long num_read_accesses = 0;
		long long num_write_accesses = 0;
		long long num_execute_accesses = 0;

	public:

		/// Constructor
		Page(Space *space,
				unsigned virtual_address,
				unsigned physical_address) :
				space(space),
				virtual_address(virtual_address),
				physical_address(physical_address)
		{
			assert((virtual_address & ~PageMask) == 0);
			assert((physical_address & ~PageMask) == 0);
		}

		/// Return the virtual address space that the page belongs to
		Space *getSpace() const { return space; }

		/// Return the page's virtual address
		unsigned getVirtualAddress() const { return virtual_address; }

		/// Return the page's physical address
		unsigned getPhysicalAddress() const { return physical_address; }
	};

	/// Virtual memory space in the MMU
	class Space
	{
		// Name of the irtual memory space
		std::string name;

		// Memory management unit that it belongs to
		Mmu *mmu;

		// Hash table of pages in this virtual space indexed by their
		// virtual address.
		std::unordered_map<unsigned, Page *> virtual_pages;

	public:

		/// Constructor
		Space(const std::string &name, Mmu *mmu);

		/// Return the name of the virtual memory space
		const std::string &getName() const { return name; }

		/// Return memory management unit that the virtual memory
		/// space belongs to.
		Mmu *getMmu() const { return mmu; }

		/// Add a new page to the virtual address space. A page must
		/// not exist with the same address.
		void addPage(Page *page);

		/// Return the page associated with the given virtual address,
		/// or `nullptr` if none is. Argument \a virtual_address must be
		/// a multiple of the page size.
		Page *getPage(unsigned virtual_address);
	};

private:

	// Output report file, as set by the user
	static std::string report_file_name;

	// File to dump MMU debug information, as set by the user
	static std::string debug_file;

	// Debugger for MMU
	static misc::Debug debug;
	
	// Name of the MMU
	std::string name;

	// Top of the physical address space. Every time a new page is
	// allocated, this value is incremented by PageSize.
	unsigned top_physical_address = 0;

	// Vector containing all virtual address spaces
	std::vector<std::unique_ptr<Space>> spaces;

	// Vector containing all allocated pages
	std::vector<std::unique_ptr<Page>> pages;

	// Hash table of pages indexed by their physical address
	std::unordered_map<unsigned, Page *> physical_pages;

public:

	//
	// Static members
	//

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();




	//
	// Class members
	//

	/// Constructor
	///
	/// \param name
	///	Name of the MMU used for debugging purposes.
	///
	Mmu(const std::string &name = "");

	/// Return the name of the MMU
	const std::string &getName() const { return name; }

	/// Create a new virtual address space
	///
	/// \param name
	///	Name of the virtual address space used for debugging purposes.
	///
	Space *newSpace(const std::string &name = "");

	/// Translate virtual to physical address.
	///
	/// \param space
	///	Virtual address space.
	///
	/// \param virtual_address
	///	Virtual memory address.
	///
	/// \return
	///	Associated physical address. If no page is currently allocated
	///	for this virtual address, a new one is internally created. A
	///	valid physical address is returned in all cases.
	///
	unsigned TranslateVirtualAddress(Space *space,
			unsigned virtual_address);

	/// Translate physical to virtual address.
	///
	/// \param physical_address
	///	Physical memory address to translate.
	///
	/// \param space
	///	Output argument containing the virtual memory space that the
	///	physical address is associated with, or `nullptr` if the
	///	physical address is invalid.
	///
	/// \param virtual_address
	///	Output argument containing the virtual memory address that the
	///	physical address is associated with, or 0 if the physical
	///	address is invalid.
	///
	/// \return
	///	The function returns `true` only if the physical memory address
	///	is associated to a valid virtual address and the translation was
	///	successful.
	///
	bool TranslatePhysicalAddress(unsigned physical_address,
			Space *&space,
			unsigned &virtual_address);
	
	/// Return `true` if the provided physical address is currently mapped
	/// to a valid virtual address.
	bool isValidPhysicalAddress(unsigned physical_address);
};


}  // namespace mem

#endif

