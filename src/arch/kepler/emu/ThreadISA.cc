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

#include <arch/kepler/asm/Inst.h>
#include <lib/cpp/Bitmap.h>
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <memory/memory.h>

#include "Emu.h"
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
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
}


void Thread::ExecuteInst_IMUL_A(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Warp* warp = getWarp();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst;
	int srcA, srcB;

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
		pred = ReadPred(pred_id);
	else
		pred = ! ReadPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		src_id = format.mod0;
		srcA = ReadGPR(src_id);
		src_id = format.srcB;

		if (format.op0 == 1  && format.op1 == 0x107 && format.srcB_mod)
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;

		else if (format.op0 == 2 && format.op1 == 0x187 && format.srcB_mod)
			srcB = ReadGPR(src_id);

		else	//check it
			ISAUnimplemented(inst);

		// Execute
		dst = srcA * srcB;

		// Write

		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());


}

void Thread::ExecuteInst_IMUL_B(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Warp* warp = getWarp();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst;
	int srcA, srcB;

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
		pred = ReadPred(pred_id);
	else
		pred = ! ReadPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		src_id = format.mod0;
		srcA = ReadGPR(src_id);
		src_id = format.srcB;

		if (format.op0 == 1  && format.op1 == 0x107 && format.srcB_mod)
		{
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;
		}
		else if (format.op0 == 2 && format.op1 == 0x187 && format.srcB_mod)
			srcB = ReadGPR(src_id);
		else	//check it
			ISAUnimplemented(inst);

		// Execute
		dst = srcA * srcB;

		// Write

		dst_id = format.dst;
		WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
}

void Thread::ExecuteInst_ISCADD_A(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst;
	int srcA, srcB;
	int shamt;

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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = format.mod0;
		srcA = this->ReadGPR(src_id);

		if (((format.mod1 >> 6) & 0x3) == 2)	//FIXME
			srcA = -srcA;
		src_id = format.srcB;
		if (format.srcB_mod == 0)
			emu->ReadConstMem(src_id << 2, 4, (char*)&srcB);
		else if (format.srcB_mod == 1)
			srcB = this->ReadGPR(src_id);
		else
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;

		if (((format.mod1 >> 6) & 0x3) == 1)	//FIXME
			srcB = -srcB;
		shamt = format.mod1 & 0xf; //45:42

		/* Execute */
		dst = (srcA << shamt) + srcB;

		/* Write */
		dst_id = format.dst;
		this->WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<this->getWarpId() <<" ISCADD op0 "<<format.op0;
		std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB

				<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
				<<std::endl;
	}
}


void Thread::ExecuteInst_ISCADD_B(Inst *inst)
{
	std:: cerr <<"ISCADD B"<<std::endl;

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
	ISAUnsupportedFeature(inst);
}

void Thread::ExecuteInst_IMAD(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

	if( active == 1 && pred == 1)
	{
		// Read
		src_id = format.mod0;
		srcA = this->ReadGPR(src_id);
		src_id = format.mod1 & 0xff;
		src3 = this->ReadGPR(src_id);
		if (format.srcB_mod == 0)
		{
			src_id = format.srcB;
			emu->ReadConstMem(src_id << 2, 4, (char*)&srcB);
		}
		else if (format.srcB_mod == 1)
		{
			src_id = format.srcB & 0x1ff;
			srcB = this->ReadGPR(src_id);
		}
		else	//check it
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;


		// Execute
		dst = srcA * srcB + src3;

		// Write
		dst_id = format.dst;
		this->WriteGPR(dst_id, dst);

	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
        		<<this->getWarpId() <<" IMAD op0 "<<format.op0;
        std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB

        		<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
        		<<std::endl;
	}
}

