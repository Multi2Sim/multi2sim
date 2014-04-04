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
 *  You should have received a cmp_opy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <math.h>
#include <iostream>
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <mem-system/memory.h>


#include "Emu.h"
#include "Grid.h"
#include "isa.h"
#include "Warp.h"
#include "Thread.h"
#include "../asm/Inst.h"
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

const char *kpl_err_isa_note =
		"\tThe NVIDIA Kepler SASS instruction set is partially supported by \n"
		"\tMulti2Sim. If your program is using an unimplemented instruction, \n"
		"\tplease email 'development@multi2sim.org' to request support for \n"
		"\tit.\n";

//#define __NOT_IMPL__ std::cerr << "Kepler instruction not implemented.\n" <<
//		kpl_err_isa_note;


#define __NOT_IMPL__  if(getenv("M2S_KPL_ISA_DEBUG"))\
		std::cerr << __func__ <<"is not supported yet.\n";\
		else std::cerr << "Kepler instruction not implemented.\n" <<\
		kpl_err_isa_note;


	;
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
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = thread->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst;
	int srcA, srcB;
	int shamt;

	// Pop sync stack at reconvergence PC
	if ((warp->getPC() != 0) && (warp->getPC() ==
			warp->getSyncStkTopRecPC()))
	{
		warp->setSyncStkTopRecPC(0);
		//bit_map_free(warp->sync_stack.entries[warp->sync_stack_top].
		//		active_thread_mask);
        warp->setSyncStkTopActive(-1);
		warp->decrSyncStkTop();
	}

	// Active
	active =  unsigned(1) & (warp->getSyncStkTopActive() >> (thread->getIdInWarp()-1));

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->GetPred(pred_id);
	else
		pred = ! thread->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = fmt.mod0;
		srcA = thread->ReadGPR(src_id);

		if (((fmt.mod1 >> 6) & 0x3) == 2)	//FIXME
			srcA = -srcA;
		src_id = fmt.srcB;
		if (fmt.srcB_mod == 0)
			emu->ReadConstMem(src_id << 2, 4, (char*)&srcB);
		else if (fmt.srcB_mod == 1)
			srcB = thread->ReadGPR(src_id);
		else
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;

		if (((fmt.mod1 >> 6) & 0x3) == 1)	//FIXME
			srcB = -srcB;
		shamt = fmt.mod1 & 0xf; //45:42

		/* Execute */
		dst = (srcA << shamt) + srcB;

		/* Write */
		dst_id = fmt.dst;
		thread->WriteGPR(dst_id, dst);
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<thread->getWarpId() <<" ISCADD op0 "<<fmt.op0;
		std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB

				<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
				<<std::endl;
	}
}

void kpl_isa_ISCADD_B_impl(Thread *thread, Inst *inst)
{
	std:: cerr <<"ISCADD B"<<std::endl;
}

void kpl_isa_IMAD_impl(Thread *thread, Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = thread->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst;
	int srcA, srcB, src3;

	// Pop sync stack at reconvergence PC
	if ((warp->getPC() != 0) && (warp->getPC() ==
			warp->getSyncStkTopRecPC()))
	{
		warp->setSyncStkTopRecPC(0);
		//bit_map_free(warp->sync_stack.entries[warp->sync_stack_top].
		//		active_thread_mask);
        warp->setSyncStkTopActive(-1);
		warp->decrSyncStkTop();
	}

	// Active
	active =  unsigned(1) & (warp->getSyncStkTopActive() >> (thread->getIdInWarp()-1));

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->GetPred(pred_id);
	else
		pred = ! thread->GetPred(pred_id - 8);

	if( active == 1 && pred == 1)
	{
		// Read
		src_id = fmt.mod0;
		srcA = thread->ReadGPR(src_id);
		src_id = fmt.mod1 & 0xff;
		src3 = thread->ReadGPR(src_id);
		if (fmt.srcB_mod == 0)
		{
			src_id = fmt.srcB;
			emu->ReadConstMem(src_id << 2, 4, (char*)&srcB);
		}
		else if (fmt.srcB_mod == 1)
		{
			src_id = fmt.srcB & 0x1ff;
			srcB = thread->ReadGPR(src_id);
		}
		else	//check it
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;


		// Execute
		dst = srcA * srcB + src3;

		// Write
		dst_id = fmt.dst;
		thread->WriteGPR(dst_id, dst);

	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
        		<<thread->getWarpId() <<" IMAD op0 "<<fmt.op0;
        std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB

        		<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
        		<<std::endl;
	}
}

