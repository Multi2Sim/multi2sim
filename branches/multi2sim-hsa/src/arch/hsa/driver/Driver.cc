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

#include <cstring>

#include <arch/hsa/emu/Emu.h>
#include <arch/hsa/emu/StackFrame.h>
#include <arch/hsa/emu/WorkItem.h>
#include <arch/hsa/asm/AsmService.h>

#include "Driver.h"

namespace HSA
{

// Initialize table of ABI call names
const char *Driver::call_name[CallCodeCount] =
{
	"Invalid",  // For code 0
#define DEFCALL(name, code, func) #name,
#include "Driver.def"
#undef DEFCALL
};


// Initialize table of ABI call functions
const Driver::CallFn Driver::call_fn[CallCodeCount] =
{
	nullptr,  // For code 0
#define DEFCALL(name, code, func) &Driver::Call##name,
#include "Driver.def"
#undef DEFCALL
};


// Initialize the map from function name to driver call number
misc::StringMap Driver::function_name_to_call_map = 
{
#define STR(x) #x
#define DEFCALL(name, code, func) {STR(func), code},
#include "Driver.def"
#undef DEFCALL
#undef STR
};


// Debug file name, as set by user
std::string Driver::debug_file;

// Singleton instance
std::unique_ptr<Driver> Driver::instance;

// Debugger
misc::Debug Driver::debug;


void Driver::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("HSA");

	// Option '--hsa-debug-driver <file>'
	command_line->RegisterString("--hsa-debug-driver <file>", debug_file,
			"Dump debug information for the HSA driver, "
			"including all ABI calls coming from the runtime.");
}


void Driver::ProcessOptions()
{
	debug.setPath(debug_file);
	//debug.setPrefix("[HSA driver]");
}


Driver *Driver::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Driver());
	return instance.get();
}


int Driver::Call(int code, mem::Memory *memory, unsigned args_ptr)
{
	// Check valid call
	if (code < 0 || code >= CallCodeCount || !call_fn[code])
	{
		throw misc::Panic(misc::fmt("Invalid call code (%d)\n", code));
		return -1;
	}

	// Debug
	debug << misc::fmt("ABI call '%s'\n", call_name[code]);

	// Invoke call
	CallFn fn = call_fn[code];
	return (this->*fn)(memory, args_ptr);
}


bool Driver::Intercept(const std::string &function_name, 
		StackFrame *stack_top)
{
	// Translate from the function name to the ABI call number
	bool error;
	int call_number = 0;
	call_number = function_name_to_call_map.MapString(function_name, 
			error);

	// This function is not a runtime function, return to emulator
	if (error)
		return false;

	// Dump interception information
	Driver::debug << misc::fmt("Function %s intercepted, "
			"executing ABI call %d.\n", function_name.c_str(),
			call_number);

	// Serialize arguments
	unsigned arg_address = 
			PassArgumentsInByValue(function_name, stack_top);

	// Redirect the runtime function call to drivers ABI call
	// If ret == 1, the execution of the runtime function is to be 
	// continued
	int ret = Call(call_number, Emu::getInstance()->getMemory(), 
			arg_address);
	
	// Avoid passing value back and moving the pc forward if the runtime 
	// function is finished
	if (ret == 1)
	{
		debug << misc::fmt("Driver execution continued.\n");
		return true;
	}
	
	// Finish the execution of the intercepted function
	ExitInterceptedEnvironment(arg_address, stack_top);

	// Return true to tell the caller that this function is excepted
	return true;
}


void Driver::ExitInterceptedEnvironment(unsigned arg_address, 
		StackFrame *stack_top)
{
	// Dump information
	debug << misc::fmt("Driver execution finished.\n\n");

	// Pass the value back
	PassBackByValue(arg_address, stack_top);

	// Move the PC in the host by one
	stack_top->getWorkItem()->MovePcForwardByOne();
}


