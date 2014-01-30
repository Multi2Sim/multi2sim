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

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <mem-system/memory.h>

#include "emu.h"
#include "grid.h"
#include "isa.h"
#include "machine.h"
#include "thread.h"
#include "thread-block.h"
#include "warp.h"


char *frm_err_isa_note =
		"\tThe NVIDIA Fermi SASS instruction set is partially supported by \n"
		"\tMulti2Sim. If your program is using an unimplemented instruction, \n"
		"\tplease email 'development@multi2sim.org' to request support for \n"
		"\tit.\n";

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
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned p_id, p, q_id, q, r_id, r;
	unsigned src1_id, src2_id;
	float src1, src2;
	union {unsigned i; float f;} gpr;

	/* Operations and intermediate values */
	unsigned cop;
	unsigned bop;
	unsigned cop_res;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
					__func__, cop);

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
					__func__, bop);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, p_id, p, q_id, q, src1_id, src1, src2_id, src2,
			r_id, r);
}

void frm_isa_FFMA_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id, src3_id;
	float dst, src1, src2, src3;
	union {unsigned i; float f;} gpr;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2,
			src3_id, src3);
}

void frm_isa_FCMP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id, src3_id;
	float dst, src1, src2, src3;
	union {unsigned i; float f;} gpr;

	/* Operations and intermediate values */
	unsigned cop;
	unsigned cop_res;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
					__func__, __LINE__, cop);
		dst = cop_res ? src1 : src2;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] %f src1 = [0x%x] %f "
			"src2 = [0x%x] %f src3 = [0x%x] %f\n",
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2,
			src3_id, src3);
}

void frm_isa_FSWZ_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_FADD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id;
	float dst, src1, src2;
	union {unsigned i; float f;} gpr;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2);
}

void frm_isa_FMUL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id;
	float dst, src1, src2;
	union {unsigned i; float f;} gpr;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
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
	/* Format */
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id;
	int dst, src1, imm;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, imm);
}

void frm_isa_IMUL32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id;
	int dst, src1, imm;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, imm);
}

void frm_isa_MOV32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id;
	int dst, imm;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, imm);
}

void frm_isa_FFMA32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_FADD32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id;
	float dst, src1;
	union {unsigned i; float f;} imm;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, imm.f);
}

void frm_isa_FMUL32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id;
	float dst, src1;
	union {unsigned i; float f;} imm;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, imm.f);
}

void frm_isa_LOP32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id;
	int dst, src1, imm;

	/* Operation */
	int bop;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
					__func__, __LINE__, bop);

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x imm = 0x%08x\n",
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, imm);
}

void frm_isa_ISCADD32I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtImm fmt = FrmInstWrapGetBytes(inst)->fmt_imm;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id;
	int dst, src1, imm, shamt;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
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
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned p_id, p, q_id, q, r_id, r;
	unsigned src1_id, src2_id;
	int src1, src2;

	/* Operations and intermediate values */
	unsigned cop;
	unsigned bop;
	unsigned cop_res;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
					__func__, cop);

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
					__func__, bop);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, p_id, p, q_id, q, src1_id, src1, src2_id, src2,
			r_id, r);
}

void frm_isa_IMAD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id, src3_id;
	int dst, src1, src2, src3;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
		{
			struct mem_t *const_mem = thread->grid->emu->const_mem;
			unsigned bank = ((fmt.src2 & 0x1) << 4) | (fmt.src2 >> 16);
			unsigned offset = fmt.src2 & 0xfffe;
			mem_read(const_mem, (bank << 16) + offset, 4, &src2);
		}
		else
			src2 = src2_id >> 19 ? src2_id | 0xfff00000 : src2_id;
		src3_id = fmt.fmod1_srco & 0x3f;
		src3 = thread->gpr[src3_id].s32;
		if (((fmt.fmod0 >> 4) & 0x3) == 1)
			src3 = -src3;

		/* Execute */
		unsigned hi = (fmt.fmod0 >> 2) & 0x1;
		long long temp = (long long)src1 * src2;
		if (hi)
			dst = (int)(temp >> 32) + src3;
		else
			dst = (int)temp + src3;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x src2 = [0x%x] 0x%08x "
			"src3 = [0x%x] 0x%08x\n",
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2,
			src3_id, src3);
}

