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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef LIB_CPP_ESIM_H
#define LIB_CPP_ESIM_H

#include <string>


namespace esim
{

// Forward declaration
class Engine;


/// Class representing a frequency domain. The user should not instantiate this
/// class directly. Instead, frequency domains should be instantiated using
/// calls to Engine::RegisterFrequencyDomain().
class FrequencyDomain
{
	// Name of the domain
	std::string name;

	// Frequency in MHz
	int frequency;

	// Cycle time in picoseconds
	long long cycle_time;

	// Simulation engine, saved for efficiency
	Engine *engine;

public:

	/// Constructor
	FrequencyDomain(const std::string &name, int frequency);

	/// Return the frequency domain name
	const std::string &getName() const { return name; }

	/// Return the frequency in MHz of this domain
	int getFrequency() const { return frequency; }

	/// Update the frequency (and as a result, also the cycle time) of this
	/// domain. This function will cause the event-driven simulation engine
	/// to automatically update the fastest frequency domain information
	/// internally.
	void setFrequency(int frequency);

	/// Return the cycle time in picoseconds
	long long getCycleTime() const { return cycle_time; }

	/// Return the current cycle in this domain, calculated based on the
	/// current cycle in the event-driven simulation engine and the
	/// frequency in this domain.
	long long getCycle() const;
};


}  // namespace esim

#endif
