/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#ifndef ARCH_HSA_DRIVER_SIGNALMANAGER_H
#define ARCH_HSA_DRIVER_SIGNALMANAGER_H

#include <memory>
#include <unordered_map>

#include "Signal.h"

namespace HSA
{

/// SignalManager is the container of signals. It is responsible for 
/// creating and destroying signals.
class SignalManager 
{
private:

	// A hash map that maps from the signal handler to the signals
	std::unordered_map<uint64_t, std::unique_ptr<Signal>> signals;

	// The handler to allocate next
	uint64_t handler_to_allocate = 0;

public:

	/// Constructor
	SignalManager();

	/// Destructor
	~SignalManager();

	/// Create a signal with an initial value. Return the handler of the 
	/// newly created signal. The signal manager keeps the ownership 
	/// of the signals.
	uint64_t CreateSignal(int64_t initial_value);

	/// Destory the signal with a particular handler
	void DestorySignal(uint64_t handler);

	/// Change the value of the signal
	void ChangeValue(uint64_t handler, int64_t value);

	/// Check if the signal handler is valid
	/// @param handler Signal handler
	/// @return if the signal handler is valid
	bool isValidSignalHandler(uint64_t handler);

	/// Get the value of a signal
	int64_t GetValue(uint64_t handler);
};

}

#endif  // ARCH_HSA_DRIVER_SIGNALMANAGER_H


