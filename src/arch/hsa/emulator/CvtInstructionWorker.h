/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#ifndef ARCH_HSA_EMULATOR_CVTINSTRUCTIONWORKER_H
#define ARCH_HSA_EMULATOR_CVTINSTRUCTIONWORKER_H

#include "HsaInstructionWorker.h"

namespace HSA
{

class CvtInstructionWorker : public HsaInstructionWorker
{
	template<typename SrcType, typename DstType>
	void Inst_CVT_chop_Aux(BrigCodeEntry *instruction);
	template<typename SrcType, typename DstType>
	void Inst_CVT_zext_Aux(BrigCodeEntry *instruction);
	template<typename SrcType, typename DstType>
	void Inst_CVT_sext_Aux(BrigCodeEntry *instruction);
	template<typename SrcType, typename DstType>
	void Inst_CVT_u2f_Aux(BrigCodeEntry *instruction);

public:
	CvtInstructionWorker(WorkItem *work_item,
			StackFrame *stack_frame);
	virtual ~CvtInstructionWorker();
	void Execute(BrigCodeEntry *instruction) override;
};

}

#endif  // ARCH_HSA_EMULATOR_CVTINSTRUCTIONWORKER_H
