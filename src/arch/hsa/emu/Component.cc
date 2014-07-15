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

#include "Component.h"

namespace HSA
{

Component *Component::getDefaultCPUComponent()
{
	// The component object allocated here with be cast into a unique_ptr
	// by the function who call this function. Therefore, no need to delete
	// it in this function
	Component *component = new Component();

	// Set attributes of the virtual CPU device
	component->setName("Multi2Sim Virtual HSA CPU");
	component->setVendorName("Northeastern University");
	component->setIsGPU(false);
	component->setWavesize(1);

	return component;
}


Component *Component::getDefaultGPUComponent()
{
	// The component object allocated here with be cast into a unique_ptr
	// by the function who call this function. Therefore, no need to delete
	// it in this function
	Component *component = new Component();

	// Set attributes of the virtual CPU device
	component->setName("Multi2Sim Virtual HSA GPU");
	component->setVendorName("Northeastern University");
	component->setIsGPU(true);
	component->setWavesize(64);

	return component;
}


void Component::addQueue(AQLQueue *queue)
{
	queue->Associate(this);
	queues.emplace_back(queue);
}


bool Component::Execute()
{
	return false;
}


void Component::Dump(std::ostream &os = std::cout) const
{
	// Set device type
	std::string deviceType = "CPU";
	if (agent_info.isGPU)
		deviceType = "GPU";

	os << misc::fmt("\t***** %s device *****\n", deviceType.c_str());
	os << misc::fmt("\t Name: %s, \n", agent_info.name.c_str());
	os << misc::fmt("\t Vendor name: %s, \n", agent_info.vendor_name.c_str());
	os << misc::fmt("\t Wavesize: %d, \n", agent_info.wavesize);
	os << "\n";
}

}  // namespace HSA