void Thread::ExecuteInst_IADD_A(Inst *inst)
{
	// Get Warp
	Warp *warp = this->getWarp();
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
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesIADD format = inst_bytes.iadd;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id;

	// Operand
	unsigned src1, src2, dst;

	// for extended precision add
	long long unsigned srcl1, srcl2;

	// Execute
	if (active == 1 && pred == 1)
	{
		if((format.op0 == 2) && (format.op2 == 1)) // src2 is register mode
		{
			// Read src1 value
			src1_id = format.src1;
			src1 = this->ReadGPR(src1_id);

			// src2 ID
			unsigned src2_id;

			// Read src2 value
			src2_id = format.src2;
			src2 = this->ReadGPR(src2_id);
		}
		else
		{
			this->ISAUnsupportedFeature(inst);
		}

		// Read and Execute .PO mode value
		if (format.po == 0)
		{
			dst = src1 + src2 + (format.x ? ReadCC() : 0);

			if (format.cc)
			{
				srcl1 = src1;
				srcl2 = src2;

				if (srcl1 + srcl2 + (format.x ? ReadCC() : 0) > 0xffffffff)
					WriteCC(1);
			}
		}

		else if (format.po == 1) // subtraction mode
		{
			dst = src1 - src2 - (format.x ? ReadCC() : 0);

			if (format.cc)
			{
				srcl1 = src1;
				srcl2 = src2;

				if (srcl1 < srcl2 + (format.x ? ReadCC() : 0))
					WriteCC(1);
			}
		}
		else if (format.po == 2)
		{
			dst = src2 - src1 - (format.x ? ReadCC() : 0);

			if (format.cc)
			{
				srcl1 = src1;
				srcl2 = src2;

				if (srcl2 < srcl1 + (format.x ? ReadCC() : 0))
					WriteCC(1);
			}
		}
		else
			this->ISAUnsupportedFeature(inst);

		// Write Result
		dst_id = format.dst;
		this->WriteGPR(dst_id, dst);
	}


	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	Emu::isa_debug << misc::fmt("At instruction %s:\tthe current PC is "
					"= %x\n",inst->getName(),warp->getPC());
	Emu::isa_debug << misc::fmt("At instruction %s:\tthe thread ID is "
					"= %u\n",inst->getName(),this->id_in_warp);
	Emu::isa_debug << misc::fmt("At instruction %s:\tinput: the src1 is = %d\n"
					,inst->getName(),src1);
	Emu::isa_debug << misc::fmt("At instruction %s:\tinput: the src2 is = %d\n"
					,inst->getName(),src2);
	Emu::isa_debug << misc::fmt("At instruction %s:\toutput: the result is "
					"= %d\n",inst->getName(),dst);
	Emu::isa_debug << misc::fmt("\n");
}


void Thread::ExecuteInst_IADD_B(Inst *inst)
{

	// Get Warp
	Emu* emu = Emu::getInstance();
	Warp *warp = this->getWarp();
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
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesIADD format = inst_bytes.iadd;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id;

	// Operand
	unsigned src1, src2, dst;

	// for extended precision add
	long long unsigned srcl1, srcl2;

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read src1 value
		src1_id = format.src1;
		src1 = this->ReadGPR(src1_id);

		// Check it
		if (format.op2 == 0)
		{
			emu->ReadConstMem(format.src2 << 2, 4, (char*)&src2);
		}
		else
			src2 = format.src2 >> 18 ? format.src2 | 0xfff80000 : format.src2;

		// Read and Execute .PO mode value
		if (format.po == 0)
		{
			dst = src1 + src2 + (format.x ? ReadCC() : 0);

			if (format.cc)
			{
				srcl1 = src1;
				srcl2 = src2;

				if (srcl1 + srcl2 + (format.x ? ReadCC() : 0) > 0xffffffff)
					WriteCC(1);
			}
		}

		else if (format.po == 1) // subtraction mode
		{
			dst = src1 - src2 - (format.x ? ReadCC() : 0);

			if (format.cc)
			{
				srcl1 = src1;
				srcl2 = src2;

				if (srcl1 < srcl2 + (format.x ? ReadCC() : 0))
					WriteCC(1);
			}
		}
		else if (format.po == 2)
		{
			dst = src2 - src1 - (format.x ? ReadCC() : 0);

			if (format.cc)
			{
				srcl1 = src1;
				srcl2 = src2;

				if (srcl2 < srcl1 + (format.x ? ReadCC() : 0))
					WriteCC(1);
			}
		}
		else
			this->ISAUnsupportedFeature(inst);

		// Write Result
		dst_id = format.dst;
		this->WriteGPR(dst_id, dst);
	}


	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	Emu::isa_debug << misc::fmt("At instruction %s:\tthe current PC is "
					"= %x\n",inst->getName(),warp->getPC());
	Emu::isa_debug << misc::fmt("At instruction %s:\tthe thread ID is "
					"= %u\n",inst->getName(),this->id_in_warp);
	Emu::isa_debug << misc::fmt("At instruction %s:\tinput: the src1 is = %d\n"
					,inst->getName(),src1);
	Emu::isa_debug << misc::fmt("At instruction %s:\tinput: the src2 is = %d\n"
					,inst->getName(),src2);
	Emu::isa_debug << misc::fmt("At instruction %s:\toutput: the result is "
					"= %d\n",inst->getName(),dst);
	Emu::isa_debug << misc::fmt("\n");
}


