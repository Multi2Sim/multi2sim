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

#include <arch/southern-islands/disassembler/Instruction.h>
#include <memory/Memory.h>


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
	
	/// Image descriptor, as specified in Table 8.11 in SI documentation
	struct ImageDescriptor
	{
		unsigned long long base_addr : 40;   //    [39:0]
		unsigned int mid_lod         : 12;   //   [51:40]
		unsigned int data_fmt        : 6;    //   [57:52]
		unsigned int num_fmt         : 4;    //   [61:58]
		unsigned int                 : 2;    //   [63:62]
		unsigned int width           : 14;   //   [77:64]
		unsigned int height          : 14;   //   [91:78]
		unsigned int perf_mod        : 3;    //   [94:92]
		unsigned int interlaced      : 1;    //       95 
		unsigned int dst_sel_x       : 3;    //   [98:96]
		unsigned int dst_sel_y       : 3;    //  [101:99]
		unsigned int dst_sel_z       : 3;    // [104:102]
		unsigned int dst_sel_w       : 3;    // [107:105]
		unsigned int base_level      : 4;    // [111:108]
		unsigned int last_level      : 4;    // [115:112]
		unsigned int tiling_idx      : 5;    // [120:116]
		unsigned int pow2pad         : 1;    //      121 
		unsigned int                 : 2;    // [123:122]
		unsigned int type            : 4;    // [127:124]
		unsigned int depth           : 13;   // [140:128]
		unsigned int pitch           : 14;   // [154:141]
		unsigned int                 : 5;    // [159:155]
		unsigned int base_array      : 13;   // [172:160]
		unsigned int last_array      : 13;   // [185:173]
		unsigned int                 : 6;    // [191:186]
		unsigned int min_lod_warn    : 12;   // [203:192]
		unsigned long long           : 52;   // [255:204]
	}__attribute__((packed));

	/// Buffer descriptor, as specified in Table 8.5 in SI documentation
	struct BufferDescriptor
	{
		unsigned long long base_addr : 48;   //    [47:0]
		unsigned int stride          : 14;   //   [61:48]
		unsigned int cache_swizzle   : 1;    //       62 
		unsigned int swizzle_enable  : 1;    //       63 
		unsigned int num_records     : 32;   //   [95:64]
		unsigned int dst_sel_x       : 3;    //   [98:96]
		unsigned int dst_sel_y       : 3;    //  [101:99]
		unsigned int dst_sel_z       : 3;    // [104:102]
		unsigned int dst_sel_w       : 3;    // [107:105]
		unsigned int num_format      : 3;    // [110:108]
		unsigned int data_format     : 4;    // [114:111]
		unsigned int elem_size       : 2;    // [116:115]
		unsigned int index_stride    : 2;    // [118:117]
		unsigned int add_tid_enable  : 1;    //      119 
		unsigned int reserved        : 1;    //      120 
		unsigned int hash_enable     : 1;    //      121 
		unsigned int heap            : 1;    //      122 
		unsigned int unused          : 3;    // [125:123]
		unsigned int type            : 2;    // [127:126]
	}__attribute__((packed));


	// Sampler descriptor, as specified in Table 8.12 in SI documentation
	struct SamplerDescriptor
	{
		unsigned int clamp_x            : 3;    //     [2:0]
		unsigned int clamp_y            : 3;    //     [5:3]
		unsigned int clamp_z            : 3;    //     [8:6]
		unsigned int max_aniso_ratio    : 3;    //    [11:9]
		unsigned int depth_cmp_func     : 3;    //   [14:12]
		unsigned int force_unnorm       : 1;    //       15 
		unsigned int aniso_thresh       : 3;    //   [18:16]
		unsigned int mc_coord_trunc     : 1;    //       19 
		unsigned int force_degamma      : 1;    //       20 
		unsigned int aniso_bias         : 6;    //   [26:21]
		unsigned int trunc_coord        : 1;    //       27 
		unsigned int disable_cube_wrap  : 1;    //       28 
		unsigned int filter_mode        : 2;    //   [30:29] 
		unsigned int                    : 1;    //       31  
		unsigned int min_lod            : 12;   //   [43:32] 
		unsigned int max_lod            : 12;   //   [55:44] 
		unsigned int perf_mip           : 4;    //   [59:56] 
		unsigned int perf_z             : 4;    //   [63:60] 
		unsigned int lod_bias           : 14;   //   [77:64] 
		unsigned int lod_bias_sec       : 6;    //   [83:78] 
		unsigned int xy_mag_filter      : 2;    //   [85:84] 
		unsigned int xy_min_filter      : 2;    //   [87:86] 
		unsigned int z_filter           : 2;    //   [89:88] 
		unsigned int mip_filter         : 2;    //   [91:90] 
		unsigned int mip_point_preclamp : 1;    //       92  
		unsigned int disable_lsb_cell   : 1;    //       93  
		unsigned int                    : 2;    //   [95:94] 
		unsigned int border_color_ptr   : 12;   //  [107:96]
		unsigned int                    : 18;   // [125:108]
		unsigned int border_color_type  : 2;    // [127:126]
	}__attribute__((packed));

	// Memory pointer object, stored in 2 consecutive 32-bit SI registers
	struct MemoryPointer
	{
		unsigned long long addr : 48;
		unsigned int unused     : 16;
	}__attribute__((packed));


