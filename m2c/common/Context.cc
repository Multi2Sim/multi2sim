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
	debug << misc::fmt("[%s] Source file added\n", path.c_str());
}
	

std::string Context::getSourceFile(int index, const std::string &extension)
{
	// Get source file
	assert(misc::inRange((unsigned) index, 0, source_files.size() - 1));
	std::string source_file = source_files[index];

	// Transform name if an extension is given
	if (!extension.empty())
	{
		source_file = misc::getBaseName(source_file);
		source_file += extension;
	}
	
	// Return name
	return source_file;
}


}  // namespace comm

