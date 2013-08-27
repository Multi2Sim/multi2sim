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


#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <mem-system/memory.h>

#include "emu.h"
#include "grid.h"
#include "isa.h"
#include "thread.h"
#include "warp.h"
#include "thread-block.h"

char *frm_err_isa_note = "\tThe NVIDIA Fermi SASS instruction set is \n"
"\tpartially supported by Multi2Sim. If your program is using an \n"
"\tunimplemented instruction, please email 'development@multi2sim.org' \n"
"\tto request support for it.\n";


#define __NOT_IMPL__ fatal("Fermi instruction '%s' not implemented.\n%s", \
			inst->info->name, frm_err_isa_note);



void frm_isa_FFMA_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src1_id, src2_id, src3_id;
	unsigned int active, pred;
	float dst, src1, src2, src3;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general0.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.general0.src1;
		src1 = thread->gpr[src1_id].f;
		src2_id = inst->bytes.general0.src2;
		if (inst->bytes.general0.src2_mod == 0)
			src2 = thread->gpr[src2_id].f;
		else if (inst->bytes.general0.src2_mod == 1)
			mem_read(emu->const_mem, src2_id, 4, &src2);
		src3_id = inst->bytes.general0_mod1_B.src3;
		src3 = thread->gpr[src3_id].f;

		/* Execute */
		dst = src1 * src2 + src3;

		/* Write */
		dst_id = inst->bytes.general0.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] %f src1 = [0x%x] %f "
			"src2 = [0x%x] %f src3 = [0x%x] %f\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src1_id, src1,
			src2_id, src2, src3_id, src3);
}

void frm_isa_FADD_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src1_id, src2_id;
	unsigned int active, pred;
	float dst, src1, src2;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general0.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.general0.src1;
		src1 = thread->gpr[src1_id].f;
		src2_id = inst->bytes.general0.src2;
		if (inst->bytes.general0.src2_mod == 0)
			src2 = thread->gpr[src2_id].f;
		else if (inst->bytes.general0.src2_mod == 1)
			mem_read(emu->const_mem, src2_id, 4, &src2);

		/* Execute */
		dst = src1 + src2;

		/* Write */
		dst_id = inst->bytes.general0.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] %f src1 = [0x%x] %f "
			"src2 = [0x%x] %f\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src1_id, src1, 
			src2_id, src2);
}

void frm_isa_FADD32I_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src1_id;
	unsigned int active, pred;
	float dst, src1;
	FrmThreadReg imm32;

	FrmWarp *warp;
        FrmWarpSyncStackEntry entry;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.imm.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.imm.src1;
		src1 = thread->gpr[src1_id].f;
		imm32.u32 = inst->bytes.imm.imm32;

		/* Execute */
		dst = src1 + imm32.f;

		/* Write */
		dst_id = inst->bytes.imm.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] %f src1 = [0x%x] %f imm32 = %f\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src1_id, src1, imm32.f);
}

void frm_isa_FCMP_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_FMUL_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src1_id, src2_id;
	unsigned int active, pred;
	float dst, src1, src2;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general0.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.general0.src1;
		src1 = thread->gpr[src1_id].f;
		src2_id = inst->bytes.general0.src2;
		if (inst->bytes.general0.src2_mod == 0)
			src2 = thread->gpr[src2_id].f;
		else if (inst->bytes.general0.src2_mod == 1)
			mem_read(emu->const_mem, src2_id, 4, &src2);

		/* Execute */
		dst = src1 * src2;

		/* Write */
		dst_id = inst->bytes.general0.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] %f src1 = [0x%x] %f "
			"src2 = [0x%x] %f\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src1_id, src1,
			src2_id, src2);
}

void frm_isa_FMUL32I_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src1_id;
	unsigned int active, pred;
	float dst, src1;
	FrmThreadReg imm32;

	FrmWarp *warp;
        FrmWarpSyncStackEntry entry;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.imm.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.imm.src1;
		src1 = thread->gpr[src1_id].f;
		imm32.u32 = inst->bytes.imm.imm32;

		/* Execute */
		dst = src1 * imm32.f;

		/* Write */
		dst_id = inst->bytes.imm.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] %f src1 = [0x%x] %f imm32 = %f\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src1_id, src1, imm32.f);
}