void frm_isa_BFI_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_ICMP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id, src3_id;
	int dst, src1, src2, src3;

	/* Operations and intermediate values */
	unsigned cop;
	unsigned cop_res;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
					__func__, __LINE__, cop);
		dst = cop_res ? src1 : src2;

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x "
			"src2 = [0x%x] 0x%08x src3 = [0x%x] 0x%08x\n",
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2,
			src3_id, src3);
}

void frm_isa_ISAD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_ISCADD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id;
	int dst, src1, src2, shamt;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2, shamt);
}

void frm_isa_IADD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id;
	int dst, src1, src2;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2);
}

void frm_isa_IMUL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id;
	int dst, src1, src2;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2);
}

void frm_isa_SHR_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id;
	int dst, src1, src2;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2);
}

void frm_isa_SHL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id;
	int dst, src1, src2;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2);
}

void frm_isa_LOP_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtReg fmt = FrmInstWrapGetBytes(inst)->fmt_reg;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id;
	int dst, src1, src2;

	/* Operation */
	unsigned bop;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			dst = src1 & src2;
		else if (bop == 1)
			dst = src1 | src2;
		else if (bop == 2)
			dst = (src1 & ~ src2) | (~ src1 & src2);
		else
			fatal("%s: bitwise operation %d not implemented",
					__func__, bop);

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x src2 = [0x%x] 0x%08x\n",
			__func__, __LINE__, warp->pc, thread->id, active,
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
	/* Format */
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src_id;
	float dst;
	union {unsigned i; float f;} src;

	/* Operation */
	unsigned rnd;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			fatal("%s: rounding mode %d not implemented", __func__, rnd);

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].f = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] %f src = [0x%x] %f\n",
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src_id, src.f);
}

void frm_isa_F2I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src_id;
	int dst;
	union {unsigned i; float f;} src;

	/* Operation */
	unsigned rnd;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src_id, src.f);
}

void frm_isa_I2F_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src_id;
	float dst;
	int src;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src_id, src);
}

void frm_isa_I2I_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src_id;
	int dst, src, tmp;
	unsigned dst_fmt, src_fmt;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
		dst_fmt = ((fmt.fmod0 >> 1) & 0x4) | (fmt.fmod1_src1 & 0x3);
		src_fmt = ((fmt.fmod0 >> 3) & 0x4) | ((fmt.fmod1_src1 >> 3) & 0x3);

		/* Execute */
		assert(dst_fmt != 3 && dst_fmt != 7 && src_fmt != 3 && src_fmt != 7);
		if (src_fmt == 0 || src_fmt == 4)
			tmp = src & 0xff;
		else if (src_fmt == 1 || src_fmt == 5)
			tmp = src & 0xffff;
		else
			tmp = src;
		if (dst_fmt <= 3)
		{
			if (src_fmt <= 3)
			{
				if (dst_fmt < src_fmt)
				{
					/* Chop */
					if (dst_fmt == 0)
						dst = tmp & 0xff;
					else if (dst_fmt == 1)
						dst = tmp & 0xffff;
					else
						dst = tmp & 0xffffffff;
				}
				else
				{
					/* Zero extension */
					dst = tmp;
				}
			}
			else
			{
				if (dst_fmt + 4 < src_fmt)
				{
					/* Chop */
					if (dst_fmt == 0)
						dst = tmp & 0xff;
					else if (dst_fmt == 1)
						dst = tmp & 0xffff;
					else
						dst = tmp & 0xffffffff;
				}
				else
				{
					/* Sign extension */
					if (src_fmt == 4)
					{
						if (dst_fmt == 1)
							dst = tmp >> 7 ? tmp | 0xff00 : tmp;
						else if (dst_fmt == 2)
							dst = tmp >> 7 ? tmp | 0xffffff00 : tmp;
					}
					else if (src_fmt == 5)
						dst = tmp >> 15 ? tmp | 0xffff0000 : tmp;
				}
			}
		}
		else
		{
			if (src_fmt <= 3)
			{
				if (dst_fmt < src_fmt + 4)
				{
					/* Chop */
					if (dst_fmt == 4)
						dst = tmp & 0xff;
					else if (dst_fmt == 5)
						dst = tmp & 0xffff;
					else
						dst = tmp & 0xffffffff;
				}
				else
				{
					/* Zero extension */
					dst = tmp;
				}
			}
			else
			{
				if (dst_fmt < src_fmt)
				{
					/* Chop */
					if (dst_fmt == 4)
						dst = tmp & 0xff;
					else if (dst_fmt == 5)
						dst = tmp & 0xffff;
					else
						dst = tmp & 0xffffffff;
				}
				else
				{
					/* Sign extension */
					if (src_fmt == 4)
					{
						if (dst_fmt == 5)
							dst = tmp >> 7 ? tmp | 0xff00 : tmp;
						else if (dst_fmt == 6)
							dst = tmp >> 7 ? tmp | 0xffffff00 : tmp;
					}
					else if (src_fmt == 5)
						dst = tmp >> 15 ? tmp | 0xffff0000 : tmp;
				}
			}
		}

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src = [0x%x] 0x%08x dst_fmt = %d "
			"src_fmt = %d\n", __func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src_id, src, dst_fmt, src_fmt);
}