void kpl_isa_IADD_A_impl(Thread *thread, Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = thread->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst;
	int srcA, srcB;

	// Pop sync stack at reconvergence PC
	if ((warp->getPC() != 0) && (warp->getPC() ==
			warp->getSyncStkTopRecPC()))
	{
		warp->setSyncStkTopRecPC(0);
		//bit_map_free(warp->sync_stack.entries[warp->sync_stack_top].
		//		active_thread_mask);
        warp->setSyncStkTopActive(-1);
		warp->decrSyncStkTop();
	}

	// Active
	active =  unsigned(1) & (warp->getSyncStkTopActive() >> (thread->getIdInWarp()-1));

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->GetPred(pred_id);
	else
		pred = ! thread->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = fmt.mod0;
		srcA = thread->ReadGPR(src_id);
		src_id = fmt.srcB;
		if (fmt.srcB_mod == 0)
		{
			emu->ReadConstMem(src_id << 2, 4, (char*)&srcB);
		}
		else if (fmt.srcB_mod == 1)
			srcB = thread->ReadGPR(src_id);
		else	//check it
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;

		if (((fmt.mod1 >> 9) & 0x1) == 1)	//FIXME
			srcB = -srcB;

		/* Execute */
		dst = srcA + srcB;

		/* Write */
		dst_id = fmt.dst;
		thread->WriteGPR(dst_id, dst);
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
    std::cerr<< "Warp id "<< std::hex
      		<<thread->getWarpId() <<" IADD op0 "<<fmt.op0;
    std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
       		<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
       		<<std::endl;
	}

}

 
void kpl_isa_IADD_B_impl(Thread *thread, Inst *inst)
{

	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = thread->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst;
	int srcA, srcB;

	// Pop sync stack at reconvergence PC
	if ((warp->getPC() != 0) && (warp->getPC() ==
			warp->getSyncStkTopRecPC()))
	{
		warp->setSyncStkTopRecPC(0);
		//bit_map_free(warp->sync_stack.entries[warp->sync_stack_top].
		//		active_thread_mask);
        warp->setSyncStkTopActive(-1);
		warp->decrSyncStkTop();
	}

	// Active
	active =  unsigned(1) & (warp->getSyncStkTopActive() >> (thread->getIdInWarp()-1));

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->GetPred(pred_id);
	else
		pred = ! thread->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = fmt.mod0;
		srcA = thread->ReadGPR(src_id);
		src_id = fmt.srcB;
		if (fmt.srcB_mod == 0)
		{
			emu->ReadConstMem(src_id << 2, 4, (char*)&srcB);
		}
		else if (fmt.srcB_mod == 1)
			srcB = thread->ReadGPR(src_id);
		else	//check it
			srcB = src_id >> 18 ? src_id | 0xfff80000 : src_id;

		if (((fmt.mod1 >> 9) & 0x1) == 1)	//FIXME
			srcB = -srcB;

		/* Execute */
		dst = srcA + srcB;

		/* Write */
		dst_id = fmt.dst;
		thread->WriteGPR(dst_id, dst);
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
    std::cerr<< "Warp id "<< std::hex
      		<<thread->getWarpId() <<" IADD op0 "<<fmt.op0;
    std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
       		<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
       		<<std::endl;
	}

}

void kpl_isa_ISETP_impl(Thread *thread, Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = thread->getWarp();
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

	// Pop sync stack at reconvergence PC
	if ((warp->getPC() != 0) && (warp->getPC() ==
			warp->getSyncStkTopRecPC()))
	{
		warp->setSyncStkTopRecPC(0);
		//bit_map_free(warp->sync_stack.entries[warp->sync_stack_top].
		//		active_thread_mask);
        warp->setSyncStkTopActive(-1);
		warp->decrSyncStkTop();
	}

	// Active
	active =  unsigned(1) & (warp->getSyncStkTopActive() >> (thread->getIdInWarp()-1));

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->GetPred(pred_id);
	else
		pred = ! thread->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Sources
		srcA_id = fmt.mod0;
		srcA = thread->ReadGPR(srcA_id);
		srcB_id = fmt.srcB;
		if (fmt.srcB_mod == 0)
		{
			emu->ReadConstMem(srcB_id << 2, 4, (char*)&srcB);
		}
		else if (fmt.srcB_mod == 1)
			srcB = thread->ReadGPR(srcB_id);
		else	//check it
			srcB = srcB_id >> 18 ? srcB_id | 0xfff80000 : srcB_id;

		// Predicates
		pred_id_1 = (fmt.dst >> 3) & 0x7;
		pred_id_2 = fmt.dst & 0x7;
		pred_id_3 = fmt.mod1 & 0x7;


		pred_3 = thread->GetPred(pred_id_3);
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
			thread->WriteGPR(pred_id_1, pred_1);
		if (pred_id_2 != 7)
			thread->WriteGPR(pred_id_2, pred_2);
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<thread->getWarpId() <<" ISETP op0 "<<fmt.op0;
		std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
				<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
				<<" cmp "<< cmp_op << " bool " << bool_op
				<<std::endl;
	}
}

