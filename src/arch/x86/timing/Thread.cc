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

#include "Cpu.h"
#include "Timing.h"
#include "Thread.h"
#include "TraceCache.h"


namespace x86
{

Thread::Thread(Core *core,
		int id_in_core) :
		core(core),
		id_in_core(id_in_core)
{
	// Assign name
	name = misc::fmt("Core %d Thread %d", core->getId(), id_in_core);

	// Global index in CPU
	id_in_cpu = core->getId() * Cpu::getNumThreads() + id_in_core;

	// Initialize branch predictor
	branch_predictor = misc::new_unique<BranchPredictor>(name +
			".BranchPredictor");

	// Initialize trace cache
	if (TraceCache::isPresent())
		trace_cache = misc::new_unique<TraceCache>(name +
				".TraceCache");

	// Initialize register file
	reg_file = misc::new_unique<RegisterFile>(core, this);
	reg_file->InitRegisterFile();
}


void Thread::InsertInFetchQueue(std::shared_ptr<Uop> uop)
{
	assert(!uop->in_fetch_queue);
	uop->in_fetch_queue = true;
	fetch_queue.push_back(uop);
}


std::shared_ptr<Uop> Thread::ExtractFromFetchQueue()
{
	std::shared_ptr<Uop> uop = fetch_queue.front();
	assert(uop->in_fetch_queue);
	assert(uop->fetch_queue_iterator == fetch_queue.begin());
	fetch_queue.pop_front();
	uop->in_fetch_queue = false;
	uop->fetch_queue_iterator = fetch_queue.end();
	return uop;
}


void Thread::InsertInUopQueue(std::shared_ptr<Uop> uop)
{
	assert(!uop->in_uop_queue);
	uop->in_uop_queue = true;
	uop_queue.push_back(uop);
}


std::shared_ptr<Uop> Thread::ExtractFromUopQueue()
{
	std::shared_ptr<Uop> uop = uop_queue.front();
	assert(uop->in_uop_queue);
	assert(uop->uop_queue_iterator == uop_queue.begin());
	uop_queue.pop_front();
	uop->in_uop_queue = false;
	uop->uop_queue_iterator = uop_queue.end();
	return uop;
}

}

