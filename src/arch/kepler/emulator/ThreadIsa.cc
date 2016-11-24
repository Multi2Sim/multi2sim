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

#include <cmath>
#include <iostream>

#include <lib/cpp/Bitmap.h>
#include "../disassembler/Instruction.h"

#include "Emulator.h"
#include "Grid.h"
#include "Warp.h"
#include "SyncStack.h"
#include "Thread.h"
#include "ThreadBlock.h"


namespace Kepler
{

enum
{
	SR_LANEID = 0,
	SR_CLOCK = 1,
	SR_VIRTCFG = 2,
	SR_VIRTID = 3,
	SR_PM0 = 4,
	SR_PM1 = 5,
	SR_PM2 = 6,
	SR_PM3 = 7,
	SR_PM4 = 8,
	SR_PM5 = 9,
	SR_PM6 = 10,
	SR_PM7 = 11,
	SR_PRIM_TYPE = 16,
	SR_INVOCATION_ID = 17,
	SR_Y_DIRECTION = 18,
	SR_THREAD_KILL = 19,
	SR_SHADER_TYPE = 20,
	SR_MACHINE_ID_0 = 24,
	SR_MACHINE_ID_1 = 25,
	SR_MACHINE_ID_2 = 26,
	SR_MACHINE_ID_3 = 27,
	SR_AFFINITY = 28,
	SR_TID = 32,
	SR_TID_X = 33,
	SR_TID_Y = 34,

	SR_TID_Z = 35,
	SR_CTA_PARAM = 36,
	SR_CTAID_X = 37,
	SR_CTAID_Y = 38,
	SR_CTAID_Z = 39,
	SR_NTID = 40,
	SR_NTID_X = 41,
	SR_NTID_Y = 42,
	SR_NTID_Z = 43,
	SR_GRIDPARAM = 44,
	SR_NCTAID_X = 45,
	SR_NCTAID_Y = 46,
	SR_NCTAID_Z = 47,
	SR_SWINLO = 48,
	SR_SWINSZ = 49,
	SR_SMEMSZ = 50,
	SR_SMEMBANKS = 51,
	SR_LWINLO = 52,
	SR_LWINSZ = 53,
	SR_LMEMLOSZ = 54,
	SR_LMEMHIOFF = 55,
	SR_EQMASK = 56,
	SR_LTMASK = 57,
	SR_LEMASK = 58,
	SR_GTMASK = 59,
	SR_GEMASK = 60,
	SR_GLOBALERRORSTATUS = 64,
	SR_WARPERRORSTATUS = 66,
	SR_WARPERRORSTATUSCLEAR = 67,
	SR_CLOCKLO = 80,
	SR_CLOCKHI = 81
};
// to be added : %globaltimer, %globaltimer_lo, %globaltimer_hi

void Thread::ExecuteInst_Special()
{

	SyncStack* stack = warp->getSyncStack()->get();
	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
			stack->setActiveMask(temp_am);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_IMUL_A(Instruction *inst)
{
	// Inst bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesIMUL format = inst_bytes.imul;

	// Predicates and active masks
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src1_id;
	unsigned long long temp, src1, src2;
	unsigned dst;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		if ((format.op0 == 1) && (format.op2 == 1))
		{
			//Immediate Mode
			src2 = format.src2;
		}

		// Execute
		temp = src1 * src2;

		// Execute .HI flag
		if (format.hi == 1)
			dst = temp >> 32;
		else
			dst = temp;

		// Execute .CC flag
		if (format.cc == 1)
		{
			unsigned zf, sf;
			zf = (dst == 0) ? 1 : 0;
			WriteCC_ZF(zf);
			sf = (dst >> 31) & 0x1;
			WriteCC_SF(sf);
			WriteCC_CF(0);
			WriteCC_OF(0);
		}

		// Write
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_IMUL_B(Instruction *inst)
{
	// Inst bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesIMUL format = inst_bytes.imul;

	// Predicates and active masks
	Emulator *emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned src1_id, src2_id, dst_id;
	unsigned long long temp, src1, src2;
	unsigned dst;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);
		src2_id = format.src2;

		// Check it
		if ((format.op0 == 2) && (format.op2 == 1))
			src2 = ReadGPR(src2_id);	// Register Mode
		else if (format.op2 == 0)	// Const mode
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);
		//else
		//	src2 = format.src2 >> 18 ? format.src2 | 0xfff80000 : format.src2;


		// Execute
		temp = src1 * src2;

		// Execute .HI flag
		if (format.hi == 1)
			dst = temp >> 32;
		else
			dst = temp;

		// Execute .CC flag
		if (format.cc == 1)
		{
			unsigned zf, sf;
			zf = (dst == 0) ? 1 : 0;
			WriteCC_ZF(zf);
			sf = (dst >> 31) & 0x1;
			WriteCC_SF(sf);
			WriteCC_CF(0);
			WriteCC_OF(0);
		}

		// Write result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_ISCADD_A(Instruction *inst)
{
	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesISCADD format = inst_bytes.iscadd;

	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id;

	// Operand
	unsigned src1, src2, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read shift bits
		unsigned shamt = format.shamt;

		// Read src1 value
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);
		src1 = (src1 << shamt) & 0xffffffff;

		// Read src2 value IMM20 mode
		src2 = format.src2 >> 18 ? format.src2 | 0xfff80000 : format.src2;

		// Determine least significant bit value for the add
		unsigned lsb = 0;

		if (format.po == 3)
			lsb = 1; // .PO Plus one(for averaging)
		else
		{
			if (format.po == 1)
			{
				src2 = ~src2; // negate src2
				lsb = 1;
			}
			if (format.po == 2)
			{
				src1 = ~src1; // negate src1
				lsb = 1;
			}
		}

		// Execute .PO flag
		dst = src1 + src2 + lsb;

		// Update .CC flag
		unsigned zf, sf, cf, of;

		// Update zero flag
		zf = (dst == 0)? 1 : 0;
		WriteCC_ZF(zf);

		// Update sign flag
		sf = (dst >> 31) & 0x1;
		WriteCC_SF(sf);

		// Update overflow flag (for signed arithmetic)
		long long of_tmp, src1_tmp, src2_tmp;
		src1_tmp = (int) src1;
		src2_tmp = (int) src2;
		of_tmp = src1_tmp + src2_tmp + lsb;
		of = ((of_tmp >> 32) & 0x1) ^ ((dst >> 31) & 0x1);
		WriteCC_OF(of);

		// Update carry flag (for unsigned arithmetic)
		unsigned long long cf_tmp, src1_tmp1, src2_tmp1;
		src1_tmp1 = src1;
		src2_tmp1 = src2;
		cf_tmp = src1_tmp1 + src2_tmp1 + lsb;
		cf = (cf_tmp >> 32) & 0x1;
		WriteCC_CF(cf);

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}


void Thread::ExecuteInst_ISCADD_B(Instruction *inst)
{
	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesISCADD format = inst_bytes.iscadd;

	// Get Warp
	Emulator* emu = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id;

	// Operand
	unsigned src1, src2, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read shift bits
		unsigned shamt = format.shamt;

		// Read src1 value
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);
		src1 = (src1 << shamt) & 0xffffffff;

