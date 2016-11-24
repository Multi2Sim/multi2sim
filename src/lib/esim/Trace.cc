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

#include <iostream>

#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "Engine.h"
#include "Trace.h"


namespace esim
{

std::unique_ptr<TraceSystem> TraceSystem::instance;


TraceSystem::~TraceSystem()
{
	// Ignore if trace is not active
	if (!active)
		return;
	
	// Close ZIP file
	gzclose(gz_file);
}


TraceSystem *TraceSystem::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();
	
	// Create instance
	instance.reset(new TraceSystem());
	return instance.get();
}

	
void TraceSystem::setPath(const std::string &path)
{
	// Trace must not have been activated yet
	if (active)
		throw misc::Panic("Trace already active");

	// Save path
	this->path = path;
	active = true;
	
	// Open ZIP file
	gz_file = gzopen(path.c_str(), "wt");
	if (!gz_file)
		throw misc::Error(misc::fmt("%s: cannot open trace file",
				path.c_str()));
}


void TraceSystem::Write(const std::string &s, bool print_cycle)
{
	// Trace system must be active
	assert(active);
	
	// Print cycle
	if (print_cycle)
	{
		esim::Engine *engine = esim::Engine::getInstance();
		long long cycle = engine->getCycle();
		if (cycle > last_cycle)
		{
			gzprintf(gz_file, "c clk=%lld\n", cycle);
			last_cycle = cycle;
		}
	}

	// Dump string
	gzwrite(gz_file, s.c_str(), s.length());
}


void TraceSystem::Header(const std::string &s)
{
	// Check that no cycle-by-cycle info has been dumped yet
	if (last_cycle >= 0)
		throw misc::Panic("Trace header written after "
				"simulation started");

	// Write it
	Write(s, false);
}




Trace::Trace()
{
	// Keep reference to trace system
	trace_system = TraceSystem::getInstance();
}


}  // namespace esim

