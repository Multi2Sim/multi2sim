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
#include "Thread.h"
#include "ThreadBlock.h"


namespace Kepler
{

typedef union
{
	unsigned u32;
	int s32;
	float f;
	unsigned long long u64;
} RegValue;

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

void Thread::ExecuteInst_IMUL_A(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_IMUL_B(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_ISCADD_A(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst;
	int srcA, srcB;
	int shamt;

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active
	active =  warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = this->GetPred(pred_id);
	else
		pred = ! this->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = fmt.mod0;
		srcA = this->ReadGPR(src_id);

		if (((fmt.mod1 >> 6) & 0x3) == 2)	//FIXME
			srcA = -srcA;
		src_id = fmt.srcB;
		if (fmt.srcB_mod == 0)
			emu->ReadConstMem(src_id << 2, 4, (char*)&srcB);
		else if (fmt.srcB_mod == 1)
			srcB = this->ReadGPR(src_id);
		else
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;

		if (((fmt.mod1 >> 6) & 0x3) == 1)	//FIXME
			srcB = -srcB;
		shamt = fmt.mod1 & 0xf; //45:42

		/* Execute */
		dst = (srcA << shamt) + srcB;

		/* Write */
		dst_id = fmt.dst;
		this->WriteGPR(dst_id, dst);
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<this->getWarpId() <<" ISCADD op0 "<<fmt.op0;
		std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB

				<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
				<<std::endl;
	}
}


void Thread::ExecuteInst_ISCADD_B(Inst *inst)
{
	std:: cerr <<"ISCADD B"<<std::endl;
}

void Thread::ExecuteInst_IMAD(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst;
	int srcA, srcB, src3;

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active
	active =  warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = this->GetPred(pred_id);
	else
		pred = ! this->GetPred(pred_id - 8);

	if( active == 1 && pred == 1)
	{
		// Read
		src_id = fmt.mod0;
		srcA = this->ReadGPR(src_id);
		src_id = fmt.mod1 & 0xff;
		src3 = this->ReadGPR(src_id);
		if (fmt.srcB_mod == 0)
		{
			src_id = fmt.srcB;
			emu->ReadConstMem(src_id << 2, 4, (char*)&srcB);
		}
		else if (fmt.srcB_mod == 1)
		{
			src_id = fmt.srcB & 0x1ff;
			srcB = this->ReadGPR(src_id);
		}
		else	//check it
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;


		// Execute
		dst = srcA * srcB + src3;

		// Write
		dst_id = fmt.dst;
		this->WriteGPR(dst_id, dst);

	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
        		<<this->getWarpId() <<" IMAD op0 "<<fmt.op0;
        std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB

        		<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
        		<<std::endl;
	}
}

void Thread::ExecuteInst_IADD_A(Inst *inst)
{
	// Get Warp
	Warp *warp = this->getWarp();

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active status
	unsigned active;

	// If the thread is active
	active =  warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

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
		pred = this->GetPred(pred_id);
	else
		pred = ! this->GetPred(pred_id - 8);

	// Operand ID
	unsigned dst_id, src1_id;

	// Operand
	unsigned src1, src2, dst;

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

		// Read .PO mode value
		if(format.po == 1) // subtraction mode
			src2 = -src2;
		else if(format.po == 2)
			src1 = -src1;
		else if(format.po == 3)
			this->ISAUnsupportedFeature(inst);

		// Execute
		dst = src1 + src2;

		// Write Result
		dst_id = format.dst;
		this->WriteGPR(dst_id, dst);
	}

	// Debug information
	Emu::isa_debug << misc::fmt("At instruction %s:\tthe current PC is "
					"= %x\n",inst->getName(),warp->getPC());
	Emu::isa_debug << misc::fmt("At instruction %s:\tthe thread ID is "
					"= %u\n",inst->getName(),this->getIdInWarp());
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

	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst;
	int srcA, srcB;

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active
	active =  warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = this->GetPred(pred_id);
	else
		pred = ! this->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = fmt.mod0;
		srcA = this->ReadGPR(src_id);
		src_id = fmt.srcB;
		if (fmt.srcB_mod == 0)
		{
			emu->ReadConstMem(src_id << 2, 4, (char*)&srcB);
		}
		else if (fmt.srcB_mod == 1)
			srcB = this->ReadGPR(src_id);
		else	//check it
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;

		if (((fmt.mod1 >> 9) & 0x1) == 1)	//FIXME
			srcB = -srcB;

		/* Execute */
		dst = srcA + srcB;

		/* Write */
		dst_id = fmt.dst;
		this->WriteGPR(dst_id, dst);
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
    std::cerr<< "Warp id "<< std::hex
      		<<this->getWarpId() <<" IADD op0 "<<fmt.op0;
    std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
       		<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
       		<<std::endl;
	}

}

