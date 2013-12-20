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

#include <math.h>

#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <mem-system/memory.h>
#include <arch/fermi/asm/Wrapper.h>

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
		FrmInstWrapGetName(inst), frm_err_isa_note);

void frm_isa_FMNMX_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_FSET_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_FSETP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned p_id, p, q_id, q, r_id, r;
	unsigned src1_id, src2_id;
	float src1, src2;
	union {unsigned i; float f;} gpr;

	// Operations and intermediate values
	unsigned cop;
	unsigned bop;
	unsigned cop_res;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Get predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].f;
		if (((fmt.fmod0 >> 3) & 0x1) == 1)
			src1 = fabsf(src1);
		if (((fmt.fmod0 >> 5) & 0x1) == 1)
			src1 = -src1;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].f;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
		{
			gpr.i = fmt.src2 << 12;
			src2 = gpr.f;
		}
		if (((fmt.fmod0 >> 2) & 0x1) == 1)
			src2 = fabsf(src2);
		if (((fmt.fmod0 >> 4) & 0x1) == 1)
			src2 = -src2;
		r_id = fmt.fmod1_srco & 0x7;
		r = thread->pr[r_id];
		cop = (fmt.fmod1_srco >> 6) & 0xf;
		bop = (fmt.fmod1_srco >> 4) & 0x3;

		/* Compare */
		if (cop == 1)
			cop_res = src1 < src2;
		else if (cop == 2)
			cop_res = src1 == src2;
		else if (cop == 3)
			cop_res = src1 <= src2;
		else if (cop == 4)
			cop_res = src1 > src2;
		else if (cop == 5)
			cop_res = src1 != src2;
		else if (cop == 6)
			cop_res = src1 >= src2;
		else
			fatal("%s: compare operation %d not implemented",
					__FUNCTION__, cop);

		/* Logic */
		if (bop == 0)
		{
			p = cop_res && r;
			q = !cop_res && r;
		}
		else if (bop == 1)
		{
			p = cop_res || r;
			q = !cop_res || r;
		}
		else if (bop == 2)
		{
			p = (cop_res && !r) || (!cop_res && r);
			q = (cop_res && r) || (!cop_res && !r);
		}
		else
			fatal("%s: bitwise operation %d not implemented",
					__FUNCTION__, bop);

		/* Write */
		p_id = (fmt.dst >> 3) & 0x7;
		q_id = fmt.dst & 0x7;
		if (p_id != 7)
			thread->pr[p_id] = p;
		if (q_id != 7)
			thread->pr[q_id] = q;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"p = [%d] %d, q = [%d] %d, src1 = [0x%x] %f, "
			"src2 = [0x%x] %f, r = [%d] %d\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, p_id, p, q_id, q, src1_id, src1, src2_id, src2,
			r_id, r);
}

void frm_isa_FFMA_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id, src3_id;
	float dst, src1, src2, src3;
	union {unsigned i; float f;} gpr;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].f;
		if (((fmt.fmod0 >> 5) & 0x1) == 1)
			src1 = -src1;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].f;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
		{
			gpr.i = fmt.src2 << 12;
			src2 = gpr.f;
		}
		if (((fmt.fmod0 >> 2) & 0x1) == 1)
			src2 = fabsf(src2);
		if (((fmt.fmod0 >> 4) & 0x1) == 1)
			src2 = -src2;
		src3_id = fmt.fmod1_srco & 0x3f;
		src3 = thread->gpr[src3_id].f;
		if (((fmt.fmod0 >> 4) & 0x1) == 1)
			src3 = -src3;

		/* Execute */
		dst = src1 * src2 + src3;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] %f src1 = [0x%x] %f "
			"src2 = [0x%x] %f src3 = [0x%x] %f\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2,
			src3_id, src3);
}

