/*  Multi2Sim
 *  Copyright (C) 2014  Xun Gong (gong.xun@husky.neu.edu)
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

#ifndef ARCH_KEPLER_MODULE_H
#define ARCH_KEPLER_MODULE_H

#include<lib/cpp/ELFReader.h>

#include <vector>


namespace Kepler
{

class Module
{
	// ID
	unsigned id;

	// Kernel binary
	ELFReader::File *elf_file;


public:
	/// Static Module List
	static std::vector<Module*> module_list;

	/// Constructor
	Module(const std::string cubin_path);

	/// Destructor
	~Module();

	/// Get ID
	unsigned getID() const { return id; }

	/// Get Kernel Binary
	ELFReader::File* getELF() const { return elf_file; }
};

} // namespace Kepler

#endif
