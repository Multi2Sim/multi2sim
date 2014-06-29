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

#include <lib/cpp/String.h>

#include "WorkItem.h"

namespace HSA
{

void WorkItem::LoadBinary()
{
	// Alternative stdin
	if (!loader->stdin_file_name.empty())
	{
		// Open new stdin
		int f = open(loader->stdin_file_name.c_str(), O_RDONLY);
		if (f < 0)
			misc::fatal("%s: cannot open stdin",
					loader->stdin_file_name.c_str());

		// Replace file descriptor 0
		file_table->freeFileDescriptor(0);
		file_table->newFileDescriptor(
				comm::FileDescriptor::TypeStandard,
				0,
				f,
				loader->stdin_file_name,
				O_RDONLY);
	}

	// Alternative stdout/stderr
	if (!loader->stdout_file_name.empty())
	{
		// Open new stdout
		int f = open(loader->stdout_file_name.c_str(),
				O_CREAT | O_APPEND | O_TRUNC | O_WRONLY,
				0660);
		if (f < 0)
			misc::fatal("%s: cannot open stdout",
					loader->stdout_file_name.c_str());

		// Replace file descriptors 1 and 2
		file_table->freeFileDescriptor(1);
		file_table->freeFileDescriptor(2);
		file_table->newFileDescriptor(
				comm::FileDescriptor::TypeStandard,
				1,
				f,
				loader->stdout_file_name,
				O_WRONLY);
		file_table->newFileDescriptor(
				comm::FileDescriptor::TypeStandard,
				2,
				f,
				loader->stdout_file_name,
				O_WRONLY);
	}

	// Reset program loader
	loader->binary.reset(new BrigFile(loader->exe));
	emu->loader_debug << misc::fmt("Program loaded\n");

	// Load function table
	int num_functions = loadFunctions();
	if (num_functions == 0)
	{
		throw Error("No function found in the Brig file provided");
	}

	// Set entry_pointer and program counter to the first inst
	loader->entry_point = findMainFunction();
	pc = loader->entry_point;

}


char* WorkItem::findMainFunction()
{
	// Traverse all the top level directives until the one with
	char *firstInst = loader->binary->findMainFunction();
	if(firstInst)
	{
		BrigInstEntry inst(firstInst, loader->binary.get());
		emu->loader_debug << "First instruction: " << inst;
	}
	else
	{
		emu->loader_debug << misc::fmt("Function (main) not found\n");
		throw Error("Function (main) not found\n");
	}

	return firstInst;
}


unsigned int WorkItem::loadFunctions()
{
	unsigned int num_functions = 0;

	// Get pointer to directive section
	BrigFile *file = loader->binary.get();
	BrigSection *dir_section = file->getBrigSection(BrigSectionDirective);
	const char *buffer = dir_section->getBuffer();
	char *buffer_ptr = (char *)buffer + 4;

	// Traverse top level directive
	while (buffer_ptr && buffer_ptr < buffer + dir_section->getSize())
	{
		BrigDirEntry dir(buffer_ptr, file);
		if (dir.getKind() == BRIG_DIRECTIVE_FUNCTION)
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


void WorkItem::parseFunction(BrigDirEntry *dir)
{
	struct BrigDirectiveFunction *dir_struct = 
			(struct BrigDirectiveFunction *)dir->getBuffer();

	// Get the name of the function
	std::string name = BrigStrEntry::GetStringByOffset(loader->binary.get(), 
			dir_struct->name);

	// Get the pointer to the first code
	char *entry_point = BrigInstEntry::GetInstByOffset(loader->binary.get(), 
			dir_struct->code);

	// Construct function object and insert into function_table
	// std::unique_ptr<Function> function(new Function(name, entry_point));
	loader->function_table.insert(
			std::make_pair(name, 
					std::unique_ptr<Function>(
						new Function(name, entry_point)
					)
			)
	);
	Function *function = loader->function_table[name].get();
	emu->loader_debug << misc::fmt("\nFunction %s loaded.\n", name.c_str());

	// Load Arguments
	unsigned short num_in_arg = dir_struct->inArgCount;
	unsigned short num_out_arg = dir_struct->outArgCount;
	char *next_dir = dir->next();
	next_dir = loadArguments(num_out_arg, next_dir, false, function);
	next_dir = loadArguments(num_in_arg, next_dir, true, function);

	emu->loader_debug << "\n";
}


char *WorkItem::loadArguments(unsigned short num_arg, char *next_dir,
		bool isInput, Function* function)
{
	// Load output arguments
	for (int i = 0; i < num_arg; i++)
	{
		// Retrieve argument pointer
		BrigDirEntry output_arg_entry(next_dir, loader->binary.get());
		struct BrigDirectiveSymbol *arg_struct =
				(struct BrigDirectiveSymbol *)next_dir;

		// Get argument information
		std::string arg_name = BrigStrEntry::GetStringByOffset(
				loader->binary.get(), arg_struct->name);
		unsigned short type = arg_struct->type;

		// Add this argument to the argument table
		function->addArgument(arg_name, type, isInput);

		// Put argument information into loader_debug log file
		emu->loader_debug << misc::fmt("\tArg %s %s loaded\n",
				BrigEntry::type2str(type).c_str(),
				arg_name.c_str());

		// Move pointer forward
		next_dir = output_arg_entry.next();
	}
	return next_dir;
}



}  // namespace HSA