void frm_isa_FCMP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id, src3_id;
	float dst, src1, src2, src3;
	union {unsigned i; float f;} gpr;

	// Operations and intermediate values
	unsigned cop;
	unsigned cop_res;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >>
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].f;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].f;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
		{
			gpr.i = fmt.src2 << 12;
			src2 = gpr.f;
		}
		src3_id = fmt.fmod1_srco & 0x3f;
		src3 = thread->gpr[src3_id].f;
		cop = (fmt.fmod1_srco >> 6) & 0xf;

		/* Execute */
		if (cop == 1)
			cop_res = src3 < 0;
		else if (cop == 2)
			cop_res = src3 == 0;
		else if (cop == 3)
			cop_res = src3 <= 0;
		else if (cop == 4)
			cop_res = src3 > 0;
		else if (cop == 5)
			cop_res = src3 != 0;
		else if (cop == 6)
			cop_res = src3 >= 0;
		else
			fatal("%s:%d: compare operation 0x%x not implemented",
					__FILE__, __LINE__, cop);
		dst = cop_res ? src1 : src2;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] %f src1 = [0x%x] %f "
			"src2 = [0x%x] %f src3 = [0x%x] %f\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2,
			src3_id, src3);
}

void frm_isa_FSWZ_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_FADD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id;
	float dst, src1, src2;
	union {unsigned i; float f;} gpr;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].f;
		if (((fmt.fmod0 >> 3) & 0x1) == 1)
			src1 = fabsf(src1);
		if (((fmt.fmod0 >> 5) & 0x1) == 1)
			src1 = -src1;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].f;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
		{
			gpr.i = fmt.src2 << 12;
			src2 = gpr.f;
		}
		if (((fmt.fmod0 >> 2) & 0x1) == 1)
			src2 = fabsf(src2);
		if (((fmt.fmod0 >> 4) & 0x1) == 1)
			src2 = -src2;

		/* Execute */
		dst = src1 + src2;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] %f src1 = [0x%x] %f src2 = [0x%x] %f\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2);
}

void frm_isa_FMUL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id;
	float dst, src1, src2;
	union {unsigned i; float f;} gpr;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].f;
		if (((fmt.fmod0 >> 3) & 0x1) == 1)
			src1 = fabsf(src1);
		if (((fmt.fmod0 >> 5) & 0x1) == 1)
			src1 = -src1;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].f;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
		{
			gpr.i = fmt.src2 << 12;
			src2 = gpr.f;
		}
		if (((fmt.fmod0 >> 2) & 0x1) == 1)
			src2 = fabsf(src2);
		if (((fmt.fmod0 >> 4) & 0x1) == 1)
			src2 = -src2;

		/* Execute */
		dst = src1 * src2;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] %f src1 = [0x%x] %f src2 = [0x%x] %f\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2);
}

void frm_isa_RRO_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_MUFU_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_DMNMX_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_DSET_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_DSETP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_DFMA_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_DADD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_DMUL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_IMAD32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_IADD32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	// Active and predicate
	FrmWarp *warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id;
	int dst, src1, imm;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		if (((fmt.fmod0 >> 4) & 0x3) == 2)
			src1 = -src1;
		imm = fmt.imm32;

		/* Execute */
		dst = src1 + imm;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x imm = 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, imm);
}

void frm_isa_IMUL32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	// Active and predicate
	FrmWarp *warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id;
	int dst, src1, imm;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		if (((fmt.fmod0 >> 5) & 0x1) == 1)
			src1 = -src1;
		imm = fmt.imm32;

		/* Execute */
		dst = src1 * imm;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x imm = 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, imm);
}

void frm_isa_MOV32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	// Active and predicate
	FrmWarp *warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id;
	int dst, imm;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		imm = fmt.imm32;

		/* Execute */
		dst = imm;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x imm = 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, imm);
}

void frm_isa_FFMA32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_FADD32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	// Active and predicate
	FrmWarp *warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id;
	float dst, src1;
	union {unsigned i; float f;} imm;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].f;
		if (((fmt.fmod0 >> 3) & 0x1) == 1)
			src1 = fabsf(src1);
		if (((fmt.fmod0 >> 5) & 0x1) == 1)
			src1 = -src1;
		imm.i = fmt.imm32;
		if (((fmt.fmod0 >> 2) & 0x1) == 1)
			imm.f = fabsf(imm.f);
		if (((fmt.fmod0 >> 4) & 0x1) == 1)
			imm.f = -imm.f;

		/* Execute */
		dst = src1 + imm.f;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] %f src1 = [0x%x] %f imm = %f\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, imm.f);
}