void frm_isa_FMNMX_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_FSWZ_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_FSET_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_FSETP_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_RRO_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_MUFU_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_DFMA_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_DADD_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_DMUL_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_DMNMX_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_DSET_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_DSETP_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_IMAD_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src1_id, src2_id, src3_id;
	unsigned int active, pred;
	int dst, src1, src2, src3;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general1.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.general0.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = inst->bytes.general0.src2;
		if (inst->bytes.general0.src2_mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (inst->bytes.general0.src2_mod == 1)
			mem_read(emu->const_mem, src2_id, 4, &src2);
		src3_id = inst->bytes.general0_mod1_B.src3;
		src3 = thread->gpr[src3_id].s32;

		/* Execute */
		dst = src1 * src2 + src3;

		/* Write */
		dst_id = inst->bytes.general0.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x "
			"src2 = [0x%x] 0x%08x src3 = [0x%x] 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src1_id, src1,
			src2_id, src2, src3_id, src3);
}

void frm_isa_IMUL_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src1_id, src2_id;
	unsigned int active, pred;
	int dst, src1, src2;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general0.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.general0.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = inst->bytes.general0.src2;
		if (inst->bytes.general0.src2_mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (inst->bytes.general0.src2_mod == 1)
			mem_read(emu->const_mem, src2_id, 4, &src2);

		/* Execute */
		dst = src1 * src2;

		/* Write */
		dst_id = inst->bytes.general0.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x "
			"src2 = [0x%x] 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src1_id, src1,
			src2_id, src2);
}

void frm_isa_IADD_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src1_id, src2_id;
	unsigned int active, pred;
	int dst, src1, src2;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general0.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.general0.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = inst->bytes.general0.src2;
		if (inst->bytes.general0.src2_mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (inst->bytes.general0.src2_mod == 1)
			mem_read(emu->const_mem, src2_id, 4, &src2);
		else if (inst->bytes.general0.src2_mod >= 2)
		{
			src2 = inst->bytes.general0.src2;
			/* Sign extension */
			if ((src2 >> 19 & 0x1) == 1)
				src2 |= 0xfff00000;
		}

		/* Execute */
		dst = src1 + src2;

		/* Write */
		dst_id = inst->bytes.general0.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x "
			"src2 = [0x%x] 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src1_id, src1,
			src2_id, src2);
}

void frm_isa_IADD32I_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src1_id;
	unsigned int active, pred;
	int dst, src1, imm32;

	FrmWarp *warp;
        FrmWarpSyncStackEntry entry;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general0.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.imm.src1;
		src1 = thread->gpr[src1_id].s32;
		imm32 = (int)inst->bytes.imm.imm32;

		/* Execute */
		dst = src1 + imm32;

		/* Write */
		dst_id = inst->bytes.imm.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x "
			"imm32 = 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src1_id, src1, imm32);
}

void frm_isa_ISCADD_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src1_id, src2_id;
	unsigned int active, pred;
	int dst, src1, src2;
	unsigned int shamt;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general0.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.general0.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = inst->bytes.general0.src2;
		if (inst->bytes.general0.src2_mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (inst->bytes.general0.src2_mod == 1)
			mem_read(emu->const_mem, src2_id, 4, &src2);
		else if (inst->bytes.general0.src2_mod >= 2)
			src2 = src2_id;
		shamt = inst->bytes.mod0_C.shamt;

		/* Execute */
		dst = (src1 << shamt) + src2;

		/* Write */
		dst_id = inst->bytes.general0.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x "
			"src2 = [0x%x] 0x%08x shamt = %d\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src1_id, src1,
			src2_id, src2, shamt);
}

void frm_isa_ISAD_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_IMNMX_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_BFE_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_BFI_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_SHR_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src1_id, src2_id;
	unsigned int active, pred;
	int dst, src1;
	unsigned int src2;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general0.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.general0.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = inst->bytes.general0.src2;
		if (inst->bytes.general0.src2_mod == 0)
			src2 = thread->gpr[src2_id].u32;
		else if (inst->bytes.general0.src2_mod == 1)
			mem_read(emu->const_mem, src2_id, 4, &src2);
		else if (inst->bytes.general0.src2_mod >= 2)
			src2 = src2_id;

		/* Execute */
		dst = src1 >> src2;

		/* Write */
		dst_id = inst->bytes.general0.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x "
			"src2 = [0x%x] 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src1_id, src1,
			src2_id, src2);
}

