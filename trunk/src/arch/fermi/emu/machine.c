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


#include <lib/util/debug.h>
#include <mem-system/memory.h>

#include "emu.h"
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



void frm_isa_FFMA_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src1_id, src2_id, src3_id;
	float dst, src1, src2, src3;

	dst_id = inst->dword.general0.dst;
	src1_id = inst->dword.general0.src1;
	src2_id = inst->dword.general0.src2;
	src3_id = inst->dword.general0_mod1_B.src3;
	src1 = thread->gpr[src1_id].v.f;
	if (inst->dword.general0.src2_mod == 0)
		src2 = thread->gpr[src2_id].v.f;
	else if (inst->dword.general0.src2_mod == 1)
		mem_read(frm_emu->const_mem, src2_id, 4, &src2);
	src3 = thread->gpr[src3_id].v.f;

	dst = src1 * src2 + src3;

        thread->gpr[dst_id].v.f = dst;
}

void frm_isa_FADD_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src1_id, src2_id;
	float dst, src1, src2;

	dst_id = inst->dword.general0.dst;
	src1_id = inst->dword.general0.src1;
	src2_id = inst->dword.general0.src2;
	src1 = thread->gpr[src1_id].v.f;
	if (inst->dword.general0.src2_mod == 0)
		src2 = thread->gpr[src2_id].v.f;
	else if (inst->dword.general0.src2_mod == 1)
		mem_read(frm_emu->const_mem, src2_id, 4, &src2);

	dst = src1 + src2;

        thread->gpr[dst_id].v.f = dst;
}

void frm_isa_FADD32I_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src1_id;
	float dst, src1, imm32;

	dst_id = inst->dword.imm.dst;
	src1_id = inst->dword.imm.src1;
	src1 = thread->gpr[src1_id].v.f;
	imm32 = inst->dword.imm.imm32;

	dst = src1 + imm32;

        thread->gpr[dst_id].v.f = dst;
}

void frm_isa_FCMP_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_FMUL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src1_id, src2_id;
	float dst, src1, src2;

	dst_id = inst->dword.general0.dst;
	src1_id = inst->dword.general0.src1;
	src2_id = inst->dword.general0.src2;
	src1 = thread->gpr[src1_id].v.f;
	if (inst->dword.general0.src2_mod == 0)
		src2 = thread->gpr[src2_id].v.f;
	else if (inst->dword.general0.src2_mod == 1)
		mem_read(frm_emu->const_mem, src2_id, 4, &src2);

	dst = src1 * src2;

        thread->gpr[dst_id].v.f = dst;
}

void frm_isa_FMUL32I_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src1_id;
	float dst, src1, imm32;

	dst_id = inst->dword.imm.dst;
	src1_id = inst->dword.imm.src1;
	src1 = thread->gpr[src1_id].v.f;
	imm32 = inst->dword.imm.imm32;

	dst = src1 * imm32;

        thread->gpr[dst_id].v.f = dst;
}

void frm_isa_FMNMX_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_FSWZ_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_FSET_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_FSETP_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)  
{
	unsigned int p_id, q_id, src1_id, src2_id, r_id;
	unsigned int p, q, r;
	float src1, src2;

	p_id = inst->dword.general1.dst >> 3;
	q_id = inst->dword.general1.dst & 0x7;
	src1_id = inst->dword.general1.src1;
	src2_id = inst->dword.general1.src2;
	r_id = inst->dword.general1.R;
	src1 = thread->gpr[src1_id].v.f;
	if (inst->dword.general1.src2_mod == 0)
		src2 = thread->gpr[src2_id].v.f;
	else if (inst->dword.general1.src2_mod == 1)
		mem_read(frm_emu->const_mem, src2_id, 4, &src2);
	r = thread->pr[r_id];

	p = (src1 >= src2) && r;
	q = !p;

        thread->pr[p_id] = p;
        thread->pr[q_id] = q;
}