void frm_isa_FMUL32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	// Active and predicate
	FrmWarp *warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id;
	float dst, src1;
	union {unsigned i; float f;} imm;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].f;
		imm.i = fmt.imm32;

		/* Execute */
		dst = src1 * imm.f;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] %f src1 = [0x%x] %f imm = %f\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, imm.f);
}

void frm_isa_LOP32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	// Active and predicate
	FrmWarp *warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id;
	int dst, src1, imm;

	// Operation
	int bop;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		if (((fmt.fmod0 >> 5) & 0x1) == 1)
			src1 = ~src1;
		imm = fmt.imm32;
		bop =(fmt.fmod0 >> 2) & 0x3;

		/* Execute */
		if (bop == 0)
			dst = src1 & imm;
		else if (bop == 1)
			dst = src1 | imm;
		else if (bop == 2)
			dst = src1 ^ imm;
		else
			fatal("%s:%d: unsupported logic operation 0x%x",
					__FILE__, __LINE__, bop);

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x imm = 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, imm);	//
}

void frm_isa_ISCADD32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	// Active and predicate
	FrmWarp *warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id;
	int dst, src1, imm, shamt;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		imm = fmt.imm32;
		shamt = (fmt.fmod0 >> 1) & 0x1f;

		/* Execute */
		dst = (src1 << shamt) + imm;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x imm = 0x%08x "
			"shamt = %d\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, imm, shamt);
}

void frm_isa_IMNMX_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_ISET_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_ISETP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned p_id, p, q_id, q, r_id, r;
	unsigned src1_id, src2_id;
	int src1, src2;

	// Operations and intermediate values
	unsigned cop;
	unsigned bop;
	unsigned cop_res;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Get predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
			src2 = src2_id >> 19 ? src2_id | 0xfff00000 : src2_id;
		r_id = fmt.fmod1_srco & 0x7;
		r = thread->pr[r_id];
		if (((fmt.fmod1_srco >> 3) & 0x1) == 1)
			r = !r;
		cop = (fmt.fmod1_srco >> 6) & 0x7;
		bop = (fmt.fmod1_srco >> 4) & 0x3;

		/* Compare */
		if (cop == 1)
			cop_res = src1 < src2;
		else if (cop == 2)
			cop_res = src1 == src2;
		else if (cop == 3)
			cop_res = src1 <= src2;
		else if (cop == 4)
			cop_res = src1 > src2;
		else if (cop == 5)
			cop_res = src1 != src2;
		else if (cop == 6)
			cop_res = src1 >= src2;
		else
			fatal("%s: compare operation %d not implemented",
					__FUNCTION__, cop);

		/* Logic */
		if (bop == 0)
		{
			p = cop_res && r;
			q = !cop_res && r;
		}
		else if (bop == 1)
		{
			p = cop_res || r;
			q = !cop_res || r;
		}
		else if (cop == 2)
		{
			p = (cop_res && !r) || (!cop_res && r);
			q = (cop_res && r) || (!cop_res && !r);
		}
		else
			fatal("%s: bitwise operation %d not implemented",
					__FUNCTION__, bop);

		/* Write */
		p_id = (fmt.dst >> 3) & 0x7;
		q_id = fmt.dst & 0x7;
		if (p_id != 7)
			thread->pr[p_id] = p;
		if (q_id != 7)
			thread->pr[q_id] = q;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"p = [%d] %d, q = [%d] %d, src1 = [0x%x] 0x%08x, "
			"src2 = [0x%x] 0x%08x, r = [%d] %d\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, p_id, p, q_id, q, src1_id, src1, src2_id, src2,
			r_id, r);
}

void frm_isa_IMAD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id, src3_id;
	int dst, src1, src2, src3;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		if (((fmt.fmod0 >> 4) & 0x3) == 2)
			src1 = -src1;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
			src2 = src2_id >> 19 ? src2_id | 0xfff00000 : src2_id;
		src3_id = fmt.fmod1_srco & 0x3f;
		src3 = thread->gpr[src3_id].s32;
		if (((fmt.fmod0 >> 4) & 0x3) == 1)
			src3 = -src3;

		/* Execute */
		dst = src1 * src2 + src3;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x src2 = [0x%x] 0x%08x "
			"src3 = [0x%x] 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2,
			src3_id, src3);
}

