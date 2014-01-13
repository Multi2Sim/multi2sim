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

#include <stdio.h>

#include <arch/common/emu.h>
//#include <arch/kepler/asm/Inst.h>

/* Forward declarations */
//struct KplAsmWrap;
//struct KplInstWrap;

/*
 * Class 'KplEmu'
 */

/* Function implementing an ISA instruction */
//typedef void (*KplEmuInstFunc)(KplThread *thread, struct KplInstWrap *inst);

CLASS_BEGIN(KplEmu, Emu)

	/* Disassembler */
	//struct KplAsmWrap *as;
	
	/* List of grids */
	struct list_t *grids;
	struct list_t *pending_grids;
	struct list_t *running_grids;
	struct list_t *finished_grids;

	/* Global memory */
	struct mem_t *global_mem;
	unsigned int global_mem_top;
	struct mem_t *shared_mem; /* shared with the CPU */

	/* Constant memory, which is organized as 16 banks of 64KB each. */
	struct mem_t *const_mem;

	/* Flags indicating whether the first 32 bytes of constant memory
	 * are initialized. A warning will be issued by the simulator
	 * if an uninitialized element is used by the kernel. */
	int const_mem_init[32];

	unsigned int free_global_mem_size;
	unsigned int total_global_mem_size;

	/* Instruction emulation table */
	//KplEmuInstFunc inst_func[KplInstOpcodeCount];

	/* Stats */
	int grid_count;  /* Number of CUDA functions executed */
	long long scalar_alu_inst_count;  /* Scalar ALU instructions executed */
	long long scalar_mem_inst_count;  /* Scalar mem instructions executed */
	long long branch_inst_count;  /* Branch instructions executed */
	long long vector_alu_inst_count;  /* Vector ALU instructions executed */
	long long lds_inst_count;  /* LDS instructions executed */
	long long vector_mem_inst_count;  /* Vector mem instructions executed */
	long long export_inst_count; /* Export instruction executed */

CLASS_END(KplEmu)


//void KplEmuCreate(KplEmu *self, struct KplAsmWrap *as);
void KplEmuDestroy(KplEmu *self);

//void KplEmuDump(Object *self, FILE *f);
//void KplEmuDumpSummary(Emu *self, FILE *f);

/* Virtual function from class 'Emu' */
//int KplEmuRun(Emu *emu);

/* Access to constant memory */
void KplEmuConstMemWrite(KplEmu *self, unsigned int addr, void *pvalue);
//void KplEmuConstMemRead(FrmEmu *self, unsigned int addr, void *pvalue);




/*
 * Public Stuff
 */

extern long long kpl_emu_max_cycles;
extern long long kpl_emu_max_inst;
extern int kpl_emu_max_functions;

extern char *kpl_emu_cuda_binary_name;

extern int kpl_emu_warp_size;
//extern char *err_kpl_cuda_note;

#endif