void frm_isa_RRO_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_MUFU_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_DFMA_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_DADD_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_DMUL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_DMNMX_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_DSET_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_DSETP_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_IMAD_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int pred_id, dst_id, src1_id, src2_id, src3_id;
	unsigned int pred, dst, src1, src2, src3;

	/* Predication */
	pred_id = inst->dword.general1.pred;
	pred = thread->pr[pred_id];

	if (pred == 1)
	{
		/* Read */
		src1_id = inst->dword.general0.src1;
		src1 = thread->gpr[src1_id].v.i;
		src2_id = inst->dword.general0.src2;
		if (inst->dword.general0.src2_mod == 0)
			src2 = thread->gpr[src2_id].v.i;
		else if (inst->dword.general0.src2_mod == 1)
			mem_read(frm_emu->const_mem, src2_id, 4, &src2);
		src3_id = inst->dword.general0_mod1_B.src3;
		src3 = thread->gpr[src3_id].v.i;

		/* Execute */
		dst = src1 * src2 + src3;

		/* Write */
		dst_id = inst->dword.general0.dst;
		thread->gpr[dst_id].v.i = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: thread[%d] pred = [%x] %x dst = [0x%x] 0x%08x "
			"src1 = [0x%x] 0x%08x src2 = [0x%x] 0x%08x "
			"src3 = [0x%x] 0x%08x\n", __FUNCTION__, __LINE__,
			thread->id, pred_id, pred, dst_id, dst, src1_id, src1,
			src2_id, src2, src3_id, src3);
}

void frm_isa_IMUL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src1_id, src2_id;
	unsigned int dst, src1, src2;

	dst_id = inst->dword.general0.dst;
	src1_id = inst->dword.general0.src1;
	src2_id = inst->dword.general0.src2;
	src1 = thread->gpr[src1_id].v.f;
	if (inst->dword.general0.src2_mod == 0)
		src2 = thread->gpr[src2_id].v.f;
	else if (inst->dword.general0.src2_mod == 1)
		mem_read(frm_emu->const_mem, src2_id, 4, &src2);

	dst = src1 * src2;

	thread->gpr[dst_id].v.f = dst;
}

void frm_isa_IADD_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src1_id, src2_id;
	unsigned int dst, src1, src2;

	dst_id = inst->dword.general0.dst;
	src1_id = inst->dword.general0.src1;
	src2_id = inst->dword.general0.src2;
	src1 = thread->gpr[src1_id].v.f;
	if (inst->dword.general0.src2_mod == 0)
		src2 = thread->gpr[src2_id].v.f;
	else if (inst->dword.general0.src2_mod == 1)
		mem_read(frm_emu->const_mem, src2_id, 4, &src2);

	dst = src1 + src2;

	thread->gpr[dst_id].v.f = dst;
}

void frm_isa_IADD32I_impl(struct frm_thread_t *thread, struct frm_inst_t *inst) 
{
	unsigned int dst_id, src1_id;
	unsigned int dst, src1, imm32;

	dst_id = inst->dword.imm.dst;
	src1_id = inst->dword.imm.src1;
	src1 = thread->gpr[src1_id].v.f;
	imm32 = inst->dword.imm.imm32;

	dst = src1 + imm32;

	thread->gpr[dst_id].v.f = dst;
}

void frm_isa_ISCADD_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src1_id, src2_id;
	unsigned int dst, src1, src2, shamt;

	dst_id = inst->dword.general0.dst;
	src1_id = inst->dword.general0.src1;
	src2_id = inst->dword.general0.src2;
	src1 = thread->gpr[src1_id].v.i;
	if (inst->dword.general0.src2_mod == 0)
		src2 = thread->gpr[src2_id].v.i;
	else if (inst->dword.general0.src2_mod == 1)
		mem_read(frm_emu->const_mem, src2_id, 4, &src2);
	shamt = inst->dword.mod0_C.shamt;

	dst = (src1 << shamt) + src2;

	thread->gpr[dst_id].v.i = dst;
}

