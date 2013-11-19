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

#include <arch/southern-islands/asm/Inst.h>
#include <mem-system/Memory.h>

#include "Emu.h"


namespace SI
{

/// Forward declarations
class Wavefront;
class WorkGroup;
class NDRange;

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
	static const int MaxLDSAccessesPerInst = 2;
	
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

	// Global and local memory
	Memory::Memory *global_mem;
	Memory::Memory *lds;

	// Vector registers
	InstReg vreg[256];

	// Last global memory access
	unsigned global_mem_access_addr;
	unsigned global_mem_access_size;

	// Last LDS accesses by last instruction
	int lds_access_count;  // Number of LDS access by last instruction
	MemoryAccess lds_access[MaxLDSAccessesPerInst];

	// Emulation of ISA. This code expands to one function per ISA
	// instruction. For example: ISA_s_mov_b32_Impl(Inst *inst)
#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size, _flags) \
	void ISA_##_name##_Impl(Inst *inst);
#include <arch/southern-islands/asm/asm.dat>
#undef DEFINST

public:

	/// Constructor
	/// \param Wavefront Wavefront that it belongs to
	/// \id Global 1D identifier of the work-item
	WorkItem(Wavefront *wavefront, int id);

	// FIXME - probably most functions below can be inline

	/// FIXME ???
	unsigned ReadSReg(int sreg);

	/// FIXME ???
	void WriteSReg(int sreg, unsigned value);

	/// FIXME ???
	unsigned ReadVReg(int vreg);

	/// FIXME ???
	void WriteVReg(int vreg, unsigned value);

	/// FIXME ???
	unsigned ReadReg(int reg);

	/// FIXME ???
	void WriteBitmaskSReg(int sreg, unsigned value);

	/// FIXME ???
	int ReadBitmaskSReg(int sreg);

	/// FIXME ???
	void ReadBufferResource(int sreg, EmuBufferDesc &buffer_desc); 
	
	/// FIXME ???
	void ReadMemPtr(int sreg, EmuMemPtr &mem_ptr);
};

}  /* namespace SI */

#endif
