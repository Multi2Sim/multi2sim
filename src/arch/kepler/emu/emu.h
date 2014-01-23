/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_KEPLER_EMU_EMU_H
#define ARCH_KEPLER_EMU_EMU_H


#include <arch/common/emu.h>
#include <arch/kepler/asm/Wrapper.h>


/*
 * Class 'KplEmu'
 */

/* Function implementing an instruction */
typedef void (*KplEmuInstFunc)(KplThread *, struct KplInstWrap *);

CLASS_BEGIN(KplEmu, Emu)

	/* Disassembler */
	struct KplAsm *as;
	
	/* List of grids */
	struct list_t *grids;
	struct list_t *pending_grids;
	struct list_t *running_grids;
	struct list_t *finished_grids;

	/* Array of pointers to instruction implementation functions */
	KplEmuInstFunc inst_func[KplInstOpcodeCount];

	/* Global memory */
	struct mem_t *global_mem;
	unsigned global_mem_top;
	unsigned total_global_mem_size;
	unsigned free_global_mem_size;

	/* Constant memory */
	struct mem_t *const_mem;

	/* Stats */
	int grid_count;
	long long branch_inst_count;
	long long alu_inst_count;
	long long shared_mem_inst_count;
	long long global_mem_inst_count;	

CLASS_END(KplEmu)


void KplEmuCreate(KplEmu *self, struct KplAsm *as);
void KplEmuDestroy(KplEmu *self);

void KplEmuDump(Object *self, FILE *f);
void KplEmuDumpSummary(Emu *self, FILE *f);
int KplEmuRun(Emu *emu);

void KplEmuConstMemWrite(KplEmu *self, unsigned addr, void *pvalue);
void KplEmuConstMemRead(KplEmu *self, unsigned addr, void *pvalue);


/*
 * Public Variables
 */

extern long long kpl_emu_max_cycles;
extern long long kpl_emu_max_inst;
extern int kpl_emu_max_functions;
extern const int kpl_emu_warp_size;

#endif

