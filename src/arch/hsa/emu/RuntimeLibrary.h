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

#ifndef ARCH_HSA_EMU_RUNTIMELIBRARY_H
#define ARCH_HSA_EMU_RUNTIMELIBRARY_H

#include "AQLQueue.h"
#include "Component.h"


namespace HSA
{

/// Encapsulates a bunch of runtime functions and definitions
class RuntimeLibrary
{
public:

	/// Create a queue
	static AQLQueue *CreateQueue(Component *component,
			unsigned int size,
			QueueType queue_type);

	/// Enqueue an AQL dispatch packet
	static void EnqueueDispatchPacket(AQLQueue *queue, AQLPacket *packet);

	/// Get the number of HSA Component
	static unsigned int getNumberOfHSAComponent();
};

} /* namespace HSA */

#endif 
