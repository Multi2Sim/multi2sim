/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_WORK_ITEM_H
#define ARCH_SOUTHERN_ISLANDS_EMU_WORK_ITEM_H


namespace SI
{


/// Abstract polymorphic class used to attach additional information to the
/// work-item. The timing simulator can created objects derived from this class
/// and link them with the work-item.
class WorkItemData
{
public:
	virtual ~WorkItemData();
};


class WorkItem
{
public:

	/// Memory accesses types
	enum MemoryAccessType
	{
		MemoryAccessInvalid = 0,
		MemoryAccessRead,
		MemoryAccessWrite
	};

	/// Used for local memory accesses
	struct MemoryAccess
	{
		MemoryAccessType type;
		unsigned addr;
		unsigned size;
	};

private:
	static int MaxLDSAccessesPerInst = 2;
	
	// IDs
	int id;  // global ID
	int id_in_wavefront;
	int id_in_work_group;  // local ID

	// 3-dimensional IDs
	int id_3d[3];  // global 3D IDs
	int id_in_work_group_3d[3];  // local 3D IDs

	// Wavefront, work-group, and NDRange where it belongs
	Wavefront *wavefront;
	WorkGroup *work_group;
	NDRange *ndrange;

	// Vector registers
	InstReg vreg[256];

	// Last global memory access
	unsigned global_mem_access_addr;
	unsigned global_mem_access_size;

	// Last LDS accesses by last instruction
	int lds_access_count;  // Number of LDS access by last instruction
	MemoryAccess lds_access[MaxLDSAccessesPerInst];

public:

	/// Constructor
	/// \param Wavefront Wavefront that it belongs to
	/// \id Global 1D identifier of the work-item
	WorkItem(Wavefront *wavefront, int id);

	/// 

unsigned SIWorkItemReadSReg(SIWorkItem *self, int sreg);
void SIWorkItemWriteSReg(SIWorkItem *self, int sreg, 
	unsigned value);
unsigned SIWorkItemReadVReg(SIWorkItem *self, int vreg);
void SIWorkItemWriteVReg(SIWorkItem *self, int vreg, 
	unsigned value);
unsigned SIWorkItemReadReg(SIWorkItem *self, int reg);
void SIWorkItemWriteBitmaskSReg(SIWorkItem *self, int sreg, 
	unsigned value);
int SIWorkItemReadBitmaskSReg(SIWorkItem *self, int sreg);

struct si_buffer_desc_t;
struct si_mem_ptr_t;
void SIWorkItemReadBufferResource(SIWorkItem *self, 
	struct si_buffer_desc_t *buf_desc, int sreg);
void SIWorkItemReadMemPtr(SIWorkItem *self, 
	struct si_mem_ptr_t *mem_ptr, int sreg);

#endif
