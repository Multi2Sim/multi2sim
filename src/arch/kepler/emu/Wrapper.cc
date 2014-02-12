/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yuqing Shi (shi.yuq@husky.neu.edu)
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

#include "Grid.h"
#include "Wrapper.h"

namespace Kepler
{
////////////////////////////////////////////////////////////////////////////////
// per for class Grid
////////////////////////////////////////////////////////////////////////////////

struct KplGrid *KplGridCreate(KplEmu *emu)
{
	return (KplGrid *) new Grid(emu);
}

void KplGridFree(struct KplGrid *self)
{
	Grid *gr = (Grid *) self;
	delete gr;
}

void KplGridDump(struct KplGrid *self, FILE *f)
{
	Grid *grid = (Grid *) self;
	__gnu_cxx::stdio_filebuf<char> filebuf(fileno(f), std::ios::out);
	std::ostream os(&filebuf);
	grid->Dump(os);
}

void KplGridSetupSize(struct KplGrid *self, unsigned *thread_block_count,
		unsigned *thread_block_size)
{
	Grid *grid = (Grid *) self;
	grid->SetupSize(thread_block_count, thread_block_size);
}

void KplGridSetupConstantMemory(struct KplGrid *self)
{
	Grid *grid = (Grid *) self;
	grid->emu->const_mem->Write(0x8, sizeof(unsigned), (const char*)grid->thread_block_size3);
}

}
