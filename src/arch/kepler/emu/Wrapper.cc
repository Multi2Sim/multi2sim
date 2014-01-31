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

#include "Grid.h"
#include "Wrapper.h"

////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Grid
////////////////////////////////////////////////////////////////////////////////

struct KplGridWrap *KplGridWrapCreate(KplEmu *emu,
		struct cuda_function_t *function)
{
	return (KplGridWrap *) new Grid(emu, function);
}

void KplGridWrapFree(struct KplGridWrap *self)
{
	Grid *gr = (Grid *) self;
	delete gr;
}

void KplGridWrapDump(struct KplGridWrap *self, FILE *f)
{
	Grid *gr = (Grid *) self;
	gr->Dump(f);
}

void KplGridWrapSetupSize(struct KplGridWrap *self, unsigned *thread_block_count,
		unsigned *thread_block_size)
{
	Grid *gr = (Grid *) self;
	gr->SetupSize(thread_block_count, thread_block_size);
}

void KplGridWrapSetupConstantMemory(struct KplGridWrap *self)
{
	Grid *gr = (Grid *) self;
	gr->SetupConstantMemory();
}
