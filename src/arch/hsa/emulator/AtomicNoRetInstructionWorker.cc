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

#include <lib/cpp/String.h>
#include <lib/cpp/Error.h>
#include <arch/hsa/disassembler/AsmService.h>
#include <arch/hsa/disassembler/Brig.h>
#include <arch/hsa/disassembler/BrigCodeEntry.h>
#include <memory/Memory.h>

#include "AtomicNoRetInstructionWorker.h"
#include "WorkItem.h"

namespace HSA
{

AtomicNoRetInstructionWorker::AtomicNoRetInstructionWorker(
		WorkItem *work_item,
		StackFrame *stack_frame,
		mem::Memory *memory) :
		HsaInstructionWorker(work_item, stack_frame),
		memory(memory)
{
}


AtomicNoRetInstructionWorker::~AtomicNoRetInstructionWorker()
{
}


template<typename T>
void AtomicNoRetInstructionWorker::Inst_AtomicNoRet_Aux(
		BrigCodeEntry *instruction)
{

	// Retrieve operand value
	uint32_t address;
	T src0;
	operand_value_retriever->Retrieve(instruction, 0, &address);
	address = work_item->getFlatAddress(instruction->getSegment(), address);
	operand_value_retriever->Retrieve(instruction, 1, &src0);

	// Get the value to process
	T original;
	T result;
	memory->Read(address, sizeof(T), (char *)&original);

	// Do different operation according to the operation field
	switch(instruction->getAtomicOperation())
	{
	case BRIG_ATOMIC_ST:

		result = original;
		break;

	case BRIG_ATOMIC_AND:

		result = original & src0;
		break;

	case BRIG_ATOMIC_OR:

		result = original | src0;
		break;

	case BRIG_ATOMIC_XOR:

		result = original ^ src0;
		break;

	case BRIG_ATOMIC_ADD:

		result = original + src0;
		break;

	case BRIG_ATOMIC_SUB:

		result = original - src0;
		break;

	case BRIG_ATOMIC_MIN:

		result = (original < src0) ? original : src0;
		break;

	case BRIG_ATOMIC_MAX:

		result = (original > src0) ? original : src0;
		break;

	case BRIG_ATOMIC_WRAPINC:

		result = (original >= src0) ? 0 : original + 1;
		break;

	case BRIG_ATOMIC_WRAPDEC:

		result = ((original == 0) || (original > src0)) ? src0 :
				original - 1;
		break;

	case BRIG_ATOMIC_CAS:
	case BRIG_ATOMIC_EXCH:
	case BRIG_ATOMIC_LD:
	case BRIG_ATOMIC_WAIT_EQ:
	case BRIG_ATOMIC_WAIT_NE:
	case BRIG_ATOMIC_WAIT_LT:
	case BRIG_ATOMIC_WAIT_GTE:
	case BRIG_ATOMIC_WAITTIMEOUT_EQ:
	case BRIG_ATOMIC_WAITTIMEOUT_NE:
	case BRIG_ATOMIC_WAITTIMEOUT_LT:
	case BRIG_ATOMIC_WAITTIMEOUT_GTE:

		break;
	}

	// Write the result back
	memory->Write(address, sizeof(T), (char *)&result);
}


void AtomicNoRetInstructionWorker::Execute(BrigCodeEntry *instruction)
{
	BrigType type = instruction->getType();
	switch(type)
	{
	case BRIG_TYPE_B32:
	case BRIG_TYPE_U32:

		Inst_AtomicNoRet_Aux<uint32_t>(instruction);
		break;

	case BRIG_TYPE_B64:
	case BRIG_TYPE_U64:

		Inst_AtomicNoRet_Aux<uint64_t>(instruction);
		break;

	case BRIG_TYPE_S32:

		Inst_AtomicNoRet_Aux<int32_t>(instruction);
		break;

	case BRIG_TYPE_S64:

		Inst_AtomicNoRet_Aux<int64_t>(instruction);
		break;

	default:

		throw misc::Panic("Unsupported source type for ATOMICNORET.");
		break;
	}

	work_item->MovePcForwardByOne();
}

}  // namespace HSA
