/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#include "Emu.h"

#include <lib/cpp/String.h>


namespace comm
{

Emu::Emu(const std::string &name)
		:
		name(name),
		timer(name)
{
	// Obtain event-driven simulator engine
	esim = esim::Engine::getInstance();
}

	
void Emu::DumpSummary(std::ostream &os) const
{
	double time_in_sec = (double) timer.getValue() / 1.0e6;
	double inst_per_sec = time_in_sec > 0.0 ? (double) instructions
			/ time_in_sec : 0.0;

	os << misc::fmt("[ %s ]\n", name.c_str());
	os << misc::fmt("RealTime = %.2f [s]\n", time_in_sec);
	os << misc::fmt("Instructions = %lld\n", instructions);
	os << misc::fmt("InstructionsPerSecond = %.0f\n", inst_per_sec);
}


}  // namespace comm