void Thread::ExecuteInst_ISETP_A(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);


	// Execute
	if (active == 1 && pred == 1)
	{
		// Sources
		x = (format.mod1 >> 4) & 0x1;

		srcA_id = format.mod0;
		srcA = this->ReadGPR(srcA_id) - (x ? ReadCC() : 0);
		srcB_id = format.srcB;
		if (format.srcB_mod == 0)
		{
			emu->ReadConstMem(srcB_id << 2, 4, (char*)&srcB);
		}
		else if (format.srcB_mod == 1)
			srcB = this->ReadGPR(srcB_id);

		// Predicates
		pred_id_1 = (format.dst >> 3) & 0x7;
		pred_id_2 = format.dst & 0x7;
		pred_id_3 = format.mod1 & 0x7;


		pred_3 = this->ReadPred(pred_id_3);
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
			this->WritePred(pred_id_1, pred_1);
		if (pred_id_2 != 7)
			this->WritePred(pred_id_2, pred_2);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<this->getWarpId() <<" ISETP op0 "<<format.op0;
		std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
				<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
				<<" cmp "<< cmp_op << " bool " << bool_op
				<<std::endl;
	}
}


void Thread::ExecuteInst_ISETP_B(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	//Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);


	// Execute
	if (active == 1 && pred == 1)
	{
		// Sources
		srcA_id = format.mod0;
		srcA = this->ReadGPR(srcA_id);

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

		pred_3 = this->ReadPred(pred_id_3);

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
			this->WritePred(pred_id_1, pred_1);
		if (pred_id_2 != 7)
			this->WritePred(pred_id_2, pred_2);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<this->getWarpId() <<" ISETP op0 "<<format.op0;
		std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
				<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
				<<" cmp "<< cmp_op << " bool " << bool_op
				<<std::endl;
	}

}


void Thread::ExecuteInst_EXIT(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	//Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

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

        warp->setTargetpc(warp->getPC() + warp->getInstSize());
	}

}

void Thread::ExecuteInst_BRA(Inst *inst)
{
	// Get warp
	Warp *warp = this->getWarp();

	// Get SyncStack
	SyncStack* stack = warp->getSyncStack()->get();

	// Predicate register
	unsigned pred;

	unsigned active;

	// Instruction bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesBRA format = inst_bytes.bra;

	// Get current PC
	unsigned pc = warp->getPC();

	unsigned target_pc = pc;

	// Get Instruction size
	unsigned inst_size = warp->getInstSize();

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
	if(pred_id <= 7)
		pred = this->ReadPred(pred_id);
	else
		pred = !this->ReadPred(pred_id-8);

	// Clear temp_entry and taken_thread before BRA execute
	if (id_in_warp == 0)
	{
		stack->resetTempMask();
	}

	// The threads taking the BRA will set temp_entry's mask to 1
	// This mask will be pushed into stack then.
	// taken_thread adds 1
	if(active == 1 && pred == 1)
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
        	warp->setTargetpc(pc + inst_size);
        }

        // If all threads in warp take the BRA
        // directly jump to the target pc
        else if (taken_thread == warp->getThreadCount())
        {
        	target_pc = pc + inst_size + offset;

        	// Update pc
        	warp->setTargetpc(target_pc);

        	// Pop all entries in syncstack between current pc and target pc
        	stack->popTillTarget(target_pc, warp->getPC());
        }

        // predicated BRA or non-predicated BRA
        else
        {
        	assert(branch_direction == 1 || branch_direction == 0);

        	if(branch_direction == 0)
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

        		warp->setTargetpc(pc + inst_size);

        	}

        	// Backward BRA
        	// We do not need stack operations, because the next instruction
        	// is the target pc or next pc.
        	else
        	{
       			// no thread takes the BRA
       			if (!taken_thread)
       			{
       				warp->setTargetpc(pc + inst_size);
       			}
       			// all active threads take the BRA
       			else if (taken_thread == active_thread)
      			{
       				warp->setTargetpc(pc + inst_size + offset);
       			}
       			else
        		// predicated backward BRA, appearing in do while loop.
        		// taken_thread < active_thread
        		{
        			target_pc = pc + inst_size + offset;
        			stack->setActiveMask(stack->getTempMask());
        			warp->setTargetpc(target_pc);

        			// There is supposed to be no entries between the pcs.
        			if(stack->popTillTarget(target_pc, pc))
        				throw misc::Panic("Stack entries found between pc to"
        						" target pc when backward BRA\n");
        		}
        	}
        }
	}
}


