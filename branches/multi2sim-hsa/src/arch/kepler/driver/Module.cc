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

#include "Module.h"


namespace Kepler
{

Module::Module(int id, const std::string &cubin_path) : elf_file(cubin_path)
{
	this->id = id;

}

Function *Module::addFunction(Module *module, const std::string &name)
{
	// The function ID is its position in the list
	int id = functions.size();

	// Create function and add it to the list of functions
	functions.emplace_back(new Function(id, module, name));
	return functions.back().get();
}

} // namespace Kepler
