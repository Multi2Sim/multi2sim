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

#include "Core.h"
#include "Uop.h"
#include "Thread.h"


namespace x86
{

long long Uop::id_counter = 0;


Uop::Uop()
{
}


Uop::Uop(Thread *thread) : thread(thread)
{
	// Initialize
	core = thread->getCore();
	id = ++id_counter;
	id_in_core = core->getUopId();
}


void Uop::CountDependencies()
{
}


int Uop::Compare(Uop *uop)
{
	// If the time when is ready for these two Uops is different, then return the time difference
	// If the time when is ready for these two Uops is same, then return the ID difference
	// (Uop with smaller ID should be handled first)
	return ready_when != uop->ready_when ?
			ready_when - uop->ready_when :
			id - uop->id;
}

}
