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
#include <memory/Memory.h>

#include "Emu.h"

namespace SI
{

/// Forward declarations
class Wavefront;
class WorkGroup;
class NDRange;

// typedef void (*WorkItemISAFuncPtr)(Inst *inst);

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
	mem::Memory *global_mem;
	mem::Memory *lds;

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
#include <arch/southern-islands/asm/Inst.def>
#undef DEFINST

	// Instruction execution table 
	typedef void (WorkItem::*ISAInstFuncPtr)(Inst *inst);
	ISAInstFuncPtr ISAInstFuncTable[InstOpcodeCount + 1];

	// Error massage for unimplemented instructions
	static void ISAUnimplemented(Inst *inst);

	// Get number of elements based on data format
	static int ISAGetNumElems(int data_format);

	// Get element size based on data format
	static int ISAGetElemSize(int data_format);

	// Float32 to Float16 conversion
	static uint16_t Float32to16(float value);

	// Float16 to Float32 conversion
	static float Float16to32(uint16_t value);

public:

	/// Constructor
	/// \param wavefront Wavefront that it belongs to
	/// \param id Global 1D identifier of the work-item
	WorkItem(Wavefront *wavefront, int id);

	/// Getters
	///
	/// Get wavefront it belongs to
	Wavefront *getWavefront() const { return wavefront; }

	/// Get global id
	unsigned getId() const { return id; }

	/// Get id_3d
	unsigned getId3D(unsigned dim) const {
		assert(dim >= 0 && dim <= 2);
		return id_3d[dim];
	}

	/// Get id_in_wavefront
	unsigned getIdInWavefront() const { return id_in_wavefront; }

	/// Get id
	unsigned getLocalId() const { return id_in_work_group; }

	/// Get id_in_work_group_3d
	/// \param dim Local id dimention
	unsigned getLocalId3D(unsigned dim) const { 
		assert(dim >= 0 && dim <= 2);
		return id_in_work_group_3d[dim]; 
	}

	/// Setters
	///
	/// Set workitem 3D global identifier
	/// \param dim Goblal dimention of identifier
	/// \param id 3D Identifier
	void setGlobalId3D(unsigned dim, unsigned id) {
		assert(dim >= 0 && dim <= 2);
		id_3d[dim] = id;		
	}

	/// Set unique identifier of workitem
	/// \param id Identifier of workitem 
	void setGlobalId(unsigned id) { this->id = id; }

	/// Set workitem 3D local identifier
	/// \param dim Local id dimention
	/// \param id 3D Identifier
	void setLocalId3D(unsigned dim, unsigned id) {
		assert(dim >= 0 && dim <= 2);
		id_in_work_group_3d[dim] = id;		
	}

	/// Set identifier of workitem in workgroup scope
	/// \param id Identifier of workitem 
	void setLocalId(unsigned id) { id_in_work_group = id; }	

	/// Set identifier of a workitem, wavefront scope
	/// \param id Identifier of workitem in a wavefront
	void setIdInWavefront(unsigned id) { id_in_wavefront = id; }

	/// Set wavefront it belongs
	/// \param wf Pointer of wavefront where it belongs
	void setWavefront(Wavefront *wf) { wavefront = wf; }

	/// Set work-group it belongs
	/// \param wg Pointer of work-group where it belongs
	void setWorkGroup(WorkGroup *wg) { work_group = wg; }

	// FIXME - probably most functions below can be inline

	/// Execute an instruction
	void Execute(InstOpcode opcode, Inst *inst);

	/// Get value of a scalar register
	/// \param sreg Scalar register identifier
	unsigned ReadSReg(int sreg);

	/// Set value of a scalar register
	/// \param sreg Scalar register identifier
	/// \param value Value given as an \a unsigned typed value
	void WriteSReg(int sreg, unsigned value);

	/// Get value of a vector register
	/// \param vreg Vector register identifier
	unsigned ReadVReg(int vreg);

	/// Set value of a vector register
	/// \param vreg Vector register idendifier
	/// \param value Value given as an \a unsigned typed value
	void WriteVReg(int vreg, unsigned value);

	/// Get value of a register, register type unspecified
	/// \param reg Register identifier
	unsigned ReadReg(int reg);

	/// Set bitmask of scalar registers
	/// \param sreg Scalar register identifier
	/// \param value Value given as an \a unsigned typed value
	void WriteBitmaskSReg(int sreg, unsigned value);

	/// Get bitmask from scalar registers
	/// \param sreg Scalar register identifier
	int ReadBitmaskSReg(int sreg);

	/// Get buffer resource descriptor from 4 successive scalar registers
	/// \param sreg Scalar register identifier of the 1st scalar registers
	/// \param &buffer_desc Reference of a buffer resource descriptor
	void ReadBufferResource(int sreg, EmuBufferDesc &buffer_desc); 
	
	/// Get memory pointer from 2 succesive scalar registers
	/// \param sreg Sclalar register identifier of the 1st scalar registers
	/// \param mem_ptr Reference of a memory pointer descripter
	void ReadMemPtr(int sreg, EmuMemPtr &mem_ptr);

};

}  /* namespace SI */

#endif
