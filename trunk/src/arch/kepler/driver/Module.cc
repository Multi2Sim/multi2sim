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

extern "C"
{
#include <lib/mhandle/mhandle.h>
#include <lib/util/elf-format.h>
}

#include "Module.h"


namespace Kepler
{

// Module List Definition
std::vector<CUmodule*> CUmodule:: module_list;

CUmodule::CUmodule(char *cubin_path)
{
	// Initialization
	id = module_list.size();
	elf_file = elf_file_create_from_path(cubin_path);

	// Add module to list
	module_list.push_back(this);
}

CUmodule::~CUmodule()
{
	module_list.pop_back();
	elf_file_free(elf_file);
}

} // namespace Kepler
