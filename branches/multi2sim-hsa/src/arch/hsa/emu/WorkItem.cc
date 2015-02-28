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

#include <arch/hsa/asm/AsmService.h>
//#include <arch/hsa/driver/Driver.h>

#include "WorkItem.h"
#include "SegmentManager.h"

namespace HSA
{

WorkItem::WorkItem(WorkGroup *work_group,
			unsigned private_segment_size,
			unsigned int abs_id_x,
			unsigned int abs_id_y,
			unsigned int abs_id_z,
			Function *root_function)
{
	// Set global emulator object
	emu = Emu::getInstance();
	loader = ProgramLoader::getInstance();
	binary = loader->getBinary();

	// Set work group
	this->work_group = work_group;

	// Set absolute ids
	this->abs_id_x = abs_id_x;
	this->abs_id_y = abs_id_y;
	this->abs_id_z = abs_id_z;

	// Set the first stack frame
	StackFrame *frame = new StackFrame(root_function, this);
	stack.push_back(std::unique_ptr<StackFrame>(frame));

	// Set the private segment memory manager
	mem::Memory *memory = Emu::getInstance()->getMemory();
	private_segment.reset(new SegmentManager(memory, private_segment_size));

	// Dump initial state of the stack frame when a work item created.
	if (Emu::isa_debug)
	{
		frame->Dump(Emu::isa_debug);
		Emu::isa_debug << "\n";
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
		StackFrame *frame = (*it).get();
		os << misc::fmt("#%d ", frame_count++);
		os << misc::fmt("%s ", (*it)->getFunction()->getName().c_str());

		// Dump arguments and their value
		os << "(";
		frame->getFunctionArguments()->DumpInLine(os);
		os << ")";
		os << "\n";
	}
	os << "***** ********* *****\n\n";

}


bool WorkItem::ReturnFunction()
{
	// Dump stack frame information
	StackFrame *callee_frame = stack.back().get();
	if (Emu::isa_debug)
	{
		Emu::isa_debug << "Callee_frame: \n";
		callee_frame->Dump(Emu::isa_debug);
		Emu::isa_debug << "\n";
	}

	// Check if this frame have a call back function when it return
	if (callee_frame->getReturnCallback())
	{
		StackFrame::CallbackFn callback = 
			callee_frame->getReturnCallback();
		callback(callee_frame->getReturnCallbackInfo());
		return true;
	}

	// Retrieve second last element
	if (stack.size() > 1)
	{
		auto it = stack.rbegin();
		it ++;
		StackFrame *caller_frame = (*it).get();

		// Process value return
		Function *function = callee_frame->getFunction();
		BrigCodeEntry *inst = caller_frame->getPc();
		function->PassBackByValue(caller_frame, callee_frame, inst);

		if (Emu::isa_debug)
		{
			Emu::isa_debug << "Caller frame \n";
			caller_frame->Dump(Emu::isa_debug);
			Emu::isa_debug << "\n";
		}

	}

	// Pop frame at stack top
	stack.pop_back();

	// When stack gets empty, the work item have finished its task.
	if (stack.empty())
	{
		return false;
	}
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
		Emu::isa_debug << misc::fmt("Argument scope created "
				"(size %d)\n", size);

		break;
	}

	case BRIG_KIND_DIRECTIVE_ARG_BLOCK_END:

		stack_top->CloseArgumentScope();
		break;

	case BRIG_KIND_DIRECTIVE_VARIABLE:

		DeclearVariable();
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


char *WorkItem::getVariableBuffer(unsigned char segment,
		const std::string &name)
{
	// Get stack top frame
	StackFrame *stack_top = stack.back().get();

	//
	switch (segment)
	{
	case BRIG_SEGMENT_NONE:

		throw misc::Panic("Unsupported segment NONE.");
		break;

	case BRIG_SEGMENT_FLAT:

		throw misc::Panic("Unsupported segment FLAT.");
		break;

	case BRIG_SEGMENT_GLOBAL:
	case BRIG_SEGMENT_GROUP:
	case BRIG_SEGMENT_PRIVATE:

	{
		VariableScope *variable_scope = stack_top->getVariableScope();
		return variable_scope->getBuffer(name);
		break;
	}

	case BRIG_SEGMENT_KERNARG:

		{
			VariableScope *kernel_arguments = work_group->
					getGrid()->getKernelArguments();
			char *buffer;
			if (kernel_arguments)
			{
				buffer = kernel_arguments->getBuffer(name);
			}

			return buffer;
			break;
		}
		break;

	case BRIG_SEGMENT_READONLY:

		throw misc::Panic("Unsupported segment READONLY.");
		break;

	case BRIG_SEGMENT_SPILL:

		throw misc::Panic("Unsupported segment SPILL.");
		break;

	case BRIG_SEGMENT_ARG:

	{
		// Get argument scope if in curve bracket, otherwise, get
		// function arguments
		VariableScope *argument_scope = stack_top->getArgumentScope();
		char *buffer;
		if (argument_scope)
		{
			buffer = argument_scope->getBuffer(name);
			if (buffer)
				return buffer;
		}

		// If variable is not declared in argument scope,
		// try function arguments.
		argument_scope = stack_top->getFunctionArguments();
		buffer = argument_scope->getBuffer(name);
		if(!buffer)
		{
			throw misc::Panic("Argument not found\n");
		}

		return buffer;
		break;
	}

	default:

		throw misc::Panic("Unsupported segment.");
		break;
	}

	return nullptr;
}


void WorkItem::DeclearVariable()
{
	StackFrame *stack_top = stack.back().get();
	BrigCodeEntry *dir = stack_top->getPc();
	unsigned int size = AsmService::TypeToSize(dir->getType());
	std::string name = dir->getName();

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

	{
		VariableScope *variable_scope = stack_top->getVariableScope();
		unsigned long long dim = dir->getDim();
		variable_scope->DeclearVariable(name, dir->getType(),
				dim, nullptr);
		Emu::isa_debug << misc::fmt("Declaring variable %s with "
				"size %d[%lld]\n", name.c_str(), size, dim);
		break;
	}

	case BRIG_SEGMENT_GROUP:

	{
		VariableScope *variable_scope = stack_top->getVariableScope();
		SegmentManager *segment = work_group->getGroupSegment();
		unsigned long long dim = dir->getDim();
		variable_scope->DeclearVariable(name, dir->getType(),
				dim, segment);
		Emu::isa_debug << misc::fmt("Declaring variable %s with "
						"size %d[%lld]\n", name.c_str(),
						size, dim);
		break;
	}

	case BRIG_SEGMENT_PRIVATE:

	{
		VariableScope *variable_scope = stack_top->getVariableScope();
		SegmentManager *segment = private_segment.get();
		unsigned long long dim = dir->getDim();
		variable_scope->DeclearVariable(name, dir->getType(),
				dim, segment);
		Emu::isa_debug << misc::fmt("Declaring variable %s with "
				"size %d[%lld]\n", name.c_str(), size, dim);
		break;
	}

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

	{
		// Get the variable scope
		VariableScope *variable_scope;
		if (stack_top->getArgumentScope())
			variable_scope = stack_top->getArgumentScope();
		else
			throw misc::Panic("Error creating argument, not in a "
					"argument scope");

		// Get the arg segment manager
		SegmentManager *arg_segment = stack_top->getArgSegment();

		// Check if the argument scope started
		if (!arg_segment || !variable_scope)
			throw misc::Panic("Argument segment not started\n");

		variable_scope->DeclearVariable(name, dir->getType(),
				dir->getDim(), arg_segment);
		Emu::isa_debug << misc::fmt("Declaring variable %s width"
				" size %d\n", name.c_str(), size);
		break;
	}

	default:

		throw misc::Panic("Unsupported segment.");
		break;
	}

	Emu::isa_debug << misc::fmt("Create variable: %s %s(%d)\n",
			AsmService::TypeToString(dir->getType()).c_str(),
			name.c_str(), size);
}


bool WorkItem::Execute()
{
	if (stack.size() == 0)
	{
		return false;
	}

	// Retrieve stack top
	StackFrame *stack_top = getStackTop();

	// Record frame status before the instruction is executed
	BrigCodeEntry *inst = stack_top->getPc();
	if (inst && inst->isInstruction())
	{
		Emu::isa_debug << "Executing: ";
		Emu::isa_debug << *inst;

		// Get the function according to the opcode and perform the inst
		BrigOpcode opcode = inst->getOpcode();
		ExecuteInstFn fn = WorkItem::execute_inst_fn[opcode];
		try
		{
			(this->*fn)();
		}
		catch(misc::Panic &panic)
		{
			std::cerr << panic.getMessage();
			exit(1);
		}
	}
	else if (inst && !inst->isInstruction())
	{
		Emu::isa_debug << "Executing: ";
		Emu::isa_debug << *inst;
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

	// Record frame status after the instruction is executed
	stack_top = getStackTop();
	if (Emu::isa_debug)
		stack_top->Dump(Emu::isa_debug);
	Emu::isa_debug << "\n";

	// Return true, since the execution is not finished
	return true;
}


}  // namespace HSA