void frm_isa_ISAD_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_IMNMX_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_BFE_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_BFI_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_SHR_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src1_id, src2_id;
	int dst, src1, src2;

	dst_id = inst->dword.general0.dst;
	src1_id = inst->dword.general0.src1;
	src2_id = inst->dword.general0.src2;
	src1 = thread->gpr[src1_id].v.f;
	src2 = thread->gpr[src2_id].v.f;

	dst = src1 >> src2;

	thread->gpr[dst_id].v.f = dst;
}

void frm_isa_SHL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src1_id, src2_id;
	int dst, src1, src2;

	dst_id = inst->dword.general0.dst;
	src1_id = inst->dword.general0.src1;
	src2_id = inst->dword.general0.src2;
	src1 = thread->gpr[src1_id].v.f;
	src2 = thread->gpr[src2_id].v.f;

	dst = src1 << src2;

	thread->gpr[dst_id].v.f = dst;
}

void frm_isa_LOP_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_LOP32I_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_FLO_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_ISET_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_ISETP_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int pred_id, p_id, q_id, src1_id, src2_id, r_id;
	unsigned int pred, p, q, src1, src2, r;

	int compare_op;
	int logic_op;
	int compare_result;

	/* Predication */
	pred_id = inst->dword.general1.pred;
	pred = thread->pr[pred_id];

	if (pred == 1)
	{
		/* Read */
		src1_id = inst->dword.general1.src1;
		src1 = thread->gpr[src1_id].v.i;
		src2_id = inst->dword.general1.src2;
		if (inst->dword.general1.src2_mod == 0)
			src2 = thread->gpr[src2_id].v.i;
		else if (inst->dword.general1.src2_mod == 1)
			mem_read(frm_emu->const_mem, src2_id, 4, &src2);
		r_id = inst->dword.general1.R;
		r = thread->pr[r_id];

		/* Compare */
		compare_op = inst->dword.general1.cmp;
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
		logic_op = inst->dword.general1.logic;
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
		p_id = inst->dword.general1.dst >> 3;
		q_id = inst->dword.general1.dst & 0x7;
		thread->pr[p_id] = p;
		thread->pr[q_id] = q;
	}

	/* Debug */
	frm_isa_debug("%s:%d: thread[%d] pred = [%d] %d p = [%d] %d, "
			"q = [%d] %d, src1 = [0x%x] 0x%08x, "
			"src2 = [0x%x]0x%08x, r = [%d] %d\n", 
			__FUNCTION__, __LINE__, thread->id, pred_id, pred, 
			p_id, p, q_id, q, src1_id, src1, src2_id, src2, 
			r_id, r);
}

void frm_isa_ICMP_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_POPC_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_F2F_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_F2I_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_I2F_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_I2I_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_MOV_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int pred_id, dst_id, src_id;
	unsigned int pred, dst, src;

	/* Predication */
	pred_id = inst->dword.general0.pred;
	pred = thread->pr[pred_id];

	if (pred == 1)
	{
		/* Read */
		src_id = inst->dword.general0.src2;
		if (inst->dword.general0.src2_mod == 0)
			src = thread->gpr[src_id].v.i;
		else if (inst->dword.general0.src2_mod == 1)
			mem_read(frm_emu->const_mem, src_id, 4, &src);

		/* Execute */
		dst = src;

		/* Write */
		dst_id = inst->dword.general0.dst;
		thread->gpr[dst_id].v.i = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: thread[%d] pred = [%x] %d dst = [0x%x] 0x%08x "
			"src = [0x%x] 0x%08x\n", __FUNCTION__, __LINE__,
			thread->id, pred_id, pred, dst_id, dst, src_id, src);
}

void frm_isa_MOV32I_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id;
	unsigned int dst, imm32;

	dst_id = inst->dword.imm.dst;
	imm32 = inst->dword.imm.imm32;

	dst = imm32;

	thread->gpr[dst_id].v.f = dst;
}

