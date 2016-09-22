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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Timing.h"


namespace Kepler
{

void ExecutionUnit::Dispatch(std::unique_ptr<Uop> uop)
{
	// Dispatch latency
	Timing *timing = Timing::getInstance();
	assert(uop->dispatch_ready == 0);
	uop->dispatch_ready = timing->getCycle() + SM::dispatch_latency;

	// For debug mode
	/*if (dispatch_buffer.size() != 0)
	{
		std::cout<< "Function ExecutionUnit Dispatch" <<std::endl;
		std::cout<< "PC" << dispatch_buffer.begin()->get()->getInstructionPC() <<std::endl;
		std::cout<<"Warp id" << dispatch_buffer.begin()->get()->getWarp()->getId() <<std::endl;
		std::cout << "cycle" <<timing->getCycle()<<std::endl;

		std::cout<<"Current Uop instruction PC" <<uop->getInstructionPC()<<std::endl;
		std::cout<<"Warp id" << uop->getWarp()->getId() <<std::endl;
	}*/
	// Insert into dispatch buffer
	dispatch_buffer.push_back(std::move(uop));
}

} // namespace Kepler