void frm_isa_BFI_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_ICMP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id, src3_id;
	int dst, src1, src2, src3;

	// Operations and intermediate values
	unsigned cop;
	unsigned cop_res;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >>
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
			src2 = src2_id >> 19 ? src2_id | 0xfff00000 : src2_id;
		src3_id = fmt.fmod1_srco & 0x3f;
		src3 = thread->gpr[src3_id].s32;
		cop = (fmt.fmod1_srco >> 6) & 0x7;

		/* Execute */
		if (cop == 1)
			cop_res = src3 < 0;
		else if (cop == 2)
			cop_res = src3 == 0;
		else if (cop == 3)
			cop_res = src3 <= 0;
		else if (cop == 4)
			cop_res = src3 > 0;
		else if (cop == 5)
			cop_res = src3 != 0;
		else if (cop == 6)
			cop_res = src3 >= 0;
		else
			fatal("%s:%d: compare operation 0x%x not implemented",
					__FILE__, __LINE__, cop);
		dst = cop_res ? src1 : src2;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x "
			"src2 = [0x%x] 0x%08x src3 = [0x%x] 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2,
			src3_id, src3);
}

void frm_isa_ISAD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_ISCADD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id;
	int dst, src1, src2, shamt;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	warp = thread->warp;
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		if (((fmt.fmod1_srco >> 6) & 0x3) == 2)
			src1 = -src1;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
			src2 = src2_id >> 19 ? src2_id | 0xfff00000 : src2_id;
		if (((fmt.fmod1_srco >> 6) & 0x3) == 1)
			src2 = -src2;
		shamt = (fmt.fmod0 >> 1) & 0x1f;

		/* Execute */
		dst = (src1 << shamt) + src2;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x src2 = [0x%x] 0x%08x "
			"shamt = %d\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2, shamt);
}

void frm_isa_IADD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id;
	int dst, src1, src2;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
			src2 = src2_id >> 19 ? src2_id | 0xfff00000 : src2_id;
		if (((fmt.fmod0 >> 4) & 0x3) == 1 || ((fmt.fmod0 >> 4) & 0x3) == 2)
			src2 = -src2;

		/* Execute */
		dst = src1 + src2;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x src2 = [0x%x] 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2);
}

void frm_isa_IMUL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id;
	int dst, src1, src2;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
			src2 = src2_id >> 19 ? src2_id | 0xfff00000 : src2_id;

		/* Execute */
		dst = src1 * src2;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x src2 = [0x%x] 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2);
}

void frm_isa_SHR_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id;
	int dst, src1, src2;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
			src2 = src2_id >> 19 ? src2_id | 0xfff00000 : src2_id;

		/* Execute */
		dst = src1 >> src2;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x src2 = [0x%x] 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2);
}

void frm_isa_SHL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id;
	int dst, src1, src2;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
			src2 = src2_id >> 19 ? src2_id | 0xfff00000 : src2_id;

		/* Execute */
		dst = src1 << src2;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x src2 = [0x%x] 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2);
}

void frm_isa_LOP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id;
	int dst, src1, src2;

	// Operation
	unsigned bop;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		if (((fmt.fmod0 >> 5) & 0x1) == 1)
			src1 = ~src1;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
			src2 = src2_id >> 19 ? src2_id | 0xfff00000 : src2_id;
		if (((fmt.fmod0 >> 4) & 0x1) == 1)
			src2 = ~src2;
		bop = (fmt.fmod0 >> 2) & 0x3;

		/* Execute */
		if (bop == 0)
			dst = src1 && src2;
		else if (bop == 1)
			dst = src1 || src2;
		else if (bop == 2)
			dst = (src1 && !src2) || (!src1 && src2);
		else
			fatal("%s: bitwise operation %d not implemented",
					__FUNCTION__, bop);

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x src2 = [0x%x] 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2);
}

