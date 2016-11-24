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

#include "Timer.h"

#include <sys/time.h>


namespace misc
{


long long Timer::getTime()
{
	struct timeval tv;
	long long value;

	gettimeofday(&tv, NULL);
	value = (long long) tv.tv_sec * 1000000 + tv.tv_usec;
	return value;
}


long long Timer::getValue() const
{
	// Timer is stopped
	if (state == StateStopped)
		return total_time;

	// Timer is running
	long long ellapsed = getTime() - start_time;
	return total_time + ellapsed;
}


void Timer::Start()
{
	// Timer already running
	if (state == StateRunning)
		return;
	
	// Start timer
	state = StateRunning;
	start_time = getTime();
}


void Timer::Stop()
{
	// Timer already stopped
	if (state == StateStopped)
		return;
	
	// Stop timer
	long long ellapsed = getTime() - start_time;
	state = StateStopped;
	total_time += ellapsed;
}


void Timer::Reset()
{
	total_time = 0;
	start_time = getTime();
}


}  // namespace misc