void Thread::ExecuteInst_ISETP(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
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

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active
	active =  warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = this->GetPred(pred_id);
	else
		pred = ! this->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Sources
		srcA_id = fmt.mod0;
		srcA = this->ReadGPR(srcA_id);
		srcB_id = fmt.srcB;
		if (fmt.srcB_mod == 0)
		{
			emu->ReadConstMem(srcB_id << 2, 4, (char*)&srcB);
		}
		else if (fmt.srcB_mod == 1)
			srcB = this->ReadGPR(srcB_id);
		else	//check it
			srcB = srcB_id >> 18 ? srcB_id | 0xfff80000 : srcB_id;

		// Predicates
		pred_id_1 = (fmt.dst >> 3) & 0x7;
		pred_id_2 = fmt.dst & 0x7;
		pred_id_3 = fmt.mod1 & 0x7;


		pred_3 = this->GetPred(pred_id_3);
		if (((fmt.mod1 >> 3) & 0x1))
			pred_3 = !pred_3;

		// Compare
        cmp_op = ((fmt.op1 & 0x1) << 2) | (fmt.mod1 >> 10);

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
		bool_op = (fmt.mod1 >> 6) & 0x3;
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
		pred_id_1 = (fmt.dst >> 3) & 0x7;
		pred_id_2 = fmt.dst & 0x7;
		if (pred_id_1 != 7)
			this->WritePred(pred_id_1, pred_1);
		if (pred_id_2 != 7)
			this->WritePred(pred_id_2, pred_2);
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<this->getWarpId() <<" ISETP op0 "<<fmt.op0;
		std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
				<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
				<<" cmp "<< cmp_op << " bool " << bool_op
				<<std::endl;
	}
}

void Thread::ExecuteInst_EXIT(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	//Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active
	active =  warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = this->GetPred(pred_id);
	else
		pred = ! this->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		if(warp->getFinishedThreadCount() >= warp->getThreadCount() )
			warp->setFinishedEmu(true);
	}

	warp->setTargetpc(warp->getPC()+warp->getInstSize());
}

