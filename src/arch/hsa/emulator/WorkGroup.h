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

#ifndef ARCH_HSA_EMULATOR_WORKGROUP_H
#define ARCH_HSA_EMULATOR_WORKGROUP_H

#include <set>

#include "Wavefront.h"
#include "WorkItem.h"
#include "Grid.h"


namespace HSA
{

class Wavefront;
class Grid;
class SegmentManager;

/// Work group of an HSA component
class WorkGroup
{
	// The grid object that this work group works in
	Grid *grid;

	// Work group id, unique in the grid.
	unsigned int group_id_x;
	unsigned int group_id_y;
	unsigned int group_id_z;

	// Current work group size
	unsigned int current_group_size_x = 0;
	unsigned int current_group_size_y = 0;
	unsigned int current_group_size_z = 0;

	// Number of workitems in the workgroup
	unsigned int num_work_items;

	// The segment memory manager
	std::unique_ptr<SegmentManager> group_segment;

	// List of wavefronts, map wave front id to wave fronts
	// Wave front id is not specified in the standard, but can be
	// calculated with work-item flattened id / wavefront size
	std::map<unsigned int, std::unique_ptr<Wavefront>> wavefronts;

	// A set of work item absolute flattened id. Those work items are
	// suspended due to a barrier
	std::set<unsigned int> work_items_on_hold;

public:

	/// Constructor
	///
	/// \param group_id
	/// 	Work group id, assigned by the grid
	WorkGroup(Grid *grid,
			unsigned group_segment_size,
			unsigned int group_id_x,
			unsigned int group_id_y,
			unsigned int group_id_z);

	/// Destructor
	~WorkGroup();

	/// Execute each wavefront of the work group
	///
	/// \return
	/// 	True, if the execution have not finished
	///	False, if the execution finished
	bool Execute();

	/// Notify the workgroup that one of the work item hits a barrier
	void HitBarrier(unsigned int abs_flat_id);

	/// Activate all work items
	void ActivateAllWorkItems();

	/// Dump the work group formation information for debug purpose
	void Dump(std::ostream &os) const;

	/// Operator \c << invoking the function Dump on an output stream
	friend std::ostream &operator<<(std::ostream &os,
			const WorkGroup &work_group)
	{
		work_group.Dump(os);
		return os;
	}

	// Add work item into current work group
	void addWorkItem(std::unique_ptr<WorkItem> work_item);

	/// Return work group flattened id
	unsigned int getGroupFlattenedId();

	/// Return the pointer the grid
	Grid *getGrid() const { return grid; };

	/// Return the work group id x
	unsigned int getGroupIdX() const { return group_id_x; }

	/// Return the work group id y
	unsigned int getGroupIdY() const { return group_id_y; }

	/// Return the work group id z
	unsigned int getGroupIdZ() const { return group_id_z; }

	/// Return the group segment memory manager
	SegmentManager *getGroupSegment() const { return group_segment.get(); }

	/// Get current workgroup size along a certain dimension
	/// Due to the fact that HSA allows partial work group, this function 
	/// is different from getWorkGroupSize();
	unsigned int getCurrentWorkGroupSize(unsigned int dim);

};

}

#endif