void Thread::ExecuteInst_MOV_A(Inst *inst)
{
	std::cerr << "MOV_A" << std::endl;
	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
	this->ISAUnsupportedFeature(inst);
}

void Thread::ExecuteInst_MOV_B(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = format.srcB;
		if (format.srcB_mod == 0)
		{
			emu->ReadConstMem(src_id << 2, 4, (char*)&src);
		}
		else if (format.srcB_mod == 1)
			src = this->ReadGPR(src_id);



		/* Execute */
		dst = src;

		/* Write */
		dst_id = format.dst;
		this->WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
        		<<this->getWarpId() <<" MOV_B op0 "<<format.op0;
        std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
        		<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
        		<<std::endl;
	}

}

void Thread::ExecuteInst_MOV32I(Inst *inst)
{
	// Get warp
	Warp *warp = this->getWarp();
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
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesImm format = inst_bytes.immediate;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;
	if(pred_id <= 7)
		pred = this->ReadPred(pred_id);
	else
		pred = !this->ReadPred(pred_id - 8);

	// Operands ID
	unsigned dst_id;

	// Operands
	RegValue src;

	// Execute
	if(active == 1 && pred == 1)
	{
		if(format.s == 0)
			{
				// Read source immediate
				src.u32 = format.imm32;

				// Read destination ID
				dst_id = format.dst;

				// Write the src value to destination register
				this->WriteGPR(dst_id, src.u32);

			}
		else
		{
			throw misc::Panic(".S = 1 in Function MOV32I");
		}
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
}

void Thread::ExecuteInst_LD(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;
	Emu* emu = Emu::getInstance();

	// Predicates and active masks
	Warp* warp = this->getWarp();
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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		if(format.op1 & 0x1) 	//offset direction
			addr = this->ReadGPR(format.mod0) - (format.mod1 << 19) - format.srcB;
		else
			addr = this->ReadGPR(format.mod0) + (format.mod1 << 19) + format.srcB;

		data_type = (format.op1 >> 2) & 0x7;

		// Execute
		emu->ReadGlobalMem(addr, 4, (char*)dst);
		if (data_type > 4)
			emu->ReadGlobalMem(addr + 4, 4, (char*)&dst[1]);
		if (data_type > 5)										//Really? FIXME
			emu->ReadGlobalMem(addr + 8, 8, (char*)&dst[2]);

		/* Write */
		dst_id = format.dst;
		this->WriteGPR(dst_id, dst[0]);


		if (data_type > 4)
			this->WriteGPR(dst_id + 1, dst[1]);
		if (data_type > 5)
		{
			this->WriteGPR(dst_id + 2, dst[2]);
			this->WriteGPR(dst_id + 3, dst[3]);
		}
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<this->getWarpId() <<" LD op0 "<<format.op0;
		std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
				<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
				<<std::endl;
	}

}

