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

#ifndef ARCH_KEPLER_EMU_WRAPPER_H
#define ARCH_KEPLER_EMU_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/cuda/function.h>
/*
#include "grid.h"
#include "emu.h"
#include "../asm/asm.h"
*/
struct KplGrid;
struct KplEmu;
struct KplAsm;

////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Emu
////////////////////////////////////////////////////////////////////////////////

KplEmu *KplWrapEmuCreate();

/// Get Kepler Emulator global memory top
unsigned KplGetGlobalMemTop();

/// Get Kelper Emulator global memory free size
unsigned KplGetGlobalMemFreeSize();

/// Get Kelper Emulator global memory Total size
unsigned KplGetGlobalMemTotalSize();

/// Get Kelper Emulator ALU instructon count
unsigned KplGetAluInstCount();

/// Set Kelper Emulator global memory top
void KplSetGlobalMemTop(unsigned value);

/// Set Kelper Emulator global memory free size
void KplSetGlobalMemFreeSize(unsigned value);

/// Set Kelper Emulator global memory total size
void KplSetGlobalMemTotalSize(unsigned value);

/// Read to Global memory
void KplReadGlobalMem(unsigned addr, unsigned size, void* buf);

/// Write to Global memory
void KplWriteGlobalMem(unsigned addr, unsigned size, void* buf);

/// Write to Constant memory
void KplWriteConstMem(unsigned addr, unsigned size, void* buf);

/// Push an element into pending grid list
void KplPushGridList(KplGrid *grid);

/// The run function of Kepler emulator
void KplRun();

/// Dump emulator
void KplWrapEmuDump(FILE *f);

////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Grid
////////////////////////////////////////////////////////////////////////////////


KplGrid *KplWrapGridCreate(struct cuda_function_t *function);

int KplGetID(KplGrid *self);

void KplGridFree(KplGrid *self);

void KplWrapGridDump(KplGrid *self, FILE *f);

void KplWrapGridSetupSize(KplGrid *self,
		unsigned *thread_block_count,
		unsigned *thread_block_size);

void KplWrapGridSetupConstantMemory(KplGrid *self);



#ifdef __cplusplus
}
#endif


#endif  // ARCH_KEPLER_EMU_WRAPPER_H
