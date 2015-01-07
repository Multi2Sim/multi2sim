/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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
#include <poll.h>
#include <vector>

#include <lib/cpp/Environment.h>
#include <lib/cpp/Misc.cc>

#include "Context.h"
#include "Emu.h"

namespace ARM
{

void Context::UpdateState(unsigned state)
{
	// If the difference between the old and new state lies in other
	// states other than 'ContextSpecMode', a reschedule is marked. */
	unsigned diff = this->state ^ state;
	if (diff & ~ContextStateSpecMode)
		emu->setScheduleSignal();

	// Update state
	this->state = state;
	if (this->state & ContextStateFinished)
		this->state = ContextStateFinished
		| (state & ContextStateAlloc);
	if (this->state & ContextStateZombie)
		this->state = ContextStateZombie
		| (state & ContextStateAlloc);
	if (!(this->state & ContextStateSuspended) &&
			!(this->state & ContextStateFinished) &&
			!(this->state & ContextStateZombie) &&
			!(this->state & ContextStateLocked))
		this->state |= ContextStateRunning;
	else
		this->state &= ~ContextStateRunning;

	// Update presence of context in emulator lists depending on its state
	emu->UpdateContextInList(ContextListRunning, this, this->state & ContextStateRunning);
	emu->UpdateContextInList(ContextListZombie, this, this->state & ContextStateZombie);
	emu->UpdateContextInList(ContextListFinished, this, this->state & ContextStateFinished);
	emu->UpdateContextInList(ContextListSuspended, this, this->state & ContextStateSuspended);
	emu->UpdateContextInList(ContextListAlloc, this, this->state & ContextListAlloc);
}


std::string Context::OpenProcSelfMaps()
{
	// Create temporary file
	int fd;
	FILE *f = NULL;
	char path[256];
	strcpy(path, "/tmp/m2s.XXXXXX");
	if ((fd = mkstemp(path)) == -1 || (f = fdopen(fd, "wt")) == NULL)
		throw misc::Panic("Cannot create temporary file");

	// Get the first page
	unsigned end = 0;
	for (;;)
	{
		// Get start of next range
		mem::Memory::Page *page = memory->getNextPage(end);
		if (!page)
			break;
		unsigned start = page->getTag();
		end = page->getTag();
		int perm = page->getPerm() & (mem::Memory::AccessRead |
				mem::Memory::AccessWrite |
				mem::Memory::AccessExec);

		// Get end of range
		for (;;)
		{
			page = memory->getPage(end + mem::Memory::PageSize);
			if (!page)
				break;
			int page_perm = page->getPerm() &
					(mem::Memory::AccessRead |
							mem::Memory::AccessWrite |
							mem::Memory::AccessExec);
			if (page_perm != perm)
				break;
			end += mem::Memory::PageSize;
			perm = page_perm;
		}

		// Dump range
		fprintf(f, "%08x-%08x %c%c%c%c 00000000 00:00\n", start,
				end + mem::Memory::PageSize,
				perm & mem::Memory::AccessRead ? 'r' : '-',
				perm & mem::Memory::AccessWrite ? 'w' : '-',
				perm & mem::Memory::AccessExec ? 'x' : '-',
				'p');
	}

	// Close file
	fclose(f);
	return path;
}

}