void frm_isa_SHL_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src1_id, src2_id;
	unsigned int active, pred;
	int dst, src1;
	unsigned int src2;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general0.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.general0.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = inst->bytes.general0.src2;
		if (inst->bytes.general0.src2_mod == 0)
			src2 = thread->gpr[src2_id].u32;
		else if (inst->bytes.general0.src2_mod == 1)
			mem_read(emu->const_mem, src2_id, 4, &src2);
		else if (inst->bytes.general0.src2_mod >= 2)
			src2 = src2_id;

		/* Execute */
		dst = src1 << src2;

		/* Write */
		dst_id = inst->bytes.general0.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x "
			"src2 = [0x%x] 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src1_id, src1,
			src2_id, src2);
}

void frm_isa_LOP_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_LOP32I_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_FLO_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_ISET_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_ISETP_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, p_id, q_id, src1_id, src2_id, r_id;
	unsigned int active, pred, p, q, src1, src2, r;

	int compare_op;
	int logic_op;
	int compare_result;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general1.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = inst->bytes.general1.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = inst->bytes.general1.src2;
		if (inst->bytes.general1.src2_mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (inst->bytes.general1.src2_mod == 1)
			mem_read(emu->const_mem, src2_id, 4, &src2);
		r_id = inst->bytes.general1.R;
		r = thread->pr[r_id];

		/* Compare */
		compare_op = inst->bytes.general1.cmp;
		switch (compare_op)
		{
			case 1: compare_result = src1 < src2; break;
			case 2: compare_result = src1 == src2; break;
			case 3: compare_result = src1 <= src2; break;
			case 4: compare_result = src1 > src2; break;
			case 5: compare_result = src1 != src2; break;
			case 6: compare_result = src1 >= src2; break;
			default: fatal("%s: unsupported .cmp operation 0x%x", 
						 __FUNCTION__, compare_op);
		}

		/* Logic */
		logic_op = inst->bytes.general1.logic;
		switch (logic_op)
		{
			case 0:
				p = compare_result && r;
				q = !compare_result && r;
				break;
			case 1:
				p = compare_result || r;
				q = !compare_result || r;
				break;
			case 2:
				p = (compare_result && !r) || 
					(!compare_result && r);
				q = (compare_result && r) || 
					(!compare_result && !r);
				break;
			default: fatal("%s: unsupported .logic operation 0x%x", 
						 __FUNCTION__, logic_op);
		}

		/* Write */
		p_id = inst->bytes.general1.dst >> 3;
		q_id = inst->bytes.general1.dst & 0x7;
		if (p_id != 7)
			thread->pr[p_id] = p;
		if (q_id != 7)
			thread->pr[q_id] = q;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"p = [%d] %d, q = [%d] %d, src1 = [0x%x] 0x%08x, "
			"src2 = [0x%x]0x%08x, r = [%d] %d\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, p_id, p, q_id, q, src1_id, src1, 
			src2_id, src2, r_id, r);
}

void frm_isa_ICMP_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_POPC_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_F2F_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_F2I_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_I2F_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_I2I_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_MOV_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src_id;
	unsigned int active, pred;
	int dst, src;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general0.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = inst->bytes.general0.src2;
		if (inst->bytes.general0.src2_mod == 0)
			src = thread->gpr[src_id].s32;
		else if (inst->bytes.general0.src2_mod == 1)
			mem_read(emu->const_mem, src_id, 4, &src);

		/* Execute */
		dst = src;

		/* Write */
		dst_id = inst->bytes.general0.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %d "
			"dst = [0x%x] 0x%08x src = [0x%x] 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src_id, src);
}

void frm_isa_MOV32I_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id;
	unsigned int active, pred;
	int dst, imm32;

	FrmWarp *warp;
        FrmWarpSyncStackEntry entry;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general0.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		imm32 = (int)inst->bytes.imm.imm32;

		/* Execute */
		dst = imm32;

		/* Write */
		dst_id = inst->bytes.imm.dst;
		thread->gpr[dst_id].u32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %d "
			"dst = [0x%x] 0x%08x imm32 = 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, imm32);
}

void frm_isa_SEL_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_PRMT_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_P2R_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_R2P_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_CSET_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_CSETP_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_PSET_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_PSETP_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_TEX_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_TLD_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_TLD4_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_TXQ_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDC_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_LD_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src_id;
	unsigned int active, pred, dst, addr;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.offs.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = inst->bytes.offs.src1;
		addr = thread->gpr[src_id].u32;

		/* Execute */
		mem_read(emu->global_mem, addr, 4, &dst);

		/* Write */
		dst_id = inst->bytes.offs.dst;
		thread->gpr[dst_id].u32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] 0x%08x src = [0x%x] 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src_id, addr);
}