private:

	//global ID
	int id = 0;

	// wavefront ID
	int id_in_wavefront = 0;

	//local ID
	int id_in_work_group = 0;

	// global 3D IDs
	int id_3d[3];
	// local 3D IDs
	int id_in_work_group_3d[3];

	// Wavefront that it belongs to
	Wavefront *wavefront = nullptr;
	
	// work-group
	WorkGroup *work_group = nullptr;
	
	// NDRange
	NDRange *ndrange = nullptr;

	// Global memory
	mem::Memory *global_mem = nullptr;
	
	// Local memory
	mem::Memory *lds = nullptr;

	// Vector registers
	Instruction::Register vreg[256];

	// Emulation of ISA. This code expands to one function per ISA
	// instruction. For example: ISA_s_mov_b32_Impl(Instruction *inst)
#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size, _flags) \
	void ISA_##_name##_Impl(Instruction *instruction);
#include <arch/southern-islands/disassembler/Instruction.def>
#undef DEFINST

	// Instruction execution table 
	typedef void (WorkItem::*ISAInstFuncPtr)(Instruction *inst);
	ISAInstFuncPtr ISAInstFuncTable[Instruction::OpcodeCount + 1];

	// Error massage for unimplemented instructions
	static void ISAUnimplemented(Instruction *inst);

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
	///
	/// \param wavefront
	///	Wavefront that it belongs to
	///
	/// \param id
	///	Global 1D identifier of the work-item
	///
	WorkItem(Wavefront *wavefront, int id);




	//
	// Static fields
	//

	/// Maximum number of LDS accesses per instruction
	static const int MaxLdsAccessesPerInst = 2;





	/// Last global memory address
	unsigned global_memory_access_address = 0;

	/// Last global memory access size
	unsigned global_memory_access_size = 0;

	/// Last LDS accesses by last instruction
	int lds_access_count = 0;

	/// Information for each lds access
	MemoryAccess lds_access[MaxLdsAccessesPerInst];




	//
	// Getters
	//

	/// Get global id
	unsigned getId() const { return id; }

	/// Get id_3d
	unsigned getId3D(unsigned dim) const 
	{
		assert(dim >= 0 && dim <= 2);
		return id_3d[dim];
	}

	/// Return the wavefront that this work-items belongs to
	Wavefront *getWavefront() const { return wavefront; }

	/// Return the work-group that this work-item belongs to
	WorkGroup *getWorkGroup() const { return work_group; }

	/// Get the work-item identifier within the wavefront
	unsigned getIdInWavefront() const { return id_in_wavefront; }

	/// Get id
	unsigned getLocalId() const { return id_in_work_group; }

	/// Get id_in_work_group_3d
	///
	/// \param dim Local id dimention
	///
	unsigned getLocalId3D(unsigned dim) const 
	{ 
		assert(dim >= 0 && dim <= 2);
		return id_in_work_group_3d[dim]; 
	}
	
	/// Get pointer to global memory
	mem::Memory *getGlobalMemory() { return global_mem;}




	//
	// Setters
	//

	/// Set workitem 3D global identifier
	///
	/// \param dim Goblal dimention of identifier
	///
	/// \param id 3D Identifier
	///
	void setGlobalId3D(unsigned dim, unsigned id) 
	{
		assert(dim >= 0 && dim <= 2);
		id_3d[dim] = id;		
	}

	/// Set unique identifier of workitem
	///
	/// \param id Identifier of workitem 
	///
	void setGlobalId(unsigned id) { this->id = id; }

	/// Set workitem 3D local identifier
	///
	/// \param dim Local id dimention
	///
	/// \param id 3D Identifier
	///
	void setLocalId3D(unsigned dim, unsigned id) 
	{
		assert(dim >= 0 && dim <= 2);
		id_in_work_group_3d[dim] = id;		
	}

	/// Set identifier of workitem in workgroup scope
	///
	/// \param id Identifier of workitem 
	///
	void setLocalId(unsigned id) { id_in_work_group = id; }	

	/// Set identifier of a workitem, wavefront scope
	///
	/// \param id Identifier of workitem in a wavefront
	///
	void setIdInWavefront(unsigned id) { id_in_wavefront = id; }

	/// Set wavefront it belongs
	///
	/// \param wf Pointer of wavefront where it belongs
	///
	void setWavefront(Wavefront *wf) { wavefront = wf; }

	/// Set work-group it belongs
	///
	/// \param wg Pointer of work-group where it belongs
	///
	void setWorkGroup(WorkGroup *wg);

	/// Set global memory
	///
	/// \param memory Pointer to global memory
	///
	void setGlobalMemory(mem::Memory *memory) { global_mem = memory; }
	
	// FIXME - probably most functions below can be inline

	/// Execute an instruction
	void Execute(Instruction::Opcode opcode, Instruction *inst);

	/// Get value of a scalar register
	///
	/// \param sreg Scalar register identifier
	///
	unsigned ReadSReg(int sreg);

	/// Set value of a scalar register
	///
	/// \param sreg Scalar register identifier
	///
	/// \param value Value given as an \a unsigned typed value
	///
	void WriteSReg(int sreg, unsigned value);

	/// Get value of a vector register
	///
	/// \param vreg Vector register identifier
	///
	unsigned ReadVReg(int vreg);

	/// Set value of a vector register
	///
	/// \param vreg Vector register identifier
	///
	/// \param value Value given as an \a unsigned typed value
	///
	void WriteVReg(int vreg, unsigned value);

	/// Get value of a register, register type unspecified
	///
	/// \param reg Register identifier
	///
	unsigned ReadReg(int reg);

	/// Set bitmask of scalar registers
	///
	/// \param sreg Scalar register identifier
	///
	/// \param value Value given as an \a unsigned typed value
	///
	void WriteBitmaskSReg(int sreg, unsigned value);

	/// Get bitmask from scalar registers
	///
	/// \param sreg Scalar register identifier
	///
	int ReadBitmaskSReg(int sreg);

	/// Get buffer resource descriptor from 4 successive scalar registers
	///
	/// \param sreg Scalar register identifier of the 1st scalar registers
	///
	/// \param &buffer_desc Reference of a buffer resource descriptor
	///
	void ReadBufferResource(int sreg, BufferDescriptor &buffer_desc); 
	
	/// Get memory pointer from 2 successive scalar registers
	///
	/// \param sreg Scalar register identifier of the 1st scalar registers
	///
	/// \param mem_ptr Reference of a memory pointer descriptor
	///
	void ReadMemPtr(int sreg, MemoryPointer &memory_pointer);

};

}  // namespace SI

#endif
