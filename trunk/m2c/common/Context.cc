/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <memory>

#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Misc.h>

#include "Context.h"


namespace comm
{


std::unique_ptr<Context> Context::instance;

misc::Debug Context::debug;

std::string Context::debug_file;


Context *Context::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create Instance
	instance.reset(new Context());
	return instance.get();
}


void Context::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("General");

	// Option --debug <file>
	command_line->RegisterString("--debug", debug_file,
			"Dump generic debug information for Multi2C.");
}


void Context::ProcessOptions()
{
	// Debug file
	if (!debug_file.empty())
		debug.setPath(debug_file);
}


void Context::AddSourceFile(const std::string &path)
{
	// Add it to the source file list
	source_files.emplace_back(path);

	// Derived names
	std::string base_name = misc::getBaseName(path);
	std::string pre_file = base_name + ".clp";
	std::string llvm_file = base_name + ".llvm";
	std::string asm_file = base_name + ".s";
	std::string bin_file = base_name + ".bin";

	// Add it to the rest of the lists
	pre_files.emplace_back(pre_file);
	llvm_files.emplace_back(llvm_file);
	asm_files.emplace_back(asm_file);
	bin_files.emplace_back(bin_file);

	// Debug
	debug << misc::fmt("[%s] Source file added\n", path.c_str());
	debug << misc::fmt("\t[%s] Pre-processed file\n", pre_file.c_str());
	debug << misc::fmt("\t[%s] LLVM file\n", llvm_file.c_str());
	debug << misc::fmt("\t[%s] Assembly file\n", asm_file.c_str());
	debug << misc::fmt("\t[%s] Binary file\n", bin_file.c_str());
}


}  // namespace comm

