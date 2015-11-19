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

#ifndef SRC_ARCH_HSA_DRIVER_SIGNALDESTROYHANDLER_H_
#define SRC_ARCH_HSA_DRIVER_SIGNALDESTROYHANDLER_H_

#include "DriverCallHandler.h"

namespace HSA
{
class SignalManager;

class SignalDestroyHandler: public DriverCallHandler
{
	struct Data
	{
		uint32_t status;
		uint64_t signal;
	};

	SignalManager *signal_manager;

public:
	SignalDestroyHandler(SignalManager *signal_manager);
	virtual ~SignalDestroyHandler();
	void Process(mem::Memory *memory, uint32_t args_ptr);
};

}  // namespace HSA

#endif /* SRC_ARCH_HSA_DRIVER_SIGNALDESTROYHANDLER_H_ */