void Thread::ExecuteInst_BRA(Inst *inst)
{
	// Get warp
	Warp *warp = this->getWarp();

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active status
	unsigned active;

	// If the thread is active
	active =  warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Instruction bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesBRA format = inst_bytes.bra;

	// Predicate register
	unsigned pred;

	// Predicate register ID
	unsigned pred_id;

	// Get predicate register value
	pred_id = format.pred;
	if(pred_id <= 7)
		pred = this->GetPred(pred_id);
	else
		pred = !this->GetPred(pred_id-8);

	// Execute and update the active mask value  ���������entry������  ������������entry������
	if(active == 1 && pred == 1)
	{
		// In execution part, we need to read the active mask in current stack
		// entry and set the value to next stack entry
		warp->incrSyncStkTop();
		warp->setSyncStkTopActiveMaskBit(this->getIdInWarp(), 1);
		//warp->setSyncStkTopInst("");  // need do that? or initialize at warp()
		warp->decrSyncStkTop();
		warp->increaseTakenThread(1);
	}
	else
	{
		warp->incrSyncStkTop();
		warp->setSyncStkTopActiveMaskBit(this->getIdInWarp(), 0);
		//warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Get current PC
	unsigned pc;
	pc = warp->getPC();

	// Get Instruction size
	unsigned inst_size;
	inst_size = warp->getInstSize();

	// Get instruction offset
	unsigned offset;
	offset = format.offset;

	// Branch direction 0: forward branch, 1: backward branch
	unsigned branch_direction;
	branch_direction = offset >> 23;

	// Update sync stack when the last thread is executed
	if(this->getIdInWarp() == warp->getThreadCount() - 1)
	{
		// Get number of taken thread
		unsigned taken_thread = warp->getTakenThread();

		// Get number of active thread
		unsigned active_thread = warp->getSyncStkTopActiveCount();

		// Update sync stack and PC
		if(taken_thread == 0)  //  no thread in the active mask taken branch
		{
			warp->setTargetpc(pc + inst_size);
			warp->incrSyncStkTop();
			warp->resetSyncStkTopActiveMask();
			warp->decrSyncStkTop();
		}
		else if (taken_thread == warp->getThreadCount()) // all thread in active
		{                                       		 // mask taken branch
			warp->setTargetpc(pc + inst_size + offset);
			warp->incrSyncStkTop();
			warp->resetSyncStkTopActiveMask();
			warp->decrSyncStkTop();
		}
		else // branch will diverge
		{
			if(branch_direction == 0) // forward branch diverge
			{ // If taken thread is less than active thread means the divergence
			  // happens. need to assign the active mask for the next two stack
			  // entries and increase the stack top pointer by 2.
				if(taken_thread < active_thread)
				{
					warp->incrSyncStkTop();
					warp->setSyncStkTopStartAddress(pc + inst_size + offset);
					unsigned mask = warp->getSyncStkTopActiveMask();
					warp->setSyncStkTopOriginalActiveThreadMask(mask);
					//warp->setSyncStkTopInst(""); // FIXME
					mask = ~mask;
					warp->incrSyncStkTop();
					warp->setSyncStkTopActiveMask(mask);
					warp->setSyncStkTopOriginalActiveThreadMask(mask);
					//warp->setSyncStkTopInst(""); // FIXME
					warp->setSyncStkTopStartAddress(pc + inst_size);
					//warp->decrSyncStkTop();
					//warp->decrSyncStkTop();  need to increase the stack top pointer by 2
					warp->setTargetpc(pc + inst_size);
				}
				else // If taken thread is equal to active thread means no
				{	 // divergence so no operation.	Just set the target pc.
					warp->setTargetpc(pc + inst_size);
				}
			}
			else // Backward diverge this case seems never used by NVCC
			{	 // All backward branches seem no diverge. In case!
				 // Do the same  thing as forward branch but set the target
				 // address as pc+inst_size+offset
				if(taken_thread < active_thread)
				{
					warp->incrSyncStkTop();
					warp->setSyncStkTopStartAddress(pc + inst_size + offset);
					unsigned mask = warp->getSyncStkTopActiveMask();
					mask = ~mask;
					warp->incrSyncStkTop();
					warp->setSyncStkTopActiveMask(mask);
					warp->setSyncStkTopStartAddress(pc + inst_size);
					//warp->decrSyncStkTop();
					//warp->decrSyncStkTop();  need to increase the stack top pointer by 2
					warp->setTargetpc(pc + inst_size + offset);
				}
				else
				{
					warp->setTargetpc(pc + inst_size);
				}
			}
		}
	}
}

void Thread::ExecuteInst_MOV_A(Inst *inst)
{
	std::cerr << "MOV_A" << std::endl;
}

void Thread::ExecuteInst_MOV_B(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = this->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst, src;

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active
	active =  warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = this->GetPred(pred_id);
	else
		pred = ! this->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = fmt.srcB;
		if (fmt.srcB_mod == 0)
		{
			emu->ReadConstMem(src_id << 2, 4, (char*)&src);
		}
		else if (fmt.srcB_mod == 1)
			src = this->ReadGPR(src_id);
		else	//check it
			src = src_id >> 18 ? src_id | 0xfff80000 : src_id;


		/* Execute */
		dst = src;

		/* Write */
		dst_id = fmt.dst;
		this->WriteGPR(dst_id, dst);
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
        		<<this->getWarpId() <<" MOV_B op0 "<<fmt.op0;
        std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
        		<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
        		<<std::endl;
	}

}

