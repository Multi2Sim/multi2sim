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

#ifndef LIB_CPP_TIMER_H
#define LIB_CPP_TIMER_H

#include <iostream>
#include <memory>


namespace misc
{


class Timer
{
public:

	/// Timer state
	enum State
	{
		StateInvalid = 0,
		StateStopped,
		StateRunning
	};

private:

	// Name
	std::string name;

	// Current timer state
	State state = StateStopped;
	
	// Time of last instant when timer started/resumed
	long long start_time = 0;

	// Accumulated value until last time the timer was stopped
	long long total_time = 0;

	// Get the current time in microseconds
	static long long getTime();

public:

	/// Create a timer with a name. The timer is initially stopped.
	Timer(const std::string &name) : name(name)
	{
	}

	/// Return the name of the timer
	const std::string &getName() const { return name; }

	/// Return the current state of the timer
	State getState() const { return state; }

	/// Return the number of microseconds ellapsed considering all periods of
	/// time when the timer has been in state \c Running.
	long long getValue() const;

	/// Start or resume the timer. Regardless of its previous state, the
	/// timer will transition to state \c Running.
	void Start();

	/// Pause the timer. Regardless of its previous state, the timer will
	/// transition to state \c Stopped after this call.
	void Stop();

	/// Reset the timer. This call can be made while the timer is stopped
	/// or when it is running.
	void Reset();
};


}  // namespace misc

#endif

