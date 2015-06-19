/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef SRC_ARCH_SOUTHERN_ISLANDS_EMU_OBJECTPOOL_H                                          
#define SRC_ARCH_SOUTHERN_ISLANDS_EMU_OBJECTPOOL_H

#include <arch/southern-islands/emu/NDRange.h>
#include <arch/southern-islands/emu/Emu.h>
#include <arch/southern-islands/emu/Wavefront.h>
#include <arch/southern-islands/emu/WorkGroup.h>
#include <arch/southern-islands/emu/WorkItem.h>
#include <arch/southern-islands/asm/Asm.h>
#include <arch/southern-islands/asm/Inst.h>
#include <gtest/gtest.h>

namespace SI
{

// ObjectPool holds all the initialized objects required to create a 
// functioning Southern Islands environment  for testing
class ObjectPool 
{
	// Disassembler
	Asm *as = nullptr;

	// Emulator
	Emu *emu = nullptr;

	// NDRange
	NDRange ndrange;

	// WorkGroup
	WorkGroup work_group;

	// Wavefront
	Wavefront wavefront;

	// WorkItem
	WorkItem work_item;

	// Inst
	Inst inst;

	// Set the local and global size of the ndrange. The function returns
	// a pointer to the ndrange member so that the work_group member
	// constructor can be called in the initialization list. For testing
	// purposes, the global and local size is set to one.
	NDRange *SetNDRangeSize();

public:
	/// Constructor
	ObjectPool();




	//
	// Getters
	//

	/// Return a pointer to the work_item member
	WorkItem *getWorkItem() { return &work_item; }	
	
	/// Return a pointer to the work_item member
	Inst *getInst() { return &inst; }	
};


} // namespace SI

#endif
