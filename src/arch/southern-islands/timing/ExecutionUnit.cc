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

#include <arch/southern-islands/emulator/WorkGroup.h>
#include <arch/southern-islands/emulator/Wavefront.h>

#include "BranchUnit.h"
#include "Timing.h"

namespace SI
{

void ExecutionUnit::Issue(std::unique_ptr<Uop> uop)
{
	// Spend issue latency
	Timing *timing = Timing::getInstance();
	assert(uop->issue_ready == 0);
	uop->issue_ready = timing->getCycle() + ComputeUnit::issue_latency;	
	
	// Insert into issue buffer
	assert(canIssue());
	issue_buffer.push_back(std::move(uop));
}

}