void frm_isa_SEL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id, src2_id, src3_id;
	int dst, src1, src2, src3;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, src2_id, src2,
			src3_id, src3);
}

void frm_isa_PRMT_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_MOV_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src_id;
	int dst, src;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src_id, src);
}

void frm_isa_S2R_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src_id;
	int dst, src;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
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
	/* Format */
	FrmFmtOther fmt = FrmInstWrapGetBytes(inst)->fmt_other;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
		warp->at_barrier = 1;

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d\n",
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred);
}

void frm_isa_POPC_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_RED_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtLdSt fmt = FrmInstWrapGetBytes(inst)->fmt_ldst;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned op;
	int dst_id, src_id;
	union {int i; float f;} dst, src;
	struct mem_t *global_mem = thread->grid->emu->global_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
		dst_id = thread->gpr[fmt.src1].s32;
		dst_id += (int)fmt.fmod1_srco;
		mem_read(global_mem, dst_id, 4, &(dst.i));
		src_id = fmt.dst;
		src.i = thread->gpr[src_id].s32;

		/* Execute */
		/* TODO: check ATOMICSIZE */
		op = (fmt.fmod0 >> 1) & 0xf;
		if (op == 0)
			dst.i += src.i;
		else if (op == 1)
			dst.i = dst.i < src.i ? dst.i : src.i;
		else if (op == 2)
			dst.i = dst.i < src.i ? src.i : dst.i;
		else if (op == 3)
			dst.i = dst.i >= src.i ? 0 : dst.i + 1;
		else if (op == 4)
			dst.i = ((dst.i == 0) | (dst.i > src.i)) ? src.i : dst.i - 1;
		else if (op == 5)
			dst.i &= src.i;
		else if (op == 6)
			dst.i |= src.i;
		else if (op == 7)
			dst.i ^= src.i;

		/* Write */
		mem_write(global_mem, dst_id, 4, &(dst.i));
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src = [0x%x] 0x%08x\n",
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst.i, src_id, src.i);
}

void frm_isa_ATOM_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtLdSt fmt = FrmInstWrapGetBytes(inst)->fmt_ldst;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned op;
	int dst_id, src1_id, src2_id;
	union {int i; float f;} dst, src1, src2;
	struct mem_t *global_mem = thread->grid->emu->global_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
		dst_id = thread->gpr[fmt.src1].s32;
		dst_id += (int)fmt.fmod1_srco;
		mem_read(global_mem, dst_id, 4, &(dst.i));
		src1_id = fmt.dst;
		src1.i = thread->gpr[src1_id].s32;
		src2_id = (fmt.fmod1_srco >> 23) & 0x3f;
		src2.i = thread->gpr[src2_id].s32;

		/* Execute */
		/* TODO: check ATOMICSIZE */
		op = (fmt.fmod0 >> 1) & 0xf;
		if (op == 0)
			dst.i += src1.i;
		else if (op == 1)
			dst.i = dst.i < src1.i ? dst.i : src1.i;
		else if (op == 2)
			dst.i = dst.i < src1.i ? src1.i : dst.i;
		else if (op == 3)
			dst.i = dst.i >= src1.i ? 0 : dst.i + 1;
		else if (op == 4)
			dst.i = ((dst.i == 0) | (dst.i > src1.i)) ? src1.i : dst.i - 1;
		else if (op == 5)
			dst.i &= src1.i;
		else if (op == 6)
			dst.i |= src1.i;
		else if (op == 7)
			dst.i ^= src1.i;
		else if (op == 8)
			dst.i = src1.i;
		else if (op == 9)
			dst.i = dst.i == src1.i ? src2.i : dst.i;

		/* Write */
		mem_write(global_mem, dst_id, 4, &(dst.i));
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x src2 = [0x%x] 0x%08x\n",
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst.i, src1_id, src1.i, src2_id, src2.i);
}

