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

#include <cassert>

#include <lib/esim/Engine.h>

#include "Frame.h"
#include "System.h"


namespace mem
{

long long Frame::id_counter = 0;
	
	
Frame::Frame(long long id, Module *module, unsigned address) :
		id(id),
		module(module),
		address(address)
{
	// Initialize iterators to past-the-end iterators
	assert(module);
	accesses_iterator = module->getAccessListEnd();
	write_accesses_iterator = module->getWriteAccessListEnd();
}


}  // namespace mem

