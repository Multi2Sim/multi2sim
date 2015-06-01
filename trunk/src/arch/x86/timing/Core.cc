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

#include "CPU.h"
#include "Core.h"


namespace x86
{

Core::Core(const std::string &name, CPU *cpu, int id)
	:
	name(name), cpu(cpu), id(id)
{
	// The prefix for each core
	std::string prefix = name + "Thread";
	std::string thread_name;

	// Create threads
	for (int i = 0; i < CPU::getNumThreads(); i++)
	{
		thread_name = prefix + misc::fmt("%d", i);
		threads.emplace_back(new Thread(thread_name, this->cpu, this, i));
	}
}

}
