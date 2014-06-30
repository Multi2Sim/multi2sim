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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_HSA_DRIVER_KERNEL_H
#define ARCH_HSA_DRIVER_KERNEL_H

#include <string>

namespace HSA
{

class Kernel{
	// unique kernel ID
	unsigned id;
	// name of the kernel
	std::string *name;
	// the program this kernel is created for
	unsigned program_id;

public:
	Kernel(unsigned id, std::string *name, unsigned program_id)
	{
		this->id = id;
		this->name = name;
		this->program_id = program_id;
	}
};

} // namespace HSA



#endif /* ARCH_HSA_DRIVER_KERNEL_H */
