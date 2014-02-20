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

#include <ext/stdio_filebuf.h>

#include "Emu.h"
#include "Grid.h"
#include "ThreadBlock.h"
#include "Warp.h"
#include "Thread.h"
#include "Wrapper.h"

////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Emu
////////////////////////////////////////////////////////////////////////////////

KplEmu *KplWrapEmuCreate(KplAsm * as)
{
	return (KplEmu *) new Kepler::Emu((Kepler::Asm*) as);
}
////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Grid
////////////////////////////////////////////////////////////////////////////////
int KplGetID(KplGrid *self)
{
	Kepler::Grid *grid = (Kepler::Grid *) self;
	return grid->getID();
}
KplGrid *KplWrapGridCreate(KplEmu *emu)
{
	return (KplGrid *) new Kepler:: Grid((Kepler::Emu *)emu);
}

void KplGridFree(KplGrid *self)
{
	Kepler::Grid *gr = (Kepler::Grid *) self;
	delete gr;
}

void KplWrapGridDump(KplGrid *self, FILE *f)
{
	Kepler::Grid *grid = (Kepler::Grid *) self;
	__gnu_cxx::stdio_filebuf<char> filebuf(fileno(f), std::ios::out);
	std::ostream os(&filebuf);
	grid->Dump(os);
}

void KplWrapGridSetupSize(KplGrid *self, unsigned *thread_block_count,
		unsigned *thread_block_size)
{
	Kepler::Grid *grid = (Kepler::Grid *) self;
	grid->SetupSize(thread_block_count, thread_block_size);
}

void KplWrapGridSetupConstantMemory(KplGrid *self)
{
	Kepler::Grid *grid = (Kepler::Grid *) self;
	grid->GridSetupConstantMemory();
}

/// Get Kepler Emulator global memory top
unsigned KplGetGlobalMemTop(KplEmu* self)
{
	Kepler::Emu *emu = (Kepler::Emu*) self;
	return emu->getGlobalMemTop();
}

/// Get global memory free size
unsigned KplGetGlobalMemFreeSize(KplEmu* self)
{
	Kepler::Emu *emu = (Kepler::Emu*) self;
	return emu->getGlobalMemFreeSize();
}

/// Get global memory Total size
unsigned KplGetGlobalMemTotalSize(KplEmu* self)
{
	Kepler::Emu *emu = (Kepler::Emu*) self;
	return emu->getGlobalMemTotalSize();
}

// Setter
/// Set Kelper Emulator global memory top
void KplSetGlobalMemTop(KplEmu* self, unsigned value)
{
	Kepler::Emu *emu = (Kepler::Emu*) self;
	emu->SetGlobalMemTop(value);
}

/// Set global memory free size
void KplSetGlobalMemFreeSize(KplEmu* self, unsigned value)
{
	Kepler::Emu *emu = (Kepler::Emu*) self;
	emu->setGlobalMemFreeSize(value);
}

/// Set global memory total size
void KplSetGlobalMemTotalSize(KplEmu* self, unsigned value)
{
	Kepler::Emu *emu = (Kepler::Emu*) self;
	emu->setGlobalMemTotalSize(value);
}

/// Read to Global memory
void KplReadGlobalMem(KplEmu *self, unsigned addr, unsigned size, void* buf)
{
	Kepler::Emu *emu = (Kepler::Emu*) self;
	emu->ReadGlobalMem(addr, size, (char*)buf);
}

/// Write to Global memory
void KplWriteGlobalMem(KplEmu *self, unsigned addr, unsigned size, void* buf)
{
	Kepler::Emu *emu = (Kepler::Emu*) self;
	emu->WriteGlobalMem(addr, size, (const char*)buf);
}

/// Write to Constant memory
void KplWriteConstMem(KplEmu *self, unsigned addr, unsigned size, void* buf)
{
	Kepler::Emu *emu = (Kepler::Emu*) self;
	emu->WriteConstMem(addr, size, (const char*)buf);
}

void KplPushGridList(KplEmu *self, KplGrid *grid)
{
	Kepler::Emu *emu = (Kepler::Emu*) self;
	Kepler::Grid *gr = (Kepler::Grid*) grid;
	emu->PushPendingGrid(gr);
}
