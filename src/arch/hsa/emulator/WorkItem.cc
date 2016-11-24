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

#include <arch/hsa/disassembler/Brig.h>
#include <arch/hsa/disassembler/AsmService.h>

#include "WorkItem.h"
#include "SegmentManager.h"

#include "AddInstructionWorker.h"
#include "AndInstructionWorker.h"
#include "AtomicNoRetInstructionWorker.h"
#include "BrInstructionWorker.h"
#include "BarrierInstructionWorker.h"
#include "CbrInstructionWorker.h"
#include "CurrentWorkGroupSizeInstructionWorker.h"
#include "CmpInstructionWorker.h"
#include "CvtInstructionWorker.h"
#include "GridSizeInstructionWorker.h"
#include "LdInstructionWorker.h"
#include "MulInstructionWorker.h"
#include "StInstructionWorker.h"
#include "SubInstructionWorker.h"
#include "LdaInstructionWorker.h"
#include "MadInstructionWorker.h"
#include "MemFenceInstructionWorker.h"
#include "MovInstructionWorker.h"
#include "OrInstructionWorker.h"
#include "RetInstructionWorker.h"
#include "ShlInstructionWorker.h"
#include "ShrInstructionWorker.h"
#include "StInstructionWorker.h"
#include "WorkItemAbsIdInstructionWorker.h"
#include "WorkItemIdInstructionWorker.h"
#include "WorkGroupIdInstructionWorker.h"