void frm_isa_SEL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_PRMT_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_P2R_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_R2P_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_CSET_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_CSETP_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_PSET_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_PSETP_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_TEX_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_TLD_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_TLD4_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_TXQ_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDC_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_LD_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src_id, addr;
	unsigned int dst;

	dst_id = inst->dword.offs.dst;
	src_id = inst->dword.offs.src1;
	addr = thread->gpr[src_id].v.i;

	mem_read(frm_emu->global_mem, addr, 4, &dst);

	thread->gpr[dst_id].v.i = dst;
}

void frm_isa_LDU_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDS_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src_id, addr;
	unsigned int dst;

	dst_id = inst->dword.offs.dst;
	src_id = inst->dword.offs.src1;
	addr = thread->gpr[src_id].v.i;

	mem_read(thread->thread_block->shared_mem, addr, 4, &dst);

	thread->gpr[dst_id].v.i = dst;
}

void frm_isa_LDLK_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDSLK_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_LD_LDU_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_LDS_LDU_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_ST_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src_id, addr;
	unsigned int dst;

	dst_id = inst->dword.offs.dst;
	dst = thread->gpr[dst_id].v.i;
	src_id = inst->dword.offs.src1;
	addr = thread->gpr[src_id].v.i;

	mem_write(frm_emu->global_mem, addr, 4, &dst);
}

void frm_isa_STL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_STUL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_STS_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int dst_id, src_id, addr;
	unsigned int dst;

	dst_id = inst->dword.offs.dst;
	dst = thread->gpr[dst_id].v.i;
	src_id = inst->dword.offs.src1;
	addr = thread->gpr[src_id].v.i;

	mem_write(thread->thread_block->shared_mem, addr, 4, &dst);
}

void frm_isa_STSUL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_ATOM_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_RED_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_CCTL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_CCTLL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_MEMBAR_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_SULD_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_SULEA_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_SUST_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_SURED_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_SUQ_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_BRA_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	//	struct frm_warp_t *warp;
	//
	//	warp = thread->warp;
	//
	//	printf("1warp->pc = %x\n", warp->pc);
	//	warp->pc = inst->dword.tgt.target - 8;
	//	printf("2warp->pc = %x\n", warp->pc);
}

void frm_isa_BRX_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_JMP_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_JMX_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_CAL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_JCAL_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_RET_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_BRK_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_CONT_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_LONGJMP_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_SSY_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
}

void frm_isa_PBK_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_PCNT_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_PRET_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_PLONGJMP_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_BPT_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_EXIT_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	if (inst->dword.tgt.pred == 7)
		thread->warp->finished = 1;
}

void frm_isa_NOP_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
}

void frm_isa_S2R_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	unsigned int pred_id, dst_id, src_id;
	unsigned int pred, dst, src;

	/* Predication */
	pred_id = inst->dword.general1.pred;
	pred = thread->pr[pred_id];

	if (pred == 1)
	{
		/* Read */
		src_id = inst->dword.general0.src2 & 0xff;
		src = thread->sr[src_id].v.i;

		/* Execute */
		dst = src;

		/* Write */
		dst_id = inst->dword.general0.dst;
		thread->gpr[dst_id].v.i = dst;
	}

	/* Debug */
	frm_isa_debug("%s:%d: thread[%d] pred = [%x] %x dst = [0x%x] 0x%08x "
			"src = [0x%x] 0x%08x\n", __FUNCTION__, __LINE__,
			thread->id, pred_id, pred, dst_id, dst, src_id, src);
}

void frm_isa_B2R_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_LEPC_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

void frm_isa_BAR_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	struct frm_thread_block_t *thread_block;
	struct frm_warp_t *warp;

	int warp_id;

	thread_block = thread->thread_block;
	warp = thread->warp;

	/* Set flag to suspend warp execution */
	warp->at_barrier = 1;

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
}

void frm_isa_VOTE_impl(struct frm_thread_t *thread, struct frm_inst_t *inst)
{
	__NOT_IMPL__
}

