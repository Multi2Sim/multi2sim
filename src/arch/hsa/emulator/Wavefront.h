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

#ifndef ARCH_HSA_EMULATOR_WAVEFRONT_H
#define ARCH_HSA_EMULATOR_WAVEFRONT_H

#include "WorkItem.h"
#include "WorkGroup.h"


namespace HSA
{

class WorkGroup;
class WorkItem;

class Wavefront
{
	// The wavefront_id
	unsigned int wavefront_id;

	// The work group it belongs to
	WorkGroup *work_group;

	// List of work items
	// FIXME: vector
	std::list<std::unique_ptr<WorkItem>> work_items;

public:

	/// Constructor
	Wavefront(unsigned int wavefront_id, WorkGroup *work_group);

	/// Destructor
	~Wavefront();

	/// Execute instructions
	bool Execute();

	/// Activate all work items
	void ActivateAllWorkItems();

	/// Operator \c << invoking the function Dump on an output stream
	friend std::ostream &operator<<(std::ostream &os,
			const Wavefront &wavefront)
	{
		wavefront.Dump(os);
		return os;
	}

	/// Dump wavefront formation
	void Dump(std::ostream &os) const;

	/// Add work item into list
	void addWorkItem(std::unique_ptr<WorkItem> work_item);
};

}  // namespace HSA

#endif 