namespace HSA
{

WorkItem::WorkItem()
{
}


void WorkItem::Initialize(WorkGroup *work_group,
		unsigned private_segment_size,
		unsigned int abs_id_x,
		unsigned int abs_id_y,
		unsigned int abs_id_z,
		Function *root_function)
{
	// Set global emulator object
	emulator = Emulator::getInstance();
	executable = root_function->getExecutable();

	// Set work group
	this->work_group = work_group;

	// Set absolute ids
	this->abs_id_x = abs_id_x;
	this->abs_id_y = abs_id_y;
	this->abs_id_z = abs_id_z;

	// Set the first stack frame
	stack.push_back(misc::new_unique<StackFrame>(
			root_function, this, nullptr));
	StackFrame *frame = stack.back().get();


	// Set the status of the work item to be active
	status = WorkItemStatusActive;

	// Set the private segment memory manager
	mem::Memory *memory = Emulator::getInstance()->getMemory();
	private_segment.reset(new SegmentManager(memory, private_segment_size));

	// Dump initial state of the stack frame when a work item created.
	if (getAbsoluteFlattenedId() == 0)
	{
		if (Emulator::isa_debug)
		{
			frame->Dump(Emulator::isa_debug);
			Emulator::isa_debug << "\n";
		}
	}
}


WorkItem::~WorkItem() 
{
}


bool WorkItem::MovePcForwardByOne()
{
	// Retrieve the stack top
	StackFrame *stack_top = stack.back().get();

	// Set the stackframe's pc to the next instuction
	auto next_pc = stack_top->getPc()->Next();

	// If next pc is beyond last inst, the last instruction of the function
	// is executed. Return the function.
	if (!next_pc ||
		next_pc->getOffset() >
		stack_top->getFunction()->getLastEntry()->getOffset())
	{
		ReturnFunction();
		return false;
	}

	// Set program counter to next instruction
	stack_top->setPc(std::move(next_pc));

	// Returns true to tell the caller that the function is not returned
	return true;
}


void WorkItem::Backtrace(std::ostream &os = std::cout) const
{
	os << "***** Backtrace *****\n";
	int frame_count = 1;
	for (auto it = stack.rbegin(); it != stack.rend(); it++)
	{
		// StackFrame *frame = (*it).get();
		os << misc::fmt("#%d ", frame_count++);
		os << misc::fmt("%s ", (*it)->getFunction()->getName().c_str());

		// Dump arguments and their value
		os << "(";
		os << "To be supported";
		//frame->getFunctionArguments()->DumpInLine(os);
		os << ")";
		os << "\n";
	}
	os << "***** ********* *****\n\n";
}


bool WorkItem::ReturnFunction()
{
	// Dump stack frame information
	StackFrame *callee_frame = stack.back().get();
	if (getAbsoluteFlattenedId() == 0) {
		if (Emulator::isa_debug)
		{
			Emulator::isa_debug << "Callee_frame: \n";
			callee_frame->Dump(Emulator::isa_debug);
			Emulator::isa_debug << "\n";
		}
	}

	// Retrieve second last element
	if (stack.size() > 1)
	{
		auto it = stack.rbegin();
		it ++;
		StackFrame *caller_frame = (*it).get();

		// Process value return
		// Function *function = callee_frame->getFunction();
		// BrigCodeEntry *inst = caller_frame->getPc();
		// function->PassBackByValue(caller_frame, callee_frame, inst);

		// Dump information
		if (getAbsoluteFlattenedId() == 0) 
		{
			if (Emulator::isa_debug)
			{
				Emulator::isa_debug << "Caller frame \n";
				caller_frame->Dump(Emulator::isa_debug);
				Emulator::isa_debug << "\n";
			}
		}

	}

	// Pop frame at stack top
	stack.pop_back();

	// When stack gets empty, the work item have finished its task.
	if (stack.empty())
		return false;
	else
	{
		// When return to the caller, the pc in caller is still
		// pointing to the function call. Therefore, we need to move
		// the pc forward.
		MovePcForwardByOne();
	}

	// Return true if stack is empty
	return !stack.empty();
}


unsigned int WorkItem::getLocalIdX() const
{
	unsigned int group_size_x = work_group->getGrid()->getGroupSizeX();
	return this->abs_id_x - work_group->getGroupIdX() * group_size_x;
}


unsigned int WorkItem::getLocalIdY() const
{
	unsigned int group_size_y = work_group->getGrid()->getGroupSizeY();
	return this->abs_id_y - work_group->getGroupIdY() * group_size_y;
}


unsigned int WorkItem::getLocalIdZ() const
{
	unsigned int group_size_z = work_group->getGrid()->getGroupSizeZ();
	return this->abs_id_z - work_group->getGroupIdZ() * group_size_z;
}


unsigned int WorkItem::getFlattenedId() const
{
	return getLocalIdX() +
			getLocalIdY() * work_group->getGrid()->getGroupSizeX()
			+ getLocalIdZ() *
			work_group->getGrid()->getGroupSizeX() *
			work_group->getGrid()->getGroupSizeY();
}


unsigned int WorkItem::getAbsoluteFlattenedId() const
{
	return abs_id_x + abs_id_y * work_group->getGrid()->getGroupSizeX() +
			abs_id_z * work_group->getGrid()->getGroupSizeY()
			* work_group->getGrid()->getGroupSizeZ();
}


void WorkItem::ExecuteDirective()
{
	// Get current instruction offset in code section
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *pc = stack_top->getPc();

	switch (pc->getKind())
	{
	case BRIG_KIND_DIRECTIVE_ARG_BLOCK_START:

	{
		auto dir = pc->Next();
		unsigned size = 0;
		while (dir->getKind() != BRIG_KIND_DIRECTIVE_ARG_BLOCK_END)
		{
			if (dir->getKind() == BRIG_KIND_DIRECTIVE_VARIABLE)
			{
				size += AsmService::TypeToSize(
						dir->getType());
			}
			dir = dir->Next();
		}
		stack_top->StartArgumentScope(size);

		// Log into debug isa
		if (getAbsoluteFlattenedId() == 0) 
		{
			Emulator::isa_debug << misc::fmt("Argument scope created "
					"(size %d)\n", size);
		}
		break;
	}

	case BRIG_KIND_DIRECTIVE_ARG_BLOCK_END:

		stack_top->CloseArgumentScope();
		break;

	case BRIG_KIND_DIRECTIVE_VARIABLE:

		DeclareVariable();
		break;

	default:
		break;
	}

	// Move next directive pointer forwards
	if (!this->MovePcForwardByOne())
		return;
	else
		pc = stack_top->getPc();


}


unsigned WorkItem::getFlatAddress(BrigSegment segment, unsigned address)
{
	// Declare the flat address to return
	unsigned flat_address = 0;
	switch (segment) 
	{
	case BRIG_SEGMENT_FLAT:

		// Do nothing, simply return the address
		flat_address = address;
		break;
	
	case BRIG_SEGMENT_GLOBAL:

		// Global memory is always the same with flat memory address
		flat_address = address;
		break;
	
	case BRIG_SEGMENT_GROUP:
	
	{
		// Get group segment manager and translate address
		SegmentManager *segment = work_group->getGroupSegment();
		flat_address = segment->getFlatAddress(address);
		break;
	}

	case BRIG_SEGMENT_PRIVATE:

		// Get private segment manager and translate address
		flat_address = private_segment->getFlatAddress(address);	
		break;

	case BRIG_SEGMENT_KERNARG:

	{
		// Get the grid
		Grid *grid = work_group->getGrid();
		SegmentManager *segment = grid->getKernargSegment();
		flat_address = segment->getFlatAddress(address);
		break;
	}
	
	default:

		// Segment net supported
		throw misc::Panic("Unsupported segment");

	}

	// Leave debug information
	// Emulator::isa_debug << misc::fmt("Converting address %d to flat address "
	//		"%d.\n", address, flat_address);
	
	// Return the flat address
	return flat_address;
}


void WorkItem::DeclareVariableGlobal(const std::string &name, BrigType type,
		unsigned long long dim)
{
	// If not an array, set dim to 1
	if (dim == 0) dim = 1;
	
	// Get the global memory manager
	mem::Manager *manager = Emulator::getInstance()->getMemoryManager();
	unsigned size = AsmService::TypeToSize(type);
	unsigned address = manager->Allocate(size * dim);

	// Create the variable 
	auto variable = misc::new_unique<Variable>(name, type, dim, 
			address, BRIG_SEGMENT_GLOBAL, false);

	// Add the variable to stack top
	StackFrame *stack_top = getStackTop();
	stack_top->addVariable(std::move(variable));
}


void WorkItem::DeclareVariableGroup(const std::string &name, BrigType type,
			unsigned long long dim)
{
	// If not an array, set dim to 1
	if (dim == 0) dim = 1;
	
	// Get the global memory manager
	SegmentManager *manager = work_group->getGroupSegment();
	unsigned size = AsmService::TypeToSize(type);
	unsigned address = manager->Allocate(size * dim);

	// Create the variable 
	auto variable = misc::new_unique<Variable>(name, type, dim, 
			address, BRIG_SEGMENT_GROUP, false);

	// Add the variable to stack top
	StackFrame *stack_top = getStackTop();
	stack_top->addVariable(std::move(variable));
}


void WorkItem::DeclareVariablePrivate(const std::string &name, BrigType type,
			unsigned long long dim)
{
	// If not an array, set dim to 1
	if (dim == 0) dim = 1;
	
	// Get the global memory manager
	SegmentManager *manager = private_segment.get();
	unsigned size = AsmService::TypeToSize(type);
	unsigned address = manager->Allocate(size * dim);

	// Create the variable 
	auto variable = misc::new_unique<Variable>(name, type, dim, 
			address, BRIG_SEGMENT_PRIVATE, false);

	// Add the variable to stack top
	StackFrame *stack_top = getStackTop();
	stack_top->addVariable(std::move(variable));
}


void WorkItem::DeclareVariableArgument(const std::string &name, BrigType type,
			unsigned long long dim)
{
	// If not an array, set dim to 1
	if (dim == 0) dim = 1;
	
	// Get the argument section memory manager
	StackFrame *stack_top = getStackTop();
	SegmentManager *manager = stack_top->getArgumentSegment();
	if (!manager) 
		throw misc::Panic("Argument scope not started.");

	// Allocate memory
	unsigned size = AsmService::TypeToSize(type);
	unsigned address = manager->Allocate(size * dim);

	// Create the variable 
	auto variable = misc::new_unique<Variable>(name, type, dim, 
			address, BRIG_SEGMENT_ARG, false);

	// Add the variable to stack top
	stack_top->addVariable(std::move(variable));
}


void WorkItem::DeclareVariable()
{
	StackFrame *stack_top = getStackTop();
	BrigCodeEntry *dir = stack_top->getPc();
	std::string name = dir->getName();
	BrigType type = dir->getType();
	unsigned long long dim = dir->getDim();

	// Allocate memory in different segment
	switch (dir->getSegment())
	{
	case BRIG_SEGMENT_NONE:

		throw misc::Panic("Unsupported segment NONE.");
		break;

	case BRIG_SEGMENT_FLAT:

		throw misc::Panic("Cannot create variable in segment FLAT.");
		break;

	case BRIG_SEGMENT_GLOBAL:

		DeclareVariableGlobal(name, type, dim);
		break;

	case BRIG_SEGMENT_GROUP:

		DeclareVariableGroup(name, type, dim);
		break;

	case BRIG_SEGMENT_PRIVATE:

		DeclareVariablePrivate(name, type, dim);
		break;

	case BRIG_SEGMENT_KERNARG:

		throw misc::Panic("Cannot declare variable in KERNARG segment");
		break;

	case BRIG_SEGMENT_READONLY:

		throw misc::Panic("Unsupported segment READONLY.");
		break;

	case BRIG_SEGMENT_SPILL:

		throw misc::Panic("Unsupported segment SPILL.");
		break;

	case BRIG_SEGMENT_ARG:
		
		DeclareVariableArgument(name, type, dim);
		break;
	
	default:

		throw misc::Panic("Unsupported segment.");
		break;

	}
	Emulator::isa_debug << misc::fmt("Create variable: %s %s(%d)[%lld]\n",
			AsmService::TypeToString(type).c_str(),
			name.c_str(), AsmService::TypeToSize(type), dim);
}


std::unique_ptr<HsaInstructionWorker> WorkItem::getInstructionWorker(
		BrigCodeEntry *instruction) 
{
	BrigOpcode opcode = instruction->getOpcode();
	StackFrame *stack_top = getStackTop();
	switch(opcode) 
	{
	case BRIG_OPCODE_ADD:

		return misc::new_unique<AddInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_AND:

		return misc::new_unique<AndInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_ATOMICNORET:

		return misc::new_unique<AtomicNoRetInstructionWorker>(this,
				stack_top,
				Emulator::getInstance()->getMemory());

	case BRIG_OPCODE_BR:

		return misc::new_unique<BrInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_BARRIER:

		return misc::new_unique<BarrierInstructionWorker>(this,
				stack_top);

	case BRIG_OPCODE_CBR:

		return misc::new_unique<CbrInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_CURRENTWORKGROUPSIZE:

		return misc::new_unique<CurrentWorkGroupSizeInstructionWorker>(
				this, stack_top);

	case BRIG_OPCODE_SHL:

		return misc::new_unique<ShlInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_SHR:

		return misc::new_unique<ShrInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_CMP:

		return misc::new_unique<CmpInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_CVT:

		return misc::new_unique<CvtInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_GRIDSIZE:

		return misc::new_unique<GridSizeInstructionWorker>(this,
				stack_top);

	case BRIG_OPCODE_LD:

		return misc::new_unique<LdInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_LDA:

		return misc::new_unique<LdaInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_MAD:

		return misc::new_unique<MadInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_MUL:

		return misc::new_unique<MulInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_MOV:

		return misc::new_unique<MovInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_MEMFENCE:

		return misc::new_unique<MemFenceInstructionWorker>(this,
				stack_top);

	case BRIG_OPCODE_OR:

		return misc::new_unique<OrInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_ST:

		return misc::new_unique<StInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_SUB:

		return misc::new_unique<SubInstructionWorker>(this, stack_top);


	case BRIG_OPCODE_RET:

		return misc::new_unique<RetInstructionWorker>(this, stack_top);

	case BRIG_OPCODE_WORKITEMABSID:

		return misc::new_unique<WorkItemAbsIdInstructionWorker>(
				this, stack_top);

	case BRIG_OPCODE_WORKITEMID:

		return misc::new_unique<WorkItemIdInstructionWorker>(
				this, stack_top);

	case BRIG_OPCODE_WORKGROUPID:

		return misc::new_unique<WorkGroupIdInstructionWorker>(
				this, stack_top);

	default:
		throw misc::Panic(misc::fmt("Opcode %s (%d) not implemented.",
				AsmService::OpcodeToString(opcode).c_str(),
				opcode));
	}
}


bool WorkItem::Execute()
{
	// Only execute the active work item
	if (status != WorkItemStatusActive)
		return true;

	// The root function has returned, then stop the execution of the
	// work-item.
	if (stack.size() == 0)
	{
		return false;
	}

	// Retrieve stack top
	StackFrame *stack_top = getStackTop();

	// Increase instruction counter
	Emulator::getInstance()->incNumInstructions();

	// Execute the instruction or directory
	BrigCodeEntry *inst = stack_top->getPc();
	if (inst && inst->isInstruction())
	{
		if (getAbsoluteFlattenedId() == 0)
		{
			Emulator::isa_debug << misc::fmt("WorkItem: %d\n",
					getAbsoluteFlattenedId());
			Emulator::isa_debug << "Executing: ";
			Emulator::isa_debug << *inst;

//			Emulator::isa_debug << "Before: ";
//			if (Emulator::isa_debug)
//				stack_top->Dump(Emulator::isa_debug);
//			Emulator::isa_debug << "\n";
		}

		// Get the function according to the opcode and perform the inst
		auto instruction_worker = getInstructionWorker(inst);
		if (instruction_worker.get())
		{
			instruction_worker->Execute(inst);
		}

		// Return false if execution finished
		if (stack.empty())
			return false;
		
		// Record frame status after the instruction is executed
		stack_top = getStackTop();
		if (getAbsoluteFlattenedId() == 0)
		{
			if (Emulator::isa_debug)
				stack_top->Dump(Emulator::isa_debug);
			Emulator::isa_debug << "\n";
		}

	}
	else if (inst && !inst->isInstruction())
	{
		ExecuteDirective();
	}
	else
	{
		if(!MovePcForwardByOne())
		{
			return false;
		}
	}

	// Return false if execution finished
	if (stack.empty())
		return false;
		
	// Return true, since the execution is not finished
	return true;
}


Grid *WorkItem::getGrid() const 
{
	return work_group->getGrid();
}

}  // namespace HSA