void frm_isa_LDU_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDL_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDS_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src_id;
	unsigned int active, pred, dst0, dst1, dst2, dst3, addr;

	FrmWarp *warp;
        FrmWarpSyncStackEntry entry;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.offs.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = inst->bytes.offs.src1;
		addr = thread->gpr[src_id].u32 + inst->bytes.offs.offset;

		/* Execute */
		mem_read(thread->thread_block->shared_mem, addr, 4, &dst0);
		if (inst->bytes.mod0_B.type == 5 || inst->bytes.mod0_B.type == 6)
			mem_read(thread->thread_block->shared_mem, addr + 4, 4, &dst1);
		if (inst->bytes.mod0_B.type == 6)
		{
			mem_read(thread->thread_block->shared_mem, addr + 8, 4, &dst2);
			mem_read(thread->thread_block->shared_mem, addr + 12, 4, &dst3);
		}

		/* Write */
		dst_id = inst->bytes.offs.dst;
		thread->gpr[dst_id].u32 = dst0;
		if (inst->bytes.mod0_B.type == 5 || inst->bytes.mod0_B.type == 6)
			thread->gpr[dst_id + 1].u32 = dst1;
		if (inst->bytes.mod0_B.type == 6)
		{
			thread->gpr[dst_id + 2].u32 = dst2;
			thread->gpr[dst_id + 3].u32 = dst3;
		}
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst0 = [0x%x] 0x%08x src = [0x%x] 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst0, src_id, addr);
}

void frm_isa_LDLK_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDSLK_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_LD_LDU_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDS_LDU_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_ST_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, value_id, addr_id;
	unsigned int active, pred, value, addr;

	FrmWarp *warp = thread->warp;
	FrmGrid *grid = thread->grid;
	FrmEmu *emu = grid->emu;
        FrmWarpSyncStackEntry entry;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.offs.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		value_id = inst->bytes.offs.dst;
		value = thread->gpr[value_id].u32;
		addr_id = inst->bytes.offs.src1;
		addr = thread->gpr[addr_id].u32;

		/* Execute */
		mem_write(emu->global_mem, addr, 4, &value);
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"value = [0x%x] 0x%08x addr = [0x%x] 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, value_id, value, addr_id, addr);
}

void frm_isa_STL_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_STUL_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_STS_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, value_id, addr_id;
	unsigned int active, pred, value, addr;

	FrmWarp *warp;
        FrmWarpSyncStackEntry entry;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.offs.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		value_id = inst->bytes.offs.dst;
		value = thread->gpr[value_id].u32;
		addr_id = inst->bytes.offs.src1;
		addr = thread->gpr[addr_id].u32 + inst->bytes.offs.offset;

		/* Execute */
		mem_write(thread->thread_block->shared_mem, addr, 4, &value);
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"value = [0x%x] 0x%08x addr = [0x%x] 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, value_id, value, addr_id, addr);
}

void frm_isa_STSUL_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_ATOM_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_RED_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_CCTL_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_CCTLL_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_MEMBAR_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_SULD_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_SULEA_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_SUST_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_SURED_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_SUQ_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_BRA_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id;
	unsigned int active, pred;
	int target;

	FrmWarp *warp;
	FrmWarpSyncStackEntry *top_entry;
	FrmWarpSyncStackEntry *new_entry;

	warp = thread->warp;
	top_entry = &(warp->sync_stack.entries[warp->sync_stack_top]);
	new_entry = &(warp->new_entry);

	/* Get active bit */
	active = (top_entry->active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Get predicate */
	pred_id = inst->bytes.tgt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Get target */
	target = inst->bytes.tgt.target;
	if ((target >> 19 & 0x1) == 1)
		target |= 0xfff00000;

	/* Update active thread mask for new entry */
	if (inst->bytes.tgt.u != 1)
	{
		if (active == 1 && pred == 1)
		{
			new_entry->active_thread_mask |= 
				1 << thread->id_in_warp;
			warp->taken |= 1 << thread->id_in_warp;
		}
		else
		{
			new_entry->active_thread_mask |= 
				0 << thread->id_in_warp;
			warp->taken |= 0 << thread->id_in_warp;
		}
	}

	/* If divergent, push sync stack and go to taken path */
	if (thread->id_in_warp == warp->thread_count - 1)
	{
		if (inst->bytes.tgt.u == 1 || 
				top_entry->active_thread_mask == warp->taken || 
				warp->taken == 0)
			warp->divergent = 0;
		else
			warp->divergent = 1;

		if (warp->divergent)
		{
			new_entry->next_path_pc = warp->pc + 8;
			warp->sync_stack.entries[warp->sync_stack_top + 1].reconv_pc = 
				new_entry->reconv_pc;
			warp->sync_stack.entries[warp->sync_stack_top + 1].next_path_pc =
				new_entry->next_path_pc;
			warp->sync_stack.entries[warp->sync_stack_top + 1].active_thread_mask =
				new_entry->active_thread_mask;
			warp->sync_stack_top++;
			new_entry->reconv_pc = 0;
			new_entry->next_path_pc = 0;
			new_entry->active_thread_mask = 0;
			if (warp->taken != 0)
				warp->pc += target;
		}
		else
		{
			if (inst->bytes.tgt.u == 0 && warp->taken != 0)
				warp->pc += target;
		}
		warp->taken = 0;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"target = 0x%x active_thread_mask = 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, 
			active, pred_id, pred, target,
			new_entry->active_thread_mask);
}