void frm_isa_BFE_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_FLO_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_CSET_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_CSETP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_PSET_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_PSETP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_F2F_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src_id;
	float dst;
	union {unsigned i; float f;} src;

	// Operation
	unsigned rnd;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Operation */
		rnd = (((fmt.fmod0 >> 3) & 0x1) << 2) | (fmt.fmod2_srco & 0x3);

		/* Read */
		src_id = fmt.src2;
		if (fmt.s2mod == 0)
			src.f = thread->gpr[src_id].f;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src_id, 4, &src.i);
		else
			src.i = fmt.src2 << 12;
		if (((fmt.fmod0 >> 2) & 0x1) == 1)
			src.f = fabsf(src.f);
		if (((fmt.fmod0 >> 4) & 0x1) == 1)
			src.f = -src.f;

		/* Execute */
		if (rnd == 4)
			dst = roundf(src.f);
		else if (rnd == 5)
			dst = floorf(src.f);
		else if (rnd == 6)
			dst = ceilf(src.f);
		else if (rnd == 7)
			dst = truncf(src.f);
		else
			fatal("%s: rounding mode %d not implemented", __FUNCTION__, rnd);

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] %f src = [0x%x] %f\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src_id, src.f);
}

void frm_isa_F2I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src_id;
	int dst;
	union {unsigned i; float f;} src;

	// Operation
	unsigned rnd;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Operation */
		rnd = (fmt.fmod2_srco) & 0x3;

		/* Read */
		src_id = fmt.src2;
		if (fmt.s2mod == 0)
			src.f = thread->gpr[src_id].f;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src_id, 4, &src.i);
		else
			src.i = fmt.src2 << 12;
		if (((fmt.fmod0 >> 2) & 0x1) == 1)
			src.f = fabsf(src.f);
		if (((fmt.fmod0 >> 4) & 0x1) == 1)
			src.f = -src.f;

		/* Execute */
		if (rnd == 0)
			dst = src.f;
		else if (rnd == 1)
			dst = floorf(src.f);
		else if (rnd == 2)
			dst = ceilf(src.f);
		else
			dst = truncf(src.f);

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src = [0x%x] %f\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src_id, src.f);
}

void frm_isa_I2F_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src_id;
	float dst;
	int src;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = fmt.src2;
		if (fmt.s2mod == 0)
			src = thread->gpr[src_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src_id, 4, &src);
		else
			src = fmt.src2 << 12;
		if (((fmt.fmod0 >> 2) & 0x1) == 1)
			src = abs(src);
		if (((fmt.fmod0 >> 4) & 0x1) == 1)
			src = -src;

		/* Execute */
		dst = src;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] %f src = [0x%x] 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src_id, src);
}

void frm_isa_I2I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src_id;
	int dst, src;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = fmt.src2;
		if (fmt.s2mod == 0)
			src = thread->gpr[src_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src_id, 4, &src);
		else
			src = fmt.src2 << 12;
		if (((fmt.fmod0 >> 2) & 0x1) == 1)
			src = abs(src);
		if (((fmt.fmod0 >> 4) & 0x1) == 1)
			src = -src;

		/* Execute */
		dst = src;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src = [0x%x] 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src_id, src);
}

void frm_isa_SEL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id, src2_id, src3_id;
	int dst, src1, src2, src3;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >>
			thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.fmod1_src1;
		src1 = thread->gpr[src1_id].s32;
		src2_id = fmt.src2;
		if (fmt.s2mod == 0)
			src2 = thread->gpr[src2_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src2_id, 4, &src2);
		else
			src2 = src2_id >> 19 ? src2_id | 0xfff00000 : src2_id;
		src3_id = fmt.fmod2_srco & 0x7;
		src3 = thread->pr[src3_id];
		if (((fmt.fmod2_srco >> 3) & 0x1) == 1)
			src3 = ! src3;

		/* Execute */
		dst = src3 ? src1 : src2;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x "
			"src2 = [0x%x] 0x%08x src3 = [%d] %d\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2,
			src3_id, src3);
}

void frm_isa_PRMT_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_MOV_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src_id;
	int dst, src;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = fmt.src2;
		if (fmt.s2mod == 0)
			src = thread->gpr[src_id].s32;
		else if (fmt.s2mod == 1 || fmt.s2mod == 2)
			mem_read(const_mem, src_id, 4, &src);
		else
			src = src_id >> 19 ? src_id | 0xfff00000 : src_id;

		/* Execute */
		dst = src;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src = [0x%x] 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src_id, src);
}

