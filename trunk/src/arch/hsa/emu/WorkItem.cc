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

#include "WorkItem.h"

namespace HSA
{

WorkItem::WorkItem(WorkGroup *work_group,
			unsigned int abs_id_x,
			unsigned int abs_id_y,
			unsigned int abs_id_z,
			Function *root_function,
			unsigned long long kernel_args)
{
	// Set global emulator object
	emu = Emu::getInstance();
	loader = ProgramLoader::getInstance();

	// Set work group
	this->work_group = work_group;

	// Set absolute ids
	this->abs_id_x = abs_id_x;
	this->abs_id_y = abs_id_y;
	this->abs_id_z = abs_id_z;

	// Set kernel args
	this->kernel_args = kernel_args;

	// Set the first stack frame
	StackFrame *frame = new StackFrame(root_function);
	stack.push_back(std::unique_ptr<StackFrame>(frame));
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
	int frame_count = 1;
	for (auto it = stack.rbegin(); it != stack.rend(); it++)
	{
		os << misc::fmt("#%d ", frame_count++);
		os << misc::fmt("%s ", (*it)->getFunction()->getName().c_str());
		// TODO: dump argument lists with argument values here
		os << "\n";
	}

}


bool WorkItem::ReturnFunction()
{
	// Pop frame at stack top
	stack.pop_back();

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

		// move next directive pointer forwards
		dir = (struct BrigDirectiveBase *)dir_entry.nextTop();
		stack_top->setNextDirective((char *)dir);
	}


}


void WorkItem::DeclearVariable()
{
	StackFrame *stack_top = stack.back().get();
	BrigDirectiveVariable *dir =
			(BrigDirectiveVariable *)stack_top->getNextDirective();

	// Allocate memory in different segment
	switch (dir->segment)
	{
	case BRIG_SEGMENT_NONE:
		break;
	case BRIG_SEGMENT_FLAT:
		break;
	case BRIG_SEGMENT_GLOBAL:
		break;
	case BRIG_SEGMENT_GROUP:
		break;
	case BRIG_SEGMENT_PRIVATE:
		break;
	case BRIG_SEGMENT_KERNARG:
		break;
	case BRIG_SEGMENT_READONLY:
		break;
	case BRIG_SEGMENT_SPILL:
		break;
	case BRIG_SEGMENT_ARG:
		CreateArgument();
		break;
	}
}


void WorkItem::CreateArgument()
{
	// Retrieve directive
	StackFrame *stack_top = stack.back().get();
	BrigDirectiveVariable *dir =
			(BrigDirectiveVariable *)stack_top->getNextDirective();

	// Get argument name
	std::string name = BrigStrEntry::GetStringByOffset(
			ProgramLoader::getInstance()->getBinary(),
			dir->name);

	// Create argument
	stack_top->CreateArgument(name, dir->size, dir->type);

	// Put information in isa_debug
	BrigDirEntry dir_entry((char *)dir,
			ProgramLoader::getInstance()->getBinary());
	Emu::isa_debug << "Create argument: " << dir_entry;
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
	Emu::isa_debug << "Stack frame before executing: ";
	Emu::isa_debug << inst;
	Emu::isa_debug << "\n";
	if (Emu::isa_debug)
		stack_top->Dump(Emu::isa_debug);
	Emu::isa_debug << "\n";

	// Process directives in front of current instruction
	ProcessRelatedDirectives();

	// Get the function according to the opcode and perform the inst
	int opcode = inst.getOpcode();
	ExecuteInstFn fn = WorkItem::execute_inst_fn[opcode];
	(this->*fn)();

	// Return false if execution finished
	if (stack.empty())
		return false;

	// Record frame status after the instruction is executed
	Emu::isa_debug << "Stack from after executing, \n\n";
	if (Emu::isa_debug)
		stack_top->Dump(Emu::isa_debug);
	Emu::isa_debug << "\n";

	// Return true, since the execution is not finished
	return true;
}


}  // namespace HSA

