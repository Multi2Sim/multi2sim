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

#include "Uop.h"

namespace x86
{

Uop::Uop()
{

}


Uop::Uop(int id, int id_in_core, Thread *thread)
	:
	id(id),
	id_in_core(id_in_core),
	thread(thread)
{

}


int Uop::Compare(Uop *uop)
{
	// If the time when is ready for these two Uops is different, then return the time difference
	// If the time when is ready for these two Uops is same, then return the ID difference
	// (Uop with smaller ID should be handled first)
	return this->ready_when != uop->getReadyWhen() ? this->ready_when - uop->getReadyWhen()
			: this->id - uop->getID();
}

}