void Thread::ExecuteInst_LDS(Inst *inst)
{
	// Inst bytes format

	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Warp* warp = this->getWarp();
	SyncStack* stack = warp->getSyncStack()->get();
	ThreadBlock* thread_block = this->thread_block;

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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

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
		thread_block->readSharedMem(src, sizeof(int), (char*)&dst);

		// write back
		WriteGPR(dst_id, dst);

		if (((format.mod1 >> 9) & 0x7) == 5)
		{
			src += sizeof(int);
			thread_block->readSharedMem(src, sizeof(int), (char*)&dst);
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
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
}

void Thread::ExecuteInst_LDC(Inst *inst)
{
	// Inst byte format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesLDC format = inst_bytes.ldc;

	// Operands Type
	// int u_or_s = format.u_or_s;

	// Operands
	unsigned dst_id, srcA_id, srcB_id1;
	int srcB_id2; // srcB_id2 signed
	unsigned mem_addr;
	RegValue srcA, srcB, dst;

	// Predicates and active masks
	Emu *emu = Emu::getInstance();
	Warp* warp = this->getWarp();
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
	if(pred_id <= 7)
		pred = this->ReadPred(pred_id);
	else
		pred = !this->ReadPred(pred_id -8);

	// Execute
	if(active == 1 && pred == 1)
	{
	    if(format.u_or_s == 4) // default case, unsigned 32
		{
			// Read
			// read GPR
			srcA_id = format.src1;

			// Read mem bank addr and offset
			srcB_id1 = format.src2_1;
			srcB_id2 = format.src2_2;   // FIXME

			// read register value
			srcA.s32 = this->ReadGPR(srcA_id);

			// Caculate mem_addr and read const mem
			mem_addr = srcB_id2 + srcA.s32 + (srcB_id1 << 16);
			emu->ReadConstMem(mem_addr, 4, (char*)&srcB.u32);

			// Execute
			dst.u32 = srcB.u32;

			// Write
			dst_id = format.dst;
			this->WriteGPR(dst_id, dst.u32);
		}
	    else if(format.u_or_s == 5)
	    {
			// Read
			// read GPR
			srcA_id = format.src1;

			// Read mem bank addr and offset
			srcB_id1 = format.src2_1;
			srcB_id2 = format.src2_2;   // FIXME

			// read register value
			srcA.s32 = this->ReadGPR(srcA_id);

			// Caculate mem_addr and read const mem
			mem_addr = srcB_id2 + srcA.s32 + (srcB_id1 << 16);

			// Read the lower 32 bits
			emu->ReadConstMem(mem_addr, 4, (char*)&srcB.u32);

			// Execute
			dst.u32 = srcB.u32;

			// Write
			dst_id = format.dst;
			this->WriteGPR(dst_id, dst.u32);

			// Read the upper 32 bits
			emu->ReadConstMem(mem_addr + 4, 4, (char*)&srcB.u32);

			// Execute the upper 32 bits
			dst.u32 = srcB.u32;

			// Write the upper 32 bits
			this->WriteGPR(dst_id + 1, dst.u32);
	    }
		else
		{
			throw misc::Panic("Unsupported feature in Kepler LDC instruction\n"
						"128 bits LDC attempted.\n");
		}

	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
                 <<this->getWarpId() <<" LDC op0 "<<format.op0;
        std::cerr<<" dst " <<format.dst <<" src1 " <<format.src1 << " pred " <<format.pred << " S " <<format.s
                 <<" src2_2 " <<format.src2_2 << " src2_1 "<< format.src2_1 <<" IS " <<format.is <<" u_or_s"
				 <<format.u_or_s << "op1" << format.op1 <<std::endl;
	}

}

void Thread::ExecuteInst_ST(Inst *inst)
{
	// Inst bytes format

	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;
	Emu* emu = Emu::getInstance();

	// Predicates and active masks
	Warp* warp = this->getWarp();
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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		if(format.op1 & 0x1) 	//offset direction
			addr = this->ReadGPR(format.mod0) - (format.mod1 << 19) - format.srcB;
		else
			addr = this->ReadGPR(format.mod0) + (format.mod1 << 19) + format.srcB;
		data_type = (format.op1 >> 2) & 0x7;
		src_id = format.dst;
		src[0] = this->ReadGPR(src_id);
		if (data_type > 4)
			src[1] = this->ReadGPR(src_id + 1);
		if (data_type > 5)										//Really? FIXME
		{
			src[2] = this->ReadGPR(src_id + 2);
			src[3] = this->ReadGPR(src_id + 3);
		}

		// Execute
		// Write
		emu->WriteGlobalMem(addr, 4, (char*)src);
		if (data_type > 4)
			emu->WriteGlobalMem(addr + 4, 4, (char*)&src[1]);
		if (data_type > 5)										//Really? FIXME
			emu->WriteGlobalMem(addr + 8, 8, (char*)&src[2]);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
                 <<this->getWarpId() <<" ST op0 "<<format.op0;
        std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
                 <<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
                 <<std::endl;
	}
}

