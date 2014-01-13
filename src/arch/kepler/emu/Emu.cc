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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <mem-system/memory.h>

#include "Emu.h"
//#include "isa.h"
#include "Grid.h"
//#include "machine.h"
#include "Warp.h"
#include "ThreadBlock.h"

/*
 * Class 'KplEmu'
 */

//void KplEmuCreate(KplEmu *self, struct KplAsmWrap *as)
//{
	/* Parent */
	//EmuCreate(asEmu(self), "Kepler");

        /* Initialize */
	//self->as = as;
	//self->grids = list_create();
	//self->pending_grids = list_create();
	//self->running_grids = list_create();
	//self->finished_grids = list_create();
        //self->global_mem = mem_create();
        //self->global_mem->safe = 0;
        //self->global_mem_top = 0;
        //self->total_global_mem_size = 1 << 30; /* 2GB */
        //self->free_global_mem_size = 1 << 30; /* 2GB */
        //self->const_mem = mem_create();
        //self->const_mem->safe = 0;

	/* Initialize instruction execution table */
//#define DEFINST(_name, _fmt_str, _category, _opcode) \
	self->inst_func[KPL_INST_##_name] = kpl_isa_##_name##_impl;
//#include <arch/kepler/asm/asm.dat>
//#undef DEFINST

        /* Virtual functions */
//        asObject(self)->Dump = KplEmuDump;
//        asEmu(self)->DumpSummary = KplEmuDumpSummary;
//        asEmu(self)->Run = KplEmuRun;
//}

void KplEmuDestroy(KplEmu *self)
{
	/* Free grids */
//	list_free(self->grids);
//	list_free(self->pending_grids);
//	list_free(self->running_grids);
//	list_free(self->finished_grids);
  //      mem_free(self->const_mem);
    //    mem_free(self->global_mem);
}


//void KplEmuDump(Object *self, FILE *f)
//{
//	/* Call parent */
//	EmuDump(self, f);
//}


//void KplEmuDumpSummary(Emu *self, FILE *f)
//{
//	/* Call parent */
//	EmuDumpSummary(self, f);
//}


//int KplEmuRun(Emu *self)
//{
//	KplEmu *emu = asKplEmu(self);
//
//	KplGrid *grid;
//	KplThreadBlock *thread_block;
//	KplWarp *warp;
//
//	int warp_id;
//
//	/* Stop emulation if no grid needs running */
//	if (!list_count(emu->grids))
//		return FALSE;
//
//	/* Remove grid and its thread blocks from pending list, and add them to
//	 * running list */
//	while ((grid = list_head(emu->pending_grids)))
//	{
//		while ((thread_block = list_head(grid->pending_thread_blocks)))
//		{
//			list_remove(grid->pending_thread_blocks, thread_block);
//			list_add(grid->running_thread_blocks, thread_block);
//		}
//
//		list_remove(emu->pending_grids, grid);
//		list_add(emu->running_grids, grid);
//	}
//
//	/* Run one instruction */
//	while ((grid = list_head(emu->running_grids)))
//	{
//		while ((thread_block = list_head(grid->running_thread_blocks)))
//		{
//			for (warp_id = 0; warp_id <
//					list_count(thread_block->running_warps);
//					warp_id++)
//			{
//				warp = list_get(thread_block->running_warps,
//						warp_id);
//				if (warp->finished || warp->at_barrier)
//					continue;
//
//				KplWarpExecute(warp);
//			}
//		}
//	}
//
//	/* Free finished grids */
//	assert(list_count(emu->pending_grids) == 0 &&
//			list_count(emu->running_grids) == 0);
//	while ((grid = list_head(emu->finished_grids)))
//	{
//		/* Remove grid from finished list */
//		list_remove(emu->finished_grids, grid);
//
//		/* Free grid */
//		delete(grid);
//	}

	/* Continue emulation */
//	return TRUE;
//}


void KplEmuConstMemWrite(KplEmu *self, unsigned int addr, void *value_ptr)
{
	/* Mark c[0][0..1c] as initialized */
	if (addr < 0x20)
		self->const_mem_init[addr] = 1;

	/* Write */
	mem_write(self->const_mem, addr, sizeof(unsigned int), value_ptr);
}


//void KplEmuConstMemRead(KplEmu *self, unsigned int addr, void *value_ptr)
//{
//	/* Warn if c[0][0..1c] is used uninitialized */
//	if (addr < 0x20 && !self->const_mem_init[addr])
//		warning("c [0] [0x%x] is used uninitialized", addr);
//
//	/* Read */
//	mem_read(self->const_mem, addr, sizeof(unsigned int), value_ptr);
//}




/*
 * Non-Class Stuff
 */

long long kpl_emu_max_cycles;
long long kpl_emu_max_inst;
int kpl_emu_max_functions;

char *kpl_emu_cuda_binary_name = "";

int kpl_emu_warp_size = 32;

