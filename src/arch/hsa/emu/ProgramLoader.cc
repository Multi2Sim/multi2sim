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

#include <arch/hsa/asm/BrigOperandEntry.h>

#include "Emu.h"
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
	BrigSection *dir_section = file->getBrigSection(BrigSectionDirective);
	const char *buffer = dir_section->getBuffer();
	char *buffer_ptr = (char *)buffer + 4;

	// Traverse top level directive
	while (buffer_ptr && buffer_ptr < buffer + dir_section->getSize())
	{
		BrigDirEntry dir(buffer_ptr, file);
		if (dir.getKind() == BRIG_DIRECTIVE_FUNCTION ||
				dir.getKind() == BRIG_DIRECTIVE_KERNEL)
		{
			// Parse and create the function, insert the function
			// in table
			parseFunction(&dir);
			num_functions++;
		}
		// move pointer to next top level directive
		buffer_ptr = dir.nextTop();
	}

	return num_functions;
}


void ProgramLoader::parseFunction(BrigDirEntry *dir)
{
	struct BrigDirectiveFunction *dir_struct =
			(struct BrigDirectiveFunction *)dir->getBuffer();

	// Get the name of the function
	std::string name = BrigStrEntry::GetStringByOffset(binary.get(),
			dir_struct->name);

	// Get the pointer to the first code
	char *entry_point = BrigInstEntry::GetInstByOffset(binary.get(),
			dir_struct->code);

	// Construct function object and insert into function_table
	// std::unique_ptr<Function> function(new Function(name, entry_point));
	Function *function = new Function(name, dir->getBuffer(), entry_point);
	function_table.insert(std::make_pair(name,
				std::unique_ptr<Function>(function)));

	// Load Arguments
	unsigned short num_in_arg = dir_struct->inArgCount;
	unsigned short num_out_arg = dir_struct->outArgCount;
	char *next_dir = dir->next();
	next_dir = loadArguments(num_out_arg, next_dir, false, function);
	next_dir = loadArguments(num_in_arg, next_dir, true, function);

	// Now next_dir should be an in-function directive
	function->setFirstInFunctionDirective(next_dir);

	// Allocate registers
	preprocessRegisters(entry_point, dir_struct->instCount, function);

	// Dump the function information into loader_debug file
	if (Emu::loader_debug)
		function->Dump(Emu::loader_debug);
}


char *ProgramLoader::loadArguments(unsigned short num_arg, char *next_dir,
		bool isInput, Function* function)
{
	// Load output arguments
	for (int i = 0; i < num_arg; i++)
	{
		// Retrieve argument pointer
		BrigDirEntry arg_entry(next_dir, binary.get());
		struct BrigDirectiveSymbol *arg_struct =
				(struct BrigDirectiveSymbol *)next_dir;

		// Get argument information
		std::string arg_name = BrigStrEntry::GetStringByOffset(
				binary.get(), arg_struct->name);
		unsigned short type = arg_struct->type;

		// Add this argument to the argument table
		Variable *argument = new Variable(arg_name, type,
				BrigEntry::type2size(type), 0);
		argument->setIndex(i);
		argument->setInput(isInput);
		function->addArgument(argument);

		// Move pointer forward
		next_dir = arg_entry.next();
	}
	return next_dir;
}


void ProgramLoader::preprocessRegisters(char *entry_point,
			unsigned int inst_count, Function* function)
{
	char *inst_ptr = entry_point;
	BrigFile *binary = ProgramLoader::getInstance()->getBinary();

	// Traverse all instructions
	for (unsigned int i = 0; i < inst_count; i++)
	{
		BrigInstEntry inst_entry(inst_ptr, binary);
		//std::cout << inst_entry << "\n";

		// Traverse each operands of an instruction
		for (int j = 0; j < 5; j++)
		{
			char *operand_ptr = inst_entry.getOperand(j);
			if (!operand_ptr) break;
			BrigOperandEntry operand_entry(operand_ptr, binary,
					&inst_entry, j);
			//std::cout << operand_entry << "\n";
			if (operand_entry.getKind() == BRIG_OPERAND_REG)
			{

				std::string reg_name =
						operand_entry.getRegisterName();
				//std::cout << "Add register: " << reg_name << "\n";
				function->addRegister(reg_name);
			}
		}

		// Set the last instruction
		if (i == inst_count-1)
			function->setLastInst(inst_ptr);

		// Move inst_ptr forward
		inst_ptr = inst_entry.next();
	}
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

