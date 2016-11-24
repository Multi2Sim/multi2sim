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

#ifndef ARCH_HSA_EMULATOR_SEGMENTMANAGER_H
#define ARCH_HSA_EMULATOR_SEGMENTMANAGER_H

#include <memory/Manager.h>

namespace HSA
{

/// A segment manager is a tool that manages the addresses in a memory segment.
class SegmentManager : public mem::Manager
{
protected:

	// The flat address that is mapped to the beginning of the segment
	// Set to 0, so that all the segment address is actually flat address
	unsigned base_address = 0;

	// The size of the segment requires
	unsigned size;

public:

	/// Constructor
	SegmentManager(mem::Memory *memory, unsigned size);

	/// Destructor
	virtual ~SegmentManager();

	/// Allocate memory from the segment, returns the in-segment address
	unsigned Allocate(unsigned size, unsigned alignment = 1);

	/// Convert inner-segment address to flat address
	unsigned getFlatAddress(unsigned address);

};

}  // namespace HSA

#endif 
