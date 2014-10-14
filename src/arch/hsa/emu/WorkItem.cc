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

#include <arch/hsa/asm/BrigDef.h>
#include <arch/hsa/driver/Driver.h>

#include "WorkItem.h"

namespace HSA
{

WorkItem::WorkItem(WorkGroup *work_group,
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
	BrigInstEntry inst(stack_top->getPc(),
				ProgramLoader::getInstance()->getBinary());
	char *next_pc = inst.next();

	// If next pc is beyond last inst, the last instruction of the function
	// is executed. Return the function.
	if (next_pc > stack_top->getFunction()->getLastInst())
	{
		ReturnFunction();
		return false;
	}

	// Set program counter to next instruction
	stack_top->setPc(next_pc);

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
		callee_frame->Dump(Emu::isa_debug);
		Emu::isa_debug << "\n";
	}

	// Check if this frame is a runtime callback.
	if (callee_frame->getReturnCallback())
	{
		stack.pop_back();
		StackFrame::CallbackFn callback = 
			callee_frame->getReturnCallback();
		callback(Emu::getInstance()->getMemory(), 
				callee_frame->getReturnCallbackArgsPtr());
		return true;
	}

	// Retrieve second last element
	if (stack.size()>1)
	{
		auto it = stack.rbegin();
		it ++;

		StackFrame *caller_frame = (*it).get();
		if (Emu::isa_debug)
		{
			Emu::isa_debug << "Caller frame \n";
			caller_frame->Dump(Emu::isa_debug);
			Emu::isa_debug << "\n";
		}

		// Process value return
		Function *function = callee_frame->getFunction();
		BrigInstEntry inst = BrigInstEntry(caller_frame->getPc(),
				binary);
		function->PassBackByValue(caller_frame->getArgumentScope(),
				callee_frame->getFunctionArguments(),
				&inst);

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


void WorkItem::ProcessRelatedDirectives()
{
	// Get current instruction offset in code section
	StackFrame *stack_top = stack.back().get();
	unsigned int code_offset = stack_top->getCodeOffset();

	// Retrieve directive
	struct BrigDirectiveBase *dir =
			(struct BrigDirectiveBase *)
			stack_top->getNextDirective();

	// Traverse all the directives in front of current inst
	while (dir && dir->code <= code_offset)
	{
		BrigDirEntry dir_entry((char *)dir,
				ProgramLoader::getInstance()->getBinary());

		// Execute code affiliated with
		if (dir->code == code_offset)
		{
			switch (dir_entry.getKind())
			{
			case BRIG_DIRECTIVE_ARG_SCOPE_START:

				stack_top->StartArgumentScope();
				break;

			case BRIG_DIRECTIVE_ARG_SCOPE_END:

				stack_top->CloseArgumentScope();
				break;

			case BRIG_DIRECTIVE_VARIABLE:

				DeclearVariable();
				break;
			}
		}

		// Move next directive pointer forwards
		dir = (struct BrigDirectiveBase *)dir_entry.nextTop();
		stack_top->setNextDirective((char *)dir);
	}


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

		throw misc::Panic("Unsupported segment KERNARG.");
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
		//argument_scope->Dump(std::cout, 0);
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
	BrigDirectiveVariable *dir =
			(BrigDirectiveVariable *)stack_top->getNextDirective();
	std::string name = BrigStrEntry::GetStringByOffset(binary, dir->name);

	// Allocate memory in different segment
	switch (dir->segment)
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
		variable_scope->DeclearVariable(name, dir->size/8, dir->type);
		break;
	}

	case BRIG_SEGMENT_KERNARG:

		throw misc::Panic("Unsupported segment KERNARG.");
		break;

	case BRIG_SEGMENT_READONLY:

		throw misc::Panic("Unsupported segment READONLY.");
		break;

	case BRIG_SEGMENT_SPILL:

		throw misc::Panic("Unsupported segment SPILL.");
		break;

	case BRIG_SEGMENT_ARG:
	{
		VariableScope *variable_scope;
		if (stack_top->getArgumentScope())
			variable_scope = stack_top->getArgumentScope();
		else
			throw misc::Panic("Error creating argument, not in a "
					"argument scope");
		variable_scope->DeclearVariable(name, dir->size/8, dir->type);
		break;
	}

	default:

		throw misc::Panic("Unsupported segment.");
		break;
	}

	Emu::isa_debug << misc::fmt("Create variable: %s %s(%d)\n",
			BrigEntry::type2str(dir->type).c_str(), name.c_str(),
			dir->size);
}


bool WorkItem::Execute()
{
	// Retrieve stack top
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Deal with empty function
	if (!stack_top->getFunction()->getLastInst())
	{
		ProcessRelatedDirectives();
		if (!MovePcForwardByOne())
		{
			return false;
		}
	}

	// Record frame status before the instruction is executed
	Emu::isa_debug << "Executing: ";
	Emu::isa_debug << inst;

	// Process directives in front of current instruction
	ProcessRelatedDirectives();

	// Get the function according to the opcode and perform the inst
	int opcode = inst.getOpcode();
	ExecuteInstFn fn = WorkItem::execute_inst_fn[opcode];
	try{
		(this->*fn)();
	}
	catch(misc::Panic &panic)
	{
		std::cerr << panic.getMessage();
		exit(1);
	}


	// Return false if execution finished
	if (stack.empty())
		return false;

	// Record frame status after the instruction is executed
	stack_top = stack.back().get();
	if (Emu::isa_debug)
		stack_top->Dump(Emu::isa_debug);
	Emu::isa_debug << "\n";

	// Return true, since the execution is not finished
	return true;
}


}  // namespace HSA

