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

#include "Emulator.h"
#include "SegmentManager.h"
#include "StackFrame.h"
#include "WorkItem.h"


namespace HSA
{

StackFrame::StackFrame(Function *function, WorkItem *work_item,
		SegmentManager *function_argument_segment)
{
	// Set function that this frame is working on
	this->function = function;

	// Set work item
	this->work_item = work_item;

	// Set the program counter to be pointing to the first entry of the
	// function
	pc = function->getFirstEntry();

	// Allocate register space
	register_storage = misc::new_unique_array<char>(
			function->getRegisterSize());

	// Set the function argument segment
	this->function_argument_segment = function_argument_segment;
}


StackFrame::~StackFrame()
{
	// Traverse all variable, and release those variables that is 
	// declared in the global segment
	for (auto it = variables.begin(); it != variables.end(); it++)
	{
		BrigSegment segment = it->second->getSegment();
		
		// Only free memory allocated in global segment. 
		// Since other segment managers allocate memory 
		// from the global memory space, when their scope ends, freeing 
		// the segment manager would free their memory.
		if (segment == BRIG_SEGMENT_GLOBAL) 
		{
			unsigned address = it->second->getAddress();
			mem::Manager *manager = Emulator::getInstance()
					->getMemoryManager();
			manager->Free(address);
		}
	}
}


void StackFrame::setPc(std::unique_ptr<BrigCodeEntry> pc)
{
	this->pc = std::move(pc);
}


void StackFrame::StartArgumentScope(unsigned size)
{
	// Check if the previous argument scope has not been closed
	if (!argument_scope.empty())
		throw misc::Error("Cannot nest argument scopes.");

	// Creates a new argument segment from memory
	mem::Memory *memory = Emulator::getInstance()->getMemory();
	argument_segment = misc::new_unique<SegmentManager>(
			memory, size);
};


void  StackFrame::CloseArgumentScope()
{
	argument_scope.clear();
	argument_segment.reset(nullptr);
};


void StackFrame::Dump(std::ostream &os = std::cout) const
{
	os << "***** Stack frame *****\n";

	// Dump function name
	os << misc::fmt("  Function: %s,\n", function->getName().c_str());

	// Dump program counter and current instruction
	os << misc::fmt("  Program counter (offset in code section): 0x%x, ",
			pc->getOffset());
	pc->Dump(os);
	os << "\n";

	// Dump Register status
	os << "  ***** Registers *****\n";
	for (auto it = function->getRegisterBegin(); 
			it != function->getRegisterEnd(); it++)
	{
		os << "    ";
		DumpRegister(it->first, os);
	}
	os << "  ***** ********* *****\n\n";

	// Dump function arguments
	os << "  ***** Function arguments *****\n";
	os << "To be supported";
	os << "  ***** ******** ********* *****\n\n";

	// If in argument scope, dump argument scope
	os << "  ***** Argument scope *****\n";
	os << "To be supported";
	os << "  ***** ******** ***** *****\n\n";

	// Kernal argument
	os << "  ***** Kernel Argument *****\n";
	os << "To be supported";
	os << "  ***** ****** ******** ****\n\n";

	// Variable scope, 4
	os << "  ***** Variables *****\n";
	os << "To be supported";
	os << "  ***** ********* *****\n\n";

	// Dump back trace information
	work_item->Backtrace(os);

	// Close
	os << "***** ***** ***** *****\n";
}


void StackFrame::DumpRegister(const std::string &name,
		std::ostream &os = std::cout) const
{
	os << misc::fmt("%s: ", name.c_str());
	switch (name[1])
	{
	case 'c':

	{
		unsigned char buf = 0;
		getRegisterValue(name, &buf);
		if (buf)
			os << "true";
		else
			os << "false";
		break;
	}

	case 's':

	{
		unsigned int buf = 0;
		getRegisterValue(name, &buf);
		os << misc::fmt("%u, %d, %f, 0x%08x",
				*(unsigned int *)&buf,
				*(int *)&buf,
				*(float *)&buf,
				*(unsigned int *)&buf);

		break;
	}

	case 'd':

	{
		unsigned long long buf = 0;
		getRegisterValue(name, &buf);
		os << misc::fmt("%llu, %lld, %f, 0x%016llx",
				*(unsigned long long *)&buf,
				*(long long *)&buf,
				*(double *)&buf,
				*(unsigned long long *)&buf);
		break;
	}

	case 'q':

	{
		unsigned int offset =
				function->getRegisterOffset(name);
		for (int i=0; i<16; i++)
		{
			if (i > 0)
			{
				os << ", ";
			}
			os << misc::fmt("0x%x",
					*(this->register_storage.get()
							+ offset + i));
		}
		break;
	}

	default:

		throw misc::Panic(misc::fmt(
				"Unknown register name %s.",
				name.c_str()));
		break;
	}
	os << "\n";
}


Variable *StackFrame::getSymbol(const std::string &name)
{
	// Declare the symbol
	Variable *symbol = nullptr;

	// Try argument scope
	symbol = getArgument(name);
	if (symbol) 
		return symbol;

	// Try variables
	symbol = getVariable(name);
	if (symbol)
		return symbol;

	// Try function arguments
	symbol = getFunctionArgument(name);
	if (symbol)
		return symbol;

	// If nothing is found, return nullptr
	return nullptr;
}

}  // namespace HSA

