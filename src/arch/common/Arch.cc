/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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
 *  You should have received as copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Arch.h"


namespace comm
{

misc::StringMap arch_sim_kind_map =
{
	{ "functional", ArchSimFunctional },
	{ "detailed", ArchSimDetailed }
};


//
// Class Arch
//

Arch::Arch(const std::string &name, const std::string &prefix)
{
	// Initialize
	this->name = name;
	this->prefix = prefix;
}
	


//
// Class ArchPool
//

std::unique_ptr<ArchPool> ArchPool::instance;

ArchPool *ArchPool::getInstance()
{
	// Return existing instance
	if (instance.get())
		return instance.get();
	
	// Create new architecture pool
	instance.reset(new ArchPool());
	return instance.get();
}
	
void ArchPool::Register(const std::string &name, const std::string &prefix)
{
	// Create new architecture in place
	arch_list.emplace_back(new Arch(name, prefix));
}


}  // namespace comm