void frm_isa_S2R_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src_id;
	int dst, src;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = fmt.src2 & 0xff;
		src = thread->sr[src_id].s32;

		/* Execute */
		dst = src;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src = [%d] %d\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src_id, src);
}

void frm_isa_P2R_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_R2P_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_B2R_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_NOP_impl(FrmThread *thread, struct FrmInstWrap *inst)
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

void frm_isa_LEPC_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_VOTE_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_BAR_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	//	FrmThreadBlock *thread_block;
	//	FrmWarp *warp;
	//
	//	int warp_id;
	//
	//	thread_block = thread->thread_block;
	//	warp = thread->warp;
	//
	//	/* Set flag to suspend warp execution */
	//	warp->at_barrier = 1;
	//
	//	if (thread->id_in_warp == warp->thread_count - 1)
	//		thread_block->num_warps_at_barrier++;
	//
	//	/* Continue execution when all warps in the thread block reach the
	//	 * barrier*/
	//	if (thread_block->num_warps_at_barrier == thread_block->warp_count)
	//	{
	//		for (warp_id = 0; warp_id < thread_block->warp_count;
	//				warp_id++)
	//			thread_block->warps[warp_id]->at_barrier = 0;
	//
	//		thread_block->num_warps_at_barrier = 0;
	//	}
	//
	//	/* Debug */
	//	frm_isa_debug("%s:%d: PC = 0x%x thread[%d]\n",
	//			__FUNCTION__, __LINE__, warp->pc, thread->id);
}

void frm_isa_POPC_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_RED_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_ATOM_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_LD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtLdSt fmt = FrmInstWrapGetBytes(inst)->fmt_ldst;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id;
	int dst, src1;
	unsigned addr;

	struct mem_t *global_mem = thread->grid->emu->global_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		addr = src1 + fmt.fmod1_srco;

		/* Execute */
		mem_read(global_mem, addr, 4, &dst);

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x addr = 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, addr);
}

void frm_isa_LDU_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_ST_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtLdSt fmt = FrmInstWrapGetBytes(inst)->fmt_ldst;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned value_id, src1_id;
	int value, src1;
	unsigned addr;

	struct mem_t *global_mem = thread->grid->emu->global_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		addr = src1 + fmt.fmod1_srco;
		value_id = fmt.dst;
		value = thread->gpr[value_id].s32;

		/* Execute */
		mem_write(global_mem, addr, 4, &value);
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"value = [0x%x] 0x%08x src1 = [0x%x] 0x%08x addr = 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, value_id, value, src1_id, src1, addr);
}

void frm_isa_CCTL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDLK_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDS_LDU_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_LD_LDU_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDX_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDS_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	// Format
	FrmFmtLdSt fmt = FrmInstWrapGetBytes(inst)->fmt_ldst;

	// Active and predicate
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	// Operands
	unsigned dst_id, src1_id;
	int dst[4], src1;
	unsigned addr;

	struct mem_t *shared_mem = thread->thread_block->shared_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = (entry.active_thread_mask >> thread->id_in_warp) & 0x1;

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		/* Read */
		src1_id = fmt.src1;
		src1 = thread->gpr[src1_id].s32;
		addr = src1 + (fmt.fmod1_srco & 0xffffff);

		/* Execute */
		mem_read(shared_mem, addr, 4, dst);
		if (((fmt.fmod0 >> 1) & 0x7) == 5)
			mem_read(shared_mem, addr + 4, 4, dst + 1);
		if (((fmt.fmod0 >> 1) & 0x7) == 6)
		{
			mem_read(shared_mem, addr + 4, 4, dst + 1);
			mem_read(shared_mem, addr + 8, 4, dst + 2);
			mem_read(shared_mem, addr + 12, 4, dst + 3);
		}

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst[0];
		if (((fmt.fmod0 >> 1) & 0x7) == 5)
			thread->gpr[dst_id + 1].s32 = dst[1];
		if (((fmt.fmod0 >> 1) & 0x7) == 6)
		{
			thread->gpr[dst_id + 1].s32 = dst[1];
			thread->gpr[dst_id + 2].s32 = dst[2];
			thread->gpr[dst_id + 3].s32 = dst[3];
		}
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst[0] = [0x%x] 0x%08x src1 = [0x%x] 0x%08x addr = 0x%08x\n",
			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst[0], src1_id, src1, addr);
}