void Thread::ExecuteInst_MOV32I(Inst *inst)
{
	// Get warp
	Warp *warp = this->getWarp();

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active status
	unsigned active;

	// If the thread is active
	active = warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

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
		pred = this->GetPred(pred_id);
	else
		pred = !this->GetPred(pred_id - 8);

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
}

void Thread::ExecuteInst_LD(Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;
	Emu* emu = Emu::getInstance();

	// Predicates and active masks
	Warp* warp = this->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id;
	int dst[4];
	unsigned addr;
	unsigned data_type;

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active
	active =  warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = this->GetPred(pred_id);
	else
		pred = ! this->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		if(fmt.op1 & 0x1) 	//offset direction
			addr = this->ReadGPR(fmt.mod0) - (fmt.mod1 << 19) - fmt.srcB;
		else
			addr = this->ReadGPR(fmt.mod0) + (fmt.mod1 << 19) + fmt.srcB;

		data_type = (fmt.op1 >> 2) & 0x7;

		// Execute
		emu->ReadGlobalMem(addr, 4, (char*)dst);
		if (data_type > 4)
			emu->ReadGlobalMem(addr + 4, 4, (char*)&dst[1]);
		if (data_type > 5)										//Really? FIXME
			emu->ReadGlobalMem(addr + 8, 8, (char*)&dst[2]);

		/* Write */
		dst_id = fmt.dst;
		this->WriteGPR(dst_id, dst[0]);


		if (data_type > 4)
			this->WriteGPR(dst_id + 1, dst[1]);
		if (data_type > 5)
		{
			this->WriteGPR(dst_id + 2, dst[2]);
			this->WriteGPR(dst_id + 3, dst[3]);
		}
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<this->getWarpId() <<" LD op0 "<<fmt.op0;
		std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
				<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
				<<std::endl;
	}

}

void Thread::ExecuteInst_LDS(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_LDC(Inst *inst)
{
	// Inst byte format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesLDC format = inst_bytes.ldc;

	// Operands Type
	// int u_or_s = fmt.u_or_s;

	// Operands
	unsigned dst_id, srcA_id, srcB_id1;
	int srcB_id2; // srcB_id2 signed
	unsigned mem_addr;
	RegValue srcA, srcB, dst;

	// Predicates and active masks
	Emu *emu = Emu::getInstance();
	Warp* warp = this->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active
	active = warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Predicte
	pred_id = format.pred;
	if(pred_id <= 7)
		pred = this->GetPred(pred_id);
	else
		pred = !this->GetPred(pred_id -8);

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
			emu->ReadConstMem(mem_addr, 8, (char*)&srcB.u64);

			// Execute
			dst.u64 = srcB.u64;

			// Write
			dst_id = format.dst;
			this->WriteGPR(dst_id, dst.u64);
	    }
		else
		{
			throw misc::Panic("Unsupported feature in Kepler LDC instruction");
		}

	}

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
	InstBytesGeneral0 fmt = inst_bytes.general0;
	Emu* emu = Emu::getInstance();

	// Predicates and active masks
	Warp* warp = this->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned src_id;
	int src[4];
	unsigned addr;
	unsigned data_type;

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active
	active =  warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = this->GetPred(pred_id);
	else
		pred = ! this->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		if(fmt.op1 & 0x1) 	//offset direction
			addr = this->ReadGPR(fmt.mod0) - (fmt.mod1 << 19) - fmt.srcB;
		else
			addr = this->ReadGPR(fmt.mod0) + (fmt.mod1 << 19) + fmt.srcB;
		data_type = (fmt.op1 >> 2) & 0x7;
		src_id = fmt.dst;
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

	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
                 <<this->getWarpId() <<" ST op0 "<<fmt.op0;
        std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
                 <<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
                 <<std::endl;
	}

}

