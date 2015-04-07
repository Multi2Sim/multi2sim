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

#include "Wavefront.h"

namespace HSA
{

Wavefront::Wavefront(unsigned int wavefront_id, WorkGroup *work_group) :
		wavefront_id(wavefront_id),
		work_group(work_group)
{
}


Wavefront::~Wavefront()
{
}


bool Wavefront::Execute()
{
	bool on_going = false;
	for (auto it = work_items.begin(); it != work_items.end(); it++)
	{
		if ((*it)->Execute())
			on_going = true;
	}
	return on_going;
}


void Wavefront::ActivateAllWorkItems()
{
	for (auto it = work_items.begin(); it != work_items.end(); it++)
	{
		(*it)->setStatus(WorkItem::WorkItemStatusActive);
	}
}


void Wavefront::Dump(std::ostream &os = std::cout) const
{
	os << "    ***** Wavefront *****\n";
	for (auto it = work_items.begin(); it != work_items.end(); it++)
	{
		os << misc::fmt("      Work item (%d, %d, %d), \n",
				(*it)->getLocalIdX(),
				(*it)->getLocalIdY(),
				(*it)->getLocalIdZ());
	}
	os << "    ***** ********* *****\n";
}


void Wavefront::addWorkItem(std::unique_ptr<WorkItem> work_item)
{
	this->work_items.push_back(std::move(work_item));
}

}  // namespace HSA