void Thread::ExecuteInst_STS(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Warp* warp = this->getWarp();
	SyncStack* stack = warp->getSyncStack()->get();
	ThreadBlock* thread_block = this->thread_block;

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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);


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

		thread_block->writeSharedMem(dst, sizeof(int), (char*)&src);

		if (((format.mod1 >> 9) & 0x7) == 5)
		{
			src = ReadGPR(src_id + 1);
			thread_block->writeSharedMem(dst + sizeof(int),
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
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

}

void Thread::ExecuteInst_DADD(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_FFMA(Inst *inst)
{
	this->ISAUnimplemented(inst);
}


void Thread::ExecuteInst_FMUL(Inst *inst)
{
	// Get Emu
	Emu* emu = Emu::getInstance();

	// Get Warp
	Warp *warp = this->getWarp();
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
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

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
			emu->ReadConstMem(src_id << 2, 4, (char*)&src2);
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
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
}


void Thread::ExecuteInst_FADD(Inst *inst)
{
	// Get Emu
	Emu* emu = Emu::getInstance();

	// Get Warp
	Warp *warp = this->getWarp();
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
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// get predicate register value
	pred_id = format.pred;
	if (pred_id <= 7)
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

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
			emu->ReadConstMem(src_id << 2, 4, (char*)&src2);
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

		/* Write */
		dst_id = format.dst;
		WriteFloatGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
}

void Thread::ExecuteInst_NOP(Inst *inst)
{
	// Get Warp
	Warp *warp = this->getWarp();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Instruction bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesNOP format = inst_bytes.nop;

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
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
}

void Thread::ExecuteInst_S2R(Inst *inst)
{
	// Inst bytes format	//FIXME S2R description missing in Inst.h
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Warp* warp = this->getWarp();
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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = format.srcB & 0xff;
		if (src_id == SR_CLOCKLO)	//No cycle count for now
			;//src = this->grid->emu->inst_count & 0xffffffff;
		else if (src_id == SR_CLOCKHI)
			;//src = (this->grid->emu->inst_count >> 32) & 0xffffffff;
		else if (format.srcB_mod == 1)
			src = this->ReadSR(src_id);

		// Execute
		dst = src;

		// Write
		dst_id = format.dst;
		this->WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<this->getWarpId() <<" S2R op0 "<<format.op0;
		std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
				<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
				<<std::endl;
	}

}

void Thread::ExecuteInst_PSETP(Inst *inst)
{
	// Get Warp
	Warp *warp = this->getWarp();
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
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesPSETP format = inst_bytes.psetp;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;
	if(pred_id <= 7)
		pred = this->ReadPred(pred_id);
	else
		pred = !this->ReadPred(pred_id - 8);

	// Operand ID
	unsigned dst_id, srcA_id, srcB_id, srcC_id;

	// Operands
	unsigned srcA, srcB, srcC, dst;

	// Bool operations of the instruction.
	unsigned bool_op0, bool_op1;

	// Execute
	if(active == 1 && pred == 1)
	{
		// Read SrcA id
		srcA_id = format.pred2;

		// Get SrcA value
		if(srcA_id <= 7)
			srcA = this->ReadPred(srcA_id);
		else
			srcA = !this->ReadPred(srcA_id - 8);

		// Read SrcB id
		srcB_id = format.pred3;

		// Get SrcB value
		if(srcB_id <= 7)
			srcB = this->ReadPred(srcB_id);
		else
			srcB = !this->ReadPred(srcB_id - 8);

		// Read SrcC id
		srcC_id = format.pred4;

		// Get SrcC value
		if(srcC_id <= 7)
			srcC = this->ReadPred(srcC_id);
		else
			srcC = !this->ReadPred(srcC_id - 8);

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
		this->WritePred(dst_id, dst);

	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
}

void Thread::ExecuteInst_SHF(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_BAR(Inst *inst)
{

	// Inst bytes format	//FIXME S2R description missing in Inst.h
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Warp* warp = this->getWarp();
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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

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

		if (thread_block->getWarpsAtBarrier()
				== thread_block->getWarpCount())
		{
			//std::cout << " num at barrier " << thread_block->getWarpsAtBarrier()
			//		<< std::endl;
			thread_block->clearWarpAtBarrier();
			thread_block->setWarpsAtBarrier(0);
		}
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<this->getWarpId() <<" S2R op0 "<<format.op0;
		std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " <<format.srcB
				<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
				<<std::endl;
	}
}

void Thread::ExecuteInst_BPT(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_JMX(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_JMP(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_JCAL(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_BRX(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_CAL(Inst *inst)
{
	// Inst bytes format	//FIXME S2R description missing in Inst.h
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesCAL format = inst_bytes.cal;

	// Predicates and active masks
	Warp* warp = this->getWarp();
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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// FIXME naive here
	}

	if (id_in_warp == warp->getThreadCount() - 1)
	{
        std::cout << "CAL PC " << std::hex << warp->getPC()
        		<< "	A.M.	" << stack->getActiveMask()<< std::endl;
        stack->Dump(std::cout);


		if (stack->getActiveMask())
		{
			std::unique_ptr<SyncStack> sync_stack(
					new SyncStack(warp->getThreadCount()));
			sync_stack->setActiveMask(stack->getActiveMask());

			warp->getReturnAddressStack()->get()->push(warp->getPC() + 8,
					stack->getActiveMask(), sync_stack);

			warp->setTargetpc(warp->getPC() + offset + warp->getInstSize());
		}
		else
		{
			warp->setTargetpc(warp->getPC() + warp->getInstSize());
		}
	}

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<this->getWarpId() <<" CAL op0 "<<format.op0 << " offset "
				<<format.offset << " op1 "<< format.op1 <<std::endl;
	}
}


void Thread::ExecuteInst_PRET(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_PLONGJMP(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_SSY(Inst *inst)
{
	// Get emulator
    Emu *emu = Emu::getInstance();

	// Get warp
	Warp *warp = this->getWarp();

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
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesSSY format = inst_bytes.ssy;

	// Operand
	unsigned offset;
	unsigned isconstmem;

	// Read Operand constmem
	isconstmem = format.isconstmem;

	offset = format.offset;

	// Execute at the last thread in warp
	if(id_in_warp == warp->getThreadCount() - 1)
	{
		if (isconstmem == 0)
		{
			// Push SSY instruction into stack and set the reconvergence address
			pc = warp->getPC();

			address = offset + pc + warp->getInstSize();
		}
		else
        {
			// check this
			if (isconstmem == 1)
              	emu->ReadConstMem(offset << 2,4, (char*) &address);
        }

		stack->push(address,
					stack->getActiveMask(),
					SyncStackEntrySSY);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
}

void Thread::ExecuteInst_PBK(Inst *inst)
{
	// Get warp
	Warp* warp = this->getWarp();

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
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesPBK format = inst_bytes.pbk;


	// Execute
    if (id_in_warp == warp->getThreadCount() - 1)
    {
        //Check this
        if (format.constant)
        {
                //emu->ReadConstMem(format.offset << 2, sizeof(address), (char*) &address);
        		throw misc::Panic("getting address from constant memory is "
        				"not supported.\n");
        }
        else
        {
        // Get operand value
                if (format.offset >> 23 == 0)
                    address = format.offset + warp->getPC() + warp->getInstSize();
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
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
}


void Thread::ExecuteInst_PCNT(Inst *inst)
{
	// Get warp
	Warp* warp = this->getWarp();

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
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesPCNT format = inst_bytes.pcnt;


	// Execute
    if (id_in_warp == warp->getThreadCount() - 1)
    {
        //Check this
        if (format.constant)
        {
                //emu->ReadConstMem(format.offset << 2, sizeof(address), (char*) &address);
        		throw misc::Panic("getting address from constant memory is "
        				"not supported.\n");

        }
        else
        {
        // Get operand value
                if (format.offset >> 23 == 0)
                    address = format.offset + warp->getPC() + warp->getInstSize();
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
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
}


void Thread::ExecuteInst_BFE(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 format = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst;
	int srcA, srcB;
	unsigned start_bit, len;

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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = format.mod0;
		srcA = this->ReadGPR(src_id);
		src_id = format.srcB;

		// FIXME == 0 has not been observed.
		if (format.srcB_mod == 1)
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;
		else
			emu->ReadConstMem(src_id << 2, 4, (char*)&srcB);

		// Execute
		// FIXME not fully verified, but logic is supposed to be correct.
		start_bit = srcB & 0xff;
		len = (srcB >> 8) & 0xff;

        dst = ((srcA & ((1 << (start_bit + len)) - 1)) >> start_bit) &
                ((1 << len) - 1);
		if (((dst >> (len - 1)) & 0x1) == 1)
			dst = dst | (0xffffffff << len);

		/* Write */
		dst_id = format.dst;
		this->WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
    std::cerr<< "Warp id "<< std::hex
      		<<this->getWarpId() <<" BFE op0 "<<format.op0;
    std::cerr<<" dst " <<format.dst <<" mod0 " <<format.mod0 << " s " <<format.s << " srcB " << srcB
       		<<" mod1 " <<format.mod1 << " op1 "<< format.op1 <<" srcB_mod " <<format.srcB_mod
       		<<std::endl;
	}
	//this->ISAUnimplemented(inst);
}


void Thread::ExecuteInst_GETCRSPTR(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_GETLMEMBASE(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_SETCRSPTR(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_SETLMEMBASE(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_LONGJMP(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_RET(Inst *inst)
{

	// Inst bytes format	//FIXME S2R description missing in Inst.h
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral2 format = inst_bytes.general2;

	// Predicates and active masks
	Warp* warp = this->getWarp();
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
		pred = this->ReadPred(pred_id);
	else
		pred = ! this->ReadPred(pred_id - 8);

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
        warp->setTargetpc(return_addr);
	}

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr << "RET Warp id "<< std::hex
				<<this->getWarpId() <<" op0 "<<format.op0
				<< " offset " <<format.mod << " mod "<< format.op1 << std::endl;
		std::cerr << "From " << std::hex << warp->getPC() << " to "
				<< return_addr << std::endl;
	}

	//this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_KIL(Inst *inst)
{
	this->ISAUnimplemented(inst);
}


void Thread::ExecuteInst_BRK(Inst *inst)
{
	// Get warp
	Warp *warp = this->getWarp();

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
	InstBytes inst_bytes =inst->getInstBytes();
	InstBytesBRK format = inst_bytes.brk;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;
	if(pred_id <= 7)
		pred = this->ReadPred(pred_id);
	else
		pred = !this->ReadPred(pred_id - 8);


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
			warp->setTargetpc(PBK_addr);

			// Pop all entries in stack until the first PBK
			stack->popTillTarget(PBK_addr, warp->getPC());
		}
		else
		{
            warp->setTargetpc(warp->getPC() + warp->getInstSize());
		}
	}
}


void Thread::ExecuteInst_CONT(Inst *inst)
{
	// Get warp
	Warp *warp = this->getWarp();

	// Get synchronization stack
	SyncStack* stack = warp->getSyncStack()->get();

	unsigned active;

	// Predicate
	unsigned pred_id;
	unsigned pred;

	// Instruction bytes foramt
	InstBytes inst_bytes =inst->getInstBytes();
	InstBytesCONT format = inst_bytes.cont;

	// Get current PC
	unsigned pc = warp->getPC();

	//unsigned target_pc;

	// Get Instruction size
	unsigned inst_size = warp->getInstSize();

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
	if(pred_id <= 7)
		pred = this->ReadPred(pred_id);
	else
		pred = !this->ReadPred(pred_id-8);

	// Set the latest PCNT active thread mask
	// Clear active mask bit of specific all entries before the PCNT
	if(active == 1 && pred == 1)
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
			warp->setTargetpc(target_pc);
			stack->popTillTarget(target_pc, pc);
		}
		else
			warp->setTargetpc(pc + inst_size);
	}
}

void Thread::ExecuteInst_RTT(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_SAM(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_RAM(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_IDE(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_LOP32I(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_FADD32I(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_FFMA32I(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_IMAD32I(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_ISCADD32I(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_SHL(Inst *inst)
{
	// Get Warp
	Warp *warp = this->getWarp();
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
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesSHL format = inst_bytes.shl;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;
	if(pred_id <= 7)
		pred = this->ReadPred(pred_id);
	else
		pred = !this->ReadPred(pred_id - 8);

	// Operand ID
	unsigned dst_id, srcA_id; //srcB_id to be added for register

	// Operands
	unsigned srcA, srcB, dst;

	// Execute
	if(active == 1 && pred == 1)
	{
		// Read SrcA id
		srcA_id = format.src1;

		// Get SrcA value
		srcA = this->ReadGPR(srcA_id);

		// Read SrcB
		if ((format.op2 == 1) && (format.op0 == 1)) // src2 is immediate value
		{
			srcB = format.src2;
		}
		else
		{
			throw misc::Panic("Unsupported feature in Kepler SHL instruction");
		}

		// Read destination id
		dst_id = format.dst;

		// Calculate result
		dst = srcA << srcB;

		// Write the value to destination register
		this->WriteGPR(dst_id, dst);
	}

	if (id_in_warp == warp->getThreadCount() - 1)
            warp->setTargetpc(warp->getPC() + warp->getInstSize());

}

}	// namespace Kepler