void frm_isa_LD_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtLdSt fmt = FrmInstWrapGetBytes(inst)->fmt_ldst;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id;
	int dst[3];
	unsigned addr;

	struct mem_t *global_mem = thread->grid->emu->global_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
		addr = thread->gpr[fmt.src1].s32 + fmt.fmod1_srco;
		unsigned read_64 = (((fmt.fmod0 >> 1) & 0x7) == 5) ||
				((((fmt.fmod0 >> 1) & 0x7) == 7) &&
				(((fmt.fmod0 >> 5) & 0x1) == 0));

		/* Execute */
		mem_read(global_mem, addr, 4, dst);
		if (read_64)
			mem_read(global_mem, addr + 4, 4, dst + 1);

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst[0];
		if (read_64)
			thread->gpr[dst_id + 1].s32 = dst[1];
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst[0] = [0x%x] 0x%08x addr = 0x%08x\n",
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst[0], addr);
}

void frm_isa_LDU_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_ST_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtLdSt fmt = FrmInstWrapGetBytes(inst)->fmt_ldst;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned value_id, src1_id;
	int value, src1;
	unsigned addr;

	struct mem_t *global_mem = thread->grid->emu->global_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
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
	/* Format */
	FrmFmtLdSt fmt = FrmInstWrapGetBytes(inst)->fmt_ldst;

	if (((fmt.fmod1_srco >> 30) & 0x1) == 0)
		frm_isa_LDL_impl(thread, inst);
	else
		frm_isa_LDS_impl(thread, inst);
}

void frm_isa_LDL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDS_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtLdSt fmt = FrmInstWrapGetBytes(inst)->fmt_ldst;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id;
	int dst[4], src1;
	unsigned addr;

	struct mem_t *shared_mem = thread->thread_block->shared_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst[0], src1_id, src1, addr);
}

void frm_isa_LDSLK_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_STX_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtLdSt fmt = FrmInstWrapGetBytes(inst)->fmt_ldst;

	if (((fmt.fmod1_srco >> 30) & 0x1) == 0)
		frm_isa_STL_impl(thread, inst);
	else
		frm_isa_STS_impl(thread, inst);
}

void frm_isa_STL_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	__NOT_IMPL__
}

