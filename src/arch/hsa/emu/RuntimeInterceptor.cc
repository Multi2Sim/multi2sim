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

#include <arch/hsa/driver/Driver.h>

#include "Emu.h"
#include "RuntimeInterceptor.h"


namespace HSA
{

// Singleton instance
std::unique_ptr<RuntimeInterceptor> RuntimeInterceptor::instance;


misc::StringMap RuntimeInterceptor::function_name_to_call_map =
{
#define STR(x) #x
#define DEFCALL(name, code, func) {STR(func), code},
#include <arch/hsa/driver/Driver.def>
#undef DEFCALL
#undef STR
};


RuntimeInterceptor::RuntimeInterceptor()
{

}


RuntimeInterceptor::~RuntimeInterceptor()
{
}


RuntimeInterceptor *RuntimeInterceptor::getInstance()
{
	if (instance.get())
		return instance.get();
	instance.reset(new RuntimeInterceptor());
	return instance.get();
}


unsigned RuntimeInterceptor::PassArgumentsInByValue(const std::string &function_name,
		StackFrame *stack_top)
{
	// Get the function instruction
	BrigFile *binary = ProgramLoader::getInstance()->getBinary();
	BrigInstEntry inst(stack_top->getPc(), binary);

	// Get function information for the argument size
	Function *function = ProgramLoader::getInstance()
			->getFunction(function_name);

	// Allocated memory for arguments
	mem::Manager *manager = Emu::getInstance()->getMemoryManager();
	unsigned arg_address = manager->Allocate(function->getArgumentSize());
	mem::Memory *memory = Emu::getInstance()->getMemory();
	char *arg_buffer = memory->getBuffer(arg_address,
			function->getArgumentSize(), mem::Memory::AccessWrite);

	// Get input and output operands
	struct BrigOperandArgumentList *out_args =
			(struct BrigOperandArgumentList *)inst.getOperand(0);
	struct BrigOperandArgumentList *in_args =
			(struct BrigOperandArgumentList *)inst.getOperand(2);

	// A pointer that keeps where the value should be copied to
	unsigned address_offset = 0;

	// Traverse output arguments
 	for (unsigned int i = 0; i < out_args->elementCount; i++)
	{
 		struct BrigDirectiveSymbol *symbol=
				(struct BrigDirectiveSymbol *)
				BrigDirEntry::GetDirByOffset(binary,
						out_args->elements[i]);

 		// Retrieve argument size
 		unsigned arg_size = BrigEntry::type2size(symbol->type);

 		// Move the pointer forward
 		 address_offset += arg_size;
	}

 	// Traverse input arguments
 	for (unsigned int i = 0; i < in_args->elementCount; i++)
 	{
 		// Retrieve argument information
 		struct BrigDirectiveSymbol *symbol=
 				(struct BrigDirectiveSymbol *)
 				BrigDirEntry::GetDirByOffset(binary,
 						in_args->elements[i]);
 		std::string arg_name =
 				BrigStrEntry::GetStringByOffset(binary,
 						symbol->name);
 		char *buf_in_caller = stack_top->getArgumentScope()
 					->getBuffer(arg_name);

 		// Retrieve argument size
 		unsigned arg_size = BrigEntry::type2size(symbol->type);

 		// Copy value into callee's buffer
 		memcpy(arg_buffer + address_offset, buf_in_caller, arg_size);

 		// Move the pointer forward
 		address_offset += arg_size;
 	}

 	// Return the argument memory space allocated
 	return arg_address;
}


void RuntimeInterceptor::PassBackByValue(unsigned arg_address,
			StackFrame *stack_top)
{
	// Get the function instruction
	BrigFile *binary = ProgramLoader::getInstance()->getBinary();
	BrigInstEntry inst(stack_top->getPc(), binary);

	// Get output arguments
	struct BrigOperandArgumentList *out_args =
			(struct BrigOperandArgumentList *)inst.getOperand(0);

	// Offset value
	unsigned address_offset = 0;

	// Traverse output arguments
	for (unsigned int i = 0; i < out_args->elementCount; i++)
	{
		struct BrigDirectiveSymbol *symbol=
				(struct BrigDirectiveSymbol *)
				BrigDirEntry::GetDirByOffset(binary,
						out_args->elements[i]);
		std::string arg_name =
				BrigStrEntry::GetStringByOffset(binary,
						symbol->name);
		char *buf_in_caller = stack_top->getArgumentScope()
					->getBuffer(arg_name);

		// Retrieve argument size
		unsigned arg_size = BrigEntry::type2size(symbol->type);

		// Get argument buffer
		char *buffer_in_callee = Emu::getInstance()->getMemory()->
				getBuffer(arg_address + address_offset,
						arg_size,
						mem::Memory::AccessWrite);

		// Copy argument calue
		memcpy(buf_in_caller, buffer_in_callee, arg_size);

		// Move the pointer forward
		 address_offset += arg_size;
	}
}


bool RuntimeInterceptor::Intercept(const std::string &function_name,
		StackFrame *stack_top)
{
	// Translate from the function name to the ABI call number
	bool error;
	int call_number = 0;
	call_number = function_name_to_call_map.MapString(function_name, error);
	if(error)
		return false;

	// Set current intercepted work item
	intercepted_work_item = stack_top->getWorkItem();

	// Copy arguments
	unsigned arg_address = PassArgumentsInByValue(function_name, stack_top);

	// Redirect the runtime function call to drivers ABI call
	Driver::getInstance()->Call(call_number,
			Emu::getInstance()->getMemory(),
			arg_address);

	// Pass return value back
	PassBackByValue(arg_address, stack_top);

	// Dump interception information into debug log
	Emu::isa_debug << misc::fmt("Runtime function %s intercepted, "
			"executing ABI call %d. \n",
			function_name.c_str(), call_number);

	return true;
}

} /* namespace HSA */