void kpl_isa_EXIT_impl(Thread *thread, Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	//Emu* emu = Emu::getInstance();
	Warp* warp = thread->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

	// Pop sync stack at reconvergence PC
	if ((warp->getPC() != 0) && (warp->getPC() ==
			warp->getSyncStkTopRecPC()))
	{
		warp->setSyncStkTopRecPC(0);
		//bit_map_free(warp->sync_stack.entries[warp->sync_stack_top].
		//		active_thread_mask);
        warp->setSyncStkTopActive(-1);
		warp->decrSyncStkTop();
	}

	// Active
	active =  unsigned(1) & (warp->getSyncStkTopActive() >> (thread->getIdInWarp()-1));

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->GetPred(pred_id);
	else
		pred = ! thread->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		if(warp->getFinishedThreadCount() >= warp->getThreadCount() )
			warp->setFinishedEmu(true);
	}

	warp->setTargetpc(warp->getPC()+warp->getInstSize());
}

void kpl_isa_BRA_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_MOV_A_impl(Thread *thread, Inst *inst)
{
	std::cerr << "MOV_A" << std::endl;
}

void kpl_isa_MOV_B_impl(Thread *thread, Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Emu* emu = Emu::getInstance();
	Warp* warp = thread->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst, src;

	// Pop sync stack at reconvergence PC
	if ((warp->getPC() != 0) && (warp->getPC() ==
			warp->getSyncStkTopRecPC()))
	{
		warp->setSyncStkTopRecPC(0);
		//bit_map_free(warp->sync_stack.entries[warp->sync_stack_top].
		//		active_thread_mask);
        warp->setSyncStkTopActive(-1);
		warp->decrSyncStkTop();
	}

	// Active
	active =  unsigned(1) & (warp->getSyncStkTopActive() >> (thread->getIdInWarp()-1));

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->GetPred(pred_id);
	else
		pred = ! thread->GetPred(pred_id - 8);

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
			src = thread->ReadGPR(src_id);
		else	//check it
			src = src_id >> 18 ? src_id | 0xfff80000 : src_id;


		/* Execute */
		dst = src;

		/* Write */
		dst_id = fmt.dst;
		thread->WriteGPR(dst_id, dst);
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
        std::cerr<< "Warp id "<< std::hex
        		<<thread->getWarpId() <<" MOV_B op0 "<<fmt.op0;
        std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
        		<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
        		<<std::endl;
	}

}

void kpl_isa_MOV32I_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_LD_impl(Thread *thread, Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;
	Emu* emu = Emu::getInstance();

	// Predicates and active masks
	Warp* warp = thread->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id;
	int dst[4];
	unsigned addr;
	unsigned data_type;

	// Pop sync stack at reconvergence PC
	if ((warp->getPC() != 0) && (warp->getPC() ==
			warp->getSyncStkTopRecPC()))
	{
		warp->setSyncStkTopRecPC(0);
		//bit_map_free(warp->sync_stack.entries[warp->sync_stack_top].
		//		active_thread_mask);
        warp->setSyncStkTopActive(-1);
		warp->decrSyncStkTop();
	}

	// Active
	active =  unsigned(1) & (warp->getSyncStkTopActive() >> (thread->getIdInWarp()-1));

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->GetPred(pred_id);
	else
		pred = ! thread->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		if(fmt.op1 & 0x1) 	//offset direction
			addr = thread->ReadGPR(fmt.mod0) - (fmt.mod1 << 19) - fmt.srcB;
		else
			addr = thread->ReadGPR(fmt.mod0) + (fmt.mod1 << 19) + fmt.srcB;

		data_type = (fmt.op1 >> 2) & 0x7;

		// Execute
		emu->ReadGlobalMem(addr, 4, (char*)dst);
		if (data_type > 4)
			emu->ReadGlobalMem(addr + 4, 4, (char*)&dst[1]);
		if (data_type > 5)										//Really? FIXME
			emu->ReadGlobalMem(addr + 8, 8, (char*)&dst[2]);

		/* Write */
		dst_id = fmt.dst;
		thread->WriteGPR(dst_id, dst[0]);
		if (data_type > 4)
			thread->WriteGPR(dst_id + 1, dst[1]);
		if (data_type > 5)
		{
			thread->WriteGPR(dst_id + 2, dst[2]);
			thread->WriteGPR(dst_id + 3, dst[3]);
		}
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<thread->getWarpId() <<" LD op0 "<<fmt.op0;
		std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
				<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
				<<std::endl;
	}

}

