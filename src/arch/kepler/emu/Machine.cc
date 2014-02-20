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
//#include <stdlib.h>

#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <mem-system/memory.h>

#include "Emu.h"
#include "Grid.h"
#include "isa.h"
#include "Thread.h"
#include "../asm/Inst.h"
#include "ThreadBlock.h"

namespace Kepler
{

const char *kpl_err_isa_note =
		"\tThe NVIDIA Kepler SASS instruction set is partially supported by \n"
		"\tMulti2Sim. If your program is using an unimplemented instruction, \n"
		"\tplease email 'development@multi2sim.org' to request support for \n"
		"\tit.\n";

#define __NOT_IMPL__ std::cerr << "Kepler instruction not implemented.\n" << \
		kpl_err_isa_note;


void kpl_isa_IMUL_A_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_IMUL_B_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_ISCADD_A_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_ISCADD_B_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_IMAD_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_IADD_A_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_IADD_B_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_ISETP_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_EXIT_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_BRA_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_MOV_A_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_MOV_B_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_MOV32I_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_LD_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_LDS_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_ST_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_STS_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_DADD_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_FFMA_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_NOP_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_S2R_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_SHF_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_BAR_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_BPT_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_JMX_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_JMP_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_JCAL_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_BRX_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_CAL_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_PRET_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_PLONGJMP_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_SSY_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_PBK_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_PCNT_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_GETCRSPTR_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_GETLMEMBASE_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_SETCRSPTR_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_SETLMEMBASE_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_LONGJMP_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_RET_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_KIL_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_BRK_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_CONT_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_RTT_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_SAM_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_RAM_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_IDE_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_LOP32I_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_FADD32I_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_FFMA32I_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_IMAD32I_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_ISCADD32I_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

}	//namespace
