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
#include <driver/cuda/function.h>

#include "Emu.h"
#include "Grid.h"
#include "ThreadBlock.h"
#include "Warp.h"
#include "Thread.h"
#include "Wrapper.h"

//////////////////////////////////////////////////////////////////
// Wrapper for class Emu
//////////////////////////////////////////////////////////////////

KplEmu *KplWrapEmuCreate()
{
	Kepler::Emu* emu;
	return (KplEmu *) emu->getInstance();
}

// Get Kepler Emulator global memory top
unsigned KplGetGlobalMemTop()
{
	Kepler::Emu *emu;
	return emu->getInstance()->getGlobalMemTop();
}

// Get global memory free size
unsigned KplGetGlobalMemFreeSize()
{
	Kepler::Emu *emu;
	return emu->getInstance()->getGlobalMemFreeSize();
}

// Get global memory Total size
unsigned KplGetGlobalMemTotalSize()
{
	Kepler::Emu *emu;
	return emu->getInstance()->getGlobalMemTotalSize();
}

// Get Kelper Emulator ALU instructon count
unsigned KplGetAluInstCount()
{
	Kepler::Emu *emu = Kepler::Emu::getInstance();
	return emu->getAluInstCount();
}

// Setter
// Set Kelper Emulator global memory top
void KplSetGlobalMemTop(unsigned value)
{
	Kepler::Emu *emu;
	emu->getInstance()->SetGlobalMemTop(value);
}

// Set global memory free size
void KplSetGlobalMemFreeSize(unsigned value)
{
	Kepler::Emu *emu;
	emu->getInstance()->setGlobalMemFreeSize(value);
}

// Set global memory total size
void KplSetGlobalMemTotalSize(unsigned value)
{
	Kepler::Emu *emu;
	emu->getInstance()->setGlobalMemTotalSize(value);
}

// Read to Global memory
void KplReadGlobalMem(unsigned addr, unsigned size, void* buf)
{
	Kepler::Emu *emu;
	emu->getInstance()->ReadGlobalMem(addr, size, (char*)buf);
}

// Write to Global memory
void KplWriteGlobalMem(unsigned addr, unsigned size, void* buf)
{
	Kepler::Emu *emu;
	emu->getInstance()->WriteGlobalMem(addr, size, (const char*)buf);
}

// Write to Constant memory
void KplWriteConstMem(unsigned addr, unsigned size, void* buf)
{
	Kepler::Emu *emu;
	emu->getInstance()->WriteConstMem(addr, size, (const char*)buf);
}

void KplPushGridList(KplGrid *grid)
{
	Kepler::Emu *emu;
	Kepler::Grid *gr = (Kepler::Grid*) grid;
	emu->getInstance()->PushPendingGrid(gr);
}

void KplRun()
{
	Kepler::Emu *emu;
	emu->getInstance()->Run();
}

// Dump emulator
void KplWrapEmuDump(FILE *f)
{
	Kepler::Emu *emu = Kepler::Emu::getInstance();
	__gnu_cxx::stdio_filebuf<char> filebuf(fileno(f), std::ios::out);
	std::ostream os(&filebuf);
	emu->Dump(os);
}

//////////////////////////////////////////////////////////////////
// Wrapper for class Grid
//////////////////////////////////////////////////////////////////
int KplGetID(KplGrid *self)
{
	Kepler::Grid *grid = (Kepler::Grid *) self;
	return grid->getID();
}
KplGrid *KplWrapGridCreate(cuda_function_t *function)
{
	return (KplGrid *) new Kepler:: Grid(function);
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

