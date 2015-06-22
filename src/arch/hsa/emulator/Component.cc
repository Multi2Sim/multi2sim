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

#include "AQLQueue.h"
#include "Component.h"

namespace HSA
{

std::unique_ptr<Component> Component::getDefaultCPUComponent(unsigned long long handler)
{
	// The component object allocated here with be cast into a unique_ptr
	// by the function who call this function. Therefore, no need to delete
	// it in this function
	auto component = misc::new_unique<Component>(handler);

	// Set attributes of the virtual CPU device
	component->setName("Multi2Sim Virtual HSA CPU");
	component->setVendorName("Northeastern University");
	component->setDeviceType(HSA_DEVICE_TYPE_CPU);
	component->setWavesize(1);

	return component;
}


std::unique_ptr<Component> Component::getDefaultGPUComponent(unsigned long long handler)
{
	// The component object allocated here with be cast into a unique_ptr
	// by the function who call this function. Therefore, no need to delete
	// it in this function
	auto component = misc::new_unique<Component>(handler);

	// Set attributes of the virtual CPU device
	component->setName("Multi2Sim Virtual HSA GPU");
	component->setVendorName("Northeastern University");
	component->setDeviceType(HSA_DEVICE_TYPE_GPU);
	component->setWavesize(64);

	return component;
}


void Component::addQueue(std::unique_ptr<AQLQueue> queue)
{
	// TODO Generate better debug information
	Emulator::aql_debug << misc::fmt("Add a queue to component %lld\n", 
			agent_info.handler);
	queue->Associate(this);
	queues.emplace_back(std::move(queue));
}


bool Component::Execute()
{
	//std::cout << misc::fmt("Component %lld executing\n", this->getHandler());
	// 1. Check if the tasks is being processed. If true, process it.
	bool has_active_grid = false;
	auto it = grids.begin();
	while (it != grids.end())
	{
		if ((*it)->Execute())
		{
			has_active_grid = true;
			it++;
		}
		else
		{
			it = grids.erase(it);
		}
	}
	if (has_active_grid)
		return true;

	// 2. Otherwise, read from queue list and grab a task to start.
	for(auto it = queues.begin(); it != queues.end(); it++)
	{
		if(!(*it)->isEmpty())
		{
			AQLDispatchPacket *packet =
					(*it)->ReadPacket();
			LaunchGrid(packet);
			return true;
		}
	}

	// 3. If this component is not running and there is no pending task,
	// 	return false indicating this component is idle.
	return false;
}


void Component::LaunchGrid(AQLDispatchPacket *packet)
{
	// Dump debug information
	Emulator::aql_debug << "Packet dispatched: \n" << *packet;

	// Create grid
	auto grid = misc::new_unique<Grid>(this, packet);
	Emulator::aql_debug << "Grid formed and launched: \n";
	if (Emulator::aql_debug)
		grid->Dump(Emulator::aql_debug);

	// Insert grid into list
	this->grids.push_back(std::move(grid));
}


void Component::Dump(std::ostream &os = std::cout) const
{
	// Set device type
	std::string deviceType = "CPU";
	if (agent_info.device_type == HSA_DEVICE_TYPE_GPU)
		deviceType = "GPU";

	os << misc::fmt("\t***** %s device *****\n", deviceType.c_str());
	os << misc::fmt("\t Name: %s, \n", agent_info.name.c_str());
	os << misc::fmt("\t Vendor name: %s, \n", agent_info.vendor_name.c_str());
	os << misc::fmt("\t Wavesize: %d, \n", agent_info.wavesize);
	os << "\n";
}

}  // namespace HSA