void frm_isa_BRX_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_JMP_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_JMX_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_CAL_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_JCAL_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_RET_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_BRK_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_CONT_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_LONGJMP_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_SSY_impl(FrmThread *thread, FrmInst *inst)
{
	int target;
	FrmWarp *warp = thread->warp;

	/* Get target. Target is the recovergence point */
	target = inst->bytes.tgt.target;
	if ((target >> 19 & 0x1) == 1)
		target |= 0xfff00000;

	/* Set reconvergence PC */
	warp->new_entry.reconv_pc = target;

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] target = 0x%x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, target);
}

void frm_isa_PBK_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_PCNT_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_PRET_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_PLONGJMP_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_BPT_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_EXIT_impl(FrmThread *thread, FrmInst *inst)
{
	FrmWarp *warp;

	warp = thread->warp;

	/* Execute */
	if (thread->id_in_warp == warp->thread_count - 1)
		warp->finished = 1;

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active_thread_mask = 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, 
			warp->sync_stack.entries[warp->sync_stack_top].
			active_thread_mask);
}

void frm_isa_NOP_impl(FrmThread *thread, FrmInst *inst)
{
//	FrmWarp *warp;
//	FrmWarpSyncStackEntry *entry;
//	static int first_time = 1;
//
//	warp = thread->warp;
//
//	if (first_time == 1)
//	{
//		entry = &(warp->sync_stack.entries[warp->sync_stack_top]);
//
//		/* Reverse thread mask */
//		entry->active_thread_mask ^= 1 << thread->id_in_warp;
//
//		/* Go to not taken path */
//		warp->pc = entry->next_path_pc - 8;
//
//		if (thread->id_in_warp == warp->thread_count - 1)
//			first_time = 0;
//	}
//	else
//	{
//		/* Pop sync stack */
//		if (thread->id_in_warp == warp->thread_count - 1)
//			warp->sync_stack_top--;
}

void frm_isa_S2R_impl(FrmThread *thread, FrmInst *inst)
{
	unsigned int pred_id, dst_id, src_id;
	unsigned int active, pred, dst, src;

	FrmWarp *warp;
	FrmWarpSyncStackEntry entry;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> 
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = inst->bytes.general1.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = inst->bytes.general0.src2 & 0xff;
		src = thread->sr[src_id].u32;

		/* Execute */
		dst = src;

		/* Write */
		dst_id = inst->bytes.general0.dst;
		thread->gpr[dst_id].u32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%x] %x "
			"dst = [0x%x] 0x%08x src = [0x%x] 0x%08x\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id, active, 
			pred_id, pred, dst_id, dst, src_id, src);
}

void frm_isa_B2R_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_LEPC_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

void frm_isa_BAR_impl(FrmThread *thread, FrmInst *inst)
{
	FrmThreadBlock *thread_block;
	FrmWarp *warp;

	int warp_id;

	thread_block = thread->thread_block;
	warp = thread->warp;

	/* Set flag to suspend warp execution */
	warp->at_barrier = 1;

	if (thread->id_in_warp == warp->thread_count - 1)
		thread_block->num_warps_at_barrier++;

	/* Continue execution when all warps in the thread block reach the
	 * barrier*/
	if (thread_block->num_warps_at_barrier == thread_block->warp_count)
	{
		for (warp_id = 0; warp_id < thread_block->warp_count;
				warp_id++)
			thread_block->warps[warp_id]->at_barrier = 0;

		thread_block->num_warps_at_barrier = 0;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d]\n", 
			__FUNCTION__, __LINE__, warp->pc, thread->id);
}

void frm_isa_VOTE_impl(FrmThread *thread, FrmInst *inst)
{
	__NOT_IMPL__
}

