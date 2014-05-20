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

#include <lib/cpp/Debug.h>

namespace MIPS
{
class Emu
{
	// Unique instance of mips emulator
	static Emu *instance;

	// Index of virtual memory space assigned to new contexts. A new ID
	// can be retrieved in increasing order by using function
	// Emu::getAddressSpaceIndex()
	int address_space_index;

public:
	///Constructor and Desctructor
	Emu();
	~Emu();

	/// The mips emulator is a singleton class. The only possible instance of
	/// it will be allocated the first time this function is invoked.
	static Emu *getInstance();

	/// Return a unique increasing ID for a virtual memory space for
	/// contexts.
	int getAddressSpaceIndex() { return address_space_index++; }

	/// Debugger for program loader
	static misc::Debug loader_debug;

};
}
