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

#include <lib/cpp/String.h>
#include <lib/cpp/Error.h>

#include "SignalManager.h"

namespace HSA
{

SignalManager::SignalManager()
{
}


SignalManager::~SignalManager()
{
}


uint64_t SignalManager::CreateSignal(int64_t initial_value)
{
	// Create signal with initial value
	auto signal = misc::new_unique<Signal>();
	signal->setValue(initial_value);

	// Put the signal in the list
	signals.emplace(handler_to_allocate, std::move(signal));
	handler_to_allocate++;

	// Return the handler
	return handler_to_allocate - 1;
}


void SignalManager::DestorySignal(uint64_t handler)
{
	signals.erase(handler);
}


void SignalManager::ChangeValue(uint64_t handler, int64_t value)
{
	signals[handler]->setValue(value);
}


bool SignalManager::isValidSignalHandler(uint64_t handler)
{
	if (signals.find(handler) != signals.end())
	{
		return true;
	}
	return false;
}


int64_t SignalManager::GetValue(uint64_t handler)
{
	return signals[handler]->getValue();
}

}