void frm_isa_LDSLK_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_STX_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_STL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_STS_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	//	unsigned int pred_id, value_id, addr_id;
	//	unsigned int active, pred, value, addr;
	//
	//	FrmWarp *warp;
	//	FrmWarpSyncStackEntry entry;
	//
	//	/* Active */
	//	warp = thread->warp;
	//	entry = warp->sync_stack.entries[warp->sync_stack_top];
	//	active = (entry.active_thread_mask >>
	//			thread->id_in_warp) & 0x1;
	//
	//	/* Predicate */
	//	pred_id = FrmInstWrapGetBytes(inst)->offs.pred;
	//	if (pred_id <= 7)
	//		pred = thread->pr[pred_id];
	//	else
	//		pred = ! thread->pr[pred_id - 8];
	//
	//	/* Execute */
	//	if (active == 1 && pred == 1)
	//	{
	//		/* Read */
	//		value_id = FrmInstWrapGetBytes(inst)->offs.dst;
	//		value = thread->gpr[value_id].u32;
	//		addr_id = FrmInstWrapGetBytes(inst)->offs.src1;
	//		addr = thread->gpr[addr_id].u32 + FrmInstWrapGetBytes(inst)->offs.offset;
	//
	//		/* Execute */
	//		mem_write(thread->thread_block->shared_mem, addr, 4, &value);
	//	}
	//
	//	/* Debug */
	//	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
	//			"value = [0x%x] 0x%08x addr = [0x%x] 0x%08x\n",
	//			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
	//			pred_id, pred, value_id, value, addr_id, addr);
}

void frm_isa_STSUL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_CCTLL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_SULD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_SURED_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_SUST_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_MEMBAR_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_SUQ_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_STUL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_SULEA_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDC_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	//	unsigned int pred_id, dst_id, src_id;
	//	unsigned int active, pred, dst, addr;
	//
	//	FrmWarp *warp = thread->warp;
	//	FrmGrid *grid = thread->grid;
	//	FrmEmu *emu = grid->emu;
	//	FrmWarpSyncStackEntry entry;
	//
	//	/* Active */
	//	entry = warp->sync_stack.entries[warp->sync_stack_top];
	//	active = (entry.active_thread_mask >>
	//			thread->id_in_warp) & 0x1;
	//
	//	/* Predicate */
	//	pred_id = FrmInstWrapGetBytes(inst)->offs.pred;
	//	if (pred_id <= 7)
	//		pred = thread->pr[pred_id];
	//	else
	//		pred = ! thread->pr[pred_id - 8];
	//
	//	/* Execute */
	//	if (active == 1 && pred == 1)
	//	{
	//		/* Read */
	//		src_id = FrmInstWrapGetBytes(inst)->offs.src1;
	//		addr = thread->gpr[src_id].u32;
	//
	//		/* Execute */
	//		mem_read(emu->const_mem, addr, 4, &dst);
	//
	//		/* Write */
	//		dst_id = FrmInstWrapGetBytes(inst)->offs.dst;
	//		thread->gpr[dst_id].u32 = dst;
	//	}
	//
	//	/* Debug */
	//	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
	//			"dst = [0x%x] 0x%08x src = [0x%x] 0x%08x\n",
	//			__FUNCTION__, __LINE__, warp->pc, thread->id, active,
	//			pred_id, pred, dst_id, dst, src_id, addr);
}