void kpl_isa_LDS_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_ST_impl(Thread *thread, Inst *inst)
{
	// Inst bytes format
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;
	Emu* emu = Emu::getInstance();

	// Predicates and active masks
	Warp* warp = thread->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned src_id;
	int src[4];
	unsigned addr;
	unsigned data_type;

	// Pop sync stack at reconvergence PC
	if ((warp->getPC() != 0) && (warp->getPC() ==
			warp->getSyncStkTopRecPC()))
	{
		warp->setSyncStkTopRecPC(0);
		//bit_map_free(warp->sync_stack.entries[warp->sync_stack_top].
		//		active_thread_mask);
        warp->setSyncStkTopActive(-1);
		warp->decrSyncStkTop();
	}

	// Active
	active =  unsigned(1) & (warp->getSyncStkTopActive() >> (thread->getIdInWarp()-1));

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->GetPred(pred_id);
	else
		pred = ! thread->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		// Read
		if(fmt.op1 & 0x1) 	//offset direction
			addr = thread->ReadGPR(fmt.mod0) - (fmt.mod1 << 19) - fmt.srcB;
		else
			addr = thread->ReadGPR(fmt.mod0) + (fmt.mod1 << 19) + fmt.srcB;
		data_type = (fmt.op1 >> 2) & 0x7;
		src_id = fmt.dst;
		src[0] = thread->ReadGPR(src_id);
		if (data_type > 4)
			src[1] = thread->ReadGPR(src_id + 1);
		if (data_type > 5)										//Really? FIXME
		{
			src[2] = thread->ReadGPR(src_id + 2);
			src[3] = thread->ReadGPR(src_id + 3);
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
                 <<thread->getWarpId() <<" ST op0 "<<fmt.op0;
        std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
                 <<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
                 <<std::endl;
	}

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

void kpl_isa_FADD_impl(Thread *thread, Inst *inst)
{
	__NOT_IMPL__
}

void kpl_isa_NOP_impl(Thread *thread, Inst *inst)
{
}

void kpl_isa_S2R_impl(Thread *thread, Inst *inst)
{
	// Inst bytes format	//FIXME S2R description missing in Inst.h
	InstBytes inst_bytes = inst->getInstBytes();
	InstBytesGeneral0 fmt = inst_bytes.general0;

	// Predicates and active masks
	Warp* warp = thread->getWarp();
	unsigned pred;
	unsigned pred_id;
	unsigned active;

    // Operands
	unsigned dst_id, src_id;
	int dst, src;

	// Pop sync stack at reconvergence PC
	if ((warp->getPC() != 0) && (warp->getPC() ==
			warp->getSyncStkTopRecPC()))
	{
		warp->setSyncStkTopRecPC(0);
		//bit_map_free(warp->sync_stack.entries[warp->sync_stack_top].
		//		active_thread_mask);
        warp->setSyncStkTopActive(-1);
		warp->decrSyncStkTop();
	}

	// Active
	active =  unsigned(1) & (warp->getSyncStkTopActive() >> (thread->getIdInWarp()-1));

	// Predicate
	pred_id = fmt.pred;
	if (pred_id <= 7)
		pred = thread->GetPred(pred_id);
	else
		pred = ! thread->GetPred(pred_id - 8);

	// Execute
	if (active == 1 && pred == 1)
	{
		/* Read */
		src_id = fmt.srcB & 0xff;
		if (src_id == SR_CLOCKLO)	//No cycle count for now
			;//src = thread->grid->emu->inst_count & 0xffffffff;
		else if (src_id == SR_CLOCKHI)
			;//src = (thread->grid->emu->inst_count >> 32) & 0xffffffff;
		else if (fmt.srcB_mod == 1)
			src = thread->ReadSR(src_id);

		// Execute
		dst = src;

		// Write
		dst_id = fmt.dst;
		thread->WriteGPR(dst_id, dst);
	}
	if(getenv("M2S_KPL_ISA_DEBUG"))
	{
		std::cerr<< "Warp id "<< std::hex
				<<thread->getWarpId() <<" S2R op0 "<<fmt.op0;
		std::cerr<<" dst " <<fmt.dst <<" mod0 " <<fmt.mod0 << " s " <<fmt.s << " srcB " <<fmt.srcB
				<<" mod1 " <<fmt.mod1 << " op1 "<< fmt.op1 <<" srcB_mod " <<fmt.srcB_mod
				<<std::endl;
	}

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