void Driver::SerializeArguments(char *arg_buffer, StackFrame *stack_top)
{
	// Get the function call instruction
	BrigCodeEntry *inst = stack_top->getPc();

	// Since we are writing each argument into the arg_buffer, we keep 
	// track of the offset the beginning place we are about to write to
	unsigned offset = 0;

	// Get input and output operands
	auto out_args = inst->getOperand(0);
	auto in_args = inst->getOperand(2);

	// Traverse output argument, do not copy value, but make space for 
	// them. 
	for (unsigned int i = 0; i < out_args->getElementCount(); i++)
	{
		// Get the argument definition
		auto symbol = out_args->getElement(i);

		// Retrieve argument size
		unsigned arg_size = AsmService::TypeToSize(symbol->getType());

		// Move the pointer forward
		offset += arg_size;
	}

	// Traverse input arguments, copy values
	for (unsigned int i = 0; i < in_args->getElementCount(); i++)
	{
		// Retrieve argument definition
		auto symbol = in_args->getElement(i);

		// Get argument name
		std::string arg_name = symbol->getName();

		// Get the variable buffer in caller's stack frame
		char *buf_in_caller = stack_top->getArgumentScope()
				->getBuffer(arg_name);

		// Get argument size
		unsigned arg_size = AsmService::TypeToSize(symbol->getType());

		// Copy value into callee's buffer
		memcpy(arg_buffer + offset, buf_in_caller, arg_size);

		// Move the pointer forward
		offset += arg_size;
	}

	return;
}


unsigned Driver::PassArgumentsInByValue(const std::string &function_name, 
		StackFrame *stack_top)
{
	// Get the function call instruction
	// BrigCodeEntry *inst = stack_top->getPc();

	// Get the function object
	Function *function = ProgramLoader::getInstance()
			->getFunction(function_name);

	// Allocate memory for arguments
	// Memory layout 
	//	-- Return argument(s)
	//	-- Input argument(s)
	//	-- 4 byte number ( 1 for call from HSAIL, 2 for call from X86)
	//	-- 8 byte number ( Workitem Address)
	mem::Manager *manager = Emu::getInstance()->getMemoryManager();
	mem::Memory *memory = Emu::getInstance()->getMemory();
	unsigned arg_address = manager->Allocate(
			function->getArgumentSize() + 12);
	char *arg_buffer = memory->getBuffer(arg_address, 
			function->getArgumentSize() + 12,
			mem::Memory::AccessWrite);

	// Serialize function arguments
	SerializeArguments(arg_buffer, stack_top);

	// Put the 4 byte number indicating if it is from HSAIL
	unsigned int *lang_buf = (unsigned int *)(arg_buffer + 
			function->getArgumentSize());
	*lang_buf = 1;
	
	// Put the workitem address into memory
	unsigned long long *buf = (unsigned long long *)(arg_buffer + 
			function->getArgumentSize() + 4);
	WorkItem *work_item = stack_top->getWorkItem();
	*buf = (unsigned long long)work_item;

	return arg_address;
}


void Driver::PassBackByValue(unsigned arg_address, StackFrame *stack_top)
{
	// Get the function instruction
	BrigCodeEntry *inst = stack_top->getPc();

	// Get output arguments
	auto out_args = inst->getOperand(0);

	// Offset value
	unsigned offset = 0;

	// Traverse output arguments
	for (unsigned int i = 0; i < out_args->getElementCount(); i++)
	{
		auto symbol= out_args->getElement(i);
		std::string arg_name = symbol->getName();
		char *buf_in_caller = stack_top->getArgumentScope()
					->getBuffer(arg_name);

		// Retrieve argument size
		unsigned arg_size = AsmService::TypeToSize(symbol->getType());

		// Get argument buffer
		char *buffer_in_callee = Emu::getInstance()->getMemory()->
				getBuffer(arg_address + offset,
						arg_size,
						mem::Memory::AccessWrite);

		// Copy argument calue
		memcpy(buf_in_caller, buffer_in_callee, arg_size);

		// Move the pointer forward
		offset += arg_size;
	}

	// At last free the memory space 
	mem::Manager *manager = Emu::getInstance()->getMemoryManager();
	manager->Free(arg_address);
	
	return;
}


Driver::~Driver()
{
}

} /* namespace HSA */
