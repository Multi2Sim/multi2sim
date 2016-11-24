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

#include "Cpu.h"
#include "Thread.h"
#include "Timing.h"


namespace x86
{

void Thread::Decode()
{
	for (int i = 0; i < Cpu::getDecodeWidth(); i++)
	{
		// Empty fetch queue
		if (fetch_queue.size() == 0)
			break;

		// Full uop queue
		if ((int) uop_queue.size() >= Cpu::getUopQueueSize())
			break;

		// Get uop at the head of the fetch queue
		assert(!fetch_queue.empty());
		std::shared_ptr<Uop> uop = fetch_queue.front();

		// If instructions come from the trace cache, i.e., are located
		// in the trace cache queue, copy all of them into the uop queue
		// in one single decode slot.
		if (uop->from_trace_cache)
		{
			do
			{
				// Extract from fetch queue
				ExtractFromFetchQueue(uop.get());

				// Add to uop queue
				InsertInUopQueue(uop);

				// Done if fetch queue empty
				if (fetch_queue.empty())
					break;

				// Next instruction from fetch queue
				assert(fetch_queue.size());
				uop = fetch_queue.front();

			} while (uop->from_trace_cache);

			// Consume entire decode width
			break;
		}

		// Decode one macro-instruction coming from a block in the
		// instruction cache. If the cache access finished, extract it
		// from the fetch queue.
		assert(!uop->mop_index);
		if (!instruction_module->isInFlightAccess(uop->fetch_access))
		{
			do
			{
				// Extract from fetch queue
				ExtractFromFetchQueue(uop.get());

				// Add to uop queue
				InsertInUopQueue(uop);

				// Trace
				Timing::trace << misc::fmt("x86.inst "
						"id=%lld "
						"core=%d "
						"stg=\"dec\"\n",
						uop->getIdInCore(),
						core->getId());

				// Done if no more instructions in fetch queue
				if (fetch_queue.empty())
					break;

				// Next instruction in fetch queue
				assert(fetch_queue.size());
				uop = fetch_queue.front();

			} while (uop->mop_index);
		}
	}
}

}