		// Read src2 value Check it
		if (format.op2 == 1) // constant mode
			emu->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);
		else if (format.op2 == 3)
		{
			unsigned src2_id;
			src2_id = format.src2;
			src2 = ReadGPR(src2_id);
		}

		// Determine least significant bit value for the add
		unsigned lsb = 0;

		if (format.po == 3)
			lsb = 1; // .PO Plus one(for averaging)
		else
		{
			if (format.po == 1)
			{
				src2 = ~src2; // negate src2
				lsb = 1;
			}
			if (format.po == 2)
			{
				src1 = ~src1; // negate src1
				lsb = 1;
			}
		}

		// Execute .PO flag
		dst = src1 + src2 + lsb;

		// Update .CC flag
		unsigned zf, sf, cf, of;

		// Update zero flag
		zf = (dst == 0)? 1 : 0;
		WriteCC_ZF(zf);

		// Update sign flag
		sf = (dst >> 31) & 0x1;
		WriteCC_SF(sf);

		// Update overflow flag (for signed arithmetic)
		long long of_tmp, src1_tmp, src2_tmp;
		src1_tmp = (int) src1;
		src2_tmp = (int) src2;
		of_tmp = src1_tmp + src2_tmp + lsb;
		of = ((of_tmp >> 32) & 0x1) ^ ((dst >> 31) & 0x1);
		WriteCC_OF(of);

		// Update carry flag (for unsigned arithmetic)
		unsigned long long cf_tmp, src1_tmp1, src2_tmp1;
		src1_tmp1 = src1;
		src2_tmp1 = src2;
		cf_tmp = src1_tmp1 + src2_tmp1 + lsb;
		cf = (cf_tmp >> 32) & 0x1;
		WriteCC_CF(cf);

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}


	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_ISAD_A(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesISAD format = inst_bytes.isad;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id;

	// Operand
	unsigned src1, src2, src3, dst;

	// Temporary register for carry flag
	unsigned long long cf_tmp, src1_tmp1, src2_tmp1;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read src1 src2 src3 value
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);
		src2 = format.src2 >> 18 ? format.src2 | 0xfff80000 : format.src2;
		unsigned src3_id;
		src3_id = format.src3;
		src3 = ReadGPR(src3_id);

		// signed mode
		if (format.u_s == 1) // sign mode
		{
			int src1_signed, src2_signed; // src3 and dst are always unsigned
			src1_signed = src1;
			src2_signed = src2;
			dst = src3 + ((src1_signed > src2_signed) ? src1_signed -
					src2_signed : src2_signed - src1_signed);

			src1_tmp1 = (src1_signed > src2_signed) ? src1_signed - src2_signed
							: src2_signed - src1_signed;
		}
		else if (format.u_s == 0) // unsigned mode
		{
			dst = src3 + ((src1 > src2) ? src1 - src2 : src2 - src1);
			src1_tmp1 = (src1 > src2) ? src1 - src2 : src2 - src1;
		}

		// Update .CC flag
		unsigned zf, sf, cf;

		// Update zero flag
		zf = (dst == 0) ? 1 : 0;
		WriteCC_ZF(zf);

		// Update sign flag
		sf = (dst >> 31) & 0x1;
		WriteCC_SF(sf);

		// Update carry flag (for unsigned arithmetic)
		src2_tmp1 = src3;
		cf_tmp = src1_tmp1 + src2_tmp1;
		cf = (cf_tmp >> 32) & 0x1;
		WriteCC_CF(cf);

		// no Update for overflow flag (for signed arithmetic)
		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_ISAD_B(Instruction *inst)
{
	// Get Warp
	Emulator *emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesISAD format = inst_bytes.isad;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id;

	// Operand
	unsigned src1, src2, src3, dst;

	// Temporary register for carry flag
	unsigned long long cf_tmp, src1_tmp1, src2_tmp1;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read src1 value
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read src2 value and src3 value
		if (format.op2 == 1) // src2 is const src3 is register
		{
			unsigned src3_id;
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);
			src3_id = format.src3;
			src3 = ReadGPR(src3_id);
		}
		else if ((format.op2 == 2)) // src2 is register src3 is const
		{
			unsigned src2_id;
			src2_id = format.src3;
			src2 = ReadGPR(src2_id);
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src3);
		}
		else if (format.op2 == 3) // both src2 src3 register
		{
			unsigned src2_id, src3_id;
			src2_id = format.src2;
			src3_id = format.src3;
			src2 = ReadGPR(src2_id);
			src3 = ReadGPR(src3_id);
		}

		// signed mode
		if (format.u_s == 1) // sign mode
		{
			int src1_signed, src2_signed; // src3 and dst are always unsigned
			src1_signed = src1;
			src2_signed = src2;
			dst = src3 + ((src1_signed > src2_signed) ? src1_signed -
					src2_signed : src2_signed - src1_signed);

			src1_tmp1 = (src1_signed > src2_signed) ? src1_signed - src2_signed
							: src2_signed - src1_signed;
		}
		else if (format.u_s == 0) // unsigned mode
		{
			dst = src3 + ((src1 > src2) ? src1 - src2 : src2 - src1);
			src1_tmp1 = (src1 > src2) ? src1 - src2 : src2 - src1;
		}

		// Update .CC flag
		unsigned zf, sf, cf;

		// Update zero flag
		zf = (dst == 0) ? 1 : 0;
		WriteCC_ZF(zf);

		// Update sign flag
		sf = (dst >> 31) & 0x1;
		WriteCC_SF(sf);

		// Update carry flag (for unsigned arithmetic)
		src2_tmp1 = src3;
		cf_tmp = src1_tmp1 + src2_tmp1;
		cf = (cf_tmp >> 32) & 0x1;
		WriteCC_CF(cf);

		// no Update for overflow flag (for signed arithmetic)
		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_BFI_A(Instruction *inst)
{
	// Inst Bytes Format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesBFI format = inst_bytes.bfi;

	// Predicates and active masks
	SyncStack *stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	unsigned pred, pred_id;

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	if (active == 1 && pred == 1)
	{
		unsigned src1_id, src1, src2, src3_id, src3;

		// Read Src1
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read Src2 Imm20 Mode
		src2 = ((format.op1 >> 5) & 1) ? format.src2 | 0xfff80000 : format.src2;

		// Read Src3
		src3_id = format.src3;
		src3 = ReadGPR(src3_id);

		// Operation
		unsigned position, size;
		position = src2 & 0xff;
		size = (src2 >> 8) & 0xff;

		unsigned mask = 0;
		if (position < 32 && size > 0)
		{
			mask = 0xffffffff;
			if (size <= 32)
				mask = mask >> (32 - size);
			mask = mask << position;
		}

		unsigned temp;
		temp = ((src1 << position) & mask) | (src3 & ~mask);

		// CC update
		if (format.cc == 1)
		{
			WriteCC_ZF((temp >> 31 == 0) ? 1 : 0);
			WriteCC_SF(temp >> 31);
			WriteCC_CF(0);
			WriteCC_OF(0);
		}

		// Write Results
		unsigned dst_id;
		dst_id = format.dst;
		WriteGPR(dst_id, temp);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_BFI_B(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_BFE_A(Instruction *inst)
{
	// Inst Bytes Format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesBFE format = inst_bytes.bfe;

	// Predicates and active masks
	SyncStack *stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	unsigned pred, pred_id;
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	if (active == 1 && pred == 1)
	{
		unsigned src1_id, src1, src2;

		// Read Src1
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read Src2
		src2 = format.src2; // IMM 20
		if (((format.op1 >> 5) & 1) == 1)
			src2 = src2 | 0xfff8000;

		// Read position and size
		unsigned position, size;
		position = src2 & 0xff;
		size = (src2 >> 8) & 0xff;

		// Operation
		unsigned temp;
		temp = src1;
		if (format.bit_reverse == 1)
			temp ^= 0xffffffff;
		if (size == 0)
			temp = 0;
		else if (position + size < 32)
		{
			// Shift the MSB of the field to bit 31
			temp = temp << (32 - (position + size));

			// Shift the LSB of the field to bit 0
			if (format.u_s == 1) // arithmetic shift
				temp = (int)temp >> (32 - size);
			else if (format.u_s == 0)
				temp = temp >> (32 - size);
		}
		else
		{
			if (position > 32)
				position = 32;

			if (format.u_s == 1) // arithmetic shift
				temp = (int)temp >> size;
			else if (format.u_s == 0)
				temp = temp >> size;
		}

		// CC update
		if (format.cc == 1)
		{
			WriteCC_ZF((temp >> 31 == 0) ? 1 : 0);
			WriteCC_SF(temp >> 31);
			WriteCC_CF(0);
			WriteCC_OF(0);
		}

		// Write Results
		unsigned dst_id;
		dst_id = format.dst;
		WriteGPR(dst_id, temp);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_BFE_B(Instruction *inst)
{
	// Inst Bytes Format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesBFE format = inst_bytes.bfe;

	// Predicates and active masks
	Emulator *emulator = Emulator::getInstance();
	SyncStack *stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	unsigned pred, pred_id;

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	if (active == 1 && pred == 1)
	{
		unsigned src1_id, src2_id, src1, src2;

		// Read Src1
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read Src2
		if (format.op2 == 1) // Const Mode
			emulator->ReadGlobalMemory(format.src2 << 2, 4, (char*)&src2);
		else if (format.op2 == 3) // Register Mode
		{
			src2_id = format.src2;
			src2 = ReadGPR(src2_id);
		}

		unsigned position, size;

		position = src2 & 0xff;
		size = (src2 >> 8) & 0xff;

		// Operation
		unsigned temp;
		temp = src1;

		if (format.bit_reverse == 1)
			temp ^= 0xffffffff;

		if (size == 0)
			temp = 0;
		else if (position + size < 32)
		{
			// Shift the MSB of the field to bit 31
			temp = temp << (32 - (position + size));

			// Shift the LSB of the field to bit 0
			if (format.u_s == 1) // arithmetic shift
				temp = (int)temp >> (32 - size);
			else if (format.u_s == 0)
				temp = temp >> (32 - size);
		}
		else
		{
			if (position > 32)
				position = 32;

			if (format.u_s == 1) // arithmetic shift
				temp = (int)temp >> size;
			else if (format.u_s == 0)
				temp = temp >> size;
		}

		// CC update
		if (format.cc == 1)
		{
			WriteCC_ZF((temp >> 31 == 0) ? 1 : 0);
			WriteCC_SF(temp >> 31);
			WriteCC_CF(0);
			WriteCC_OF(0);
		}

		// Write Results
		unsigned dst_id;
		dst_id = format.dst;
		WriteGPR(dst_id, temp);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_IMAD(Instruction *inst)
{
	// Inst bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Emulator* emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst;
	int srcA, srcB, src3;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	if ( active == 1 && pred == 1)
	{
		// Read
		src_id = format.mod0;
		srcA = ReadGPR(src_id);
		src_id = format.mod1 & 0xff;
		src3 = ReadGPR(src_id);
		if (format.srcB_mod == 0)
		{
			src_id = format.srcB;
			emulator->ReadConstantMemory(src_id << 2, 4, (char*)&srcB);
		}
		else if (format.srcB_mod == 1)
		{
			src_id = format.srcB & 0x1ff;
			srcB = ReadGPR(src_id);
		}
		else	//check it
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;


		// Execute
		dst = srcA * srcB + src3;

		// Write
		dst_id = format.dst;
		WriteGPR(dst_id, dst);

	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());

	if (getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
        		<<getWarpId() <<" IMAD op0 "<<format.op0;
        std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB

        		<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
        		<<std::endl;
	}
}

void Thread::ExecuteInst_IMADSP_A(Instruction *inst)
{
	ISAUnsupportedFeature(inst);
}

void Thread::ExecuteInst_IMADSP_B(Instruction *inst)
{
	ISAUnsupportedFeature(inst);
}

void Thread::ExecuteInst_IADD_A(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesIADD format = inst_bytes.iadd;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id;

	// Operand
	unsigned src1, src2, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read src1 value
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read src2 value IMM20
		src2 = ((format.op1 >> 5) & 1) ? format.src2 | 0xfff80000 : format.src2;

		// Determine least significant bit value for the add
		unsigned lsb = 0;
		if (format.po == 3)
			lsb = 1; // .PO Plus one(for averaging)
		else
		{
			if (format.po == 1)
			{
				src2 = ~src2; // negate src2
				lsb = 1;
			}
			if (format.po == 2)
			{
				src1 = ~src1; // negate src1
				lsb = 1;
			}
		}

		// Extended precision addition read carry bit
		if (format.x == 1)
			lsb = ReadCC_CF(); // Illegal to combine .PO and .X

		// Execute .PO and .X flag
		dst = src1 + src2 + lsb;

		// Update .CC flag
		if (format.cc == 1)
		{
			unsigned zf, sf, cf, of;

			// Update zero flag
			if (format.x == 1)
			{
				zf = ((dst == 0) && ReadCC_ZF()) ? 1 : 0;
				WriteCC_ZF(zf);
			}
			else
			{
				zf = (dst == 0)? 1 : 0;
				WriteCC_ZF(zf);
			}

			// Update sign flag
			sf = (dst >> 31) & 0x1;
			WriteCC_SF(sf);

			// Update overflow flag (for signed arithmetic)
			long long of_tmp, src1_tmp, src2_tmp;
			src1_tmp = (int) src1;
			src2_tmp = (int) src2;
			of_tmp = src1_tmp + src2_tmp + lsb;
			of = ((of_tmp >> 32) & 0x1) ^ ((dst >> 31) & 0x1);
			WriteCC_OF(of);

			// Update carry flag (for unsigned arithmetic)
			unsigned long long cf_tmp, src1_tmp1, src2_tmp1;
			src1_tmp1 = src1;
			src2_tmp1 = src2;
			cf_tmp = src1_tmp1 + src2_tmp1 + lsb;
			cf = (cf_tmp >> 32) & 0x1;
			WriteCC_CF(cf);
		}

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());

	Emulator::isa_debug << misc::fmt("At instruction %s:\tthe current PC is "
					"= %x\n",inst->getName(),warp->getPC());
	Emulator::isa_debug << misc::fmt("At instruction %s:\tthe thread ID is "
					"= %u\n",inst->getName(),id_in_warp);
	Emulator::isa_debug << misc::fmt("At instruction %s:\tinput: the src1 is = %d\n"
					,inst->getName(),src1);
	Emulator::isa_debug << misc::fmt("At instruction %s:\tinput: the src2 is = %d\n"
					,inst->getName(),src2);
	Emulator::isa_debug << misc::fmt("At instruction %s:\toutput: the result is "
					"= %d\n",inst->getName(),dst);
	Emulator::isa_debug << misc::fmt("\n");
}

void Thread::ExecuteInst_IADD_B(Instruction *inst)
{
	// Get Warp
	Emulator* emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesIADD format = inst_bytes.iadd;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id;

	// Operand
	unsigned src1, src2, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read src1 value
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read src2 value
		if ((format.op0 == 2) && (format.op2 == 3)) // src2 is register mode
		{
			// src2 ID
			unsigned src2_id;

			// Read src2 value
			src2_id = format.src2;
			src2 = ReadGPR(src2_id);
		}
		else if (format.op2 == 1) // constant mode
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);

		// Determine least significant bit value for the add
		unsigned lsb = 0;

		if (format.po == 3)
			lsb = 1; // .PO Plus one(for averaging)
		else
		{
			if (format.po == 1)
			{
				src2 = ~src2; // negate src2
				lsb = 1;
			}
			if (format.po == 2)
			{
				src1 = ~src1; // negate src1
				lsb = 1;
			}
		}

		// Extended precision addition read carry bit
		if (format.x == 1)
			lsb = ReadCC_CF(); // Illegal to combine .PO and .X

		// Execute .PO and .X flag
		dst = src1 + src2 + lsb;

		// Update .CC flag
		unsigned zf, sf, cf, of;

		// Update zero flag
		if (format.x == 1)
		{
			zf = ((dst == 0) && ReadCC_ZF()) ? 1 : 0;
			WriteCC_ZF(zf);
		}
		else
		{
			zf = (dst == 0)? 1 : 0;
			WriteCC_ZF(zf);
		}

		// Update sign flag
		sf = (dst >> 31) & 0x1;
		WriteCC_SF(sf);

		// Update overflow flag (for signed arithmetic)
		long long of_tmp, src1_tmp, src2_tmp;
		src1_tmp = (int) src1;
		src2_tmp = (int) src2;
		of_tmp = src1_tmp + src2_tmp + lsb;
		of = ((of_tmp >> 32) & 0x1) ^ ((dst >> 31) & 0x1);
		WriteCC_OF(of);

		// Update carry flag (for unsigned arithmetic)
		unsigned long long cf_tmp, src1_tmp1, src2_tmp1;
		src1_tmp1 = src1;
		src2_tmp1 = src2;
		cf_tmp = src1_tmp1 + src2_tmp1 + lsb;
		cf = (cf_tmp >> 32) & 0x1;
		WriteCC_CF(cf);

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}


	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());

	Emulator::isa_debug << misc::fmt("At instruction %s:\tthe current PC is "
					"= %x\n",inst->getName(),warp->getPC());
	Emulator::isa_debug << misc::fmt("At instruction %s:\tthe thread ID is "
					"= %u\n",inst->getName(),id_in_warp);
	Emulator::isa_debug << misc::fmt("At instruction %s:\tinput: the src1 is = %d\n"
					,inst->getName(),src1);
	Emulator::isa_debug << misc::fmt("At instruction %s:\tinput: the src2 is = %d\n"
					,inst->getName(),src2);
	Emulator::isa_debug << misc::fmt("At instruction %s:\toutput: the result is "
					"= %d\n",inst->getName(),dst);
	Emulator::isa_debug << misc::fmt("\n");
}

void Thread::ExecuteInst_IADD32I(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesIADD32I format = inst_bytes.iadd32i;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;

	// Predicate register
	unsigned pred;

	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src_id;

	// Operand
	unsigned src, dst, imm32;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read src1 value
		src_id = format.src;
		src = ReadGPR(src_id);

		// Read src2 value Check it
		imm32 = format.imm32;

		// Determine least significant bit value for the add
		unsigned lsb = 0;

		if (format.po == 3)
			lsb = 1; // .PO Plus one(for averaging)
		else
		{
			if (format.po == 1)
			{
				imm32 = ~imm32; // negate src2
				lsb = 1;
			}
			if (format.po == 2)
			{
				src = ~src; // negate src1
				lsb = 1;
			}
		}

		// Extended precision addition read carry bit
		if (format.x == 1)
			lsb = ReadCC_CF(); // Illegal to combine .PO and .X

		// Execute .PO and .X flag
		dst = src + imm32 + lsb;

		// Update .CC flag
		unsigned zf, sf, cf, of;

		// Update zero flag
		if (format.x == 1)
		{
			zf = ((dst == 0) && ReadCC_ZF()) ? 1 : 0;
			WriteCC_ZF(zf);
		}
		else
		{
			zf = (dst == 0)? 1 : 0;
			WriteCC_ZF(zf);
		}

		// Update sign flag
		sf = (dst >> 31) & 0x1;
		WriteCC_SF(sf);

		// Update overflow flag (for signed arithmetic)
		long long of_tmp, src1_tmp, src2_tmp;
		src1_tmp = (int) src;
		src2_tmp = (int) imm32;
		of_tmp = src1_tmp + src2_tmp + lsb;
		of = ((of_tmp >> 32) & 0x1) ^ ((dst >> 31) & 0x1);
		WriteCC_OF(of);

		// Update carry flag (for unsigned arithmetic)
		unsigned long long cf_tmp, src1_tmp1, src2_tmp1;
		src1_tmp1 = src;
		src2_tmp1 = imm32;
		cf_tmp = src1_tmp1 + src2_tmp1 + lsb;
		cf = (cf_tmp >> 32) & 0x1;
		WriteCC_CF(cf);

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_ISET_A(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_ISET_B(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesISET format = inst_bytes.iset;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;

	// Predicate register
	unsigned pred;

	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned src1_id, dst_id, pred_src_id;

	// Operand
	unsigned src1, src2, dst, pred_src;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read Src1
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read Src2
		if (format.op2 == 1) // src is const
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);
		else if (format.op2 == 3) // src is register mode
		{
			// src2 ID
			unsigned src2_id;

			// Read src2 value
			src2_id = format.src2;
			src2 = ReadGPR(src2_id);
		}

		// Read Predicate Src
		pred_src_id = format.pred_src;
		if (pred_src_id <= 7)
			pred_src = ReadPredicate(pred_src_id);
		else
			pred_src = !ReadPredicate(pred_src_id - 8);

		// Read Boolean mask .BM is default(.bval=0) .BF(.bavl=1)
		unsigned mask_value = (format.bval == 1) ? 0x3f800000 : 0xffffffff;

		// Compare operation
        unsigned cmp_op = format.comp;
        bool cmp_result;

        if (format.x == 1)
        	ISAUnsupportedFeature(inst);
        else
        {

			if (cmp_op == 0)
				cmp_result = false;
			else if (cmp_op == 1)
				cmp_result = format.u_s ? ((signed) src1 < (signed) src2) :
							(src1 < src2);
			else if (cmp_op == 2)
				cmp_result = format.u_s ? ((signed) src1 == (signed) src2) :
							(src1 == src2);
			else if (cmp_op == 3)
				cmp_result = format.u_s ? ((signed) src1 <= (signed) src2) :
							(src1 <= src2);
			else if (cmp_op == 4)
				cmp_result = format.u_s ? ((signed) src1 > (signed) src2) :
							(src1 > src2);
			else if (cmp_op == 5)
				cmp_result = format.u_s ? ((signed) src1 != (signed) src2) :
							(src1 != src2);
			else if (cmp_op == 6)
				cmp_result = format.u_s ? ((signed) src1 >= (signed) src2) :
							(src1 >= src2);
			else if (cmp_op == 7)
				cmp_result = true;
        }

		// Boolean operation
		unsigned bool_op = format.bop;
		if (bool_op == 0) // AND
			dst = (cmp_result && pred_src) ? mask_value : 0;
		else if (bool_op == 1) // OR
			dst = (cmp_result || pred_src) ? mask_value : 0;
		else if (cmp_op == 2) // XOR
			dst = (cmp_result ^ pred_src) ? mask_value : 0;

		if (format.cc == 1)
			ISAUnsupportedFeature(inst);

		// Execute
		dst_id = format.dst;
		dst = pred_src ? src1 : src2;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_ISETP_A(Instruction *inst)
{
	// Inst bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Emulator* emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned pred_id_1, pred_id_2, pred_id_3;
	unsigned pred_1, pred_2, pred_3;
	unsigned srcA_id, srcB_id;
	int srcA, srcB;
	unsigned cmp_op;
	unsigned bool_op;
	bool cmp_res;
	bool x;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);


	// Execute
	if (active == 1 && pred == 1)
	{
		// Sources
		x = (format.mod1 >> 4) & 0x1;

		srcA_id = format.mod0;
		srcA = ReadGPR(srcA_id) - (x ? ReadCC_CF() : 0);
		srcB_id = format.srcB;
		if (format.srcB_mod == 0)
		{
			emulator->ReadConstantMemory(srcB_id << 2, 4, (char*)&srcB);
		}
		else if (format.srcB_mod == 1)
			srcB = ReadGPR(srcB_id);

		// Predicates
		pred_id_1 = (format.dst >> 3) & 0x7;
		pred_id_2 = format.dst & 0x7;
		pred_id_3 = format.mod1 & 0x7;


		pred_3 = ReadPredicate(pred_id_3);
		if (((format.mod1 >> 3) & 0x1))
			pred_3 = !pred_3;

		// Compare
        cmp_op = ((format.op1 & 0x1) << 2) | (format.mod1 >> 10);

		if (cmp_op == 1)
			cmp_res = srcA < srcB;
		else if (cmp_op == 2)
			cmp_res = srcA == srcB;
		else if (cmp_op == 3)
			cmp_res = srcA <= srcB;
		else if (cmp_op == 4)
			cmp_res = srcA > srcB;
		else if (cmp_op == 5)
			cmp_res = srcA != srcB;
		else if (cmp_op == 6)
			cmp_res = srcA >= srcB;
		else
			std::cerr << __func__ <<": compare operation (" << cmp_op << ") not implemented\n";

		// Logic
		bool_op = (format.mod1 >> 6) & 0x3;
		if (bool_op == 0)
		{
			pred_1 = cmp_res && pred_3;
			pred_2 = !cmp_res && pred_3;
		}
		else if (bool_op == 1)
		{
			pred_1 = cmp_res || pred_3;
			pred_2 = !cmp_res || pred_3;
		}
		else if (cmp_op == 2)
		{
			pred_1 = (cmp_res && !pred_3) || (!cmp_res && pred_3);
			pred_2 = (cmp_res && pred_3) || (!cmp_res && !pred_3);
		}
		else
			std::cerr << __func__ <<": bitwise operation " << bool_op << " not implemented\n";

		/* Write */
		pred_id_1 = (format.dst >> 3) & 0x7;
		pred_id_2 = format.dst & 0x7;
		if (pred_id_1 != 7)
			WritePredicate(pred_id_1, pred_1);
		if (pred_id_2 != 7)
			WritePredicate(pred_id_2, pred_2);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());

	if (getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<getWarpId() <<" ISETP op0 "<<format.op0;
		std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
				<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
				<<" cmp "<< cmp_op << " bool " << bool_op
				<<std::endl;
	}
}

void Thread::ExecuteInst_ISETP_B(Instruction *inst)
{
	// Inst bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned pred_id_1, pred_id_2, pred_id_3;
	unsigned pred_1, pred_2, pred_3;
	unsigned srcA_id;
	int srcA, srcB;
	unsigned cmp_op;
	unsigned bool_op;
	bool cmp_res;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);


	// Execute
	if (active == 1 && pred == 1)
	{
		// Sources
		srcA_id = format.mod0;
		srcA = ReadGPR(srcA_id);

		if (format.srcB_mod == 1)
		{
            srcB = format.srcB;
            if (srcB >> 18)
            	srcB |= 0xfff80000;
		}

		else if (format.srcB_mod == 0)
		{
			throw misc::Panic("Undetected behavior srcB_mod == 0\n");
		}
		else
			throw misc::Panic("Error srcB_mod\n");

		// Predicates
		pred_id_1 = (format.dst >> 3) & 0x7;
		pred_id_2 = format.dst & 0x7;
		pred_id_3 = format.mod1 & 0x7;

		pred_3 = ReadPredicate(pred_id_3);

		if (((format.mod1 >> 3) & 0x1))
			pred_3 = !pred_3;

		// Compare
        cmp_op = ((format.op1 & 0x1u) << 2) | (format.mod1 >> 10);

		if (cmp_op == 1)
			cmp_res = srcA < srcB;
		else if (cmp_op == 2)
			cmp_res = srcA == srcB;
		else if (cmp_op == 3)
			cmp_res = srcA <= srcB;
		else if (cmp_op == 4)
			cmp_res = srcA > srcB;
		else if (cmp_op == 5)
			cmp_res = srcA != srcB;
		else if (cmp_op == 6)
			cmp_res = srcA >= srcB;
		else
			std::cerr << __func__ <<": compare operation (" << cmp_op << ") not implemented\n";

		// Logic
		bool_op = (format.mod1 >> 6) & 0x3;
		if (bool_op == 0)
		{
			pred_1 = cmp_res && pred_3;
			pred_2 = !cmp_res && pred_3;
		}
		else if (bool_op == 1)
		{
			pred_1 = cmp_res || pred_3;
			pred_2 = !cmp_res || pred_3;
		}
		else if (cmp_op == 2)
		{
			pred_1 = (cmp_res && !pred_3) || (!cmp_res && pred_3);
			pred_2 = (cmp_res && pred_3) || (!cmp_res && !pred_3);
		}
		else
			std::cerr << __func__ <<": bitwise operation " << bool_op << " not implemented\n";

		/* Write */
		pred_id_1 = (format.dst >> 3) & 0x7;
		pred_id_2 = format.dst & 0x7;
		if (pred_id_1 != 7)
			WritePredicate(pred_id_1, pred_1);
		if (pred_id_2 != 7)
			WritePredicate(pred_id_2, pred_2);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());

	if (getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<getWarpId() <<" ISETP op0 "<<format.op0;
		std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
				<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
				<<" cmp "<< cmp_op << " bool " << bool_op
				<<std::endl;
	}

}

void Thread::ExecuteInst_LOP_A(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesLOP format = inst_bytes.lop;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned src1_id, dst_id;

	// Operand
	unsigned src1, src2, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read Src1
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read Src2
		if (format.op0 == 1) // src2 is IMM20
			src2 = format.src2 >> 18 ? format.src2 | 0xfff80000 : format.src2;

		// Execute
		if (format.src1_negate == 1)
			src1 = ~src1;
		if (format.src2_negate == 1)
			src2 = ~src2;
		if (format.lop == 0)
			dst = src1 & src2;
		else if (format.lop == 1)
			dst = src1 | src2;
		else if (format.lop == 2)
			dst = src1 ^ src2;
		else if (format.lop == 3)
			dst = src2;

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_LOP_B(Instruction *inst)
{
	// Get Warp
	Emulator *emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesLOP format = inst_bytes.lop;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned src1_id, dst_id;

	// Operand
	unsigned src1, src2, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read Src1
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read Src2
		if ((format.op0 == 2) && (format.op2 == 1 )) // src is const
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);
		else if ((format.op0 == 2 && format.op2 == 3)) // src is register mode
		{
			// src2 ID
			unsigned src2_id;

			// Read src2 value
			src2_id = format.src2;
			src2 = ReadGPR(src2_id);
		}

		// Execute
		if (format.src1_negate == 1)
			src1 = ~src1;
		if (format.src2_negate == 1)
			src2 = ~src2;

		if (format.lop == 0)
			dst = src1 & src2;
		else if (format.lop == 1)
			dst = src1 | src2;
		else if (format.lop == 2)
			dst = src1 ^ src2;
		else if (format.lop == 3)
			dst = src2;

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_ICMP_A(Instruction *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_ICMP_B(Instruction *inst)
{
	// Get Warp
	Emulator *emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesICMP format = inst_bytes.icmp;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id;

	// Operand
	unsigned src1, src2, src3, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read src1 value
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read src2 value and src3 value
		if (format.op2 == 1) // src2 is const src3 is register
		{
			unsigned src3_id;
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);
			src3_id = format.src3;
			src3 = ReadGPR(src3_id);
		}
		else if ((format.op2 == 2)) // src2 is register src3 is const
		{
			unsigned src2_id;
			src2_id = format.src3;
			src2 = ReadGPR(src2_id);
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src3);
		}
		else if (format.op2 == 3) // both src2 src3 register
		{
			unsigned src2_id, src3_id;
			src2_id = format.src2;
			src3_id = format.src3;
			src2 = ReadGPR(src2_id);
			src3 = ReadGPR(src3_id);
		}

		unsigned int src3_signed;

		// signed mode
		if (format.u_s == 1) // sign mode
			src3_signed = src3;

		unsigned cmp_op = format.comp;
		bool cmp_result;

		if (cmp_op == 0)
			cmp_result = false;
		else if (cmp_op == 1)
			cmp_result = format.u_s ? (src3_signed < 0) : (src3 < 0);
		else if (cmp_op == 2)
			cmp_result = format.u_s ? (src3_signed == 0) : (src3 == 0);
		else if (cmp_op == 3)
			cmp_result = format.u_s ? (src3_signed <= 0) : (src3 <= 0);
		else if (cmp_op == 4)
			cmp_result = format.u_s ? (src3_signed > 0) : (src3 > 0);
		else if (cmp_op == 5)
			cmp_result = format.u_s ? (src3_signed != 0) : (src3_signed != 0);
		else if (cmp_op == 6)
			cmp_result = format.u_s ? (src3_signed >= 0) : (src3_signed >= 0);
		else if (cmp_op == 7)
			cmp_result = true;

		dst = cmp_result ? src1 : src2;
		// no Update for overflow flag (for signed arithmetic)
		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_EXIT(Instruction *inst)
{
	// Inst bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;

	//emulator* emulator = emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	// Predicates and active masks
	unsigned pred;
	unsigned pred_id;
	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	if (id_in_warp == 0)
		stack->resetTempMask();

	// Execute
	// Clear specific bit of all entries in stack
	// Clear the bit of current active mask
	// Set the finished thread bit
	if (active == 1 && pred == 1)
	{
		stack->mask(id_in_warp, SyncStackMaskEXIT);
		stack->clearActiveMaskBit(id_in_warp);
		warp->setFinishedThreadBit(id_in_warp);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
	{
		if (warp->getFinishedThreadCount() == warp->getThreadCount() )
			warp->setFinishedEmu(true);
		else
            if (warp->getFinishedThreadCount() > warp->getThreadCount() )
            	throw misc::Panic("More threads finished than warp thread count.\n");

        warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
	}

}

void Thread::ExecuteInst_BRA(Instruction *inst)
{
	// Get SyncStack
	SyncStack* stack = warp->getSyncStack()->get();

	// Predicate register
	unsigned pred;

	unsigned active;

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesBRA format = inst_bytes.bra;

	// Get current PC
	unsigned pc = warp->getPC();

	unsigned target_pc = pc;

	// Get Instruction size
	unsigned inst_size = warp->getInstructionSize();

	// Get instruction offset
	int offset = format.offset >> 23 ?
						format.offset | 0xff000000 : format.offset;

	// Branch direction 0: forward branch, 1: backward branch
	unsigned branch_direction = format.offset >> 23;

    // number of taken thread
    unsigned taken_thread;

    // number of active thread
    unsigned active_thread;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);


	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id-8);

	// Clear temp_entry and taken_thread before BRA execute
	if (id_in_warp == 0)
	{
		stack->resetTempMask();
	}

	// The threads taking the BRA will set temp_entry's mask to 1
	// This mask will be pushed into stack then.
	// taken_thread adds 1
	if (active == 1 && pred == 1)
	{
		stack->setTempMaskBit(id_in_warp);
	}

	// At the last thread, push temp_entry's mask into stack,
	// along with BRA target address.
	if (id_in_warp == warp->getThreadCount() - 1)

	{

        taken_thread = stack->getTempMaskBitCount();
        active_thread = stack->getActiveMaskBitCount();

        /*
        std::cout << "BRA PC " << std::hex << warp->getPC()
        		<< "	A.M.	" << stack->getActiveMask()<< std::endl;
        stack->Dump(std::cout);
        */

		// Update sync stack and PC
        //  no thread in the active mask taken branch
        if (!taken_thread)
        {
        	warp->setTargetPC(pc + inst_size);
        }

        // If all threads in warp take the BRA
        // directly jump to the target pc
        else if (taken_thread == warp->getThreadCount())
        {
        	target_pc = pc + inst_size + offset;

        	// Update pc
        	warp->setTargetPC(target_pc);

        	// Pop all entries in syncstack between current pc and target pc
        	stack->popTillTarget(target_pc, warp->getPC());
        }

        // predicated BRA or non-predicated BRA
        else
        {
        	assert(branch_direction == 1 || branch_direction == 0);

        	if (branch_direction == 0)
        	{
        		// push reconvergence pc and the active mask at that pc into stack
        		// Clear all current active mask bits taking the BRA.
        		// Target pc goes to the next instruction
        		stack->push(pc + inst_size + offset,
							stack->getTempMask(),
							SyncStackEntryBRA
							);

        		stack->setActiveMask(
					stack->getActiveMask() &
					~stack->getTempMask());

        		assert(active_thread ==
        				stack->getActiveMaskBitCount() + taken_thread);

        		warp->setTargetPC(pc + inst_size);

        	}

        	// Backward BRA
        	// We do not need stack operations, because the next instruction
        	// is the target pc or next pc.
        	else
        	{
       			// no thread takes the BRA
       			if (!taken_thread)
       			{
       				warp->setTargetPC(pc + inst_size);
       			}
       			// all active threads take the BRA
       			else if (taken_thread == active_thread)
      			{
       				warp->setTargetPC(pc + inst_size + offset);
       			}
       			else
        		// predicated backward BRA, appearing in do while loop.
        		// taken_thread < active_thread
        		{
        			target_pc = pc + inst_size + offset;
        			stack->setActiveMask(stack->getTempMask());
        			warp->setTargetPC(target_pc);

        			// There is supposed to be no entries between the pcs.
        			if (stack->popTillTarget(target_pc, pc))
        				throw misc::Panic("Stack entries found between pc to"
        						" target pc when backward BRA\n");
        		}
        	}
        }
	}
}

void Thread::ExecuteInst_MOV_A(Instruction *inst)
{
	std::cerr << "MOV_A" << std::endl;
	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
	ISAUnsupportedFeature(inst);
}

void Thread::ExecuteInst_MOV_B(Instruction *inst)
{
	// Inst bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Emulator* emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	unsigned dst, src;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = format.srcB;
		if (format.srcB_mod == 0)
		{
			emulator->ReadConstantMemory(src_id << 2, 4, (char*)&src);
		}
		else if (format.srcB_mod == 1)
			//src = ReadGPR(src_id);
			Read_register(&src, src_id);

		/* Execute */
		dst = src;

		/* Write */
		dst_id = format.dst;
		//WriteGPR(dst_id, dst);
		Write_register(&dst, dst_id);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());

	if (getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
        		<<getWarpId() <<" MOV_B op0 "<<format.op0;
        std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
        		<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
        		<<std::endl;
	}

}

void Thread::ExecuteInst_MOV32I(Instruction *inst)
{
	// Get warp
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Inst bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesImm format = inst_bytes.immediate;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operands ID
	unsigned dst_id;

	// Operands
	RegValue src;

	// Execute
	if (active == 1 && pred == 1)
	{
		if (format.s == 0)
			{
				// Read source immediate
				src.u32 = format.imm32;

				// Read destination ID
				dst_id = format.dst;

				// Write the src value to destination register
				WriteGPR(dst_id, src.u32);

			}
		else
		{
			throw misc::Panic(".S = 1 in Function MOV32I");
		}
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_SEL_A(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesSEL format = inst_bytes.sel;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned src1_id, dst_id, pred_src_id;

	// Operand
	unsigned src1, src2, dst, pred_src;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read Src1
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read Src2
		if (format.op0 == 1) // src2 is IMM20
			src2 = format.src2 >> 18 ? format.src2 | 0xfff80000 : format.src2;

		// Read Predicate Src
		pred_src_id = format.pred_src;
		if (pred_src_id <= 7)
			pred_src = ReadPredicate(pred_src_id);
		else
			pred_src = ! ReadPredicate(pred_src_id - 8);

		// Execute
		dst_id = format.dst;
		dst = pred_src ? src1 : src2;
		Write_register(&dst, dst_id);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_SEL_B(Instruction *inst)
{
	// Get Warp
	Emulator *emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesSEL format = inst_bytes.sel;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned src1_id, dst_id, pred_src_id;

	// Operand
	unsigned src1, src2, dst, pred_src;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read Src1
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read Src2
		if ((format.op0 == 2) && (format.op2 == 1 )) // src is const
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);
		else if ((format.op0 == 2 && format.op2 == 3)) // src is register mode
		{
			// src2 ID
			unsigned src2_id;

			// Read src2 value
			src2_id = format.src2;
			src2 = ReadGPR(src2_id);
		}

		// Read Predicate Src
		pred_src_id = format.pred_src;
		if (pred_src_id <= 7)
			pred_src = ReadPredicate(pred_src_id);
		else
			pred_src = ! ReadPredicate(pred_src_id - 8);

		// Execute
		dst_id = format.dst;
		dst = pred_src ? src1 : src2;
		Write_register(&dst, dst_id);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_I2F_A(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesI2F format = inst_bytes.i2f;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id;

	// Operand
	int src;
	float dst;

	// Execute
	if (active == 1 && pred == 1)
	{

		if (format.op0 == 1) // src2 is IMM20
			src = format.src >> 18 ? format.src | 0xfff80000 : format.src;

		// Negate
		if (format.src_negate == 1)
			src = -src;

		// Absolute Value
		if (format.src_abs == 1)
			src = abs(src);

		// Execute
		if (format.s_fmt == 2 && format.d_fmt == 2) //Currently support 32 bit
			dst = src;
		else
			ISAUnsupportedFeature(inst);

		// Write Result
		dst_id = format.dst;
		WriteFloatGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_I2F_B(Instruction *inst)
{
	// Get Warp
	Emulator* emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesI2F format = inst_bytes.i2f;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id;

	// Operand
	int src;
	float dst;

	// Execute
	if (active == 1 && pred == 1)
	{

		if ((format.op0 == 2) && (format.op2 == 1 )) // src is const
			emulator->ReadConstantMemory(format.src << 2, 4, (char*)&src);
		else if ((format.op0 == 2 && format.op2 == 3)) // src is register mode
		{
			// src2 ID
			unsigned src_id;

			// Read src2 value
			src_id = format.src;
			src = ReadGPR(src_id);
		}

		// Negate
		if (format.src_negate == 1)
			src = -src;
		// Absolute value
		if (format.src_abs == 1)
			src = abs(src);

		// Execute
		if (format.s_fmt == 2 && format.d_fmt == 2) //Currently support 32 bit
			dst = src;
		else
			ISAUnsupportedFeature(inst);

		// Write Result
		dst_id = format.dst;
		WriteFloatGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_I2I_A(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesI2I format = inst_bytes.i2i;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id;

	// Operand
	int src, dst;

	// Execute
	if (active == 1 && pred == 1)
	{

		if (format.op0 == 1) // src2 is IMM20
			src = format.src >> 18 ? format.src | 0xfff80000 : format.src;

		// Negate
		if (format.src_negate == 1)
			src = -src;

		// Absolute Value
		if (format.src_abs == 1)
			src = abs(src);

		// Execute
		if (format.s_fmt == 2 && format.d_fmt == 2) //Currently support 32 bit
			dst = src;
		else
			ISAUnsupportedFeature(inst);

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_I2I_B(Instruction *inst)
{
	// Get Warp
	Emulator* emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesI2I format = inst_bytes.i2i;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id;

	// Operand
	int src, dst;

	// Execute
	if (active == 1 && pred == 1)
	{

		if ((format.op0 == 2) && (format.op2 == 1 )) // src is const
			emulator->ReadConstantMemory(format.src << 2, 4, (char*)&src);
		else if ((format.op0 == 2 && format.op2 == 3)) // src is register mode
		{
			// src2 ID
			unsigned src_id;

			// Read src2 value
			src_id = format.src;
			src = ReadGPR(src_id);
		}

		// Negate
		if (format.src_negate == 1)
			src = -src;
		// Absolute value
		if (format.src_abs == 1)
			src = abs(src);

		// Execute
		if (format.s_fmt == 2 && format.d_fmt == 2) //Currently support 32 bit
			dst = src;
		else
			ISAUnsupportedFeature(inst);

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_F2I_A(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesF2I format = inst_bytes.f2i;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id;

	// Operand
	float src;
	unsigned dst;

	// Execute
	if (active == 1 && pred == 1)
	{

		if (format.op0 == 1) // src2 is IMM20
		{
			src = format.src;
			if (format.s_fmt == 2) // FP32
				src = format.src << 12;
			else
				ISAUnsupportedFeature(inst); // FP16 or FP64 not supported yet
		}

		// Negate
		if (format.src_negate == 1)
			src = -src;

		// Absolute Value
		if (format.src_abs == 1)
			src = fabsf(src);

		// Execute
		if (format.s_fmt == 2 && format.d_fmt == 2) //Currently support 32 bit
		{
			if (format.round == 0)
				dst = roundf(src);
			else if (format.round == 1)
				dst = floorf(src);
			else if (format.round == 2)
				dst = ceilf(src);
			else if (format.round == 3)
				dst = truncf(src);
		}
		else
			ISAUnsupportedFeature(inst);

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_F2I_B(Instruction *inst)
{
	// Get Warp
	Emulator* emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesF2I format = inst_bytes.f2i;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id;

	// Operand
	float src;
	unsigned dst;


	// Execute
	if (active == 1 && pred == 1)
	{

		if ((format.op0 == 2) && (format.op2 == 1 )) // src is const
			emulator->ReadConstantMemory(format.src << 2, 4, (char*)&src);
		else if ((format.op0 == 2 && format.op2 == 3)) // src is register mode
		{
			// src ID
			unsigned src_id;

			// Read src value
			src_id = format.src;
			src = ReadFloatGPR(src_id);
		}

		// Negate
		if (format.src_negate == 1)
			src = -src;

		// Absolute Value
		if (format.src_abs == 1)
			src = fabsf(src);

		// Execute
		if (format.s_fmt == 2 && format.d_fmt == 2) //Currently support 32 bit
		{
			if (format.round == 0)
				dst = roundf(src);
			else if (format.round == 1)
				dst = floorf(src);
			else if (format.round == 2)
				dst = ceilf(src);
			else if (format.round == 3)
				dst = truncf(src);
		}
		else
			ISAUnsupportedFeature(inst);

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_F2F_A(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_F2F_B(Instruction *inst)
{
	// Get Warp
	Emulator* emulator = Emulator::getInstance();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesF2F format = inst_bytes.f2f;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id;

	// Operand
	float src;
	unsigned dst;

	// Execute
	if (active == 1 && pred == 1)
	{

		if (format.op2 == 1) // src is const
			emulator->ReadConstantMemory(format.src << 2, 4, (char*)&src);
		else if (format.op2 == 3) // src is register mode
		{
			// src ID
			unsigned src_id;

			// Read src value
			src_id = format.src;
			src = ReadFloatGPR(src_id);
		}

		// Absolute Value
		if (format.src_abs == 1)
			src = fabsf(src);

		// Negate
		if (format.src_negate == 1)
			src = -src;

		// Handle source denormal flush
		if ((format.s_fmt == 2) && (format.d_fmt != 3) && (format.ftz == 1))
		{
			if (std::fpclassify(src) == FP_SUBNORMAL)
				src = 0.0f;
		}

		// Execute
		if (format.s_fmt == 2 && format.d_fmt == 2) //Currently support 32 bit
		{
			if (format.pass == 0) // pass mode
				dst = src;
			else if (format.pass == 1) // round mode
			{
				if (format.round == 0)
					dst = roundf(src);
				else if (format.round == 1)
					dst = floorf(src);
				else if (format.round == 2)
					dst = ceilf(src);
				else if (format.round == 3)
					dst = truncf(src);
			}
		}
		else
			ISAUnsupportedFeature(inst);

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_LD(Instruction *inst)
{
	// Inst bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;
	Emulator* emulator = Emulator::getInstance();

	// Predicates and active masks

	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id;
	int dst[4];
	unsigned addr;
	unsigned data_type;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		if (format.op1 & 0x1) 	//offset direction
			addr = ReadGPR(format.mod0) - (format.mod1 << 19) - format.srcB;
		else
			addr = ReadGPR(format.mod0) + (format.mod1 << 19) + format.srcB;

		data_type = (format.op1 >> 2) & 0x7;

		// Execute
		if (addr > (emulator->getGlobalMemoryTotalSize() +
						emulator->getSharedMemoryTotalSize())) // Local Memory
		{
			unsigned local_mem_addr = addr - emulator->getGlobalMemoryTotalSize()
					- emulator->getSharedMemoryTotalSize() -
							id * local_memory_size;
			local_memory->Read(local_mem_addr, 4, (char*)dst);
		}
		else if (addr > emulator->getGlobalMemoryTotalSize()) // Shared  Memory
		{
			unsigned shared_memory_addr = addr -
						emulator->getGlobalMemoryTotalSize()
							- thread_block->getId() *
								thread_block->getSharedMemorySize();
			thread_block->ReadFromSharedMemory(shared_memory_addr, 4, (char*)dst);
		}
		else
			emulator->ReadGlobalMemory(addr, 4, (char*)dst); // Global Memory

		// Data type > 4
		if (data_type > 4)
		{
			if (addr > (emulator->getGlobalMemoryTotalSize() +
							emulator->getSharedMemoryTotalSize()))
			{
				unsigned local_mem_addr = addr -
						emulator->getGlobalMemoryTotalSize() -
							emulator->getSharedMemoryTotalSize() - id *
								local_memory_size;
				local_memory->Read(local_mem_addr + 4, 4, (char*)&dst[1]);
			}
			else if (addr > emulator->getGlobalMemoryTotalSize())
			{
				unsigned shared_memory_addr = addr -
							emulator->getGlobalMemoryTotalSize()
								- thread_block->getId() *
									thread_block->getSharedMemorySize();
				thread_block->ReadFromSharedMemory(shared_memory_addr, 4,
								(char*)&dst[1]);
			}
			else
				emulator->ReadGlobalMemory(addr + 4, 4, (char*)&dst[1]);
		}

		if (data_type > 5)										//Really? FIXME
		{
			if (addr > (emulator->getGlobalMemoryTotalSize() +
							emulator->getSharedMemoryTotalSize()))
			{
				unsigned local_mem_addr = addr -
						emulator->getGlobalMemoryTotalSize() -
							emulator->getSharedMemoryTotalSize() - id *
								local_memory_size;
				local_memory->Read(local_mem_addr + 8, 8, (char*)&dst[2]);
			}
			else if (addr > emulator->getGlobalMemoryTotalSize())
			{
				unsigned shared_memory_addr = addr -
							emulator->getGlobalMemoryTotalSize()
								- thread_block->getId() *
									thread_block->getSharedMemorySize();
				thread_block->ReadFromSharedMemory(shared_memory_addr, 4,
							(char*)&dst[2]);
			}
			else
				emulator->ReadGlobalMemory(addr + 8, 8, (char*)&dst[2]);
		}

		/* Write */
		dst_id = format.dst;
		WriteGPR(dst_id, dst[0]);


		if (data_type > 4)
			WriteGPR(dst_id + 1, dst[1]);
		if (data_type > 5)
		{
			WriteGPR(dst_id + 2, dst[2]);
			WriteGPR(dst_id + 3, dst[3]);
		}
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());

	if (getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<getWarpId() <<" LD op0 "<<format.op0;
		std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
				<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
				<<std::endl;
	}

}

void Thread::ExecuteInst_LDS(Instruction *inst)
{
	// Inst bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks

	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned src_id;
	unsigned long long src;
	unsigned dst_id;
	unsigned dst;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{

		dst_id = format.dst;
		dst = ReadGPR(dst_id);

		src_id = format.mod0;
		src = ReadGPR(src_id);

		// Execute
		// Write
		if (id_in_warp == 0 && getenv("M2S_KPL_ISA_DEBUG"))
		{
			std::cout << "LDS PC = " << std::hex << warp->getPC() << std::endl;
			std::cerr<<" dst " << std::hex <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
                 <<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
                 <<std::endl;

		}
		thread_block->ReadFromSharedMemory(src, sizeof(int), (char*)&dst);

		// write back
		WriteGPR(dst_id, dst);

		if (((format.mod1 >> 9) & 0x7) == 5)
		{
			src += sizeof(int);
			thread_block->ReadFromSharedMemory(src, sizeof(int), (char*)&dst);
			WriteGPR(dst_id + 1, dst);
		}

		// FIXME not verified
		if (((format.mod1 >> 9) & 0x7) == 6)
		{
			throw misc::Panic("Instruction LDS: 128 bit operation.\n");

			// if the panic above is met, remove the comment sign below
			/*
			src += sizeof(int);
			thread_block->readSharedMem(src, sizeof(int), (char*)&dst);
			WriteGPR(dst_id + 1, dst);

			src += sizeof(int);
			thread_block->readSharedMem(src, sizeof(int), (char*)&dst);
			WriteGPR(dst_id + 2, dst);
			src += sizeof(int);
			thread_block->readSharedMem(src, sizeof(int), (char*)&dst);
			WriteGPR(dst_id + 3, dst);
			*/
		}


	}


	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_LDC(Instruction *inst)
{
	// Inst byte format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesLDC format = inst_bytes.ldc;

	// Operands Type
	// int u_or_s = format.u_or_s;

	// Operands
	unsigned dst_id, srcA_id, srcB_id1;
	int srcB_id2; // srcB_id2 signed
	unsigned mem_addr;
	RegValue srcA, srcB, dst;

	// Predicates and active masks
	Emulator *emulator = Emulator::getInstance();

	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id -8);

	// Execute
	if (active == 1 && pred == 1)
	{
	    if (format.u_or_s == 4) // default case, unsigned 32
		{
			// Read
			// read GPR
			srcA_id = format.src1;

			// Read mem bank addr and offset
			srcB_id1 = format.src2_1;
			srcB_id2 = format.src2_2;   // FIXME

			// read register value
			srcA.s32 = ReadGPR(srcA_id);

			// Caculate mem_addr and read const mem
			mem_addr = srcB_id2 + srcA.s32 + (srcB_id1 << 16);
			emulator->ReadConstantMemory(mem_addr, 4, (char*)&srcB.u32);

			// Execute
			dst.u32 = srcB.u32;

			// Write
			dst_id = format.dst;
			WriteGPR(dst_id, dst.u32);
		}
	    else if (format.u_or_s == 5)
	    {
			// Read
			// read GPR
			srcA_id = format.src1;

			// Read mem bank addr and offset
			srcB_id1 = format.src2_1;
			srcB_id2 = format.src2_2;   // FIXME

			// read register value
			srcA.s32 = ReadGPR(srcA_id);

			// Caculate mem_addr and read const mem
			mem_addr = srcB_id2 + srcA.s32 + (srcB_id1 << 16);

			// Read the lower 32 bits
			emulator->ReadConstantMemory(mem_addr, 4, (char*)&srcB.u32);

			// Execute
			dst.u32 = srcB.u32;

			// Write
			dst_id = format.dst;
			WriteGPR(dst_id, dst.u32);

			// Read the upper 32 bits
			emulator->ReadConstantMemory(mem_addr + 4, 4, (char*)&srcB.u32);

			// Execute the upper 32 bits
			dst.u32 = srcB.u32;

			// Write the upper 32 bits
			WriteGPR(dst_id + 1, dst.u32);
	    }
		else
		{
			throw misc::Panic("Unsupported feature in Kepler LDC instruction\n"
						"128 bits LDC attempted.\n");
		}

	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());

	if (getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
                 <<getWarpId() <<" LDC op0 "<<format.op0;
        std::cerr<<" dst " <<format.dst <<" src1 " <<format.src1 << " pred " <<format.pred << " S " <<format.s
                 <<" src2_2 " <<format.src2_2 << " src2_1 "<< format.src2_1 <<" IS " <<format.is <<" u_or_s"
				 <<format.u_or_s << "op1" << format.op1 <<std::endl;
	}

}

void Thread::ExecuteInst_ST(Instruction *inst)
{
	// Inst bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;
	Emulator* emulator = Emulator::getInstance();

	// Predicates and active masks
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned src_id;
	int src[4];
	unsigned addr;
	unsigned data_type;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		if (format.op1 & 0x1) 	//offset direction
			addr = ReadGPR(format.mod0) - (format.mod1 << 19) - format.srcB;
		else
			addr = ReadGPR(format.mod0) + (format.mod1 << 19) + format.srcB;
		data_type = (format.op1 >> 2) & 0x7;
		src_id = format.dst;
		src[0] = ReadGPR(src_id);
		if (data_type > 4)
			src[1] = ReadGPR(src_id + 1);
		if (data_type > 5)										//Really? FIXME
		{
			src[2] = ReadGPR(src_id + 2);
			src[3] = ReadGPR(src_id + 3);
		}

		// Execute
		// Write
		if (addr > (emulator->getGlobalMemoryTotalSize() +
						emulator->getSharedMemoryTotalSize()))
		{
			unsigned local_mem_addr = addr - emulator->getGlobalMemoryTotalSize()
					- emulator->getSharedMemoryTotalSize() -
								id * local_memory_size;
			local_memory->Write(local_mem_addr, 4, (char*)src);
		}
		else if (addr > emulator->getGlobalMemoryTotalSize()) // Shared  Memory
		{
			unsigned shared_memory_addr = addr -
						emulator->getGlobalMemoryTotalSize()
							- thread_block->getId() *
								thread_block->getSharedMemorySize();
			thread_block->WriteToSharedMemory(shared_memory_addr, 4, (char*)src);
		}
		else
			emulator->WriteGlobalMemory(addr, 4, (char*)src);

		// Data type > 4
		if (data_type > 4)
		{
			if (addr > (emulator->getGlobalMemoryTotalSize() +
							emulator->getSharedMemoryTotalSize()))
			{
				unsigned local_mem_addr = addr -
						emulator->getGlobalMemoryTotalSize() -
							emulator->getSharedMemoryTotalSize() - id *
								local_memory_size;
				local_memory->Write(local_mem_addr + 4, 4, (char*)&src[1]);
			}
			else if (addr > emulator->getGlobalMemoryTotalSize()) // Shared  Memory
			{
				unsigned shared_memory_addr = addr -
							emulator->getGlobalMemoryTotalSize()
								- thread_block->getId() *
									thread_block->getSharedMemorySize();
				thread_block->WriteToSharedMemory(shared_memory_addr, 4,
								(char*)&src[1]);
			}
			else
				emulator->WriteGlobalMemory(addr + 4, 4, (char*)&src[1]);
		}

		// Data type > 5
		if (data_type > 5)										//Really? FIXME
		{
			if (addr > (emulator->getGlobalMemoryTotalSize() +
							emulator->getSharedMemoryTotalSize()))
			{
				unsigned local_mem_addr = addr -
						emulator->getGlobalMemoryTotalSize() -
							emulator->getSharedMemoryTotalSize() - id *
								local_memory_size;
				local_memory->Write(local_mem_addr + 8, 8, (char*)&src[2]);
			}
			else if (addr > emulator->getGlobalMemoryTotalSize()) // Shared  Memory
			{
				unsigned shared_memory_addr = addr -
							emulator->getGlobalMemoryTotalSize()
								- thread_block->getId() *
									thread_block->getSharedMemorySize();
				thread_block->WriteToSharedMemory(shared_memory_addr, 4,
								(char*)&src[2]);
			}
			else
				emulator->WriteGlobalMemory(addr + 8, 8, (char*)&src[2]);
		}
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());

	if (getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
                 <<getWarpId() <<" ST op0 "<<format.op0;
        std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
                 <<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
                 <<std::endl;
	}
}

void Thread::ExecuteInst_STS(Instruction *inst)
{
	// Inst bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned src_id;
	unsigned src;
	unsigned dst_id;
	unsigned dst;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);


	// Execute

	if (active == 1 && pred == 1)
	{
		// Read, note that dst and src are changed with each other
		// comparing with other instructions
		dst_id = format.mod0;
		dst = ReadGPR(dst_id);

		src_id = format.dst;
		src = ReadGPR(src_id);

		if (id_in_warp == 0 && getenv("M2S_KPL_ISA_DEBUG"))
		{
			std::cerr << "PC = " << std::hex << warp->getPC() << std::endl;
        std::cerr<<" dst " << std::hex <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
                 <<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
                 <<std::endl;
		}
		// Execute
		// Write

		thread_block->WriteToSharedMemory(dst, sizeof(int), (char*)&src);

		if (((format.mod1 >> 9) & 0x7) == 5)
		{
			src = ReadGPR(src_id + 1);
			thread_block->WriteToSharedMemory(dst + sizeof(int),
					sizeof(int), (char*)&src);
		}

		// FIXME not verified
		if (((format.mod1 >> 9) & 0x7) == 6)
		{
			throw misc::Panic("Instruction STS: 128 bit operation.\n");
			// if the panic above is met, remove the comment sign below
			/*
			src = ReadGPR(src_id + 1);
			dst += sizeof(int);

			thread_block->writeSharedMem(dst, sizeof(int), (char*)&src);

			src = ReadGPR(src_id + 2);
			dst += sizeof(int);

			thread_block->writeSharedMem(dst, sizeof(int), (char*)&src);

			src = ReadGPR(src_id + 3);
			dst += sizeof(int);
			thread_block->writeSharedMem(dst, sizeof(int), (char*)&src);
			*/
		}
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());

}

void Thread::ExecuteInst_DADD(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_FMUL(Instruction *inst)
{
	// Get emulator
	Emulator* emulator = Emulator::getInstance();

	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();


	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src_id;

	// Operand
	float src1, src2, dst;
	union
	{
		unsigned i;
		float f;
	} gpr_t;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		src_id = format.mod0;
		src1 = ReadFloatGPR(src_id);

		if (((format.mod1 >> 3) & 0x1) == 1)
			src1 = fabsf(src1);
		if (((format.mod1 >> 5) & 0x1) == 1)
			src1 = -src1;

		src_id = format.srcB;

		if (format.srcB_mod == 0)
		{
			emulator->ReadConstantMemory(src_id << 2, 4, (char*)&src2);
		}
		else if (format.srcB_mod == 1 || format.srcB_mod == 2)
			src2 = ReadFloatGPR(src_id);
		else 	// check it
		{
			gpr_t.i = format.srcB << 12;
			src2 = gpr_t.f;
		}

		if (((format.mod1 >> 2) & 0x1) == 1)
			src2 = fabsf(src2);
		if (((format.mod1 >> 4) & 0x1) == 1)
			src2 = -src2;

		// Execute
		dst = src1 * src2;

		/* Write */
		dst_id = format.dst;
		WriteFloatGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}


/* void Thread::ExecuteInst_FADD(Instruction *inst)
{
	// Get emulator
	Emulator* emulator = Emulator::getInstance();

	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();


	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src_id;

	// Operand
	float src1, src2, dst;
	union
	{
		unsigned i;
		float f;
	} gpr_t;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		src_id = format.mod0;
		src1 = ReadFloatGPR(src_id);

		if (((format.mod1 >> 3) & 0x1) == 1)
			src1 = fabsf(src1);
		if (((format.mod1 >> 5) & 0x1) == 1)
			src1 = -src1;

		src_id = format.srcB;

		if (format.srcB_mod == 0)
		{
			emulator->ReadConstantMemory(src_id << 2, 4, (char*)&src2);
		}
		else if (format.srcB_mod == 1 || format.srcB_mod == 2)
			src2 = ReadFloatGPR(src_id);
		else 	// check it
		{
			gpr_t.i = format.srcB << 12;
			src2 = gpr_t.f;
		}

		if (((format.mod1 >> 2) & 0x1) == 1)
			src2 = fabsf(src2);
		if (((format.mod1 >> 4) & 0x1) == 1)
			src2 = -src2;

		// Execute
		dst = src1 + src2;

		// Write
		dst_id = format.dst;
		WriteFloatGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}
*/
void Thread::ExecuteInst_FADD_A(Instruction *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_FADD_B(Instruction *inst)
{
	// Get emulator
	Emulator* emulator = Emulator::getInstance();

	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesFADD format = inst_bytes.fadd;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand
	unsigned src1_id;
	float src1, src2, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read Src1
		src1_id = format.src1;
		src1 = ReadFloatGPR(src1_id);

		// Read Src2
		if (format.op2 == 1)
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);
		else if (format.op2 == 3)
		{
			unsigned src2_id;
			src2_id = format.src2;
			src2 = ReadFloatGPR(src2_id);
		}

		// Src1 abs
		if (format.src1_abs == 1)
			src1 = fabsf(src1);

		// Src1 negate
		if (format.src1_negate == 1)
			src1 = -src1;

		// Src2 abs
		if (format.src2_abs == 1)
			src2 = fabsf(src2);

		// Src2 negate
		if (format.src2_negate == 1)
			src2 = -src2;

		if (format.ftz == 1)
		{
			if (std::fpclassify(src1) == FP_SUBNORMAL)
				src1 = 0.0f;
			if (std::fpclassify(src1) == FP_SUBNORMAL)
				src2 = 0.0f;
		}


		// Execute
		dst = src1 + src2;

		// Ftz for dst
		if (format.ftz == 1)
		{
			if (std::fpclassify(dst) == FP_SUBNORMAL)
				dst = 0.0f;
		}

		// Round mode
		/*
		if (format.round == 0)
			dst = roundf(dst);
		else if (format.round == 1)
			dst = floorf(dst);
		else if (format.round == 2)
			dst = ceilf(dst);
		else if (format.round == 3)
			dst = truncf(dst);
		*/

		// Saturate mode
		if(format.sat == 1)
			this->ISAUnsupportedFeature(inst);

		// Conditional code
		if(format.cc == 1)
			this->ISAUnsupportedFeature(inst);

		// Write result
		unsigned dst_id;
		dst_id = format.dst;
		WriteFloatGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_MUFU(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesMUFU format = inst_bytes.mufu;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned src_id, dst_id;

	// Operand
	float src, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read src2 value
		src_id = format.src;
		src = ReadFloatGPR(src_id);

		// Negate
		if (format.src_negate == 1)
			src = -src;

		// Absolute value
		if (format.src_abs == 1)
			src = abs(src);

		// Execute
		if (format.mufu_op == 4) // RCP mode
			dst = 1.0f / src;
		else
			ISAUnsupportedFeature(inst);

		// Write Result
		dst_id = format.dst;
		WriteFloatGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_FFMA_A(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_FFMA_B(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesFFMA format = inst_bytes.ffma;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned src1_id, dst_id;

	// Operand
	float src1, src2, src3, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read src1 value
		src1_id = format.src1;
		src1 = ReadFloatGPR(src1_id);

		// Read src2 and src3
		if (format.op2 == 1) // src2 is const src3 is register
		{
			Emulator *emulator = Emulator::getInstance();
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);
			unsigned src3_id;
			src3_id = format.src3;
			src3 = ReadFloatGPR(src3_id);
		}
		else if (format.op2 == 2) // src2 is register src3 is const
		{
			unsigned src2_id;
			src2_id = format.src3; // format.src3 is for register mode
			src2 = ReadFloatGPR(src2_id);
			Emulator *emulator = Emulator::getInstance();
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src3);
		}
		else if (format.op2 == 3) // both src2 and src3 are register mode
		{
			unsigned src2_id, src3_id;
			src2_id = format.src2;
			src2 = ReadFloatGPR(src2_id);
			src3_id = format.src3;
			src3 = ReadFloatGPR(src3_id);
		}

		if(format.fmz == 1)
			ISAUnsupportedFeature(inst);

		// Multiply src1 and src2
		float temp;
		temp = src1 * src2;

		// Negate
		if (format.negate_ab == 1)
			temp = -temp;
		else if (format.negate_c == 1)
			src3 = -src3;

		// Add src3
		temp += src3;
		dst = temp;

		// Round mode
		/*
		if (format.round == 0)
			dst = roundf(temp);
		else if (format.round == 1)
			dst = floorf(temp);
		else if (format.round == 2)
			dst = ceilf(temp);
		else if (format.round == 3)
			dst = truncf(temp);
*/
		// Saturate
		if (format.sat == 1)
			ISAUnsupportedFeature(inst);

		// Write Result
		dst_id = format.dst;
		WriteFloatGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_FSET_A(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_FSET_B(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesFSET format = inst_bytes.fset;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;

	// Predicate register
	unsigned pred;

	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned src1_id, dst_id, pred_src_id;

	// Operand
	float src1, src2, dst;
	unsigned pred_src;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read Src1
		src1_id = format.src1;
		src1 = ReadFloatGPR(src1_id);

		// Read Src2
		if (format.op2 == 1) // src is const
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);
		else if (format.op2 == 3) // src is register mode
		{
			// src2 ID
			unsigned src2_id;

			// Read src2 value
			src2_id = format.src2;
			src2 = ReadFloatGPR(src2_id);
		}

		// Absolute Value for src1
		if (format.src1_abs == 1)
			src1 = fabsf(src1);

		// Negate src1
		if (format.src1_negate == 1)
			src1 = -src1;

		// Absolute Value for src2
		if (format.src2_abs == 1)
			src2 = fabsf(src2);

		// Negate src2
		if (format.src2_negate == 1)
			src2 = -src2;

		// Read Predicate Src
		pred_src_id = format.pred_src;
		if (pred_src_id <= 7)
			pred_src = ReadPredicate(pred_src_id);
		else
			pred_src = !ReadPredicate(pred_src_id - 8);

		// Read Boolean mask .BM is default(.bval=0) .BF(.bavl=1)
		unsigned mask_value = (format.bval == 1) ? 0x3f800000 : 0xffffffff;

		// Compare operation
        unsigned cmp_op = format.comp;
        bool cmp_result;

 		if (cmp_op == 0)
			cmp_result = false;
		else if (cmp_op == 1)
			cmp_result = ((src1 < src2) && (!std::isnan(src1)) && (!std::isnan(src2)));
		else if (cmp_op == 2)
			cmp_result = ((src1 == src2) && (!std::isnan(src1)) && (!std::isnan(src2)));
		else if (cmp_op == 3)
			cmp_result = ((src1 <= src2) && (!std::isnan(src1)) && (!std::isnan(src2)));
		else if (cmp_op == 4)
			cmp_result = ((src1 > src2) && (!std::isnan(src1)) && (!std::isnan(src2)));
		else if (cmp_op == 5)
			cmp_result = ((src1 != src2) && (!std::isnan(src1)) && (!std::isnan(src2)));
		else if (cmp_op == 6)
			cmp_result = ((src1 >= src2) && (!std::isnan(src1)) && (!std::isnan(src2)));
		else if (cmp_op == 7)
			cmp_result = ((!std::isnan(src1)) && (!std::isnan(src2)));
		else if (cmp_op == 8)
			cmp_result = ((std::isnan(src1)) && (std::isnan(src2)));
		else if (cmp_op == 9)
			cmp_result = ((src1 < src2) && (std::isnan(src1)) && (std::isnan(src2)));
		else if (cmp_op == 10)
			cmp_result = ((src1 == src2) && (std::isnan(src1)) && (std::isnan(src2)));
		else if (cmp_op == 11)
			cmp_result = ((src1 <= src2) && (std::isnan(src1)) && (std::isnan(src2)));
		else if (cmp_op == 12)
			cmp_result = ((src1 > src2) && (std::isnan(src1)) && (std::isnan(src2)));
		else if (cmp_op == 13)
			cmp_result = ((src1 != src2) && (std::isnan(src1)) && (std::isnan(src2)));
		else if (cmp_op == 14)
			cmp_result = ((src1 >= src2) && (std::isnan(src1)) && (std::isnan(src2)));
		else if (cmp_op == 15)
			cmp_result = true;

		// Boolean operation
		unsigned bool_op = format.bop;
		if (bool_op == 0) // AND
			dst = (cmp_result && pred_src) ? mask_value : 0;
		else if (bool_op == 1) // OR
			dst = (cmp_result || pred_src) ? mask_value : 0;
		else if (cmp_op == 2) // XOR
			dst = (cmp_result ^ pred_src) ? mask_value : 0;

		if (format.cc == 1)
			ISAUnsupportedFeature(inst);

		// Execute
		dst_id = format.dst;
		dst = pred_src ? src1 : src2;
		WriteFloatGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_NOP(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesNOP format = inst_bytes.nop;

	// Get Operand
	unsigned s = format.s;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	active = 1u & (stack->getActiveMask() >> id_in_warp);

	if (active == 1 && s == 1)
	{

	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_S2R(Instruction *inst)
{
	// Inst bytes format	//FIXME S2R description missing in Inst.h
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst, src;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = format.srcB & 0xff;
		if (src_id == SR_CLOCKLO)	//No cycle count for now
			;//src = grid->emulator->inst_count & 0xffffffff;
		else if (src_id == SR_CLOCKHI)
			;//src = (grid->emulator->inst_count >> 32) & 0xffffffff;
		else if (format.srcB_mod == 1)
			src = ReadSpecialRegister(src_id);

		// Execute
		dst = src;

		// Write
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());

	if (getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<getWarpId() <<" S2R op0 "<<format.op0;
		std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
				<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
				<<std::endl;
	}

}

void Thread::ExecuteInst_PSETP(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesPSETP format = inst_bytes.psetp;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, srcA_id, srcB_id, srcC_id;

	// Operands
	unsigned srcA, srcB, srcC, dst;

	// Bool operations of the instruction.
	unsigned bool_op0, bool_op1;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read SrcA id
		srcA_id = format.pred2;

		// Get SrcA value
		if (srcA_id <= 7)
			srcA = ReadPredicate(srcA_id);
		else
			srcA = !ReadPredicate(srcA_id - 8);

		// Read SrcB id
		srcB_id = format.pred3;

		// Get SrcB value
		if (srcB_id <= 7)
			srcB = ReadPredicate(srcB_id);
		else
			srcB = !ReadPredicate(srcB_id - 8);

		// Read SrcC id
		srcC_id = format.pred4;

		// Get SrcC value
		if (srcC_id <= 7)
			srcC = ReadPredicate(srcC_id);
		else
			srcC = !ReadPredicate(srcC_id - 8);

		// Get Opcode
		bool_op0 = format.bool_op0;
		bool_op1 = format.bool_op1;

		// temp value
		int temp;

		// Execute bool opcode0
		if (bool_op0 == 0) // And operation for srcA and srcB
			temp = srcA && srcB;
		else if (bool_op0 == 1) // Or operation
			temp = srcA || srcB;
		else if (bool_op0 == 3) // Xor operation
			temp = srcA ^ srcB;

		// Execute bool opcode1
		if (bool_op1 == 0) // And operation for temp and srcC
			dst = temp && srcC;
		else if (bool_op1 == 1)
			dst = temp || srcC; // Or operation
		else if (bool_op1 == 3)
			dst = temp ^ srcC; // Xor operation

		// Write Result
		dst_id = format.pred0;
		WritePredicate(dst_id, dst);

	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_SHF(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_BAR(Instruction *inst)
{

	// Inst bytes format	//FIXME S2R description missing in Inst.h
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;


	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// FIXME naive here
	}

	if (id_in_warp == warp->getThreadCount() - 1)
	{
		warp->setAtBarrier(true);
		thread_block->incWarpsAtBarrier();
		/*
		std::cout << "at barrier inc at warp id " << warp->getId();
		std::cout << " to " << thread_block->getWarpsAtBarrier()
				<< "warp count is " << thread_block->getWarpCount()<< std::endl;
				*/

		if (thread_block->getNumWarpsAtBarrier()
				== thread_block->getWarpCount())
		{
			//std::cout << " num at barrier " << thread_block->getWarpsAtBarrier()
			//		<< std::endl;
			thread_block->clearWarpAtBarrier();
			thread_block->setWarpsAtBarrier(0);
		}
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());

	if (getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<getWarpId() <<" S2R op0 "<<format.op0;
		std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
				<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
				<<std::endl;
	}
}

void Thread::ExecuteInst_BPT(Instruction *inst)
{
	if (id_in_warp == warp->getThreadCount() - 1)
		warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_JMX(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_JMP(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_JCAL(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_BRX(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_CAL(Instruction *inst)
{
	// Inst bytes format	//FIXME S2R description missing in Inst.h
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesCAL format = inst_bytes.cal;

	// Predicates and active masks
	SyncStack* stack = warp->getSyncStack()->get();

	// Get instruction offset
	int offset = format.offset >> 23 ?
						format.offset | 0xff000000 : format.offset;

	unsigned pred;
	unsigned pred_id;
	unsigned active;


	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		//stack->Dump();
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// FIXME naive here
	}

	if (id_in_warp == warp->getThreadCount() - 1)
	{
		if (stack->getActiveMask())
		{
			std::unique_ptr<SyncStack> sync_stack(
					new SyncStack(warp->getThreadCount()));
			sync_stack->setActiveMask(stack->getActiveMask());

			warp->getReturnAddressStack()->get()->push(warp->getPC() + 8,
					stack->getActiveMask(), sync_stack);

			warp->setTargetPC(warp->getPC() + offset + warp->getInstructionSize());
		}
		else
		{
			warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
		}
	}

	if (getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<getWarpId() <<" CAL op0 "<<format.op0 << " offset "
				<<format.offset << " op1 "<< format.op1 <<std::endl;
	}
}


void Thread::ExecuteInst_PRET(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_PLONGJMP(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_SSY(Instruction *inst)
{
	// Get emulator
    Emulator *emulator = Emulator::getInstance();

	// Get synchronization stack
	SyncStack* stack = warp->getSyncStack()->get();

    unsigned address;
    unsigned pc;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Instbytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesSSY format = inst_bytes.ssy;

	// Operand
	unsigned offset;
	unsigned isconstmem;

	// Read Operand constmem
	isconstmem = format.isconstmem;

	offset = format.offset;

	// Execute at the last thread in warp
	if (id_in_warp == warp->getThreadCount() - 1)
	{
		if (isconstmem == 0)
		{
			// Push SSY instruction into stack and set the reconvergence address
			pc = warp->getPC();

			address = offset + pc + warp->getInstructionSize();
		}
		else
        {
			// check this
			if (isconstmem == 1)
              	emulator->ReadConstantMemory(offset << 2,4, (char*) &address);
        }

		stack->push(address,
					stack->getActiveMask(),
					SyncStackEntrySSY);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_PBK(Instruction *inst)
{
	// Get synchronization stack
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned address;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesPBK format = inst_bytes.pbk;


	// Execute
    if (id_in_warp == warp->getThreadCount() - 1)
    {
        //Check this
        if (format.constant)
        {
                //emulator->ReadConstMem(format.offset << 2, sizeof(address), (char*) &address);
        		throw misc::Panic("getting address from constant memory is "
        				"not supported.\n");
        }
        else
        {
        // Get operand value
                if (format.offset >> 23 == 0)
                    address = format.offset + warp->getPC() + warp->getInstructionSize();
                else
                        //means offset is negative value
                        throw misc::Panic("Negative PKB address offset."
                                        "Not supported.");
        }

    	// Push the address and current active mask into stack
		stack->push(address,
					stack->getActiveMask(),
					SyncStackEntryPBK);
    }

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}


void Thread::ExecuteInst_PCNT(Instruction *inst)
{
	// Get synchronization stack
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned address;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesPCNT format = inst_bytes.pcnt;


	// Execute
    if (id_in_warp == warp->getThreadCount() - 1)
    {
        //Check this
        if (format.constant)
        {
                //emulator->ReadConstMem(format.offset << 2, sizeof(address), (char*) &address);
        		throw misc::Panic("getting address from constant memory is "
        				"not supported.\n");

        }
        else
        {
        // Get operand value
                if (format.offset >> 23 == 0)
                    address = format.offset + warp->getPC() + warp->getInstructionSize();
                else
                        //means offset is negative value
                        throw misc::Panic("Negative PCNT address offset."
                                        "Not supported.");
        }

    	// Push the address and current active mask into stack
		stack->push(address,
					0,
					SyncStackEntryPCNT);
    }

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_GETCRSPTR(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_GETLMEMBASE(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_SETCRSPTR(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_SETLMEMBASE(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_LONGJMP(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_RET(Instruction *inst)
{

	// Inst bytes format	//FIXME S2R description missing in Inst.h
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesGeneral2 format = inst_bytes.general2;

	// Predicates and active masks
	SyncStack* stack = warp->getSyncStack()->get();
	ReturnAddressStack* ret_stack = warp->getReturnAddressStack()->get();

	unsigned temp_am;
	unsigned return_addr;

	unsigned pred;
	unsigned pred_id;
	unsigned active;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{

		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Predicate
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// FIXME naive here
	}


	if (id_in_warp == warp->getThreadCount() - 1)
	{
		return_addr = ret_stack->getReturnAddress();
		temp_am = ret_stack->getActiveMask();

		assert(temp_am == stack->getActiveMask());

		if (!ret_stack->pop())
		{
			std::cerr << "Get address" << std::hex << return_addr << std::endl;
			ret_stack->Dump(std::cerr);
			throw misc::Panic("Instruction RET: Return Address Stack Empty\n");
		}

		/*
        std::cout << "RET PC " << std::hex << warp->getPC()
        		<< "	A.M.	" << stack->getActiveMask()<< std::endl;
        stack->Dump(std::cout);
        */
        warp->setTargetPC(return_addr);
	}

	if (getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr << "RET Warp id "<< std::hex
				<<getWarpId() <<" op0 "<<format.op0
				<< " offset " <<format.mod << " mod "<< format.op1 << std::endl;
		std::cerr << "From " << std::hex << warp->getPC() << " to "
				<< return_addr << std::endl;
	}

	//ISAUnimplemented(inst);
}

void Thread::ExecuteInst_KIL(Instruction *inst)
{
	ISAUnimplemented(inst);
}


void Thread::ExecuteInst_BRK(Instruction *inst)
{
	// Get synchronization stack
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;
	unsigned active_mask;

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active_mask = stack->getActiveMask();
	active = 1u & (active_mask >> id_in_warp);

	// Instruction bytes foramt
	Instruction::Bytes inst_bytes =inst->getInstBytes();
	Instruction::BytesBRK format = inst_bytes.brk;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);


	// Execute
	if (active == 1 && pred == 1)
	{
		// Clear the specific bit in active masks of all stack entries
		// above the latest PBK entry (there must be at least one)
		// To mask the break thread and prevent all active mask recoveries
		stack->mask(id_in_warp, SyncStackMaskBRK);

		// Clear current active mask.
		stack->clearActiveMaskBit(id_in_warp);

	}

	// Update target PC
	if (id_in_warp == warp->getThreadCount() - 1)
	{
		unsigned PBK_addr = 0;

		// Check if all threads in loop break or not.
		if (stack->checkBRK(PBK_addr) && stack->getActiveMask() == 0)
		{
			// update pc
			warp->setTargetPC(PBK_addr);

			// Pop all entries in stack until the first PBK
			stack->popTillTarget(PBK_addr, warp->getPC());
		}
		else
		{
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
		}
	}
}


void Thread::ExecuteInst_CONT(Instruction *inst)
{
	// Get synchronization stack
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Predicate
	unsigned pred_id;
	unsigned pred;

	// Instruction bytes foramt
	Instruction::Bytes inst_bytes =inst->getInstBytes();
	Instruction::BytesCONT format = inst_bytes.cont;

	// Get current PC
	unsigned pc = warp->getPC();

	//unsigned target_pc;

	// Get Instruction size
	unsigned inst_size = warp->getInstructionSize();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id-8);

	// Set the latest PCNT active thread mask
	// Clear active mask bit of specific all entries before the PCNT
	if (active == 1 && pred == 1)
	{
		stack->mask(id_in_warp, SyncStackMaskCONT);

		// Clear current active mask.
		stack->clearActiveMaskBit(id_in_warp);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
	{
		// Update PC
		unsigned target_pc = 0;

        // If all thread in loop taken CONT, pc jumps to PCNT address,
		// and then pop all entries before the target pc.
		if (stack->checkCONT(target_pc))
		{
			warp->setTargetPC(target_pc);
			stack->popTillTarget(target_pc, pc);
		}
		else
			warp->setTargetPC(pc + inst_size);
	}
}

void Thread::ExecuteInst_RTT(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_SAM(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_RAM(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_IDE(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_LOP32I(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesLOP32I format = inst_bytes.lop32i;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned src1_id, dst_id;

	// Operand
	unsigned src1, src2, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read Src1
		src1_id = format.src1;
		src1 = ReadGPR(src1_id);

		// Read Src2 IMM32
		src2 = format.src2;

		// Execute
		if (format.src1_negate == 1)
			src1 = ~src1;
		if (format.src2_negate == 1)
			src2 = ~src2;
		if (format.lop == 0)
			dst = src1 & src2;
		else if (format.lop == 1)
			dst = src1 | src2;
		else if (format.lop == 2)
			dst = src1 ^ src2;
		else if (format.lop == 3)
			dst = src2;

		if (format.cc == 1)
			ISAUnsupportedFeature(inst);

		// Write Result
		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_FADD32I(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_FFMA32I(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_IMAD32I(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_ISCADD32I(Instruction *inst)
{
	ISAUnimplemented(inst);
}

void Thread::ExecuteInst_SHL_A(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesSHL format = inst_bytes.shl;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, srcA_id; //srcB_id to be added for register

	// Operands
	unsigned srcA, srcB, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read SrcA id
		srcA_id = format.src1;

		// Get SrcA value
		srcA = ReadGPR(srcA_id);

		// Read SrcB
		if ((format.op2 == 3) && (format.op0 == 1)) // src2 is immediate value
		{
			srcB = format.src2;

			// Shift mode
			if ((format.mode == 0) && (srcB > 32)) // Clamp(default)
				srcB = 32;
			else if (format.mode == 1) // Wrap
				srcB &= 0x1f;
		}
		else
		{
			throw misc::Panic("Unsupported feature in Kepler SHL_A instruction");
		}

		if (format.cc == 1)
			ISAUnsupportedFeature(inst);

		// Read destination id
		dst_id = format.dst;

		// Calculate result
		dst = srcA << srcB;

		// Write the value to destination register
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_SHL_B(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesSHL format = inst_bytes.shl;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id;

	// Operands
	unsigned src1, src2, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read Src1 id
		src1_id = format.src1;

		// Get Src1 value
		src1 = ReadGPR(src1_id);

		// Read SrcB
		if (format.op2 == 1) // src2 is constant mode
		{
			// Get emulator instance
			Emulator *emulator = Emulator::getInstance();

			// Read src2
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);
		}
		else if (format.op2 == 3) // src2 is register mode
		{
			unsigned src2_id;
			src2_id = format.src2;
			src2 = ReadGPR(src2_id);
		}

		// Shift mode
		if ((format.mode == 0) && (src2 > 32)) // Clamp(default)
			src2 = 32;
		else if (format.mode == 1) // Wrap
			src2 &= 0x1f;

		// Conditional code
		if (format.cc == 1)
			ISAUnsupportedFeature(inst);

		// Read destination id
		dst_id = format.dst;

		// Calculate result
		dst = src1 << src2;

		// Write the value to destination register
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_SHR_A(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesSHR format = inst_bytes.shr;

	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;

	// Predicate register
	unsigned pred;

	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id; //srcB_id to be added for register

	// Operands
	unsigned src1, src2, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read SrcA id
		src1_id = format.src1;

		// Get SrcA value
		src1 = ReadGPR(src1_id);

		// Read SrcB
		if ((format.op2 == 3) && (format.op0 == 1)) // src2 is immediate value
			src2 = format.src2;

		if ((format.shift_mode == 0) && (src2 > 32)) // .C Clamp mode default
			src2 = 32;
		else if (format.shift_mode == 1)
			src2 &= 0x001f;

		if (format.bit_reverse == 1)
			src1 ^= 0xffffffff;

		// Calculate result
		if (format.shift_mode == 1) // arithmatic shift
			dst = (int)src1 >> src2;
		else if (format.shift_mode == 0) // logic shift
			dst = src1 >> src2;

		if (format.cc == 1)
			ISAUnsupportedFeature(inst);

		// Read destination id
		dst_id = format.dst;

		// Write the value to destination register
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

void Thread::ExecuteInst_SHR_B(Instruction *inst)
{
	// Get Warp
	SyncStack* stack = warp->getSyncStack()->get();

	// Determine whether the warp reaches reconvergence pc.
	// If it is, pop the synchronization stack top and restore the active mask
	// Only effect on thread 0 in warp
	if ((id_in_warp == 0) && warp->getPC())
	{
		unsigned temp_am;
		if (stack->pop(warp->getPC(), temp_am))
				stack->setActiveMask(temp_am);
	}

	// Active
	unsigned active = 1u & (stack->getActiveMask() >> id_in_warp);

	// Instruction bytes format
	Instruction::Bytes inst_bytes = inst->getInstBytes();
	Instruction::BytesSHR format = inst_bytes.shr;

	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;

	// Predicate register
	unsigned pred;

	if (pred_id <= 7)
		pred = ReadPredicate(pred_id);
	else
		pred = !ReadPredicate(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id; //srcB_id to be added for register

	// Operands
	unsigned src1, src2, dst;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read SrcA id
		src1_id = format.src1;

		// Get SrcA value
		src1 = ReadGPR(src1_id);

		// Read Src2
		if (format.op2 == 1) // src2 is const mode
		{
			// Get emulator instance
			Emulator *emulator = Emulator::getInstance();

			// Read src2
			emulator->ReadConstantMemory(format.src2 << 2, 4, (char*)&src2);
		}
		else if (format.op2 == 3) // src2 is register mode
		{
			// Get src2 ID
			unsigned src2_id;
			src2_id = format.src2;

			// Read src2
			src2 = ReadGPR(src2_id);
		}

		if ((format.shift_mode == 0) && (src2 > 32)) // .C Clamp mode default
			src2 = 32;
		else if (format.shift_mode == 1)
			src2 &= 0x001f;

		if (format.bit_reverse == 1)
			src1 ^= 0xffffffff;

		// Calculate result
		if (format.shift_mode == 1) // arithmatic shift
			dst = (int)src1 >> src2;
		else if (format.shift_mode == 0) // logic shift
			dst = src1 >> src2;

		if (format.cc == 1)
			ISAUnsupportedFeature(inst);

		// Read destination id
		dst_id = format.dst;

		// Write the value to destination register
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetPC(warp->getPC() + warp->getInstructionSize());
}

}	// namespace Kepler
