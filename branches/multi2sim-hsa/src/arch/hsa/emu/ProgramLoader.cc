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

#include <fcntl.h>
#include <unistd.h>

#include <arch/hsa/asm/BrigDef.h>
#include <arch/hsa/asm/AsmService.h>
#include <arch/hsa/asm/BrigOperandEntry.h>

#include "Emu.h"
#include "AQLQueue.h"
#include "ProgramLoader.h"

namespace HSA
{

// Singleton instance
std::unique_ptr<ProgramLoader> ProgramLoader::instance;

void ProgramLoader::LoadProgram(const std::vector<std::string> &args,
			const std::vector<std::string> &env = { },
			const std::string &cwd = "",
			const std::string &stdin_file_name = "",
			const std::string &stdout_file_name = "")
{
	instance.reset(new ProgramLoader());
	instance->exe = misc::getFullPath(args[0], cwd);
	instance->args = args;
	instance->cwd = cwd.empty() ? misc::getCwd() : cwd;
	instance->stdin_file_name = misc::getFullPath(stdin_file_name, cwd);
	instance->stdout_file_name = misc::getFullPath(stdout_file_name, cwd);

	instance->file_table.reset(new comm::FileTable());

	instance->LoadBinary();
}


ProgramLoader *ProgramLoader::getInstance()
{
	if (instance.get())
		return instance.get();
	throw misc::Panic("Program not loaded!");
}


void ProgramLoader::LoadBinary()
{
	// Alternative stdin
	if (!stdin_file_name.empty())
	{
		// Open new stdin
		int f = open(stdin_file_name.c_str(), O_RDONLY);
		if (f < 0)
			throw Error(stdin_file_name +
					": Cannot open standard input");

		// Replace file descriptor 0
		file_table->freeFileDescriptor(0);
		file_table->newFileDescriptor(
				comm::FileDescriptor::TypeStandard,
				0,
				f,
				stdin_file_name,
				O_RDONLY);
	}

	// Alternative stdout/stderr
	if (!stdout_file_name.empty())
	{
		// Open new stdout
		int f = open(stdout_file_name.c_str(),
				O_CREAT | O_APPEND | O_TRUNC | O_WRONLY,
				0660);
		if (f < 0)
			throw Error(stdout_file_name +
					": Cannot open standard output");

		// Replace file descriptors 1 and 2
		file_table->freeFileDescriptor(1);
		file_table->freeFileDescriptor(2);
		file_table->newFileDescriptor(
				comm::FileDescriptor::TypeStandard,
				1,
				f,
				stdout_file_name,
				O_WRONLY);
		file_table->newFileDescriptor(
				comm::FileDescriptor::TypeStandard,
				2,
				f,
				stdout_file_name,
				O_WRONLY);
	}

	// Reset program loader
	binary.reset(new BrigFile(exe));
	Emu::loader_debug << misc::fmt("Program %s loaded\n", exe.c_str());

	// Load function table
	int num_functions = loadFunctions();
	if (num_functions == 0)
	{
		throw Error("No function found in the Brig file provided");
	}

}

unsigned int ProgramLoader::loadFunctions()
{
	unsigned int num_functions = 0;

	// Get pointer to directive section
	BrigFile *file = binary.get();
	BrigSection *section = file->getBrigSection(BrigSectionHsaCode);
	auto entry = section->getFirstEntry<BrigCodeEntry>();
	auto next_entry = entry->NextTopLevelEntry();

	// Traverse top level directive
	while (entry.get())
	{
		unsigned int kind = entry->getKind();
		if (kind == BRIG_KIND_DIRECTIVE_FUNCTION ||
				kind == BRIG_KIND_DIRECTIVE_KERNEL ||
				kind == BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION)
		{
			// Parse and create the function, insert the function
			// in table
			parseFunction(std::move(entry));
			num_functions++;
		}
		entry = std::move(next_entry);
		if (entry.get())
			next_entry = entry->NextTopLevelEntry();
		else
			return num_functions;
	}

	return num_functions;
}


void ProgramLoader::parseFunction(std::unique_ptr<BrigCodeEntry> entry)
{
	// Get the name of the function
	std::string name = entry->getName();

	// Get the pointer to the first code
	auto first_entry = entry->getFirstCodeBlockEntry();

	// Construct function object and insert into function_table
	// std::unique_ptr<Function> function(new Function(name, entry_point));
	auto function = misc::new_unique<Function>(name);

	// Load Arguments
	unsigned short num_in_arg = entry->getInArgCount();
	unsigned short num_out_arg = entry->getOutArgCount();
	auto next_entry = entry->Next();
	next_entry = loadArguments(num_out_arg, std::move(next_entry), false,
			function.get());
	next_entry = loadArguments(num_in_arg, std::move(next_entry), true,
			function.get());

	// Allocate registers
	preprocessRegisters(std::move(first_entry),
			entry->getCodeBlockEntryCount(), function.get());

	// Set some information for the function
	first_entry = entry->getFirstCodeBlockEntry();
	function->setFirstEntry(std::move(first_entry));
	function->setFunctionDirective(std::move(entry));

	// Dump the function information into loader_debug file
	if (Emu::loader_debug)
		function->Dump(Emu::loader_debug);

	function_table.insert(std::make_pair(name, std::move(function)));
}


std::unique_ptr<BrigCodeEntry> ProgramLoader::loadArguments(
		unsigned short num_arg, std::unique_ptr<BrigCodeEntry> entry,
		bool isInput, Function* function)
{
	// Load output arguments
	for (int i = 0; i < num_arg; i++)
	{
		// Retrieve argument pointer
		// BrigDirEntry arg_entry(next_dir, binary.get());
		// struct BrigDirectiveSymbol *arg_struct =
		// 		(struct BrigDirectiveSymbol *)next_dir;

		// Get argument information
		std::string arg_name = entry->getName();
		BrigTypeX type = entry->getType();
		unsigned long long dim = entry->getDim();

		// Add this argument to the argument table
		Variable *argument = new Variable(arg_name, type,
				dim, 0, nullptr,
				true);
		argument->setIndex(i);
		argument->setInput(isInput);
		function->addArgument(argument);

		// Move pointer forward
		entry = entry->Next();
	}
	return entry;
}


void ProgramLoader::preprocessRegisters(
		std::unique_ptr<BrigCodeEntry> first_entry,
		unsigned int inst_count, Function* function)
{
	auto entry = std::move(first_entry);

	// Record the maximum register used
	unsigned int max_reg[4] = {0, 0, 0, 0};

	// Traverse all instructions
	for (unsigned int i = 0; i < inst_count; i++)
	{
		// Skip directives
		if (entry->isInstruction())
		{
			// Traverse each operands of an instruction
			for (unsigned int j = 0; j < entry->getOperandCount(); j++)
			{
				auto operand = entry->getOperand(j);
				if (!operand.get()) break;

				//operand->Dump(entry->getOperandType(j), std::cout);
				if (operand->getKind() != BRIG_KIND_OPERAND_REG)
					continue;

				BrigRegisterKind kind = operand->getRegKind();
				unsigned short number = operand->getRegNumber() + 1;
				if (number> max_reg[kind])
				{
					max_reg[kind] = number;

				}
			}
		}

		// Move inst_ptr forward
		if (i < inst_count - 1)
		{
			entry = entry->Next();
		}
	}

	// Set last entry in the function
	unsigned int offset = entry->getOffset();
	function->setLastEntry(std::move(binary->getCodeEntryByOffset(offset)));

	// Allocate registers
	function->AllocateRegister(max_reg);
}


Function *ProgramLoader::getMainFunction() const
{
	auto it = function_table.find("&main");
	if (it == function_table.end())
	{
		throw Error("Undefined reference to &main()");
	}
	return it->second.get();
}

Function *ProgramLoader::getFunction(const std::string &name) const
{
	auto it = function_table.find(name);
	if (it == function_table.end())
	{
		throw Error(misc::fmt("Undefined reference to %s",
				name.c_str()));
	}
	return it->second.get();
}


}  // namespace HSA

