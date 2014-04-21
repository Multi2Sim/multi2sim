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
#include <mem-system/Memory.h>

#include "Regs.h"

namespace mips
{
class Emu;

// mips Context
class Context
{
	// Emulator it belongs to
	Emu *emu;

	// Context memory. This object can be shared by multiple contexts, so it
	// is declared as a shared pointer. The last freed context pointing to
	// this memory object will be the one automatically freeing it.
	std::shared_ptr<mem::Memory> memory;

	// Register file. Each context has its own copy always.
	Regs regs;

public:
	Context();
	~Context();

	/// Run one instruction for the context at the position pointed to by
	/// register program counter.
	void Execute();

};
}
