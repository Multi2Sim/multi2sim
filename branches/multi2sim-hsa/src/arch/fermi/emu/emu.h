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

#ifndef ARCH_FERMI_EMU_EMU_H
#define ARCH_FERMI_EMU_EMU_H

#include <arch/common/emu.h>
#include <arch/fermi/asm/Wrapper.h>


/*
 * Class 'FrmEmu'
 */

/* Function implementing an instruction */
typedef void (*FrmEmuInstFunc)(FrmThread *, struct FrmInstWrap *);

CLASS_BEGIN(FrmEmu, Emu)

	/* Disassembler */
	struct FrmAsmWrap *as;
	
	/* Lists of grids */
	struct list_t *grids;
	struct list_t *pending_grids;
	struct list_t *running_grids;
	struct list_t *finished_grids;

	/* Array of pointers to instruction implementation functions */
	FrmEmuInstFunc inst_func[FrmInstIdCount];

	/* Global memory */
	struct mem_t *global_mem;
	unsigned global_mem_top;
	unsigned total_global_mem_size;
	unsigned free_global_mem_size;

	/* Constant memory */
	struct mem_t *const_mem;

	/* Stats */
	int grid_count;
	long long inst_count;
	long long branch_inst_count;
	long long alu_inst_count;
	long long shared_mem_inst_count;
	long long global_mem_inst_count;

CLASS_END(FrmEmu)


void FrmEmuCreate(FrmEmu *self, struct FrmAsmWrap *as);
void FrmEmuDestroy(FrmEmu *self);

void FrmEmuDump(Object *self, FILE *f);
void FrmEmuDumpSummary(Emu *self, FILE *f);
int FrmEmuRun(Emu *emu);


/*
 * Public Variables
 */

extern long long frm_emu_max_cycles;
extern long long frm_emu_max_inst;
extern int frm_emu_max_functions;
extern const int frm_emu_warp_size;

#endif