void frm_isa_TEX_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_TLD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_TLD4_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_TXQ_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_JMP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_JMX_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_JCAL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_BRA_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	//	unsigned int pred_id;
	//	unsigned int active, pred;
	//	int target;
	//
	//	FrmWarp *warp;
	//	FrmWarpSyncStackEntry *top_entry;
	//	FrmWarpSyncStackEntry *new_entry;
	//
	//	warp = thread->warp;
	//	top_entry = &(warp->sync_stack.entries[warp->sync_stack_top]);
	//	new_entry = &(warp->new_entry);
	//
	//	/* Get active bit */
	//	active = (top_entry->active_thread_mask >> thread->id_in_warp) & 0x1;
	//
	//	/* Get predicate */
	//	pred_id = FrmInstWrapGetBytes(inst)->tgt.pred;
	//	if (pred_id <= 7)
	//		pred = thread->pr[pred_id];
	//	else
	//		pred = ! thread->pr[pred_id - 8];
	//
	//	/* Get target */
	//	target = FrmInstWrapGetBytes(inst)->tgt.target;
	//	if ((target >> 19 & 0x1) == 1)
	//		target |= 0xfff00000;
	//
	//	/* Update active thread mask for new entry */
	//	if (FrmInstWrapGetBytes(inst)->tgt.u != 1)
	//	{
	//		if (active == 1 && pred == 1)
	//		{
	//			new_entry->active_thread_mask |=
	//					1 << thread->id_in_warp;
	//			warp->taken |= 1 << thread->id_in_warp;
	//		}
	//		else
	//		{
	//			new_entry->active_thread_mask |=
	//					0 << thread->id_in_warp;
	//			warp->taken |= 0 << thread->id_in_warp;
	//		}
	//	}
	//
	//	/* If divergent, push sync stack and go to taken path */
	//	if (thread->id_in_warp == warp->thread_count - 1)
	//	{
	//		if (FrmInstWrapGetBytes(inst)->tgt.u == 1 ||
	//				top_entry->active_thread_mask == warp->taken ||
	//				warp->taken == 0)
	//			warp->divergent = 0;
	//		else
	//			warp->divergent = 1;
	//
	//		if (warp->divergent)
	//		{
	//			new_entry->next_path_pc = warp->pc + 8;
	//			warp->sync_stack.entries[warp->sync_stack_top + 1].reconv_pc =
	//					new_entry->reconv_pc;
	//			warp->sync_stack.entries[warp->sync_stack_top + 1].next_path_pc =
	//					new_entry->next_path_pc;
	//			warp->sync_stack.entries[warp->sync_stack_top + 1].active_thread_mask =
	//					new_entry->active_thread_mask;
	//			warp->sync_stack_top++;
	//			new_entry->reconv_pc = 0;
	//			new_entry->next_path_pc = 0;
	//			new_entry->active_thread_mask = 0;
	//			if (warp->taken != 0)
	//				warp->pc += target;
	//		}
	//		else
	//		{
	//			if (FrmInstWrapGetBytes(inst)->tgt.u == 0 && warp->taken != 0)
	//				warp->pc += target;
	//		}
	//		warp->taken = 0;
	//	}
	//
	//	/* Debug */
	//	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
	//			"target = 0x%x active_thread_mask = 0x%08x\n",
	//			__FUNCTION__, __LINE__, warp->pc, thread->id,
	//			active, pred_id, pred, target,
	//			new_entry->active_thread_mask);
}

void frm_isa_BRX_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_CAL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_PLONGJMP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_SSY_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	//	int target;
	//	FrmWarp *warp = thread->warp;
	//
	//	/* Get target. Target is the recovergence point */
	//	target = FrmInstWrapGetBytes(inst)->tgt.target;
	//	if ((target >> 19 & 0x1) == 1)
	//		target |= 0xfff00000;
	//
	//	/* Set reconvergence PC */
	//	warp->new_entry.reconv_pc = target;
	//
	//	/* Debug */
	//	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] target = 0x%x\n",
	//			__FUNCTION__, __LINE__, warp->pc, thread->id, target);
}

void frm_isa_PBK_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_PCNT_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_PRET_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_EXIT_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	//	FrmWarp *warp;
	//
	//	warp = thread->warp;
	//
	//	/* Execute */
	//	if (thread->id_in_warp == warp->thread_count - 1)
	//		warp->finished = 1;
	//
	//	/* Debug */
	//	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active_thread_mask = 0x%08x\n",
	//			__FUNCTION__, __LINE__, warp->pc, thread->id,
	//			warp->sync_stack.entries[warp->sync_stack_top].
	//			active_thread_mask);
}

void frm_isa_LONGJMP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_RET_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_BRK_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_CONT_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_BPT_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}
