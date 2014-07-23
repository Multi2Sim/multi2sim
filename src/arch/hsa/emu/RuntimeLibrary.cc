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

#include "RuntimeLibrary.h"

namespace HSA
{

AQLQueue *RuntimeLibrary::CreateQueue(Component *component,
		unsigned int size,
		QueueType type)
{
	AQLQueue *queue = new AQLQueue(size, type);
	component->addQueue(queue);

	return queue;
}

unsigned int RuntimeLibrary::getNumberOfHSAComponent()
{
	Emu *emu = Emu::getInstance();
	return emu->getNumberOfComponent();
}

void RuntimeLibrary::EnqueueDispatchPacket(AQLQueue *queue, AQLPacket *packet)
{
	queue->Enqueue(packet);
}

} /* namespace HSA */
