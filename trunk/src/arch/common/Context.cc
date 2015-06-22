/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#include <lib/cpp/String.h>

#include "Context.h"
#include "Emulator.h"


namespace comm
{

int Context::id_counter = 1000;


Context::Context(Emulator *emulator) :
		emulator(emulator)
{
	// Assign ID
	id = id_counter++;

	// Compute name
	name = misc::fmt("%s context %d",
			emulator->getName().c_str(),
			id);
}


void Context::Suspend()
{
	throw misc::Panic("Not implemented");
}


void Context::Wakeup()
{
	throw misc::Panic("Not implemented");
}


bool Context::isSuspended()
{
	throw misc::Panic("Not implemented");
}

}  // namespace comm

