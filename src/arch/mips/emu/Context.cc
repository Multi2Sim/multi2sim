/*
 *  Multi2Sim
 *  Copyright (C) 2014  Sida Gu(dudlykoo@gmail.com)
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
#include "Context.h"
#include "Emu.h"

namespace mips
{
Context::Context()
{
	// Save emulator instance
	emu = Emu::getInstance();
}

void Context::Execute()
{
	/// read 32 bits mips instruction from memory into buffer
	char buffer[32];
	char *buffer_ptr = memory->getBuffer(regs.getPC(), 32,
				mem::MemoryAccessExec);
	if (!buffer_ptr)
		{
			// Disable safe mode. If a part of the 32 read bytes does not
			// belong to the actual instruction, and they lie on a page with
			// no permissions, this would generate an undesired protection
			// fault.
			memory->setSafe(false);
			buffer_ptr = buffer;
			memory->Access(regs.getPC(), 32, buffer_ptr,
					mem::MemoryAccessExec);
		}

	// Return to default safe mode
	memory->setSafeDefault();

}
}
