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

#ifndef ARCH_HSA_DRIVER_DRIVERCALLBACKINFO_H
#define ARCH_HSA_DRIVER_DRIVERCALLBACKINFO_H

namespace HSA
{
class WorkItem;

/// DriverCallbackInfo holds information that need to be passed to the 
/// callback function.
class DriverCallbackInfo
{
protected:
	
	// The workitem that the callback is working on 
	WorkItem *work_item;

	// The guest memory 
	mem::Memory *memory;

	// The address of the argument that have been passed to the driver
	// call
	unsigned args_ptr;

public:

	/// Constructor
	DriverCallbackInfo(WorkItem *work_item, mem::Memory *memory,
			unsigned args_ptr):
			work_item(work_item),
			memory(memory),
			args_ptr(args_ptr)
	{
	}

	/// Virtual distructor
	virtual ~DriverCallbackInfo(){}

	/// Get argument pointer
	unsigned getArgsPtr() const
	{
		return args_ptr;
	}

	/// Get guest memory
	mem::Memory* getMemory() const
	{
		return memory;
	}

	/// Get workitem
	WorkItem* getWorkItem() const
	{
		return work_item;
	}

};


/// Information required by iterate next function
class AgentIterateNextInfo : public DriverCallbackInfo
{
protected:

	unsigned long long last_component_handler;

public:

	/// Constructor
	AgentIterateNextInfo(WorkItem *work_item, mem::Memory *memory,
			unsigned args_ptr,
			unsigned long long last_component_handler):
			DriverCallbackInfo(work_item, memory, args_ptr),
			last_component_handler(last_component_handler)
	{}

	/// Get last component handler
	unsigned long long getLastComponentHandler() const
	{
		return last_component_handler;
	}

};

}

#endif