void frm_isa_STS_impl(FrmThread *thread, struct FrmInstWrap *inst)
{
	/* Format */
	FrmFmtLdSt fmt = FrmInstWrapGetBytes(inst)->fmt_ldst;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned value_id, src1_id;
	int value[4], src1;
	unsigned addr;

	struct mem_t *shared_mem = thread->thread_block->shared_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
		value_id = fmt.dst;
		value[0] = thread->gpr[value_id].s32;
		if (((fmt.fmod0 >> 1) & 0x7) == 5)
			value[1] = thread->gpr[value_id + 1].s32;
		if (((fmt.fmod0 >> 1) & 0x7) == 6)
		{
			value[1] = thread->gpr[value_id + 1].s32;
			value[2] = thread->gpr[value_id + 2].s32;
			value[3] = thread->gpr[value_id + 3].s32;
		}

		/* Execute */
		mem_write(shared_mem, addr, 4, value);
		if (((fmt.fmod0 >> 1) & 0x7) == 5)
			mem_write(shared_mem, addr + 4, 4, value + 1);
		if (((fmt.fmod0 >> 1) & 0x7) == 6)
		{
			mem_write(shared_mem, addr + 4, 4, value + 1);
			mem_write(shared_mem, addr + 8, 4, value + 2);
			mem_write(shared_mem, addr + 12, 4, value + 3);
		}
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"value[0] = [0x%x] 0x%08x src1 = [0x%x] 0x%08x addr = 0x%08x\n",
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, value_id, value[0], src1_id, src1, addr);
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
	/* Format */
	FrmFmtLdSt fmt = FrmInstWrapGetBytes(inst)->fmt_ldst;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Operands */
	unsigned dst_id, src1_id;
	int dst, src1;
	unsigned addr;

	struct mem_t *const_mem = thread->grid->emu->const_mem;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

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
		addr = src1 + (fmt.fmod1_srco & 0xffff);

		/* Execute */
		mem_read(const_mem, addr, 4, &dst);

		/* Write */
		dst_id = fmt.dst;
		thread->gpr[dst_id].s32 = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"dst = [0x%x] 0x%08x src1 = [0x%x] 0x%08x addr = 0x%08x\n",
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred, dst_id, dst, src1_id, src1, addr);
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
	/* Format */
	FrmFmtCtrl fmt = FrmInstWrapGetBytes(inst)->fmt_ctrl;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	int target_pc;
	int taken_thread_count;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Allocate bit map */
	if (thread->id_in_warp == 0)
		warp->taken_threads = bit_map_create(warp->thread_count);

	/* Save branch outcome for each thread */
	if (active == 1 && pred == 1)
		bit_map_set(warp->taken_threads, thread->id_in_warp, 1, 1);
	else
		bit_map_set(warp->taken_threads, thread->id_in_warp, 1, 0);

	/* Update warp state when the last thread is executed. */
	if (thread->id_in_warp == warp->thread_count - 1)
	{
		/* Determine if branch is divergent */
		taken_thread_count = bit_map_count_ones(warp->taken_threads, 0,
				warp->thread_count);
		if (((fmt.mmod >> 1) & 0x1) == 1 || taken_thread_count == 0 ||
				taken_thread_count == warp->thread_count)
			warp->divergent = 0;
		else
			warp->divergent = 1;

		/* Pre-compute the target PC */
		target_pc = fmt.imm32 & 0xffffff;
		if ((target_pc >> 23 & 0x1) == 1)
			target_pc |= 0xfff00000;

		/* Update warp state */
		if (warp->divergent)
		{
			/* Divergent branch, push sync stack and jump to taken path */
			warp->sync_stack_top++;
			warp->sync_stack.entries[warp->sync_stack_top].reconv_pc = 0;
			warp->sync_stack.entries[warp->sync_stack_top].next_path_pc =
					warp->pc + warp->inst_size;
			warp->sync_stack.entries[warp->sync_stack_top].active_thread_mask =
					warp->taken_threads;

			warp->target_pc = warp->pc + warp->inst_size + target_pc;
		}
		else
		{
			/* Uniform branch */
			if (bit_map_count_ones(warp->taken_threads, 0,
					warp->thread_count) != warp->thread_count)
			{
				/* Not taken */
				warp->target_pc = warp->pc + warp->inst_size;
			}
			else
			{
				/* Taken */
				warp->target_pc = warp->pc + warp->inst_size + target_pc;
			}
		}

		/* Reset */
		bit_map_free(warp->taken_threads);
		warp->taken_threads = NULL;
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d "
			"warp->target_pc = 0x%x\n",
			__func__, __LINE__, warp->pc, thread->id,
			active, pred_id, pred, warp->target_pc);
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
	//			__func__, __LINE__, warp->pc, thread->id, target);
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
	/* Format */
	FrmFmtCtrl fmt = FrmInstWrapGetBytes(inst)->fmt_ctrl;

	/* Active and predicate */
	FrmWarp *warp = thread->warp;
	FrmWarpSyncStackEntry entry;
	unsigned active;
	unsigned pred_id, pred;

	/* Active */
	entry = warp->sync_stack.entries[warp->sync_stack_top];
	active = bit_map_get(entry.active_thread_mask, thread->id_in_warp, 1);

	/* Predicate */
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->pr[pred_id];
	else
		pred = ! thread->pr[pred_id - 8];

	/* Execute */
	if (active == 1 && pred == 1)
	{
		warp->finished_thread_count++;
		if (warp->finished_thread_count == warp->thread_count)
		{
			warp->finished = 1;
			warp->finished_thread_count = 0;
		}
	}

	/* Debug */
	frm_isa_debug("%s:%d: PC = 0x%x thread[%d] active = %d pred = [%d] %d\n",
			__func__, __LINE__, warp->pc, thread->id, active,
			pred_id, pred);
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
