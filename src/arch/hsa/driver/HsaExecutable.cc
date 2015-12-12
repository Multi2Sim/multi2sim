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

#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>
#include <lib/cpp/Error.h>
#include <arch/hsa/disassembler/BrigFile.h>
#include <arch/hsa/disassembler/BrigCodeEntry.h>
#include <arch/hsa/disassembler/BrigOperandEntry.h>
#include <arch/hsa/emulator/Variable.h>
#include <arch/hsa/emulator/Function.h>
#include <arch/hsa/emulator/Emulator.h>

#include "HsaProgram.h"
#include "HsaExecutable.h"
#include "HsaExecutableSymbol.h"


namespace HSA
{

HsaExecutable::HsaExecutable():
	modules()
{
}


void HsaExecutable::LoadCodeObject(HsaCodeObject *code_object)
{
	const std::vector<std::unique_ptr<BrigFile>> *modules
		= code_object->getModules();
	for (auto it = modules->begin(); it != modules->end(); it++)
	{
		AddModule((*it)->getBuffer());
	}
}


void HsaExecutable::AddModule(const char *module)
{
	auto binary = misc::new_unique<BrigFile>();
	binary->LoadFileFromBuffer(module);
	loadFunctions(binary.get());
	modules.push_back(std::move(binary));
}


Function *HsaExecutable::getFunction(const std::string &name) const
{
	auto it = function_table.find(name);
	if (it == function_table.end())
	{
		throw Error(misc::fmt("Undefined reference to %s",
				name.c_str()));
	}
	return it->second.get();
}


void HsaExecutable::preprocessRegisters(
		BrigFile *binary,
		std::unique_ptr<BrigCodeEntry> first_entry,
		std::unique_ptr<BrigCodeEntry> next_module_entry,
		Function* function)
{
	auto entry = std::move(first_entry);

	// Record the maximum register used
	unsigned int max_reg[4] = {0, 0, 0, 0};

	// Set a last entry offset to keeps where the function ends
	unsigned int last_entry_offset = 0;

	// Traverse all instructions
	while(true)
	{
		// Terminate if end reached
		if (entry.get() == nullptr)
			break;
		else if (next_module_entry.get() &&
				next_module_entry->getOffset() ==
						entry->getOffset())
			break;

		// Set last entry offset
		last_entry_offset = entry->getOffset();

		// Skip directives
		if (!entry->isInstruction())
		{
			entry = entry->Next();
			continue;
		}

		// Traverse each operands of an instruction
		for (unsigned int j = 0; j < entry->getOperandCount(); j++)
		{
			auto operand = entry->getOperand(j);
			if (!operand.get()) break;

			// operand->Dump(entry->getOperandType(j), std::cout);
			if (operand->getKind() != BRIG_KIND_OPERAND_REGISTER)
				continue;

			BrigRegisterKind kind = operand->getRegKind();
			unsigned short number = operand->getRegNumber() + 1;
			if (number > max_reg[kind])
			{
				max_reg[kind] = number;

			}
		}

		// Move entry to next
		entry = entry->Next();
	}

	// Set last entry in the function
	function->setLastEntry(std::move(binary->getCodeEntryByOffset(
			last_entry_offset)));

	// Allocate registers
	function->AllocateRegister(max_reg);
}


std::unique_ptr<BrigCodeEntry> HsaExecutable::loadArguments(
		BrigFile *file,
		unsigned short num_arg,
		std::unique_ptr<BrigCodeEntry> entry,
		bool isInput,
		Function* function)
{
	// Load output arguments
	for (int i = 0; i < num_arg; i++)
	{
		// Get argument information
		std::string name = entry->getName();
		BrigType type = entry->getType();
		unsigned long long dim = entry->getDim();

		// Add this argument to the argument table
		auto argument = misc::new_unique<Variable>(name, type, 
				dim, 0, BRIG_SEGMENT_NONE, true);
		argument->setIndex(i);
		argument->setInput(isInput);
		function->addArgument(std::move(argument));

		// Move pointer forward
		entry = entry->Next();
	}
	return entry;
}


void HsaExecutable::parseFunction(BrigFile *file,
		std::unique_ptr<BrigCodeEntry> entry)
{
	// Get the name of the function
	std::string name = entry->getName();

	// Get the pointer to the first code
	auto first_entry = entry->getFirstCodeBlockEntry();
	auto next_module_entry = entry->getNextModuleEntry();

	// Construct function object and insert into function_table
	auto function = misc::new_unique<Function>(name);
	function->setModule(file);
	function->setExecutable(this);

	// Load Arguments
	unsigned short num_in_arg = entry->getInArgCount();
	unsigned short num_out_arg = entry->getOutArgCount();
	auto next_entry = entry->Next();
	next_entry = loadArguments(file, num_out_arg, std::move(next_entry),
			false, function.get());
	next_entry = loadArguments(file, num_in_arg, std::move(next_entry),
			true, function.get());

	// Allocate registers
	preprocessRegisters(file, std::move(first_entry),
			std::move(next_module_entry), function.get());

	// Set some information for the function
	first_entry = entry->getFirstCodeBlockEntry();
	function->setFirstEntry(std::move(first_entry));
	function->setFunctionDirective(std::move(entry));

	if (Emulator::loader_debug)
		function->Dump(Emulator::loader_debug);

	function_table.emplace(name, std::move(function));
}


unsigned int HsaExecutable::loadFunctions(BrigFile *file)
{
	Emulator::loader_debug << misc::fmt("Preprocessing brig file: %s\n",
			file->getPath().c_str());

	unsigned int num_functions = 0;

	// Get pointer to directive section
	BrigSection *section = file->getBrigSection(BRIG_SECTION_INDEX_CODE);
	auto entry = section->getFirstEntry<BrigCodeEntry>();
	auto next_entry = entry->NextTopLevelEntry();

	// Traverse top level directive
	while (entry.get())
	{
		unsigned int kind = entry->getKind();
		if ((kind == BRIG_KIND_DIRECTIVE_FUNCTION ||
				kind == BRIG_KIND_DIRECTIVE_KERNEL ||
				kind == BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION)
				&& entry->isDefinition())
		{
			// Parse and create the function, insert the function
			// in table
			parseFunction(file, std::move(entry));
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


HsaExecutableSymbol *HsaExecutable::getSymbol(const char *symbol_name)
{
	Function *function = getFunction(std::string(symbol_name));
	HsaExecutableSymbol *symbol = new HsaExecutableSymbol(
			this, function->getFunctionDirective());
	return symbol;
}

}  // namespace HSA
