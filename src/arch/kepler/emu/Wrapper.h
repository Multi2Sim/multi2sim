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

#ifndef ARCH_KEPLER_EMU_WRAPPER_H
#define ARCH_KEPLER_EMU_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

struct KplGridWrap;

////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Grid
////////////////////////////////////////////////////////////////////////////////


struct KplGridWrap *KplGridWrapCreate(KplEmu *emu,
		struct cuda_function_t *function);
void KplGridWrapFree(struct KplGridWrap *self);
void KplGridWrapDump(struct KplGridWrap *self, FILE *f);
void KplGridWrapSetupSize(unsigned *thread_block_count,
		unsigned *thread_block_size);
void KplGridSetupConstantMemory(KplGridWrap *self);

#ifdef __cplusplus
}
#endif


#endif  // ARCH_KEPLER_EMU_WRAPPER_H