void Thread::ExecuteInst_STS(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_DADD(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_FFMA(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_FADD(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_NOP(Inst *inst)
{
	// Get Warp
	Warp *warp = this->getWarp();

	// Instruction bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesNOP format = inst_bytes.nop;

	// Get Operand
	unsigned s = format.s;

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Execute if s=1, pop all the stack until meet the first ssy instruction
	// the stack is popped only one time for each warp
	if((s == 1) && (this->getIdInWarp() == warp->getThreadCount() - 1))
	{
		while(warp->getSyncStkTopInst() != "SSY")
		{
			warp->setSyncStkTopActiveMask( (1ull << warp->getThreadCount()) - 1);
			//warp->setSyncStkTopInst("");
			warp->setSyncStkTopRecPC(0);
			warp->setSyncStkTopStartAddress(0);
			warp->decrSyncStkTop();
		}
		warp->setSyncStkTopActiveMask( (1ull << warp->getThreadCount()) - 1);
		warp->setSyncStkTopInst("");
		warp->setSyncStkTopRecPC(0);
		warp->setSyncStkTopStartAddress(0);
		warp->decrSyncStkTop();
	}
}

void Thread::ExecuteInst_S2R(Inst *inst)
{
	// Inst bytes format	//FIXME S2R description missing in Inst.h
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Warp* warp = this->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst, src;

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active
	active =  warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = this->GetPred(pred_id);
	else
		pred = ! this->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = fmt.srcB & 0xff;
		if (src_id == SR_CLOCKLO)	//No cycle count for now
			;//src = this->grid->emu->inst_count & 0xffffffff;
		else if (src_id == SR_CLOCKHI)
			;//src = (this->grid->emu->inst_count >> 32) & 0xffffffff;
		else if (fmt.srcB_mod == 1)
			src = this->ReadSR(src_id);

		// Execute
		dst = src;

		// Write
		dst_id = fmt.dst;
		this->WriteGPR(dst_id, dst);
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<this->getWarpId() <<" S2R op0 "<<fmt.op0;
		std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
				<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
				<<std::endl;
	}

}

void Thread::ExecuteInst_PSETP(Inst *inst)
{
	// Get Warp
	Warp *warp = this->getWarp();

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active status
	unsigned active;

	// If the thread is active
	active = warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

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
		pred = this->GetPred(pred_id);
	else
		pred = !this->GetPred(pred_id - 8);

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
			srcA = this->GetPred(srcA_id);
		else
			srcA = !this->GetPred(srcA_id - 8);

		// Read SrcB id
		srcB_id = format.pred3;

		// Get SrcB value
		if(srcB_id <= 7)
			srcB = this->GetPred(srcB_id);
		else
			srcB = !this->GetPred(srcB_id - 8);

		// Read SrcC id
		srcC_id = format.pred4;

		// Get SrcC value
		if(srcC_id <= 7)
			srcC = this->GetPred(srcC_id);
		else
			srcC = !this->GetPred(srcC_id - 8);

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
}

void Thread::ExecuteInst_SHF(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_BAR(Inst *inst)
{
	this->ISAUnimplemented(inst);
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
	this->ISAUnimplemented(inst);
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
	// Get warp
	Warp *warp = this->getWarp();

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Active status
	unsigned active;

	// If the thread is active
	active = warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Instbytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesSSY format = inst_bytes.ssy;

	// Operand
	unsigned offset;
	unsigned isconstmem;

	// Read Operand constmem
	isconstmem = format.isconstmem;

	// Execute only one time for per warp
	if(active == 1 && (this->getIdInWarp() == warp->getThreadCount() - 1))
	{
		if (isconstmem == 0)
		{
			// Push SSY instruction into stack and set the reconvergence address
			// value of the next two nodes of the stack to offset + pc + 8
			// after the operation there are 3 more entries in stack and then
			// let the stack top pointer points to the ssy entry.
			offset = format.offset;
			unsigned address;
			unsigned pc = warp->getPC();
			address = offset + pc + 8;
			warp->incrSyncStkTop();
			warp->setSyncStkTopInst("SSY");
			warp->resetSyncStkTopActiveMask();
			warp->incrSyncStkTop();
			warp->setSyncStkTopRecPC(address);
			warp->incrSyncStkTop();
			warp->setSyncStkTopRecPC(address);
			warp->decrSyncStkTop();
			warp->decrSyncStkTop();
		}
		else if (isconstmem == 1)
		{
			Emu *emu = Emu::getInstance();
			unsigned address;
			emu->ReadConstMem(1,4, (char*) &address);
			warp->incrSyncStkTop();
			warp->setSyncStkTopInst("SSY");
			warp->resetSyncStkTopActiveMask();
			warp->incrSyncStkTop();
			warp->setSyncStkTopRecPC(address);
			warp->incrSyncStkTop();
			warp->setSyncStkTopRecPC(address);
			warp->decrSyncStkTop();
			warp->decrSyncStkTop();
		}
	}

}

void Thread::ExecuteInst_PBK(Inst *inst)
{
	// Get warp
	Warp *warp = this->getWarp();

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	//Active status
	unsigned active;

	// If the thread is active
	active = warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

	// Instruction bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesPBK format = inst_bytes.pbk;

	//Pre relative address
	unsigned address;

	//Get current Pre-relative address for reconvergence stack top entry
	address = warp->getSyncStkTopPreRelativeAddress();

	// Execute. Execute only once for each warp.
	if((active == 1) && (address!= 0))
	{
		// Operand
		unsigned offset;

		// Get operand value
		offset = format.offset;
		warp->setSyncStkTopPreRelativeAddress(offset);
	}
}

void Thread::ExecuteInst_PCNT(Inst *inst)
{
	this->ISAUnimplemented(inst);
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
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_KIL(Inst *inst)
{
	this->ISAUnimplemented(inst);
}

void Thread::ExecuteInst_BRK(Inst *inst)
{
	// Get warp
	Warp *warp = this->getWarp();

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if((this->getIdInWarp() == 0) &&
			(warp->getPC() ==
					warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active status
	unsigned active;

	// Determine if the thread is active
	active = warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

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
		pred = this->GetPred(pred_id);
	else
		pred = !this->GetPred(pred_id - 8);

	// Execute
	if(active == 1 && pred == 1)
	{
		warp->setSyncStkTopActiveMaskBit(0,this->getIdInWarp());
	}

}

void Thread::ExecuteInst_CONT(Inst *inst)
{
	this->ISAUnimplemented(inst);
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

	// Determine whether the warp arrives the PBK address. If it is, restore the
	// active mask to the original active mask.
	if(warp->getPC() == warp->getSyncStkTopPreRelativeAddress())
	{
		unsigned mask = warp->getSyncStkTopOriginalActiveThreadMask();
		warp->setSyncStkTopActiveMask(mask);
	}

	// Pop sync stack when the warp finish else(if) part and begin to execute if
	// (else) part. Must start at the first thread
	if ((this->getIdInWarp() == 0) &&
			(warp->getPC() == warp->getSyncStkStartAddress(warp->getSyncStkTop() - 1)))
	{
		warp->setSyncStkTopRecPC(0);
		warp->resetSyncStkTopActiveMask();
		warp->setSyncStkTopStartAddress(0);
		warp->setSyncStkTopInst("");
		warp->decrSyncStkTop();
	}

	// Active status
	unsigned active;

	// If the thread is active
	active = warp->getSyncStkTopActiveMaskBit(this->getIdInWarp());

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
		pred = this->GetPred(pred_id);
	else
		pred = !this->GetPred(pred_id - 8);

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
}
}	// namespace Kepler
