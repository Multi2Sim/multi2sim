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

#ifndef ARCH_HSA_EMU_WORKGROUP_H
#define ARCH_HSA_EMU_WORKGROUP_H

#include "Component.h"
#include "Wavefront.h"


namespace HSA
{

class Component;
class Wavefront;

/// Work group of an HSA component
class WorkGroup
{
	// Pointer to the HSA component it belongs to
	Component *component;

	// List of wavefronts
	std::list<std::unique_ptr<Wavefront>> wavefronts;

	// Determines if the work item is active
	bool is_active = false;

public:

	/// Constructor
	///
	/// \param component
	///	The HSA component this work group belongs to
	WorkGroup(Component *component);

	// Set is_active
	void setActive(bool is_active) {this->is_active = is_active;}

	// Get is_active field
	bool isActive() const {return is_active;}

	// Execute each wavefront of the work group
	//
	// \return
	// 	True, if the execution have not finished
	//	False, if the execution finished
	bool Execute();
};

}

#endif
