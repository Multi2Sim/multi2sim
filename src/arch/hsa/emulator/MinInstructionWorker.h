/*
 * MinInstructionWorker.h
 *
 *  Created on: May 18, 2016
 *      Author: yifan
 */

#ifndef SRC_ARCH_HSA_EMULATOR_MININSTRUCTIONWORKER_H
#define SRC_ARCH_HSA_EMULATOR_MININSTRUCTIONWORKER_H

#include "HsaInstructionWorker.h"

namespace HSA
{

class MinInstructionWorker : public HsaInstructionWorker
{
	template<typename T>
	void Inst_Min_Aux(BrigCodeEntry *instruction);
public:
	MinInstructionWorker (WorkItem *work_item,
			      StackFrame *stack_frame);
	virtual ~MinInstructionWorker ();
	void Execute(BrigCodeEntry *instruction) override;
};

}   // namespace HSA

#endif  // SRC_ARCH_HSA_EMULATOR_MININSTRUCTIONWORKER_H
